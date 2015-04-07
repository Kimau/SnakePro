/*
 * UART Interface
 *
 * Supports serial IO using AT91 UART
 *    Character reception is interrupt supported:
 *       ReceiveLine(char* line, int timeout)
 *       RecvData( char* pData, int Size )
 *
 *    Character transmission via:
 *       SendLine(char* line)
 *
 * Call the following to initialise the UART
 *    UartInit(AT91UartGetchar, AT91UartPutchar);
 *    AT91InitInterrupt(TimerBeat, UartRxrdy);
 *    AT91UartInit();
 */

#include <string.h>
#include "config.h"
#include "uart.h"
#include "timer.h"
#include <intrinsic.h>


/* Timeout for reading new data - ms */
#define RD_TIMEOUT 10

/* The receive buffer */
static char rbuf[RXBUF_SIZE];

/* Pointer to next available vacant byte in the buffer */
static volatile int rptr = 0;

/* The serviced routines for reception and transmission via the UART */
static int(*getchar_function)();
static void(*putchar_function)(int);



/*
 * Connect UART receive and transmit functions
 */
void UartInit(int(*getchar_func)(), void(*putchar_func)(int)) {
  getchar_function = getchar_func;
  putchar_function = putchar_func;
}




/*
 * UartRxrdy()
 *
 * Interrupt service for UART Rx
 * Character has been received - read and store in buffer
 * The buffer index, rptr, points at the next available vacant
 * space in the buffer, rbuff[]
 *
 */
void UartRxrdy() {
  unsigned char value;

  value = (*getchar_function)();      /* Read character from UART */

  if (rptr >= RXBUF_SIZE)             /* Is there enough space? */
    return;
    
  rbuf[rptr++] = value;               /* Store character and update pointer */
}



/*
 * ReceiveLine()
 *
 * API function to extract all available characters from the serial receive
 * buffer within a timebound.
 * Returns zero if no characters are available within the timeound.
 * If at least one character is available within the timebound it returns
 * with the number of characters read and a pointer to the string.
 * If at least one character is available, any new characters must be
 * available within RD_TIMEOUT ms or they will be ignored.
 *
 * Reads characters from the serial receive buffer, rbuf[].  The index, rptr,
 * points just beyond the last character available.  So, the buffer contains
 * rbuf[0]..rbuf[rptr-1] characters.
 *
 * Parameters:
 *    line: pointer to a string
 *    timeout: time limit in ms
 * Return value: number of characters read
 *
 * Note - this function is insecure since interrupts are not disabled
 * while the buffer is flushed.
 *
 * WDHenderson, September 2008
 *
 */
int ReceiveLine(char* line, int timeout) {
  int n;
  int elapsed = 0;
  
  while (rptr == 0)
  {
    Sleep(RD_TIMEOUT);
    elapsed += RD_TIMEOUT;
    if (timeout && elapsed > timeout)
      return 0;
  }
  
  for (;;)
  {
    n = rptr;
    Sleep(RD_TIMEOUT);
    elapsed += RD_TIMEOUT;
    if (n == rptr)
      break;
    if (timeout && elapsed > timeout)
      return 0;
  }
  
  rptr = 0;
  memcpy(line, rbuf, n);
  rbuf[n] = 0;

  return n;
}




/*
 * RecvData()
 *
 * Reads precisely size bytes from the serial buffer
 * Returns zero of fewer than size bytes are available.  Otherwise
 * returns the number of bytes read and a pointer to the string
 *
 * Reads characters from the serial receive buffer, rbuf[].  The index, rptr,
 * points just beyond the last character available.  So, the buffer contains
 * rbuf[0]..rbuf[rptr-1] characters.
 *
 * Parameters:
 *    size: number of bytes to be read
 *    pData: pointer to the start of the character buffer
 *
 * Documentation and clarification - WDH, September 2008
 *
 */
int RecvData( char* pData, int Size ) {

/* Terminate if no data. */

   if ( rptr < Size ) {               /* Test number of chars available */
      return 0;                       /* Too few chars available */
   }
	
/* NOTE: Disable interrupts here to protect the buffer */
   __disable_interrupt();
        
   rptr = rptr - Size;                /* Adjust buffer index to remove size chars */
   memcpy( pData, &rbuf[0], Size );   /* Copy receive buffer to parameter buffer */
   memcpy( rbuf, &rbuf[Size], rptr ); /* Shift remaining chars to front of buffer */
		
/* NOTE: Reenable interrupts here */
   __enable_interrupt();

   return Size;
}




/*
 * SendLine()
 *
 * API function to transmit a, null terminated, string
 *
 * Parameter:
 *    line: pointer to, null terminated, string
 *
 */
void SendLine(char* line) {
  for ( ; *line; line++)
  {
    (*putchar_function)(*line);
  }
}




/*
 * SendData()
 *
 * API function to transmit a character string of specified size
 *
 * Parameters:
 *     pData: pointer to string
 *     Size: number of characters to transmit
 *
 */
void SendData( char* pData, int Size ) {
  int i;
  for (i = 0; i < Size; ++i)
  {
    (*putchar_function)(pData[i]);
  }
}
