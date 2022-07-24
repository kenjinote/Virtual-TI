#ifndef __JPEGENCODER
#define __JPEGENCODER
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
//  JPEG Encoder Library.
//
//  Title: JpegEncoder class definition.
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Description:
//
//    This class is an encoder for JPEG image. The process for using this
//    class is to call the property functions to set the attributes for the
//    stream then call the WriteImage function to create a JPEG stream with
//    those attributes.
//

#include <stdio.h>
#include "bitimage.h"
#include "jpencomp.h"
#include "jpgexcep.h"

class JpegEncoder : public BitmapImageEncoder
{
public:
  JpegEncoder () ;
  virtual ~JpegEncoder () ;
  // Required Member Functions
  JpegEncoder (const JpegEncoder &) ;
  JpegEncoder &operator=(const JpegEncoder &) ;

  virtual void WriteImage (FILE *strm,
                           BitmapImage &image) ;

  //************************
  //** Property Functions **
  //************************

  // Image Quality (1-100)
  unsigned int GetQuality () const ;
  void SetQuality (unsigned int) ;

  // Grayscale Mode (True=Gray Scale, False=Color)
  bool GetGrayscale () const ;
  void SetGrayscale (bool) ;

  // Progressive Mode (True=Progressive, False=Sequential)
  bool GetProgressive () const ;
  void SetProgressive (bool) ;

  // Number of rows between restart markers (0=> No restart markers)
  unsigned int GetRowsPerRestart () const ;
  void SetRowsPerRestart (unsigned int) ;

  // Comment String
  char* GetComment () const ;
  void SetComment (const char*) ;

  // Component Sampling Frequencies (1-4)
  void SetSamplingFrequency (unsigned int component, unsigned int hf, unsigned int vf) ;
  void GetSamplingFrequency (unsigned int component, unsigned int &hf, unsigned int &vf) ;

  // Scan Attributes {scan number, component bit mask,
  // spectral selection end (0-63), successive approximation (0-13) }
  void SetScanAttributes (unsigned int scan, unsigned long components, unsigned int sse, unsigned int ssa) ;
  void GetScanAttributes (unsigned int scan, unsigned long &components, unsigned int &sse, unsigned int &ssa) ;

  // Maximum number of scans
  enum { MaxScans = 256, } ;

  // Component Identifiers
  enum { YComponent = 1, CbComponent = 2, CrComponent = 3, } ;

protected:
  // Output Functions used by other classes to write data to the output stream.
  void OutputBits (int bits, unsigned int count) ;
  void OutputByte (UBYTE1) ;
  void OutputWord (UBYTE2) ;

  void CallProgressFunction (unsigned int) ;

private:
  void DoCopy (const JpegEncoder &) ;
  void Initialize () ;

  // This structure is used to represent a scan.
  struct Scan
  {
    // Bitmap of components in the scan.
    unsigned long component_mask ;
    // Spectral Selection for the scan.
    unsigned int spectral_selection_start ;
    unsigned int spectral_selection_end ;
    // Successive Approximation for the first iteration of the scan
    unsigned int successive_approximation ;
    // Successive Approximation Progress. Updated as scans are output.
    int successive_approximation_high ;
    int successive_approximation_low ;
  } ;

  // This function determines if the output scan parameters are valid. It
  // throwse the EJpegError exception an inconsistency is found.
  void ValidateParameters () ;

  // Basic Output Methods
  void FlushBitBuffer () ;

  void OutputMarker (UBYTE1) ;

  // Block Output Methods
  void PrintQuantizationTables () ;
  void PrintSequentialFrame (BitmapImage &image) ;
  void PrintProgressiveFrame (BitmapImage &image) ;
  void PrintComment (const char*) ;
  void OutputJfifHeader () ;

  void OutputRestartInterval (unsigned int restartinterval) ;

  void PrintHuffmanTables (const Scan &scan, bool usedc, bool useac) ;

  // Sequential Scan Output
  void PrintSequentialScan (const Scan &scan) ;

