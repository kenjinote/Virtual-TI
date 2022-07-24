#ifndef __JPEGDECO_H
#define __JPEGDECO_H
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
//  Title:  JPEG Decoder Class Implementation
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//


#include <stdio.h>

#include "datatype.h"
#include "jpgexcep.h"
#include "bitimage.h"

class JpegDecoderComponent ;
class JpegHuffmanDecoder ;
class JpegDecoderQuantizationTable ;


class JpegDecoder : public BitmapImageDecoder
{
public:
  JpegDecoder () ;
  JpegDecoder (const JpegDecoder &) ;
  virtual ~JpegDecoder () ;
  JpegDecoder &operator=(const JpegDecoder &) ;

  virtual void ReadImage (FILE *istrm, BitmapImage &image) ;
  bool GetVerbose () const ;
  void SetVerbose (bool) ;

  virtual void UpdateImage () ;

protected:
  // While we don't expect classes to be derived from this one,
  // we have placed the functions we expect friend classes to
  // call here.

  UBYTE1 ReadByte () ;
  UBYTE2 ReadWord () ;
  int NextBit () ;
  int Receive (unsigned int count) ;
  bool IsProgressive () const ;
  unsigned int McuRows () const ;
  unsigned int McuCols () const ;
  unsigned int FrameHeight () const ;
  unsigned int FrameWidth () const ;
  unsigned int MaxVFrequency () const ;
  unsigned int MaxHFrequency () const ;

  unsigned int RestartInterval () const ;

private:
  void Initialize () ;
  void DoCopy (const JpegDecoder &) ;
  void CallProgressFunction (unsigned int progress) ;
  void GetScanCount () ;

  void ReadStreamHeader () ;
  void ReadMarker () ;
  void ReadApplication (UBYTE1 type) ;
  void ReadHuffmanTable () ;
  void ReadQuantization () ;
  void ReadRestartInterval () ;
  void ReadStartOfFrame (UBYTE1 type) ;
  void ReadStartOfScan () ;
  void CalculateMcuDimensions ();
  bool ScanIsInterleaved () const ;

  void ReadProgressiveScanData (unsigned int sss, unsigned int sse,
                                unsigned int sah, unsigned int sal) ;
  void ReadDcFirst (unsigned int ssa) ;
  void ReadDcRefine (unsigned int ssa) ;
  void ReadAcFirst (unsigned int sss, unsigned int sse, unsigned int ssa) ;
  void ReadAcRefine (unsigned int sss,
                     unsigned int sse,
                     unsigned int ssa) ;
  void FreeAllocatedResources () ;

  void ReadSequentialScanData () ;

  void ReadSequentialInterleavedScan () ;
  void ReadSequentialNonInterleavedScan () ;
  void ResetDcDifferences () ;
  void ProcessRestartMarker () ;

  void RefineAcCoefficient (BYTE2 &value, unsigned int ssa) ;

  // Huffman tables
  JpegHuffmanDecoder *ac_tables ;
  JpegHuffmanDecoder *dc_tables ;

  // Quantization tables
  JpegDecoderQuantizationTable *quantization_tables ;

  // Bit I/O state
  int bit_position ;        // Called CNT in Section F.2.2.5
  unsigned char bit_data ;  // Called B in Section F.2.2.5

  bool eoi_found ;
  bool sof_found ;

  bool verbose_flag ;

  FILE *input_stream ;

  unsigned int restart_interval ;

  unsigned int frame_width ;
  unsigned int frame_height ;
  unsigned int frame_type ;

  unsigned max_horizontal_frequency ;
  unsigned max_vertical_frequency ;

  bool progressive_frame ;

  unsigned int component_count ;
  JpegDecoderComponent *components ;
  unsigned int *component_indices ;

  // Address of the image that is currently being processed.
  BitmapImage *current_image ;

  // Progress Counters
  unsigned int current_scan ;
  unsigned int scan_count ;

  unsigned int mcu_rows ;
  unsigned int mcu_cols ;

  unsigned int mcu_height ;
  unsigned int mcu_width ;

  unsigned int scan_component_count ;
  JpegDecoderComponent **scan_components ;

  unsigned int expected_restart ;

  bool strict_jfif ;

  friend class JpegDecoderQuantizationTable ;
  friend class JpegHuffmanDecoder ;
  friend class JpegDecoderComponent ;
} ;


inline bool JpegDecoder::IsProgressive () const
{
  return progressive_frame ;
}

inline unsigned int JpegDecoder::McuRows () const
{
  return mcu_rows ;
}

inline unsigned int JpegDecoder::McuCols () const
{
  return mcu_cols ;
}

inline unsigned int JpegDecoder::RestartInterval () const
{
  return restart_interval ;
}

inline unsigned int JpegDecoder::FrameHeight () const
{
  return frame_height ;
}

inline unsigned int JpegDecoder::FrameWidth () const
{
  return frame_width ;
}

inline unsigned int JpegDecoder::MaxVFrequency () const
{
  return max_vertical_frequency ;
}

inline unsigned int JpegDecoder::MaxHFrequency () const
{
  return max_horizontal_frequency ;
}

inline bool JpegDecoder::ScanIsInterleaved () const
{
  if (scan_component_count != 1)
    return true ;
  else
    return false ;
}

inline bool JpegDecoder::GetVerbose () const
{
  return verbose_flag ;
}

inline void JpegDecoder::SetVerbose (bool value)
{
  verbose_flag = value ;
  return ;
}

#endif
