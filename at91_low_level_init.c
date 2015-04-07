/**************************************************
 *
 * This module contains the function `__low_level_init', a function
 * that is called before the `main' function of the program.  Normally
 * low-level initializations - such as setting the prefered interrupt
 * level or setting the watchdog - can be performed here.
 *
 * Note that this function is called before the data segments are
 * initialized, this means that this function cannot rely on the
 * values of global or static variables.
 *
 * When this function returns zero, the startup code will inhibit the
 * initialization of the data segments. The result is faster startup,
 * the drawback is that neither global nor static data will be
 * initialized.
 *
 * Copyright 1999-2004 IAR Systems. All rights reserved.
 *
 * $Revision: 1.4 $
 *
 **************************************************/

#include "config.h"

/* ICODE is the same segment as cstartup. By placing __low_level_init
 * in the same segment, we make sure it can be reached with BL. */

#pragma language=extended


#pragma location="ICODE"
__interwork int __blow_level_init(void)
{
  /*==================================*/
  /*  Initialize hardware.            */
  /*==================================*/

  // Enable peripheral clocks.
  AT91EnablePeripheralClocks();

  // The PIO is enabled early to make the LED's available for tracing.
  AT91InitPIO();


  /*==================================*/
  /* Choose if segment initialization */
  /* should be done or not.           */
  /* Return: 0 to omit seg_init       */
  /*         1 to run seg_init        */
  /*==================================*/
  return (1);
}

#pragma language=default

