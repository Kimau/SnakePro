/* Delay.c */

/*
 * Delay for us intervals - approximately
 * replace this with a clock delay at a later date ;-)
 */
void Delay_us( int period ) {
  int count;
  while ( period > 0 ) {
     for (count= 0; count < 10; count++);
     period--;
  }
}


/*
 * Delay for ms intervals
 *
 * Calls Delay_us for the requisite number of times to obtain
 * a delay in ms
 */
void Delay_ms( int period ) {
   while ( period != 0 ) {
      Delay_us(1000);
      period--;
   }
}
