
/*
 * $Revision: 1.3 $
 */

#include "config.h"
#include "timer.h"

static volatile int ms_ctr = 0;
static volatile int tick = 0;

extern int test_number;

static void ProcessInput(void);


void TimerBeat(void)
{
  // Called at 1000 Hz rate.
  ms_ctr++; // Sleep counter.
  tick++; // Button debounce counter.
  ProcessInput(); // Check buttons.
//  SND_Callback(); // Sound callback for good use.
 
}


void Sleep(int milliseconds)
{
  ms_ctr = 0;
  while (ms_ctr < milliseconds) ;
}


static void ProcessInput(void)
{
  /* Scan buttons with 50 ms interval     */
  if (tick >= 50)       
  {
//     ButtonScan();
     tick = 0;
  }
}

