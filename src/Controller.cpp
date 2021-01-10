#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <limits>

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cstdio>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include <wiringPi.h>

#include "Controller.hpp"


void controller::init(){
  //wiringPiSetup();

  pinMode(id, PWM_OUTPUT);

  pwmSetMode(PWM_MODE_MS);

  pwmSetRange(range);

  pwmSetClock(clock);
}

void controller::set_power(unsigned int v){
  pwmWrite(id, v);
}

