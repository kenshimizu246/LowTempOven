
#include <iostream>
#include <cctype>
#include <sstream>
#include <string>
#include <string>
#include <vector>
#include <memory>

#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filewritestream.h>

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>
#include <set>

/*#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>*/
#include <websocketpp/common/thread.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

#include "Config.hpp"
#include "Message.hpp"
#include "Command.hpp"
#include "worker/max31855_worker.hpp"

#include <wiringPi.h>

#define DAEMON_NAME "simpledaemon"

using namespace std;
using namespace rapidjson;

namespace tamageta {

enum action_type {
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE
};

struct action {
    action(action_type t, connection_hdl h) : type(t), hdl(h) {}
    action(action_type t, connection_hdl h, server::message_ptr m)
      : type(t), hdl(h), msg(m) {}

    action_type type;
    websocketpp::connection_hdl hdl;
    server::message_ptr msg;
};

class LowTempOven : public max31855_observer, public command_observer {
  public:
    LowTempOven();
    void init();
    void daemonize(string rundir, string pidfile);
    void run();
    void process_messages();
    void update(max31855_event& event);
    void update(command_event& event);

  private:
    static void signal_handler(int sig);
    static void sigIntHndlr(int sig);
    static void sigIntExHndlr(int sig);
    static void daemonShutdown();

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);

    int force_exitx;
    static int force_exit;
    static int pidFilehandle;

    typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;

    server m_server;
    con_list m_connections;
    std::queue<action> m_actions;

    mutex m_action_lock;
    mutex m_connection_lock;
    condition_variable m_action_cond;

    max31855_worker max31855;
    controller ctrl{1, 2000, 192};
    command_factory cmd_factory{ctrl};
};

int LowTempOven::force_exit = 0;
int LowTempOven::pidFilehandle;

LowTempOven::LowTempOven() {
  force_exitx = 0;

  wiringPiSetup(); // use wiringPi

  ctrl.init();
  max31855.init();

  // Initialize Asio Transport
  m_server.init_asio();

  // Register handler callbacks
  m_server.set_open_handler(bind(&LowTempOven::on_open,this,::_1));
  m_server.set_close_handler(bind(&LowTempOven::on_close,this,::_1));
  m_server.set_message_handler(bind(&LowTempOven::on_message,this,::_1,::_2));
}

void LowTempOven::daemonShutdown(){
//  (pidFilehandle);
}

void LowTempOven::update(max31855_event& event){
  websocketpp::lib::error_code ec;

  //std::cout << "received temperature: " << event.getTemperature() << std::endl;
  lock_guard<mutex> guard(m_connection_lock);

  std::string msg;
  message_handler::toJSON(event, msg);

  con_list::iterator it;
  for (it = m_connections.begin(); it != m_connections.end(); ++it) {
    m_server.send(*it,msg,websocketpp::frame::opcode::text, ec);
  }
}

void LowTempOven::update(command_event& event){
  websocketpp::lib::error_code ec;

  lock_guard<mutex> guard(m_connection_lock);

  std::string msg;
  if(typeid(event) == typeid(set_power_event&)){
    message_handler::toJSON(static_cast<set_power_event&>(event), msg);
  }

  con_list::iterator it;
  for (it = m_connections.begin(); it != m_connections.end(); ++it) {
    m_server.send(*it,msg,websocketpp::frame::opcode::text, ec);
  }
}

void LowTempOven::signal_handler(int sig){
  switch(sig){
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
      daemonShutdown();
      exit(EXIT_SUCCESS);
      break;
    default:
      break;
  }
}

void LowTempOven::daemonize(string rundir, string pidfile) {
  int pid, sid, i;
  char str[10];
  struct sigaction newSigAction;
  sigset_t newSigSet;

  if (getppid() == 1) {
    return;
  }

  sigemptyset(&newSigSet);
  sigaddset(&newSigSet, SIGCHLD);
  sigaddset(&newSigSet, SIGTSTP);
  sigaddset(&newSigSet, SIGTTOU);
  sigaddset(&newSigSet, SIGTTIN);
  sigprocmask(SIG_BLOCK, &newSigSet, NULL);

  newSigAction.sa_handler = signal_handler;
  sigemptyset(&newSigAction.sa_mask);
  newSigAction.sa_flags = 0;

  sigaction(SIGHUP, &newSigAction, NULL);
  sigaction(SIGTERM, &newSigAction, NULL);
  sigaction(SIGINT, &newSigAction, NULL);

  // fork returns child process id if process is parent or 0 if process is child process
  // this kills the parent process to be independent process and under init/kernel process
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(027);

  sid = setsid();

  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  for (i = getdtablesize(); i >= 0; --i) {
    close(i);
  }

  i = open("/dev/null", O_RDWR);

  dup(i);
  dup(i);

  chdir(rundir.c_str());

  pidFilehandle = open(pidfile.c_str(), O_RDWR|O_CREAT, 0600);

  if (pidFilehandle == -1 ) {
//    BOOST_LOG_TRIVIAL(info) << "daemonize.pidFilehandle" << pidfile;
    exit(EXIT_FAILURE);
  }

  if (lockf(pidFilehandle,F_TLOCK,0) == -1) {
    exit(EXIT_FAILURE);
  }

  sprintf(str,"%d\n",getpid());

  write(pidFilehandle, str, strlen(str));

  close(pidFilehandle);
}

