#define OLC_PGE_APPLICATION

// // *IMPORTANT NOTE*: the olc PixelGameEngine includes X11 headers and puts these
// //                   into a dedicated namespace (Nice !); since the X11 headers
// //                   have already been included, circumvent the double header
// //                   inclusion guard
// #if defined (X_H)
// #undef X_H
// #endif // X_H
// #if defined (_X11_XLIB_H_)
// #undef _X11_XLIB_H_
// #endif // _X11_XLIB_H_
#undef OK

#include "olcPixelGameEngine.h"
