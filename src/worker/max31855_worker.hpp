
#include <chrono>
#include <csignal>
#include <exception>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <set>
#include <vector>
#include <time.h>

#include <ctime>
#include <iostream>
#include <locale>

#include <wiringPi.h>

#ifndef _max31855_worker_hpp
#define _max31855_worker_hpp

#define PIN_CLK 14
#define PIN_DO 12
#define PIN_CS1 10
#define PIN_CS2 11

#define HIST_SIZE 60

class max31855_error : public std::runtime_error{
public:
  max31855_error(const char *_message)
      : runtime_error(_message) {}
};

enum max31855_status {
  SUCCESS,
  FAIL
}; 

class max31855_event {
public:
  max31855_event(uint16_t id, float v)
      : id(id), temperature(v),
        status(max31855_status::SUCCESS)
  {
    int ret = clock_gettime(CLOCK_REALTIME, &ts); // time is stored in the ts.
    temperature = v;
  }

  max31855_event(uint16_t id, float v, max31855_status s) : id(id), status(s) {
    int ret = clock_gettime(CLOCK_REALTIME, &ts);
    temperature = v;
  }

  uint16_t get_id(){return id;}
  float get_temperature(){return temperature;}
  struct timespec get_time(){return ts;}
  struct tm* get_gmtime(){return gmtime(&ts.tv_sec);}
  int get_strtime(char* b){
    //strftime(b, sizeof(b), "%D %T", gmtime(&ts.tv_sec));
    return strftime(b, sizeof(b), "%D %T", gmtime(&ts.tv_sec));
  }
  max31855_status get_status(){return status;};

private:
  uint16_t id;
  float temperature;
  max31855_status status;
  struct timespec ts;
};

class max31855_observer {
public:
  virtual ~max31855_observer() = default;
  virtual void update(max31855_event&) = 0;
};

class max31855_worker {
public:
  max31855_worker(): sclk(PIN_CLK), mosi(PIN_DO) {
    add_slave(PIN_CS1);
  };
  max31855_worker(unsigned int sclk, unsigned int mosi): sclk(sclk), mosi(mosi) {
    add_slave(PIN_CS1);
  };
  max31855_worker(unsigned int sclk, unsigned int mosi, unsigned int cs): sclk(sclk), mosi(mosi) {
    add_slave(cs);
  };
  ~max31855_worker();
  void init();
  void run();
  void start();
  static void* executeLauncher(void* args);
  void add(max31855_observer& o);
  void remove(max31855_observer& o);

  void add_slave(unsigned int s);

  const static uint64_t TEMPERATURE_ERROR = 8096;

private:
  volatile bool stop = false;
  std::vector<max31855_observer*> observers;
  void update(max31855_event& event);

  unsigned int sclk;
  unsigned int mosi;
  std::vector<unsigned int> slaves;

  // Clock is normally high in mode 2 and 3.
  // Clock is normally low in mode 0 and 1.
  unsigned int clock_base = LOW;

  // Read on trailing edge in mode 1 and 3.
  // Read on leading edge in mode 0 and 2.
  bool read_leading = true;

  // Bit order is MSBFIRST
  unsigned char mask = 0x80;
  // write, lshift
  // read, rshift

  pthread_t thread_handler;
  pthread_mutex_t mutex;
};

#endif
