
/*
 * $Revision: DK Oct 2004 $
 */

// To build with memory controller remap code.
#ifdef FLASHCODE // Defined in AARM/ICCARM options tab (#define/preprocessor)
#define AT91_REMAP 1
#endif // FLASHCODE

// Serial port speed is 38400 baud.
//#define BAUD_RATE 38400
#define BAUD_RATE 9600

// Serial port receive buffer size.
#define RXBUF_SIZE 4096

// cstartup build flags
//#define __THUMB_LIBRARY__ 1
#define __ARM_LIBRARY__ 1


#define AT91_EBXX  1
#define AT91_MCK 66000000

#include <ioat91x40.h>

#if __IAR_SYSTEMS_ICC__
#include <inarm.h>
#endif

#if AT91_EBXX && __IAR_SYSTEMS_ICC__
#include "at91.h"
#endif
