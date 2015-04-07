/*
 * XBee.c
 *
 * XBee interface
 *
 * A simple driver for the XBee OEM RF Module manufactured by MaxStream
 * Full device specification in M100232 (2006) @ wwww.maxstream
 *
 * This driver supports Transparent Operation only - the XBee module acts
 * as a serial line replacement.  The network will operate in NonBeacon mode
 * supporting peer-to-peer links and broadcast communications to all devices
 * in a PAN.
 *
 * The uart module (uart.c) provides a serial interface to the XBee; the
 * uart interface must be configured prior to calling XBeeInit using:
 *    UartInit(AT91UartGetchar, AT91UartPutchar),
 *    AT91InitInterrupt(TimerBeat, UartRxrdy);
 *    AT91UartInit();
 *
 * XBeeInit allows the XBee PAN, destination and sources addresses to be
 * assigned.  The device is left in Transparent Operation mode.  The uart
 * interface provides:
 *    ReceiveLine() to read any bytes received within a timeout and
 *    SendData() to transmit a string of bytes
 *
 * WDH, August, 2008
 *
 */

#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "pg12864.h"
#include "Delay.h"

/*
 * XBeeInit - Initialise XBee module
 *
 * Enter command mode - "+++"
 * Issue AT command prefix - "AT"
 * Assign PanID - "ID"
 * Assign MY address - "MY"
 * Assign Destination address - "DH", "DL"
 * Exit command mode - "CN"
 *
 * Parameters:
 *     PanID: PANID in {0x0000..0xFFFF}
 *     Source address: MY in {0x0000..0xFFFF}
 *     Desination address high: DH in {0x00000000..0xFFFFFFFF}
 *     Desination address low:  DL in {0x00000000..0xFFFFFFFF}
 *
 * Point-to-point (unicast) addressing:
 *     16-bit source address:
 *        Set MY < 0xFFFE
 *        Set DH= 0
 *        Set DL < 0xFFFE
 *        Destination address of Tx must match MY address of Rx
 *        Example:
 *                     Module 1         Module 2
 *            MY         0x01             0x02
 *            DH            0                0
 *            DL         0x02             0x01
 *     64-bit source address:
 *        SH and SL parames are concatenated to configure a 64-bit
 *        source address and MY is disabled.
 *        Set MY = 0xFFFF or 0xFFFE
 *        Source address is factory set in SH and SL
 *        To send a packet to a a specific module, the DH+DL must match
 *        the SH+SL of the other.
 *
 * Broadcast addressing:
 *     Regardless of use of 16- or 64-bit addressing, any module may accept
 *     a broadcast.
 *     Broadcast destination address to the PAN is DH+DL= 0x00000000 0000FFFF)
 *
 * Returns -1 - argument error
 *         -2 - command failed to get XBee response
 *          0 - success
 *
 * Requires UartInit(), SendData() and ReceiveLine() in uart.c and
 * AT91InitInterrupt() and AT91UartInit() in at91.c
 */
 int XBeeInit( unsigned int PANID,
               unsigned int MY,
               unsigned int DH,
               unsigned int DL ){

    char commandString[100];
    int n;
    
/* Check parameters */
   if ( PANID > 0xFFFF )
      return -1;
   if ( MY > 0xFFFF )
      return -1;

/* Flush Rx buffer */
   ReceiveLine(commandString, 2000);

/* Send command prefix */
   SendData( "+++", 3 );
   Delay_ms(2000);
   
/* Assemble command string */
   sprintf( commandString, "ATID%.4X,MY%.4X,DH%.8X,DL%.8X,CN\r",
            PANID, MY, DH, DL );

/* Send command string to XBee module */
   SendData( commandString, 41 );
//   Delay_ms(2000);

/* Wait for 6 "OK" response */
   n= ReceiveLine(commandString, 2000);
   commandString[n]= 0;

/* Check response from XBee module */
   if ( *strstr(commandString,"OKOKOKOKOKOK") == NULL )
      return -2;
   else
      return 0;

}
