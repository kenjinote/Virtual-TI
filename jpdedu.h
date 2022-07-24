#ifndef __JPDEDU_H
#define __JPDEDU_H
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
//  Decoder Data Unit Class Definition
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Descrition:
//
//    The DataUnit class represents an 8x8 sample block for one
//    component of the JPEG image.
//
//

#include <iostream>

#include "jpeg.h"
#include "jpdequan.h"
#include "jpdecobk.h"

class JpegDecoderDataUnit ;

std::ostream &operator<<(std::ostream &, JpegDecoderDataUnit &) ;

class JpegDecoderDataUnit
{
public:
  // Declaration of a type for pointers to member functions
  // for implementing the IDCT. The input parameters are
  // The IDCT coefficients and the [de]quantization table.
  typedef JpegDecoderDataUnit &(JpegDecoderDataUnit::*IDctFunction) (
                                const JpegDecoderCoefficientBlock,
                                const JpegDecoderQuantizationTable  &) ;

  JpegDecoderDataUnit() {}
  virtual ~JpegDecoderDataUnit () {}

  // Utility function to write the IDCT values to an output stream.
  void Print (std::ostream &) const ;

  // General IDCT Function
  JpegDecoderDataUnit &JpegDecoderDataUnit::InverseDCT (
                        const JpegDecoderCoefficientBlock cb,
                        const JpegDecoderQuantizationTable  &qt) ;

  // These are the IDCT implementations.
  JpegDecoderDataUnit &FloatInverseDCT (const JpegDecoderCoefficientBlock,
                             const JpegDecoderQuantizationTable  &) ;
  JpegDecoderDataUnit &IntegerInverseDCT (const JpegDecoderCoefficientBlock,
                               const JpegDecoderQuantizationTable  &) ;

  // Operators to retrieve the individual IDCT values.
  JPEGSAMPLE *operator [] (unsigned int ii) ;
  JPEGSAMPLE const* operator [] (unsigned int ii) const ;

private:
  // Dummy Declarations For Required Members Functions 
  JpegDecoderDataUnit (const JpegDecoderDataUnit &) ;
  JpegDecoderDataUnit &operator=(const JpegDecoderDataUnit &) ;

  // The IDCT values.
  UBYTE1 values [JpegSampleWidth][JpegSampleWidth] ;

  // This is a pointer to the member function that implements
  // the desired IDCT function.
  static IDctFunction idct_function ;
} ;

inline JPEGSAMPLE *JpegDecoderDataUnit::operator [] (unsigned int ii)
{
 return values [ii] ;
}

inline JPEGSAMPLE const* JpegDecoderDataUnit::operator [] (unsigned int ii) const
{
  return values [ii] ;
}

inline std::ostream &operator<<(std::ostream &strm, const JpegDecoderDataUnit &du)
{
  du.Print (strm) ;
  return strm ;
}

inline JpegDecoderDataUnit &JpegDecoderDataUnit::InverseDCT (
                        const JpegDecoderCoefficientBlock cb,
                        const JpegDecoderQuantizationTable  &qt)
{
  return (this->*idct_function) (cb, qt) ;
}


#endif
