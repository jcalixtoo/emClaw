#ifndef DRIVERS_H
#define DRIVERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_BUS_PATH        "/dev/i2c-2"
#define PCA_SLAVE_ADRESS    0x40
#define ADC_PATH_FORMAT     "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw"
#define ADC_CHANNEL_COUNT   4
#define ADC_MAX_VALUE       4095

                

#endif




