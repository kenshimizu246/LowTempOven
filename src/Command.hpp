
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

#include "Controller.hpp"

#ifndef _command_hpp
#define _command_hpp


class command_event {
public:
  struct timespec get_time(){ return ts; }
  struct tm* get_gmtime(){ return gmtime(&ts.tv_sec); }
  std::string get_event_name(){ return event_name; }
  void set_event_name(std::string s){event_name = s; }
private:
  struct timespec ts;
  std::string event_name;
};

class command_observer {
public:
  virtual ~command_observer() = default;
  virtual void update(command_event&) = 0;
};

class command {
public:
  virtual void do_command() = 0;

  void add(command_observer& o){
    observers.push_back(&o);
  }
  void remove(command_observer& o){
    observers.erase(std::remove(observers.begin(), observers.end(), &o));
  }
  void update(command_event& e){
    for(auto* o : observers){
//      o->update(e);
    }
  }
private:
  std::vector<command_observer*> observers;
};

class set_power_event : public command_event {
public:
  set_power_event(unsigned int value): value(value) { }
  unsigned int get_power(){ return value; }

private:
  unsigned int value;
};

class set_power_command : public command {
public:
  set_power_command(controller& c, int value): ctrl(c), value(value) {}
  unsigned int get_value() {return value; }
  void do_command(){
    std::cout << "controller.set_power(" << value << ")" << std::endl;
    ctrl.set_power(value);

    set_power_event e(value);
    update(e);
  }

private:
  controller& ctrl;
  unsigned int value;
};

class command_factory {
public:
  command_factory(controller& c): ctrl(c){}
  void set(controller& c){
    ctrl = c;
  }
  std::shared_ptr<command> create_set_power_command(unsigned int v){
    return std::shared_ptr<command>(new set_power_command(ctrl, v));
  }
private:
  controller& ctrl;
};

#endif

