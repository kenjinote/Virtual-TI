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
// JPEG Decoder Quantization Table Class Implementation
//
// Author:  John M. Miano  miano@colosseumbuilders.com
//

#include <stdio.h>
#include <string.h>
#include "jpdequan.h"
#include "jpegdeco.h"
#include "jpgexcep.h"

using namespace std ;

#pragma comment (exestr, "Copyright 1997 Colosseum Builders Inc.")

//
// This table consists of the values
//
//   F (i, j) = X (i) X (j) / 8 
// 
// where
// 
//  X (n) = 1, n = 0, 4
//  X (n) = 1 / sqrt(2) / cos (n*PI/16) 
//  

static const double floatscaling [JpegSampleWidth][JpegSampleWidth] = 
{
{ 0.125,                  0.09011997775086849627, 0.09567085809127244544, 0.1063037618459070632,  0.125,                  0.159094822571604233,  0.2309698831278216846, 0.4530637231764438333, },
{ 0.09011997775086849627, 0.0649728831185362593,  0.0689748448207357645,  0.07664074121909414394, 0.09011997775086849627, 0.1147009749634507608, 0.1665200058287998886, 0.3266407412190940884, },
{ 0.09567085809127244544, 0.0689748448207357645,  0.0732233047033631207,  0.08136137691302557096, 0.09567085809127244544, 0.1217659055464329343, 0.1767766952966368932, 0.3467599613305368256, },
{ 0.1063037618459070632,  0.07664074121909414394, 0.08136137691302557096, 0.09040391826073060355, 0.1063037618459070632,  0.135299025036549253,  0.1964237395967755595, 0.3852990250365491698, },
{ 0.125,                  0.09011997775086849627, 0.09567085809127244544, 0.1063037618459070632,  0.125,                  0.159094822571604233,  0.2309698831278216846, 0.4530637231764438333, },
{ 0.159094822571604233,   0.1147009749634507608,  0.1217659055464329343,  0.135299025036549253,   0.159094822571604233,   0.2024893005527218515, 0.2939689006048396558, 0.5766407412190940329, },
{ 0.2309698831278216846,  0.1665200058287998886,  0.1767766952966368932,  0.1964237395967755595,  0.2309698831278216846,  0.2939689006048396558, 0.4267766952966368654, 0.8371526015321518744, },
{ 0.4530637231764438333,  0.3266407412190940884,  0.3467599613305368256,  0.3852990250365491698,  0.4530637231764438333,  0.5766407412190940329, 0.8371526015321518744, 1.642133898068010689,  },
} ;

//
//  Description:
//
//    Class Default Constructor
//
JpegDecoderQuantizationTable::JpegDecoderQuantizationTable ()
{
  table_defined = false ;
  memset (data_values, 0, sizeof (data_values)) ;
  return ;
}

//
//  Description:
//
//    This function reads a quantization table from a JPEG stream.
//
//  Parameters:
//    decoder:  The JPEG decoder that owns the table and the JPEG stream.
//    precision: The quantization table precision
//
void JpegDecoderQuantizationTable::ReadTable (JpegDecoder &decoder,
                                              unsigned int precision)
{
  // B 2.4.1
  // Determine if 16-bit or 8-bit precision is used for the quantization
  // values in the file.
  if (precision == 1)
  {
// Our source code only allows 8-bit data. The standard says
// 16-bit quantization tables are not allowed with 8-bit data.
// The commented code shows how 16-bit tables would be implemented.
//
//    // Read 16-bit values.
//    for (unsigned int ii = 0 ; ii < SampleSize ; ++ ii)
//    {
//      data_values[ii] = decoder.ReadWord () ;
//      if (data_values[ii] == 0)
//        throw EJpegBadData ("Zero value in quantization table") ;
//    }
    throw EJpegBadData ("Only 8-bit Data is Supported") ;
  }
  else if (precision == 0)
  {
    // Read 8-bit values.
    for (unsigned int ii = 0 ; ii < JpegSampleSize ; ++ ii)
    {
      data_values[ii] = decoder.ReadByte () ;
      if (data_values[ii] == 0)
        throw EJpegBadData ("Zero value in quantization table") ;
    }
  }
  else
  {
    throw EJpegBadData ("Invalid Quantization Table Precision") ;
  }

  BuildScaledTables () ;

  table_defined = true ;
  return ;
}

//
//  Description:
//
//    This function creates scaled quantization tables that
//    allow quantization to be merged with the IDCT process.
//    We factor the DCT matrix so that the first step in the
//    IDCT is to multiply each value by a constant. Here we
//    merge that constant with the quantization table valus.
//
void JpegDecoderQuantizationTable::BuildScaledTables ()
{
  unsigned int ii ;  // Overcome MSVC++ Wierdness

  for (ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    for (int jj = 0 ; jj < JpegSampleWidth ; ++ jj)
    {
      float_scaling [ii][jj] = data_values [JpegZigZagOutputOrder (ii * 8 + jj)]
                             * floatscaling [ii][jj] ;
    }
  }

  for (ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    for (int jj = 0 ; jj < JpegSampleWidth ; ++ jj)
    {
      integer_scaling [ii][jj] = (long) ((1 << QuantizationIntegerScale)
                               * floatscaling [ii][jj]
                               * data_values  [JpegZigZagOutputOrder (ii * 8 + jj)]) ;
    }
  }
  return ;
}

//
//  Description:
//
//    This is a debugging function that prints the contents
//    of the quantization table to a stream.
//
//  Parameters:
//
//    strm:  The output stream
//
void JpegDecoderQuantizationTable::Print (FILE *strm) const
{
/*  for (unsigned int ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    strm << endl << "        "  ;
    for (unsigned int jj = 0 ; jj < JpegSampleWidth ; ++ jj)
    {
      strm << dec
           << data_values [ii * JpegSampleWidth + jj]
           << " " ;
    }
  }
  return ;*/
}
