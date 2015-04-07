/* Sound.h */

#define SND 0x00000040  /* P6 */


/* Names of note durations - semi-breve...demi-semi-quaver */

enum dur {sb,m,c,q,sq,dsq};

/* Names of notes, middle C is c4 */

enum note {c0,c0s,d0,d0s,e0,f0,f0s,g0,g0s,a0,a0s,b0,
           c1,c1s,d1,d1s,e1,f1,f1s,g1,g1s,a1,a1s,b1,
           c2,c2s,d2,d2s,e2,f2,f2s,g2,g2s,a2,a2s,b2,
           c3,c3s,d3,d3s,e3,f3,f3s,g3,g3s,a3,a3s,b3,
           c4,c4s,d4,d4s,e4,f4,f4s,g4,g4s,a4,a4s,b4,
           c5,c5s,d5,d5s,e5,f5,f5s,g5,g5s,a5,a5s,b5,
           c6,c6s,d6,d6s,e6,f6,f6s,g6,g6s,a6,a6s,b6,
           c7,c7s,d7,d7s,e7,f7,f7s,g7,g7s,a7,a7s,b7,
           c8,c8s,d8,d8s};

void soundInit( void );
void Click ( void );
void playNote(int t, int n);
void playRest( int n );
