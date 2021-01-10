#include <chrono>
#include <pthread.h>
#include <memory>
#include <vector>
#include <algorithm>

#include "max31855_worker.hpp"

void max31855_worker::init(){
  //wiringPiSetup();

  pinMode(sclk, OUTPUT);
  pinMode(mosi, INPUT);
  for(unsigned int ss : slaves){
    pinMode(ss,   OUTPUT);
  }
}

void max31855_worker::run() {
  while(true){
    for(unsigned int ss : slaves){
      digitalWrite(ss, LOW);
      unsigned char raw[4];
      for(unsigned int b = 0; b < 4; b++){
        for(unsigned int i = 0; i < 8; i++){
          digitalWrite(sclk, (clock_base == LOW ? HIGH : LOW));
          // read_leading
          int r;
          r = digitalRead(mosi);
          //printf("%d", r);
          digitalWrite(sclk, clock_base);
          raw[b] <<= 1;
          raw[b] = raw[b] | r;
        }
        //printf("|0x%02x, ", raw[b]);
      }
      //printf("\n");
      int value = raw[0] << 24 | raw[1] << 16 | raw[2] << 8 | raw[3];
      //printf("value:%x\n", value);

      // 0x7 -> b...111 D2 D1 D0
      float tempC = std::numeric_limits<float>::quiet_NaN();
      if(value & 0x7){
        //printf("float NaN\n");
      } else {
        // cut off D0 - D17
        value >>= 18;
        // Scale by 0.25 degrees C per bit and return value.
        tempC = value * 0.25;
      }
      //printf("xtempC:%f\n", tempC);

      max31855_event event{0, tempC};
      for(max31855_observer *l : observers){
        //l->update(ss, tempC);
        l->update(event);
      }
      digitalWrite(ss, HIGH);
    }
    usleep(1000000);
  }
}

void max31855_worker::start() {
  std::cout << "max31855_worker::start()..." << std::endl;
//  if((this->thread_handler) == NULL){
    std::cout << "max31855_worker::start()... thread init" << std::endl;
    pthread_mutex_init(&(this->mutex), NULL);
    pthread_create(
      &(this->thread_handler),
      NULL,
      max31855_worker::executeLauncher,
      this
    );
    std::cout << "max31855_worker::start()... thread created" << std::endl;
    pthread_detach(this->thread_handler);
    std::cout << "max31855_worker::start()... thread detached" << std::endl;
//  }else{
//    std::cout << "max31855_worker::start()... thread_handler is not null" << std::endl;
//  }
}

void* max31855_worker::executeLauncher(void* args){
  std::cout << "max31855_worker::executeLauncher()... start" << std::endl;
  reinterpret_cast<max31855_worker*>(args)->run();
  std::cout << "max31855_worker::executeLauncher()... end" << std::endl;
}

void max31855_worker::add(max31855_observer& o){
  observers.push_back(&o);
}

void max31855_worker::remove(max31855_observer& o){
  observers.erase(std::remove(observers.begin(), observers.end(), &o));
}

void max31855_worker::add_slave(unsigned int s){
  slaves.push_back(s);
}

max31855_worker::~max31855_worker(){

}


