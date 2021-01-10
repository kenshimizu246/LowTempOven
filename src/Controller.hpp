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

#ifndef _Controller_hpp
#define _Controller_hpp

/*
  19.2MHz / (pwm range * pwm clock divider)

  ex:
  19,200,000 / (2,000 * 192) = 50 (Hz)
*/

/***
 * Functionality
 * - Set PWM Value to control power.
 * - Set Temparature to control power.
 * - Set Menu to control power.
 */
enum control_mode {
  POWER,
  TEMPERATURE,
  MENU
};

class controller {
public:
  controller(unsigned int id, unsigned int range, unsigned int clock): id(id), range(range), clock(clock) {}
  void init();
  void set_mode(control_mode m);
  void set_power(unsigned int v);

private:
  unsigned int id;
  unsigned int range;
  unsigned int clock;
  unsigned int power;

  control_mode mode = POWER;
};

#endif /* _Controller_hpp */
