#ifndef __JFIF_H
#define __JFIF_H
//
// Copyright (c) 1997,1998 Colosseum Builders, Inc.
// All rights reserved.
//
// Colosseum Builders, Inc. makes no warranty, expressed or implied
// with regards to this software. It is provided as is.
//
// See the README.TXT file that came with this software for information
// on redistribution or send E-mail to info@colosseumbuilders.com
//
// o The user assumes all risk for using this software. The authors of this
//   software shall be liable for no damages of any kind.
//
// o If the source code is distributed then this copyright notice must
//   remain unaltered and any modification must be noted.
//
// o If this code is shipped in binary format the accompanying documentation
//   should state that "this software is based, in part, on the work of
//   Colosseum Builders, Inc."
//

//
//  JFIF Definitions
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  These definitions match version 1.02 of the
//  "JPEG File Interchange Format Specification" by Eric Hamilton
//

#include "jpeg.h"

struct JfifHeader
{
  UBYTE2 length ;
  UBYTE1 identifier [5] ;
  UBYTE1 version [2] ;
  UBYTE1 units ;
  UBYTE2 xdensity ;
  UBYTE2 ydensity ;
  UBYTE1 xthumbnail ;
  UBYTE1 ythumbnail ;
  // 3 * xthumbnail * ythumbnail bytes of thumbnail data follow.
} ;

struct JfifExtension
{
  UBYTE2 length ;
  UBYTE1 identifier [5] ;
  UBYTE1 extension_code ;
  enum { JpegThumbnail = 0x10,
         OneByteThumbnail = 0x11,
         ThreeByteThumbnail = 0x13, } ;
  // Extension Data follows
} ;

// To implement 12-bit data the return values would have to be
// at least 12 bits wide.

// Functions for YCbCr/RGB colorspace conversion
JPEGSAMPLE YCbCrToR (int yy, int cb, int cr) ;
JPEGSAMPLE YCbCrToG (int yy, int cb, int cr) ;
JPEGSAMPLE YCbCrToB (int yy, int cb, int cr) ;

JPEGSAMPLE RGBToY (JPEGSAMPLE red, JPEGSAMPLE green, JPEGSAMPLE blue) ;
JPEGSAMPLE RGBToCb (JPEGSAMPLE red, JPEGSAMPLE green, JPEGSAMPLE blue) ;
JPEGSAMPLE RGBToCr (JPEGSAMPLE red, JPEGSAMPLE green, JPEGSAMPLE blue) ;

#endif
