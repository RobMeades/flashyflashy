/* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <wiringPi.h>
#include <time.h>

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

// Things to help with parsing filenames.
#define DIR_SEPARATORS "\\/"
#define EXT_SEPARATOR "."

/* ----------------------------------------------------------------
 * STATIC VARIABLES
 * -------------------------------------------------------------- */

// The GPIO pin to toggle.
static int gGpio = -1;

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

// Print the usage text
static void printUsage(char * pExeName) {
    printf("\n%s: flashy flashy!  Usage:\n", pExeName);
    printf("    %s  pin duration\n", pExeName);
    printf("where:\n");
    printf("    pin is the number of the GPIO pin to flash (i.e. the x from GPIOx),\n");
    printf("    duration is the duration of the flash in milliseconds,\n");
    printf("For example:\n");
    printf("    %s 0 500\n", pExeName);
    printf("...will flash GPIO0 on for 500 ms and off for 500 ms.\n\n");
    printf("Press CTRL-C to exit.\n");
}

// Signal handler for CTRL-C
static void exitHandlerSignal(int signal)
{
    printf("\nNo more flashing.\n");
    digitalWrite(gGpio, LOW);
    exit(0); 
}

/* ----------------------------------------------------------------
 * MAIN
 * -------------------------------------------------------------- */

// Main.
int main(int argc, char *argv[])
{
    int x = 0;
    char *pExeName = NULL;
    char *pChar;
    struct sigaction sigIntHandler;
    long duration = -1;
    struct timespec flashTime;

    // Find the exe name in the first argument
    pChar = strtok(argv[x], DIR_SEPARATORS);
    while (pChar != NULL) {
        pExeName = pChar;
        pChar = strtok(NULL, DIR_SEPARATORS);
    }
    if (pExeName != NULL) {
        // Remove the extension
        pChar = strtok(pExeName, EXT_SEPARATOR);
        if (pChar != NULL) {
            pExeName = pChar;
        }
    }
    x++;

    // Look for all the command line parameters
    while(x < argc) {
        // Test for the GPIO pin number
        if (x == 1) {
            gGpio = atoi(argv[x]);
            // Test for the duration of the flash
        } else if (x == 2) {
            duration = atol(argv[x]);
        }
        x++;
    }

    // Must have the two mandatory command-line parameters
    if ((gGpio >= 0) && (duration > 0)) {
        printf("Flashing GPIO%d on and then off for %ld ms each.\n", gGpio, duration);

        // Set up the CTRL-C handler
        sigIntHandler.sa_handler = exitHandlerSignal;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);

        // Set up wiringPi and the LED pin
        wiringPiSetup();
        pinMode(gGpio, OUTPUT);

        flashTime.tv_nsec = (duration * 1000000) % 1000000000;
        flashTime.tv_sec = duration / 1000;
        // Toggle forever
        while (1) {
            x = 0;
            // Toggle the LED
            if (digitalRead(gGpio) == 0) {
                x = 1;
            }
            digitalWrite(gGpio, x);
            // Sleep
            nanosleep(&flashTime, NULL);
        }
    } else {
        printUsage(pExeName);
    }

    return -1;
}

// End of file
