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
// JPEG Encoder Library.
//
// Title:   JFIF Implementation
//
// Author: John M. Miano  miano@colosseumbuilders.com
//
// Description:
//
//    This module contains the RGB/YcbCr conversion functions specified
//    by the JFIF standard. This functions are implemented using scaled
//    integers.
//
//    If you are using MS C++ you may wish to re-write this since that compile
//    generates terrible code for these function. Borland C++ is great here,
//    and G++ works well if you turn on optimizations.
//

//
#include "jfif.h"


const int ScaleFactor = 12 ;
const int ScaleValue = (1<<ScaleFactor) ;
const int Rounding = (1<<(ScaleFactor-1)) ;

static inline int Scaled (double value)
{
  return (int) (ScaleValue * value) ;
}

JPEGSAMPLE YCbCrToR (int yy, int /*cb*/, int cr)
{
  int result = yy + ((Scaled (1.402) * (cr - JpegMidpointSampleValue) + Rounding) >> ScaleFactor) ;
  if (result < 0)
    result = 0 ;
  else if (result > JpegMaxSampleValue)
    result = JpegMaxSampleValue ;
  return (JPEGSAMPLE) result ;
}

JPEGSAMPLE YCbCrToG (int yy, int cb, int cr)
{
  int result = yy - ((Scaled (0.34414) * (cb - JpegMidpointSampleValue)
                     + Scaled (0.71414) * (cr - JpegMidpointSampleValue)
                     + Rounding) >> ScaleFactor) ;

  if (result < 0)
    result = 0 ;
  else if (result > JpegMaxSampleValue)
    result = JpegMaxSampleValue ;
  return (JPEGSAMPLE) result ;
}

JPEGSAMPLE YCbCrToB (int yy, int cb, int /*cr*/)
{
  int result = yy + ((Scaled (1.772) * (cb - JpegMidpointSampleValue)
             + Rounding) >> ScaleFactor) ;
  if (result < 0)
    result = 0 ;
  else if (result > JpegMaxSampleValue)
    result = JpegMaxSampleValue ;
  return (JPEGSAMPLE) result ;
}

JPEGSAMPLE RGBToY (JPEGSAMPLE red, JPEGSAMPLE green, JPEGSAMPLE blue)
{
  int result = (Scaled (0.299) * red
             + Scaled (0.587) * green + Scaled (0.114) * blue
             + Rounding) >> ScaleFactor ;
  if (result > JpegMaxSampleValue)
  	result = JpegMaxSampleValue ;
  else if (result < JpegMinSampleValue)
  	result = JpegMinSampleValue ;
  return result ;
}

JPEGSAMPLE RGBToCb (JPEGSAMPLE red, JPEGSAMPLE green, JPEGSAMPLE blue)
{
  int result = ((JpegMidpointSampleValue<<ScaleFactor)
             + Rounding - Scaled (0.1687) * red
             - Scaled (0.3313) * green + Scaled (0.5) * blue) >> ScaleFactor  ;
  if (result > JpegMaxSampleValue)
  	result = JpegMaxSampleValue ;
  else if (result < JpegMinSampleValue)
  	result = JpegMinSampleValue ;
  return result ;
}

JPEGSAMPLE RGBToCr (JPEGSAMPLE red, JPEGSAMPLE green, JPEGSAMPLE blue)
{
  int result = ((JpegMidpointSampleValue<<ScaleFactor)
             + Rounding + Scaled (0.5) * red
             - Scaled (0.4187) * green - Scaled (0.0813) * blue) >> ScaleFactor ;
  if (result > JpegMaxSampleValue)
  	result = JpegMaxSampleValue ;
  else if (result < JpegMinSampleValue)
  	result = JpegMinSampleValue ;
  return result ;
}

