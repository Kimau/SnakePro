/*
 * sound.c
 *
 * Implements a crude sound interface for EBA40A Atmel ARM7 board.
 *
 * Sends periodic AF signals via GPIO P6 to a simple piezoelectric
 * sounder.  The AT91R40008 processor does not possess a DAC so
 * the periodic signals are square waves with the usual "unmusical"
 * harmonics.
 *
 * Call soundInit() first followed by any sequence of playNote(int t, int n)
 * and playRest( int n ), this:
 *
 *    soundInit();
 *    playNote( g3, c);        play g3 crotchet
 *    playNote( a3, q);        play a3 quaver
 *    playRest( sb );          play semi-breve rest
 *
 * Note durations are drawn from {sb,m,c,q,sq,dsq}
 * Note frequencies are drawn from {c0,c0s,d0,d0s, ... ,c8s,d8,d8s};
 * c4 is "middle C" 
 *
 * Original WDH, June, 2006
 * Updated WDH, August, 2008
 *
 */

#include "AT91PIO.h"
#include "Sound.h"
#include "Delay.h"

/* Structure holding the halfduration and cycles values for a note */

struct note_record {
   int halfDuration; /* Micro seconds */
   int cycles[6];    /* Cycles for a semi-breve */
} noteTable[100];



/* An array of note frequency values - Hz */

float noteFrequency[]= {  16.35,  17.32,  18.35,  19.45,  20.6 ,  21.83,
                          23.12,  24.50,  25.96,  27.50,  29.14,  30.87,
                          32.7,   34.65,  36.71,  38.89,  41.20,  43.65,
                          46.25,  49.00,  51.91,  55.00,  58.27,  61.74,
                          65.41,  69.30,  73.42,  77.78,  82.41,  87.31,
                          92.50,  98.00, 103.83, 110.00, 116.54, 123.47,
                         130.81, 138.59, 146.83, 155.56, 164.81, 174.61,
                         185.00, 196.00, 207.65, 220.00, 233.08, 246.94,
                         261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
                         369.99, 392.00, 415.30, 440.00, 466.16, 493.88,
                         523.25, 554.37, 587.33, 622.25, 659.26, 698.46,
                         739.99, 783.99, 830.61, 880.00, 932.33, 987.77,
                        1046.5, 1108.73,1174.66,1244.51,1318.51,1396.91,
                        1479.98,1567.98,1661.22,1760.00,1864.66,1975.53,
                        2093.0, 2217.46,2349.32,2489.02,2637.02,2793.83,
                        2959.96,3135.96,3322.44,3520.00,3729.31,3951.07,
                        4186.01,4434.92,4698.64,4978.03 };

/*
 * Sounder_on
 * Turn sounder on
 */
static void Sounder_on(){
   OutputHigh( SND );
}

/*
 * Sounder_off
 * Turn sounder off
 */
static void Sounder_off(){
   OutputLow( SND );
}


/*
 * playNote( );
 * Parameters t: half period in us
 *            n: Number of cycles - duration
 */
void playNote(int t, int n) {
   int i;
   for (i=0; i< noteTable[t].cycles[n]; i++){
      Sounder_on();
      Delay_us(noteTable[t].halfDuration);
      Sounder_off();
      Delay_us(noteTable[t].halfDuration);
   }
   Delay_ms(15); // Inter-note articulation
}



/* 
 * playRest();
 * Parameter n: Number of cycles - duration
 */
void playRest( int n ) {
   int i;
   for (i=0; i< noteTable[0].cycles[n]; i++){
      Sounder_off();
      Delay_us(noteTable[0].halfDuration);
      Sounder_off();
      Delay_us(noteTable[0].halfDuration);
   }
}

/*
 * soundInit()
 *
 * Fills noteTable values - halfDurations and cycles
 * Call this once before calling playRest() or playNote()
 *
 */
void soundInit() {
   int n;
   int mult;
   int d;
   for (n= 0; n < 100; n++) {
      noteTable[n].halfDuration= 1.0 / noteFrequency[n] / 2 * 1000000;
      mult= 1;
      for ( d= 0; d <= 5; d++ ) {
         noteTable[n].cycles[d]= noteFrequency[n] / mult; 
         mult <<= 1; 
      }
   }
}


/* Make a click sound
 */
void Click( void ) {
int i;
   for ( i= 0; i< 5; i++ ) {
      OutputHigh( SND ); 
      Delay_ms(1);
      OutputLow( SND ); 
   }
}


