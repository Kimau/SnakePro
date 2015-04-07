/*
 * XBee.h
 *
 */

/* Initialise the XBee module - assign PANID, MY and DH,DL addresses */
int XBeeInit( unsigned int PANID,
              unsigned int MY,
              unsigned int DH,
              unsigned int DL );
