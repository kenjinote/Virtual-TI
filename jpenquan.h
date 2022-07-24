#ifndef __JPENQUAN_H
#define __JPENQUAN_H
//
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
//  Title:  JpegEncoderQuantizationTable class definition
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Description:
//
//    This class represents quantization table in the JPEG encoder.
//

#include "jpeg.h"
class JpegDecoder ;

class JpegEncoderQuantizationTable
{
public:
  JpegEncoderQuantizationTable() ;
  ~JpegEncoderQuantizationTable() {}

  UBYTE2 operator[](unsigned int index) const ;
  UBYTE2 &operator[](unsigned int index) ;

  void BuildScaledTables () ;

private:
  JpegEncoderQuantizationTable (const JpegEncoderQuantizationTable&) ;
  JpegEncoderQuantizationTable &operator=(const JpegEncoderQuantizationTable&) ;

  UBYTE2 data_values [JpegSampleSize] ;
  double float_scaling [JpegSampleWidth][JpegSampleWidth] ;

  friend class JpegEncoderDataUnit ;
} ;

inline UBYTE2 JpegEncoderQuantizationTable::operator[](unsigned int index) const
{
  return data_values [index] ;
}

inline UBYTE2 &JpegEncoderQuantizationTable::operator[](unsigned int index)
{
  return data_values [index] ;
}



#endif
