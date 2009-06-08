/**
Serial communications module.
http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/

#include <string.h>
#include <stdio.h>
#include "hardware.h"
#include "serialComm.h"
#include "tasklist.h"


unsigned short serRxIndex;        //receive position
char serRxBuf[SERBUFSIZE];

/**
Handler for serial data (called in RX interrupt from UART).

return signals if the buffer contains a complete message.

serRxIndex is reset on idle line (after a timeout).
*/
unsigned short procchar(void) {
    char character = RXBUF0;
    P2OUT ^= BIT2;
    P1OUT = character;
    switch (character)
    {
        case '\r':
            //ignore CR
            break;
        case '\n':
            serRxBuf[serRxIndex] = '\0';        //null terminate string
            return TASK_serTask;                //start serial handler task on completed line
        default:
            if (serRxIndex < (SERBUFSIZE-1)) {  //buffer check, account null byte with -1
                serRxBuf[serRxIndex++] = character;
            } else {
                //ignore character
            }
    }
    return 0;
}

/**
Task which handles incomming packets from the serial port.
Its started from the serial receiver interrupt.
*/
void serTask(void) {
    printf("len: %d buf: \"%s\"\n", strlen(serRxBuf), serRxBuf);
    serRxIndex = 0;
}

