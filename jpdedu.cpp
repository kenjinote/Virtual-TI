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
// JPEG Decoder Data Unit class implementation
//
// Author:  John M. Miano miano@colosseumbuilders.com
//
// This class represents an 8x8 sample block for one
// component of the JPEG image. Its main function is to perform the
// Inverse Discrete Cosine Transform.
//
// We have two IDCT implementation defined here. One uses floating point
// and the other uses scaled integers. The integer implementation is much
// faster but it is slightly less precise than the floating point.
//
// (Right now the choice of the two is made at compile time. In the
//  future there may be a run-time switch).
//
// The algorithm is a matrix factorization that makes use extensive of the
// cosine product formula. The first phase of the IDCT is merged with
// quantization.
//

#include <math.h>

#include "jpdedu.h"

using namespace std ;

#if defined (_MSC_VER) // For whatever reason MSVC++ does not define PI.
const double M_PI = acos (-1.0) ;
#endif 

JpegDecoderDataUnit::IDctFunction JpegDecoderDataUnit::idct_function
                                  = &JpegDecoderDataUnit::IntegerInverseDCT ;
// Could also be set to &JpegDecoderDataUnit::FloatInverseDCT ;

const int IntegerScale = 6 ;

const long IC4 = (long)((1<<IntegerScale) * cos (M_PI * 4.0/16.0)) ;
const long ISEC2 = (long)((1<<(IntegerScale - 1)) / cos (M_PI * 2.0 / 16.0)) ;
const long ISEC6 = (long)((1<<(IntegerScale - 1)) / cos (M_PI * 6.0 / 16.0)) ;

const double FC4 = cos (M_PI * 4.0 / 16.0) ;
const double FSEC2 = 0.5 / cos (M_PI * 2.0 / 16.0) ;
const double FSEC6 = 0.5 / cos (M_PI * 6.0 / 16.0) ;

//
//  Description:
//
//    The IDCT process can produce rounding errors that result in sample
//    values being outside the legal range.  This function clamps
//    sample value into the legal range.
//
//    Unclamped values give strange results when converted to bytes.
//     -1 (0xFFFFFFFF) would be converted to 255 (0xFF) instead of 0 and
//    256 (0x00000100) gets converted to 0.
//
//  Parameters:
//    value: The value to clamp
//
//  Return Value:
//    "value" clamped to MinSampleValue..MaxSampleValue
//

static inline JPEGSAMPLE SampleRange (long value)
{
  if (value < JpegMinSampleValue)
    return JpegMinSampleValue ;
  else if (value > JpegMaxSampleValue)
    return JpegMaxSampleValue ;
  else
    return (JPEGSAMPLE) value ;
}

//
//  Description:
//
//    This function descales a scaled integer value.
//
//    This implementation is simplay a shift operation. We
//    use an inline function to give one place to change in case
//    we want to switch to a rounded scale.
//
//  Parameters:
//    value: The value to descale
//    amount:  The amount to descale
//
//  Return Value:
//    The descaled value.
//
static inline long Descale (long value, int amount)
{
  // A more precise value would be
  // result = (value + (1 << (amount - 1))) >> amount ;
  return value >> amount ;
}

//
//  Description:
//
//    Class Copy Constructor
//
JpegDecoderDataUnit::JpegDecoderDataUnit(const JpegDecoderDataUnit &du)
{
  memcpy (values, du.values, sizeof (values)) ;
  return ;
}

//
//  Description:
//
//    Class assignment operator
//
JpegDecoderDataUnit &JpegDecoderDataUnit::operator=(const JpegDecoderDataUnit&du)
{
  memcpy (values, du.values, sizeof (values)) ;
  return *this ;
}

