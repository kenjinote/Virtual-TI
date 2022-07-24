#ifndef __JPDEQUAN_H
#define __JPDEQUAN_H
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
//  Title:  JPEG Decoder Quantization Table Class Implementation
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Description:
//
//    This class represents a quantization table used by the
//    JPEG decoder.
//


#include <stdio.h>
#include "jpeg.h"
#include "bitimage.h"

class JpegDecoder ;

class JpegDecoderQuantizationTable
{
public:
  JpegDecoderQuantizationTable() ;
  ~JpegDecoderQuantizationTable() {}

  // This function tells the caller if the quantization table has been
  // defined by the JPEG input stream.
  bool Defined () const ;

  // Function to read the quantization table from the input stream.
  void ReadTable (JpegDecoder &decoder, unsigned int precision) ;

  // This function builds the scaled quantization tables used in
  // fast IDCT implementations.
  void BuildScaledTables () ;

  // This function prints the contents of the quantization table to
  // an output stream.
  void Print (FILE *) const ;

private:
  // Dummy Declarations for Required Member Functions
  JpegDecoderQuantizationTable (const JpegDecoderQuantizationTable&) ;
  JpegDecoderQuantizationTable &operator=(const JpegDecoderQuantizationTable&) ;

  // Quantization Values in Zig-Zag Order.
  UBYTE2 data_values [JpegSampleSize] ;


  // Scaling factor used for the scaled integer values.
  enum { QuantizationIntegerScale = 12, } ;

  // Scaled quantization values used for the fast IDCT implementations.
  double float_scaling [JpegSampleWidth][JpegSampleWidth] ;
  long integer_scaling [JpegSampleWidth][JpegSampleWidth] ;

  // This flag gets set to true when the quantization is defined in the
  // JPEG input stream. It is used to ensure that an compressed scan does
  // not attempt to use an undefined quantization table.
  bool table_defined ;

  friend class JpegDecoderDataUnit ;
} ;

inline bool JpegDecoderQuantizationTable::Defined () const
{
  return table_defined ;
}


/*inline std::ostream &operator<<(std::ostream &strm,
                                const JpegDecoderQuantizationTable &du)
{
  du.Print (strm) ;
  return strm ;
}*/

#endif
