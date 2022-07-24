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
// Title:   EncoderQuantizationTable Class Implementation
//
// Author: John M. Miano  miano@colosseumbuilders.com
//
//

#include "jpenquan.h"

#include "jpegenco.h"
#include "jpgexcep.h"
#include <string.h>

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
//    Default Class Constructor
//

JpegEncoderQuantizationTable::JpegEncoderQuantizationTable ()
{
  memset (data_values, 0, sizeof (data_values)) ;
  return ;
}

//
//  Description:
//
//    This function creates the scaled quantization tables used by the fast
//    FDCT algorithm.
//
void JpegEncoderQuantizationTable::BuildScaledTables ()
{
  for (int ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    for (int jj = 0 ; jj < JpegSampleWidth ; ++ jj)
    {
      float_scaling [ii][jj] = floatscaling [ii][jj] /
                   data_values [JpegZigZagOutputOrder (ii * JpegSampleWidth + jj)] ;
    }
  }
  return ;
}

