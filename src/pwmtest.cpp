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

    /*
     +----------+---------------------+----------------------------------------------------------------
     | BIT      | NAME                | DESCRIPTION
     +----------+---------------------+----------------------------------------------------------------
     | D[31:18] | 14-Bit Thermocouple | Temperature Data These bits contain 
     |          |                     | the signed 14-bit thermocouple temperature
     |          |                     | value. See Table 4.|
     +----------+---------------------+----------------------------------------------------------------
     | D17      | Reserved            | This bit always reads 0.
     +----------+---------------------+----------------------------------------------------------------
     | D16      | Fault               | This bit reads at 1 when any of the SCV, SCG, or OC 
     |          |                     | faults are active. Default value is 0.
     +----------+---------------------+----------------------------------------------------------------
     | D[15:4]  | 12-Bit Internal     | These bits contain the signed 12-bit value of
     |          | Temperature Data    | the reference junction temperature. See Table 5.
     +----------+---------------------+----------------------------------------------------------------
     | D3       | Reserved            | This bit always reads 0.
     +----------+---------------------+----------------------------------------------------------------
     | D2       | SCV Fault           | This bit is a 1 when the thermocouple is short-circuited
     |          |                     | to VCC. Default value is 0.
     +----------+---------------------+----------------------------------------------------------------
     | D1       | SCG Fault           | This bit is a 1 when the thermocouple is short-circuited
     |          |                     | to GND. Default value is 0.
     +----------+---------------------+----------------------------------------------------------------
     | D0       | OC Fault            | This bit is a 1 when the thermocouple is open (no connections).
     |          |                     | Default value is 0.
     +----------+---------------------+----------------------------------------------------------------
    */

#define PIN_CLK 14
#define PIN_DO 12
#define PIN_CS1 10
#define PIN_CS2 11

#define HIST_SIZE 60

#define PIN_SWITCH 25
#define PITCH_MIN 1000000



int main(int argc, char *argv[])
{
  int i = 0;
  int v = 1;

  printf("start!\n");

  wiringPiSetup();

  pinMode(1, PWM_OUTPUT);

  pwmSetMode(PWM_MODE_MS);

  pwmSetRange(200);

  pwmSetClock(100);

  i = 0;
  while(i <= 20){
    v = i * 10;
    printf("v:%d\n", v);
    pwmWrite(1, v);

    delay(1000);
    i++;
  }
  pwmWrite(1, 0);

  printf("done!\n");
}
