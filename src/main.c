#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "functions.h"

volatile sig_atomic_t running = 1;

static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

int main()
{
    signal(SIGTSTP, signal_handler);
    signal(SIGINT, signal_handler);


    int file = setup();
    if(file < 0)    return EXIT_FAILURE;

    
    while(running)
    {
        int adc0 = read_potentiometer(0);
        int adc1 = read_potentiometer(1);
        int adc2 = read_potentiometer(2);
        int adc3 = read_potentiometer(3);

        servo_write(file, 0, adc0);
        servo_write(file, 1, adc1);
        servo_write(file, 2, adc2);
        servo_write(file, 3, adc3);

        usleep(20000);
    }

    close(file);
    puts("\nAplicacao encerrada.");
    return EXIT_SUCCESS;
}