#ifndef __JPENCOMP_H
#define __JPENCOMP_H
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
//  Title:  JpegEncoderComponent class definition
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Description:
//
//    This class represents a single component while writing a JPEG image.
//

#include "bitimage.h"

#include "jpeg.h"

class JpegEncoder ;
class JpegEncoderComponent ;
class JpegEncoderHuffmanTable ;
class JpegEncoderQuantizationTable ;
class JpegEncoderCoefficientBlock ;

class JpegEncoderComponent
{
public:
  JpegEncoderComponent () ;
  virtual ~JpegEncoderComponent () ;

  void PrintAcData (int, int, int) ;
  void PrintDcData (int, int) ;
  void GatherAcData (int, int, int) ;
  void GatherDcData (int, int) ;

  typedef void (JpegEncoderComponent::*DCOUTPUTFUNCTION) (int, int) ;
  typedef void (JpegEncoderComponent::*ACOUTPUTFUNCTION) (int, int, int) ;
  typedef void (JpegEncoderComponent::*COMPONENTPASSFUNCTION) (
                            unsigned int row, unsigned int col,
                            DCOUTPUTFUNCTION, ACOUTPUTFUNCTION,
                            unsigned int sss, unsigned int sse,
                            unsigned int ssa) ;
  

  void EncodeSequential (unsigned int row, unsigned int col,
                         DCOUTPUTFUNCTION dcfunction,
                         ACOUTPUTFUNCTION acfunction,
                         unsigned int, unsigned int, unsigned int) ;
  void ProgressiveDcFirst (unsigned int row, unsigned int col,
                           DCOUTPUTFUNCTION dcfunction, ACOUTPUTFUNCTION,
                           unsigned int, unsigned int,
                           unsigned int ssa) ;
  void ProgressiveDcRefine (unsigned int row, unsigned int col,
                            DCOUTPUTFUNCTION dcfunction, ACOUTPUTFUNCTION,
                            unsigned int, unsigned int,
                            unsigned int) ;
  void ProgressiveAcFirst (unsigned int row, unsigned int col,
                           ACOUTPUTFUNCTION acfunction,
                           unsigned int sss, unsigned int sse,
                           unsigned int ssa) ;
  void ProgressiveAcRefine (unsigned int row, unsigned int col,
                            ACOUTPUTFUNCTION acfunction,
                            unsigned int sss, unsigned int sse,
                            unsigned int ssa) ;

  void ResetEobRun () ;
  void PrintEobRun (ACOUTPUTFUNCTION acfunction) ;
  void PrintRefineEobRun (ACOUTPUTFUNCTION acfunction,
                          unsigned int sss, unsigned int sse,
                          unsigned int ssa) ;

  void SampleYComponent (JpegEncoder &encoder, const BitmapImage &, unsigned int maxhf, unsigned int maxvf) ;
  void SampleCbComponent (JpegEncoder &encoder, const BitmapImage &, unsigned int maxhf, unsigned int maxvf) ;
  void SampleCrComponent (JpegEncoder &encoder, const BitmapImage &, unsigned int maxhf, unsigned int maxvf) ;

  unsigned int GetHorizontalFrequency () const ;
  void SetHorizontalFrequency (unsigned int) ;
  unsigned int GetVerticalFrequency () const ;
  void SetVerticalFrequency (unsigned int) ;

  unsigned int DataUnitRows () const ;
  unsigned int DataUnitCols () const ;
  void ResetDcDifference () ;

  void SetHuffmanTables (JpegEncoderHuffmanTable &dc,
                         JpegEncoderHuffmanTable &ac) ;
  void SetQuantizationTable (JpegEncoderQuantizationTable &table) ;

  void FreeDynamicStorage () ;

private:
  JpegEncoderComponent (const JpegEncoderComponent &) ;
  JpegEncoderComponent &operator=(const JpegEncoderComponent &) ;

  void CalculateDuDimensions (const BitmapImage &image, unsigned int maxhf, unsigned int maxvf) ;

  typedef JPEGSAMPLE (*RGBTOYCBCRFUNCTION)(
                         JPEGSAMPLE red,
                         JPEGSAMPLE green,
                         JPEGSAMPLE blue) ;
  void Sample1to1Component (const BitmapImage &, RGBTOYCBCRFUNCTION) ;
  void SampleNtoNComponent (const BitmapImage &image,
                            RGBTOYCBCRFUNCTION function) ;

  JpegEncoder *jpeg_encoder ;

  // DCT Coefficients and dimensions
  unsigned int du_rows ;
  unsigned int du_cols ;
  JpegEncoderCoefficientBlock *dct_coefficients ;

  // EOB-run in context
  unsigned int eob_run ;
  unsigned int eob_start_du_row ;
  unsigned int eob_start_du_col ;
  unsigned int eob_start_position ;

  // Sampling Frequencies and p
  unsigned int v_frequency ;
  unsigned int h_frequency ;
  unsigned int v_period ;
  unsigned int h_period ;

  // Huffman and Quantization tables
  JpegEncoderHuffmanTable *ac_table ;
  JpegEncoderHuffmanTable *dc_table ;
  JpegEncoderQuantizationTable *quantization_table ;

  // Last DC value used to calculate DC differences
  int last_dc_value ;

  friend class JpegEncoderComponent ;
} ;

inline unsigned int JpegEncoderComponent::GetHorizontalFrequency () const
{
  return h_frequency ;
}

inline unsigned int JpegEncoderComponent::GetVerticalFrequency () const
{
  return v_frequency ;
}

inline unsigned int JpegEncoderComponent::DataUnitRows () const
{
  return du_rows ;
}

inline unsigned int JpegEncoderComponent::DataUnitCols () const
{
  return du_cols ;
}

inline void JpegEncoderComponent::ResetDcDifference ()
{
  last_dc_value = 0 ;
  return ;
}

inline void JpegEncoderComponent::SetHuffmanTables (JpegEncoderHuffmanTable &dc,
                                                    JpegEncoderHuffmanTable &ac)
{
  dc_table = &dc ;
  ac_table = &ac ;
  return ;
}


inline void JpegEncoderComponent::SetQuantizationTable (JpegEncoderQuantizationTable &table)
{
  quantization_table = &table ;
  return ;
}


#endif

