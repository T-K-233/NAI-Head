/*
 * app.h
 *
 *  Created on: Nov 6, 2024
 *      Author: TK
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include <actuators.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "gc9a01a.h"
#include "actuators.h"

// http://elm-chan.org/junk/32bit/binclude.html
#define INCLUDE_FILE(section, filename, symbol) asm (\
    ".section "#section"\n"                   /* Change section */\
    ".balign 4\n"                             /* Word alignment */\
    ".global "#symbol"_start\n"               /* Export the object start address */\
    ".global "#symbol"_data\n"                /* Export the object address */\
    #symbol"_start:\n"                        /* Define the object start address label */\
    #symbol"_data:\n"                         /* Define the object label */\
    ".incbin \""filename"\"\n"                /* Import the file */\
    ".global "#symbol"_end\n"                 /* Export the object end address */\
    #symbol"_end:\n"                          /* Define the object end address label */\
    ".balign 4\n"                             /* Word alignment */\
    ".section \".text\"\n")                   /* Restore section */


#define N_STATES    9




#define EYE_MOVEMENT_X_SCALE      30.f
#define EYE_MOVEMENT_Y_SCALE      30.f



void APP_init();

void APP_main();


#endif /* INC_APP_H_ */