//
//  Description:
//
//    This is a floating point implementation of the Inverse
//    Discrete Cosine Transform (IDCT).
//
//    This implementation uses a factorization of the DCT matrix.
//    The first steps in this factorization is a matrix multiplication
//    is the multiplication of each row/column by a scale. This
//    scalor multipliation has been combined with quantization
//    to eliminate 128 multiplication steps.
//
//    We use a lot of temporaries here in order to clearly
//    show the matrix multiplication steps.  Hopefully
//    your compiler will optimize out the unnecessary
//    intermediate variables.
//
//    If your compiler does not aggressively optimize. It is possible
//    to reorder the operations to reduce the number of temporaries
//    required.
//
//  Parameters:
//    data: The 8x8 matrix to perform the IDCT on.
//    qt: The prescaled quantization table.
//
JpegDecoderDataUnit &JpegDecoderDataUnit::FloatInverseDCT (
                            const JpegDecoderCoefficientBlock data,
                            const JpegDecoderQuantizationTable &qt)
{
  double tmp [JpegSampleWidth][JpegSampleWidth] ;
  unsigned int ii ;
  for (ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    double a0 = data [ii][0] * qt.float_scaling [ii][0] ;
    double a1 = data [ii][4] * qt.float_scaling [ii][4] ;
    double a2 = data [ii][2] * qt.float_scaling [ii][2] ;
    double a3 = data [ii][6] * qt.float_scaling [ii][6] ;
    double a4 = data [ii][1] * qt.float_scaling [ii][1] ;
    double a5 = data [ii][5] * qt.float_scaling [ii][5] ;
    double a6 = data [ii][3] * qt.float_scaling [ii][3] ;
    double a7 = data [ii][7] * qt.float_scaling [ii][7] ;

    double b0 = a0 ;
    double b1 = a1 ;
    double b2 = a2 - a3 ;
    double b3 = a2 + a3 ;
    double b4 = a4 - a7 ;
    double b5 = a5 + a6;
    double b6 = a5 - a6 ;
    double b7 = a4 + a7 ;

    double c0 = b0 ;
    double c1 = b1 ;
    double c2 = b2 ;
    double c3 = b3 ;
    double c4 = FSEC2 * b4 ;
    double c5 = b7 - b5 ;
    double c6 = FSEC6 * b6 ;
    double c7 = b5 + b7 ;

    double d0 = c0 ;
    double d1 = c1 ;
    double d2 = c2 ;
    double d3 = c3 ;
    double d4 = c4 + c6 ;
    double d5 = c5 ;
    double d6 = c4 - c6 ;
    double d7 = c7 ;

    double e0 = d0 + d1 ;
    double e1 = d0 - d1 ;
    double e2 = d2 * FC4 ;
    double e3 = d3 ;
    double e4 = d4 * FC4 ;
    double e5 = d5 * FC4 ;
    double e6 = d6 ;
    double e7 = d7 ;

    double f0 = e0 ;
    double f1 = e1 ;
    double f2 = e2 ;
    double f3 = e3 ;
    double f4 = e4 ;
    double f5 = e5 ;
    double f6 = e4 + e6 ;
    double f7 = e7 ;

    double g0 = f0 ;
    double g1 = f1 ;
    double g2 = f2 ;
    double g3 = f2 + f3 ;
    double g4 = f4 ;
    double g5 = f4 + f5 ;
    double g6 = f5 + f6 ;
    double g7 = f6 + f7 ;

    double h0 = g0 + g3 ;
    double h1 = g1 + g2 ;
    double h2 = g1 - g2 ;
    double h3 = g0 - g3 ;
    double h4 = g4 ;
    double h5 = g5 ;
    double h6 = g6 ;
    double h7 = g7 ;

    tmp [ii][0] = h0 + h7 ;
    tmp [ii][1] = h1 + h6 ;
    tmp [ii][2] = h2 + h5 ;
    tmp [ii][3] = h3 + h4 ;
    tmp [ii][4] = h3 - h4 ;
    tmp [ii][5] = h2 - h5 ;
    tmp [ii][6] = h1 - h6 ;
    tmp [ii][7] = h0 - h7 ;
  }

  for (ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    double a0 = tmp [0][ii] ;
    double a1 = tmp [4][ii] ;
    double a2 = tmp [2][ii] ;
    double a3 = tmp [6][ii] ;
    double a4 = tmp [1][ii] ;
    double a5 = tmp [5][ii] ;
    double a6 = tmp [3][ii] ;
    double a7 = tmp [7][ii] ;

    double b0 = a0 ;
    double b1 = a1 ;
    double b2 = a2 - a3 ;
    double b3 = a2 + a3 ;
    double b4 = a4 - a7 ;
    double b5 = a5 + a6;
    double b6 = a5 - a6 ;
    double b7 = a4 + a7 ;

    double c0 = b0 ;
    double c1 = b1 ;
    double c2 = b2 ;
    double c3 = b3 ;
    double c4 = FSEC2 * b4 ;
    double c5 = b7 - b5 ;
    double c6 = FSEC6 * b6 ;
    double c7 = b5 + b7 ;

    double d0 = c0 ;
    double d1 = c1 ;
    double d2 = c2 ;
    double d3 = c3 ;
    double d4 = c4 + c6 ;
    double d5 = c5 ;
    double d6 = c4 - c6 ;
    double d7 = c7 ;

    double e0 = d0 + d1 ;
    double e1 = d0 - d1 ;
    double e2 = d2 * FC4 ;
    double e3 = d3 ;
    double e4 = d4 * FC4 ;
    double e5 = d5 * FC4 ;
    double e6 = d6 ;
    double e7 = d7 ;

    double f0 = e0 ;
    double f1 = e1 ;
    double f2 = e2 ;
    double f3 = e3 ;
    double f4 = e4 ;
    double f5 = e5 ;
    double f6 = e4 + e6 ;
    double f7 = e7 ;

    double g0 = f0 ;
    double g1 = f1 ;
    double g2 = f2 ;
    double g3 = f2 + f3 ;
    double g4 = f4 ;
    double g5 = f4 + f5 ;
    double g6 = f5 + f6 ;
    double g7 = f6 + f7 ;

    double h0 = g0 + g3 ;
    double h1 = g1 + g2 ;
    double h2 = g1 - g2 ;
    double h3 = g0 - g3 ;
    double h4 = g4 ;
    double h5 = g5 ;
    double h6 = g6 ;
    double h7 = g7 ;

    static const double rounding = JpegMidpointSampleValue + 0.5 ;
    values [0][ii] = SampleRange ((long)((h0 + h7) + rounding)) ;
    values [1][ii] = SampleRange ((long)((h1 + h6) + rounding)) ;
    values [2][ii] = SampleRange ((long)((h2 + h5) + rounding)) ;
    values [3][ii] = SampleRange ((long)((h3 + h4) + rounding)) ;
    values [4][ii] = SampleRange ((long)((h3 - h4) + rounding)) ;
    values [5][ii] = SampleRange ((long)((h2 - h5) + rounding)) ;
    values [6][ii] = SampleRange ((long)((h1 - h6) + rounding)) ;
    values [7][ii] = SampleRange ((long)((h0 - h7) + rounding)) ;
  }
  return *this ;
}
//
//  Description:
//
//    This is a scaled integer implementation of the Inverse
//    Discrete Cosine Transform (IDCT).
//
//    This implementation uses a factorization of the DCT matrix.
//    The first steps in this factorization is a matrix multiplication
//    is the multiplication of each row/column by a scale. This
//    scalor multipliation has been combined with quantization
//    to eliminate 128 multiplication steps.
//
//    We use a lot of temporaries here in order to clearly
//    show the matrix multiplication steps.  Hopefully
//    your compiler will optimize out the unnecessary
//    intermediate variables.
//
//    If your compiler does not aggressively optimize. It is possible
//    to reorder the operations to reduce the number of temporaries
//    required.
//
//  Parameters:
//    data: The 8x8 matrix to perform the IDCT on.
//    qt: The prescaled quantization table.
//
JpegDecoderDataUnit &JpegDecoderDataUnit::IntegerInverseDCT (
                          const JpegDecoderCoefficientBlock data,
                          const JpegDecoderQuantizationTable  &qt)
{
  unsigned int ii ; 
  long tmp [JpegSampleWidth][JpegSampleWidth] ;

  for (ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    // This optimization does not seem to be worth the trouble in the
    // second loop.
    if ((data [ii][1] | data [ii][2] | data [ii][3] | data [ii][4] |
        data [ii][5] | data [ii][6] | data [ii][7]) == 0)
    {
      tmp [ii][0] = data [ii][0] * qt.integer_scaling [ii][0] ;
      tmp [ii][1] = tmp [ii][0] ;
      tmp [ii][2] = tmp [ii][0] ;
      tmp [ii][3] = tmp [ii][0] ;
      tmp [ii][4] = tmp [ii][0] ;
      tmp [ii][5] = tmp [ii][0] ;
      tmp [ii][6] = tmp [ii][0] ;
      tmp [ii][7] = tmp [ii][0] ;
    }
    else
    {
      long a0 = data [ii][0] * qt.integer_scaling [ii][0] ;
      long a1 = data [ii][4] * qt.integer_scaling [ii][4] ;
      long a2 = data [ii][2] * qt.integer_scaling [ii][2] ;
      long a3 = data [ii][6] * qt.integer_scaling [ii][6] ;
      long a4 = data [ii][1] * qt.integer_scaling [ii][1] ;
      long a5 = data [ii][5] * qt.integer_scaling [ii][5] ;
      long a6 = data [ii][3] * qt.integer_scaling [ii][3] ;
      long a7 = data [ii][7] * qt.integer_scaling [ii][7] ;

      long b0 = a0 ;
      long b1 = a1 ;
      long b2 = a2 - a3 ;
      long b3 = a2 + a3 ;
      long b4 = a4 - a7 ;
      long b5 = a5 + a6;
      long b6 = a5 - a6 ;
      long b7 = a4 + a7 ;

      long c0 = b0 ;
      long c1 = b1 ;
      long c2 = b2 ;
      long c3 = b3 ;
      long c4 = Descale (ISEC2 * b4, IntegerScale) ;
      long c5 = b7 - b5 ;
      long c6 = Descale (ISEC6 * b6, IntegerScale) ;
      long c7 = b5 + b7 ;

      long d0 = c0 ;
      long d1 = c1 ;
      long d2 = c2 ;
      long d3 = c3 ;
      long d4 = c4 + c6 ;
      long d5 = c5 ;
      long d6 = c4 - c6 ;
      long d7 = c7 ;

      long e0 = d0 + d1 ;
      long e1 = d0 - d1 ;
      long e2 = Descale (d2 * IC4, IntegerScale) ;
      long e3 = d3 ;
      long e4 = Descale (d4 * IC4, IntegerScale) ;
      long e5 = Descale (d5 * IC4, IntegerScale) ;
      long e6 = d6 ;
      long e7 = d7 ;

      long f0 = e0 ;
      long f1 = e1 ;
      long f2 = e2 ;
      long f3 = e3 ;
      long f4 = e4 ;
      long f5 = e5 ;
      long f6 = e4 + e6 ;
      long f7 = e7 ;

      long g0 = f0 ;
      long g1 = f1 ;
      long g2 = f2 ;
      long g3 = f2 + f3 ;
      long g4 = f4 ;
      long g5 = f4 + f5 ;
      long g6 = f5 + f6 ;
      long g7 = f6 + f7 ;

      long h0 = g0 + g3 ;
      long h1 = g1 + g2 ;
      long h2 = g1 - g2 ;
      long h3 = g0 - g3 ;
      long h4 = g4 ;
      long h5 = g5 ;
      long h6 = g6 ;
      long h7 = g7 ;

      tmp [ii][0] = h0 + h7 ;
      tmp [ii][1] = h1 + h6 ;
      tmp [ii][2] = h2 + h5 ;
      tmp [ii][3] = h3 + h4 ;
      tmp [ii][4] = h3 - h4 ;
      tmp [ii][5] = h2 - h5 ;
      tmp [ii][6] = h1 - h6 ;
      tmp [ii][7] = h0 - h7 ;
    }
  }

  for (ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    long a0 = tmp [0][ii] ;
    long a1 = tmp [4][ii] ;
    long a2 = tmp [2][ii] ;
    long a3 = tmp [6][ii] ;
    long a4 = tmp [1][ii] ;
    long a5 = tmp [5][ii] ;
    long a6 = tmp [3][ii] ;
    long a7 = tmp [7][ii] ;

    long b0 = a0 ;
    long b1 = a1 ;
    long b2 = a2 - a3 ;
    long b3 = a2 + a3 ;
    long b4 = a4 - a7 ;
    long b5 = a5 + a6;
    long b6 = a5 - a6 ;
    long b7 = a4 + a7 ;

    long c0 = b0 ;
    long c1 = b1 ;
    long c2 = b2 ;
    long c3 = b3 ;
    long c4 = Descale (ISEC2 * b4, IntegerScale) ;
    long c5 = b7 - b5 ;
    long c6 = Descale (ISEC6 * b6, IntegerScale) ;
    long c7 = b5 + b7 ;

    long d0 = c0 ;
    long d1 = c1 ;
    long d2 = c2 ;
    long d3 = c3 ;
    long d4 = c4 + c6 ;
    long d5 = c5 ;
    long d6 = c4 - c6 ;
    long d7 = c7 ;

    long e0 = d0 + d1 ;
    long e1 = d0 - d1 ;
    long e2 = Descale (d2 * IC4, IntegerScale) ;
    long e3 = d3 ;
    long e4 = Descale (d4 * IC4, IntegerScale) ;
    long e5 = Descale (d5 * IC4, IntegerScale) ;
    long e6 = d6 ;
    long e7 = d7 ;

    long f0 = e0 ;
    long f1 = e1 ;
    long f2 = e2 ;
    long f3 = e3 ;
    long f4 = e4 ;
    long f5 = e5 ;
    long f6 = e4 + e6 ;
    long f7 = e7 ;

    const long rounding = (JpegMaxSampleValue + 2) << (JpegDecoderQuantizationTable::QuantizationIntegerScale-1) ;
    long g0 = f0 + rounding ;
    long g1 = f1 + rounding ;
    long g2 = f2 ;
    long g3 = f2 + f3 ;
    long g4 = f4 ;
    long g5 = f4 + f5 ;
    long g6 = f5 + f6 ;
    long g7 = f6 + f7 ;

    long h0 = g0 + g3 ;
    long h1 = g1 + g2 ;
    long h2 = g1 - g2 ;
    long h3 = g0 - g3 ;
    long h4 = g4 ;
    long h5 = g5 ;
    long h6 = g6 ;
    long h7 = g7 ;

    values [0][ii] = SampleRange (Descale (h0 + h7, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
    values [1][ii] = SampleRange (Descale (h1 + h6, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
    values [2][ii] = SampleRange (Descale (h2 + h5, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
    values [3][ii] = SampleRange (Descale (h3 + h4, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
    values [4][ii] = SampleRange (Descale (h3 - h4, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
    values [5][ii] = SampleRange (Descale (h2 - h5, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
    values [6][ii] = SampleRange (Descale (h1 - h6, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
    values [7][ii] = SampleRange (Descale (h0 - h7, JpegDecoderQuantizationTable::QuantizationIntegerScale)) ;
  }
  return *this ;
}

//
//  Description:
//
//    This function writes the contents of the data unit to a stream.
//    It is only for debugging.
//
//  Parameters:
//    strm:  The output stream
//
void JpegDecoderDataUnit::Print (std::ostream &strm) const
{
/*  for (int ii = 0 ; ii < JpegSampleWidth ; ++ ii)
  {
    for (int jj = 0 ; jj < JpegSampleWidth ; ++ jj)
    {
      strm << (int) values [ii][jj] << " " ;
    }
    strm << endl ;
  }
  return ;*/
}