  // Progressive Output Functions
  void PrintProgressiveScan (const Scan &scan) ;
  void PrintDcFirst (const Scan &scan) ;
  void PrintDcRefine (const Scan &scan) ;
  void PrintAcFirst (const Scan &scan) ;
  void PrintAcRefine (const Scan &scan) ;
  void FirstAcData (const Scan &scan, bool outputrestarts, JpegEncoderComponent::ACOUTPUTFUNCTION acfunction) ;
  void RefineAcData (const Scan &scan, bool outputrestarts, JpegEncoderComponent::ACOUTPUTFUNCTION acfunction) ;


  void InterleavedPass (
                      bool writedata,
                      JpegEncoderComponent::COMPONENTPASSFUNCTION passfunction,
                      JpegEncoderComponent::DCOUTPUTFUNCTION dcfunction,
                      JpegEncoderComponent::ACOUTPUTFUNCTION acfunction,
                      unsigned int sss,
                      unsigned int sse,
                      unsigned int ssa) ;
  void NoninterleavedPass (
                      bool writedata,
                      JpegEncoderComponent::COMPONENTPASSFUNCTION passfunction,
                      JpegEncoderComponent::DCOUTPUTFUNCTION dcfunction,
                      JpegEncoderComponent::ACOUTPUTFUNCTION acfunction,
                      unsigned int sss,
                      unsigned int sse,
                      unsigned int ssa) ;

  void ResetDcValues () ;
  void CalculateMcuDimensions () ;
  void CountPassesForProgressReporting () ;

  void FindComponentsInScan (Scan &scan) ;
  void CreateQuantizationTables (unsigned int quality) ;

  // Data used for bit I/O
  unsigned int bit_count ;  // Number of bits that habe been buffered
  UBYTE1 bit_buffer ;

  // Quantization Tables
  JpegEncoderQuantizationTable *chrominance_quanttbl ;  // For Y
  JpegEncoderQuantizationTable *luminance_quanttbl ;    // For Cb and Cr

  // Huffman Tables
  JpegEncoderHuffmanTable *ac_tables ;
  JpegEncoderHuffmanTable *dc_tables ;

  // Properties
  bool gray_scale ;
  unsigned int rows_per_restart ;
  unsigned int restart_interval ;
  unsigned int image_quality ;
  char* comment_string ;
  bool progressive_mode ;

  unsigned int total_passes ;
  unsigned int current_pass ;

  // Image Size
  unsigned int frame_width ;
  unsigned int frame_height ;
  // Maximum Frequencies in all components
  unsigned int max_horizontal_frequency ;
  unsigned int max_vertical_frequency ;
  // MCU Dimensions
  unsigned int  mcu_rows ;
  unsigned int  mcu_cols ;

  BitmapImage *current_image ;
  FILE *output_stream ;

  unsigned int scan_count ;

  // Scan Descriptors
  Scan image_scans [MaxScans] ;
  // Components
  enum { MaxComponents = 4, } ;
  JpegEncoderComponent image_components [MaxComponents] ;
  // Components used in the scan being processed.
  unsigned int scan_component_count ;
  JpegEncoderComponent *scan_components [JpegMaxComponentsPerScan] ;

  friend class JpegEncoderHuffmanTable ;
  friend class JpegEncoderComponent ;
} ;

//
//  The RowsPerRestart parameter specifies the number of MCU rows
//  output between restart markers. A value of zero means no restart
//  markers are used.
//
inline unsigned int JpegEncoder::GetRowsPerRestart () const
{
  return rows_per_restart ;
}

inline void JpegEncoder::SetRowsPerRestart (unsigned int rows)
{
  rows_per_restart = rows ;
  return ;
}

inline void JpegEncoder::SetGrayscale (bool value)
{
  gray_scale = value ;
  return ;
}

inline bool JpegEncoder::GetGrayscale () const
{
  return gray_scale ;
}

//
//  This function calls the progress function if one has been specified.
//
//  Parameters:
//      progress: Percent completed (0-100)
//
inline void JpegEncoder::CallProgressFunction (unsigned int progress)
{
  if (progress_function == NULL)
    return ;

  bool abort = false ;
  bool update = false;
  unsigned int percent = progress ;
  if (percent > 100)
    percent = 100 ;
  progress_function (*this,
                     progress_data,
                     current_pass,
                     total_passes,
                     percent,
                     abort) ;

  // See if the user wants to stop.
  if (abort)
    throw EJpegAbort () ;
  return ;
}


#endif