void LowTempOven::sigIntHndlr(int sig)
{
  force_exit = 1;
}

void LowTempOven::sigIntExHndlr(int sig)
{
  force_exit = 1;
  exit(EXIT_SUCCESS);
}

void LowTempOven::init(){
  int size = 0;
  
//  wiringPiSetupGpio();

//  size = Config::getInstance().getI2CSize();
//  for(int i = 0; i < size; i++){
//    shared_ptr<I2CConf> p = Config::getInstance().getI2CConf(i);
//    shared_ptr<PCA9685> pca = shared_ptr<PCA9685>(new PCA9685());
//    pca->Setup(p->address, p->hertz);
//    pca->PWMReset();
//    tamageta::AppCtx::getInstance().add(pca);
//  }
//
//  size = Config::getInstance().getHcSr04ConfSize();
//  for(int i = 0; i < size; i++){
//    shared_ptr<HcSr04Conf> p = Config::getInstance().getHcSr04Conf(i);
//    shared_ptr<HcSr04> hcsr04 = shared_ptr<HcSr04>(new HcSr04(p->pinTrig, p->pinEcho));
//    tamageta::AppCtx::getInstance().add(hcsr04);
//  }
}

void LowTempOven::run(){
  int opts = 0;
  int n = 0;
  unsigned int ms, oldms = 0;   

  cout << "LowTempOven::run() ... start\n" << endl;

  /* Deamonize */
  if(Config::getInstance().isDaemon()){
    cout << "daemonize... " << Config::getInstance().getAppDir() << " : " << Config::getInstance().getPidFile() << endl;
    daemonize(Config::getInstance().getAppDir().c_str(), Config::getInstance().getPidFile().c_str());
    signal(SIGINT, sigIntHndlr);
    cout << "daemonized!" << endl;
  } else {
    signal(SIGINT, sigIntExHndlr);
    cout << "not daemonized!" << endl;
  }

//  init_log();

//  signal(SIGINT, sigIntHndlr);

  // listen on specified port
  m_server.listen(Config::getInstance().getPort());

  cout << "LowTempOven::run() ... before accept\n" << endl;
  // Start the server accept loop
  m_server.start_accept();

  cout << "LowTempOven::run() ... before start max31855 worker\n" << endl;
  max31855.add((*this));

  max31855.start();

  // Start the ASIO io_service run loop
  cout << "LowTempOven::run() ... Start the ASIO io_service run loop\n" << endl;
  try {
    m_server.run();
  } catch (const std::exception & e) {
    std::cout << e.what() << std::endl;
  }
 
  cout << "LowTempOven::run() ... end\n" << endl;
}

void LowTempOven::on_open(connection_hdl hdl) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(action(SUBSCRIBE,hdl));
  }
  m_action_cond.notify_one();
}

void LowTempOven::on_close(connection_hdl hdl) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(action(UNSUBSCRIBE,hdl));
  }
  m_action_cond.notify_one();
}

void LowTempOven::on_message(connection_hdl hdl, server::message_ptr msg) {
  // queue message up for sending by processing thread
  {
    lock_guard<mutex> guard(m_action_lock);
    std::cout << "on_message" << std::endl;
    m_actions.push(action(MESSAGE,hdl,msg));
  }
  m_action_cond.notify_one();
}

void LowTempOven::process_messages() {
  while(1) {
    unique_lock<mutex> lock(m_action_lock);

    while(m_actions.empty()) {
      m_action_cond.wait(lock);
    }

    action a = m_actions.front();
    m_actions.pop();

    lock.unlock();

    if (a.type == SUBSCRIBE) {
      lock_guard<mutex> guard(m_connection_lock);
      m_connections.insert(a.hdl);
    } else if (a.type == UNSUBSCRIBE) {
      lock_guard<mutex> guard(m_connection_lock);
      m_connections.erase(a.hdl);
    } else if (a.type == MESSAGE) {
      lock_guard<mutex> guard(m_connection_lock);

      std::string cc{a.msg->get_payload()};

      std::cout << "MESSAGE: " << cc << std::endl;

      std::shared_ptr<command> mp = message_handler::toCommand(cmd_factory, cc);
      mp->add((*this));
      mp->do_command();
    } else {
      // undefined.
    }
  }
}


} // end of namespace tamageta


int main(int argc, char **argv)
{
  cout << "Start main..." << endl;

  if ( argc != 2) {
    cerr << "argument count mismatch error." << endl;
    exit(EXIT_FAILURE);
  }
  tamageta::Config::getInstance().load(argv[1]);

  cout << "AppDir:" << tamageta::Config::getInstance().getAppDir() << endl;
  cout << "LogDir:" << tamageta::Config::getInstance().getLogDir() << endl;
  cout << "PidFile:" << tamageta::Config::getInstance().getPidFile() << endl;

  tamageta::LowTempOven server;

  server.init();

  cout << "after init..." << endl;

  thread t(bind(&tamageta::LowTempOven::process_messages,&server));

  server.run();

  t.join();
 
  cout << "after server.run()..." << endl;

  return 0;
} //main

