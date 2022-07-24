#ifndef __JPDECOMP_H
#define __JPDECOMP_H
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
//  Title:  JPEG Decoder Component Class Definition
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Description:
//
//    This class represents a component within the JPEG decoder.
//


#include "jpeg.h"
#include "jfif.h"
#include "jpdedu.h"
#include "jpdehuff.h"
#include "jpdecobk.h"
#include "jpdehuff.h"
#include "bitimage.h"

class JpegDecoderComponent
{
public:
  JpegDecoderComponent () ;
  ~JpegDecoderComponent () ;

  // We have made the color conversions static because RGB
  // conversion requires the data from three components.
  // Grayscale conversion is static strictly for consistency
  // with RGB.
  static void RGBConvert (JpegDecoderComponent &c1,
                          JpegDecoderComponent &c2,
                          JpegDecoderComponent &c3,
                          BitmapImage &image) ;
  static void GrayscaleConvert (JpegDecoderComponent &cc, BitmapImage &image) ;

  unsigned int HorizontalFrequency () const ;
  void HorizontalFrequency (unsigned int) ;

  unsigned int VerticalFrequency () const ;
  void VerticalFrequency (unsigned int) ;

  void SetQuantizationTable (JpegDecoderQuantizationTable &table) ;
  void AllocateComponentBuffers (const JpegDecoder &decoder) ;
  void FreeComponentBuffers () ;
  void SetHuffmanTables (JpegHuffmanDecoder &dc, JpegHuffmanDecoder &ac) ;
  void Upsample () ;

  void CheckAcTable () ;
  void CheckDcTable () ;
  void CheckQuantizationTable () ;

  void DecodeSequential (JpegDecoder &decoder,
                         unsigned int mcurow,
                         unsigned int mcucol) ;

  unsigned long NoninterleavedRows () const ;
  unsigned long NoninterleavedCols () const ;
  void ResetDcDifference () ;

  void DecodeDcFirst (JpegDecoder &decoder,
                                    unsigned int row,
                                    unsigned int col,
                                    unsigned int ssa) ;
  void DecodeDcRefine (JpegDecoder &decoder,
                                     unsigned int row,
                                     unsigned int col,
                                     unsigned int ssa) ;

  void DecodeAcFirst (JpegDecoder &decoder,
                                    unsigned int row,
                                    unsigned int col,
                                    unsigned int sss,
                                    unsigned int sse,
                                    unsigned int ssa) ;

  void DecodeAcRefine (JpegDecoder &decoder,
                                     unsigned int row,
                                     unsigned int col,
                                     unsigned int sss,
                                     unsigned int sse,
                                     unsigned int ssa) ;

  void ProgressiveInverseDct () ;

private:
  // Dummy Declarations for Required Member Functions
  JpegDecoderComponent (const JpegDecoderComponent &) ;
  JpegDecoderComponent operator=(const JpegDecoderComponent &) ;

  // Jfif/Frame component ID
  unsigned int component_id ;

  // Sampling Frequencies
  unsigned int horizontal_frequency ;
  unsigned int vertical_frequency ;

  // These values are the numnber of samples to take for each data
  // point. They come from the sampling frequencies and the maximum
  // sampling frequencies of all the components in the image.
  // sampling frequencies of all the components in the image.
  unsigned int v_sampling ;
  unsigned int h_sampling ;

  // Last encoded DC value.
  int last_dc_value ;

  // Entropy tables used by the component.
  JpegHuffmanDecoder *ac_table ;
  JpegHuffmanDecoder *dc_table ;

  // Quantization table used by the component
  JpegDecoderQuantizationTable *quantization_table ;

  // End of band Run - Progressive Specific
  unsigned int eob_run ;

  // Non-interleaved dimensions.
  unsigned int noninterleaved_rows ;
  unsigned int noninterleaved_cols ;

  unsigned int du_rows ;
  unsigned int du_cols ;

  JpegDecoderDataUnit *data_units ;
  JPEGSAMPLE *upsample_data ;
  JpegDecoderCoefficientBlock *coefficient_blocks ;
} ;

inline unsigned int JpegDecoderComponent::HorizontalFrequency () const
{
  return horizontal_frequency ;
}

inline unsigned int JpegDecoderComponent::VerticalFrequency () const
{
  return vertical_frequency ;
}

inline unsigned long JpegDecoderComponent::NoninterleavedRows () const
{
  return noninterleaved_rows ;
}

inline unsigned long JpegDecoderComponent::NoninterleavedCols () const
{
  return noninterleaved_cols ;
}

inline void JpegDecoderComponent::ResetDcDifference ()
{
  last_dc_value = 0 ;
  return ;
}

#endif
