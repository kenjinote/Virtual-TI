#ifndef __JPENDU_H
#define __JPENDU_H
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
//  Title:  JpegEncoderDataUnit class definition
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Description:
//
//    This class represents a data unit in the JPEG encoder.
//

#include "jpeg.h"
#include "jpencobk.h"
#include "jpenquan.h"

class JpegEncoderDataUnit
{
public:
  typedef JPEGSAMPLE DATAUNITVALUE ;

  JpegEncoderDataUnit () {} ;
  ~JpegEncoderDataUnit () {} ; //Must not be virtual.

  DATAUNITVALUE *operator[] (unsigned int) ;
  DATAUNITVALUE &ZigZagInput (unsigned int) ;

  void ForwardDct (JpegEncoderQuantizationTable &,
                   JpegEncoderCoefficientBlock &) ;

private:
  JpegEncoderDataUnit (const JpegEncoderDataUnit &) ;
  JpegEncoderDataUnit &operator=(const JpegEncoderDataUnit &) ;

  DATAUNITVALUE data [JpegSampleWidth][JpegSampleWidth] ;
} ;

inline JpegEncoderDataUnit::DATAUNITVALUE *JpegEncoderDataUnit::operator [](unsigned int ii)
{
  return data [ii] ;
}

inline JpegEncoderDataUnit::DATAUNITVALUE &JpegEncoderDataUnit::ZigZagInput (unsigned int ii)
{
  unsigned int order = JpegZigZagInputOrder (ii) ;
  return ((DATAUNITVALUE *) data) [order] ;
}

#endif
