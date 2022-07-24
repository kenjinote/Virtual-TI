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
// Title:   JpegEncoder Class Implementation
//
// Author: John M. Miano  miano@colosseumbuilders.com
//
//

#include <math.h>
#include <string.h>
#include "jpegenco.h"
#include "bitimage.h"
#include "jfif.h"
#include "jpenquan.h"
#include "jpenhuff.h"         


//
// These are sample quantization tables defined in the the JPEG Standard.
//

// From Section K.1 Table K-1
static const unsigned int default_luminance_table [JpegSampleSize]  =
{
16, 11, 10, 16,  24,  40,  51,  61,
12, 12, 14, 19,  26,  58,  60,  55,
14, 13, 16, 24,  40,  57,  69,  56,
14, 17, 22, 29,  51,  87,  80,  62,
18, 22, 37, 56,  68, 109, 103,  77,
24, 35, 55, 64,  81, 104, 113,  92,
49, 64, 78, 87, 103, 121, 120, 101,
72, 92, 95, 98, 112, 100, 103,  99,
} ;

// From Section K.1 Table K-2
static const unsigned int default_chrominance_table [JpegSampleSize] =
{
17, 18, 24, 47, 99, 99, 99, 99,
18, 21, 26, 66, 99, 99, 99, 99,
24, 26, 56, 99, 99, 99, 99, 99,
47, 66, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99,
} ;

//
//  Description:
//
//    Class Default Constructor
//
JpegEncoder::JpegEncoder ()
{
  Initialize () ;
  return ;
}

//
//  Description:
//
//    Class Copy Constructor
//
JpegEncoder::JpegEncoder (const JpegEncoder &source)
{
  Initialize () ;
  DoCopy (source) ;
  return ;
}

//
//  Description:
//
//    Class Destructor
//
JpegEncoder::~JpegEncoder ()
{
  delete [] ac_tables ; ac_tables = NULL ;
  delete [] dc_tables ; dc_tables = NULL ;
  delete chrominance_quanttbl ; chrominance_quanttbl = NULL ;
  delete luminance_quanttbl ; luminance_quanttbl = NULL ;
  return ;
}
//
//  Description:
//
//    Class assignment operator.
//
JpegEncoder &JpegEncoder::operator=(const JpegEncoder &source)
{
  DoCopy (source) ;
  return *this ;
}
//
//  Description:
//
//    Class copy function. This function gets called from the
//    copy constructor and assignment operator.  The only thing
//    we copy are the use settable parameters.
//
void JpegEncoder::DoCopy (const JpegEncoder &source)
{
  gray_scale = source.gray_scale ;
  rows_per_restart = source.rows_per_restart ;
  comment_string = source.comment_string ;
  image_quality = source.image_quality ;
  progressive_mode = source.progressive_mode ;

  for (unsigned int ii = 0 ; ii < MaxScans ; ++ ii)
    image_scans [ii] = source.image_scans [ii] ;

  BitmapImageCoder::operator=(source) ;
  return ;
}
//
//  Description:
//
//    This function writes an image to the output stream.
//
//  Parameters:
//    strm:   The output stream to write the image to. This most be opened in
//            binary mode
//    image:  The image to output.
//
void JpegEncoder::WriteImage (FILE *strm, BitmapImage &image)
{
  bit_count = 0 ;
  current_image = &image ;

  output_stream = strm ;

  CreateQuantizationTables (image_quality) ;

  frame_height = image.Height () ;
  frame_width = image.Width () ;

  // Find the MCU size and maximum sampling frequencies.
  CalculateMcuDimensions () ;

  // Validate Parameters. If there is an error this function will throw
  // an exception.
  ValidateParameters () ;

  CountPassesForProgressReporting () ;

  // Write the image header.
  OutputMarker (SOI) ;
  OutputMarker (APP0) ;
  OutputJfifHeader () ;

  if (comment_string)
    PrintComment (comment_string) ;

  PrintComment ("Created using the Colosseum Builders JPEG library") ;

  if (progressive_mode)
    PrintProgressiveFrame (image) ;
  else
    PrintSequentialFrame (image) ;

  OutputMarker (EOI) ;

  // Make sure that we are not holding on to any memory we do not
  // need any more.
  image_components [YComponent].FreeDynamicStorage () ;
  image_components [CbComponent].FreeDynamicStorage () ;
  image_components [CrComponent].FreeDynamicStorage () ;

  return ;
}
//
//  Description:
//
//    Class Initialization function. This function is intended to be
//    called from constructors.
//
void JpegEncoder::Initialize ()
{
  memset (image_scans, 0, sizeof (image_scans)) ;
  image_scans [0].component_mask = (1<<YComponent)
                                 |(1<<CbComponent)
                                 |(1<<CrComponent) ;
  progressive_mode = false ;
  image_quality = 75 ;
  progress_function = NULL ;
  progress_data = NULL ;
  restart_interval = 0 ;
  rows_per_restart = 4 ;
  comment_string=NULL;
  for (unsigned int ii = 0 ; ii < MaxComponents ; ++ ii)
  {
    image_components [ii].SetHorizontalFrequency (1) ;
    image_components [ii].SetVerticalFrequency (1) ;
  }

  ac_tables = new JpegEncoderHuffmanTable [2] ;
  dc_tables = new JpegEncoderHuffmanTable [2] ;
  chrominance_quanttbl = new JpegEncoderQuantizationTable ;
  luminance_quanttbl = new JpegEncoderQuantizationTable ;

  image_components [YComponent].SetHuffmanTables (dc_tables [0],
                                                  ac_tables [0]) ;
  image_components [CbComponent].SetHuffmanTables (dc_tables [1],
                                                   ac_tables [1]) ;
  image_components [CrComponent].SetHuffmanTables (dc_tables [1],
                                                   ac_tables [1]) ;

  image_components [YComponent].SetQuantizationTable (*luminance_quanttbl) ;
  image_components [CbComponent].SetQuantizationTable (
                                          *chrominance_quanttbl) ;
  image_components [CrComponent].SetQuantizationTable (
                                          *chrominance_quanttbl) ;

  return ;
}


//
//  Description:
//
//    This function creates the quantization tables for writing the image.
//
//    Rather than have the user specify all 128 quantization table
//    values we have the user specify a quality vale in the range
//    1..100 then scale the sample quantization tables in the JPEG
//    specification. A quality value of 50 uses the unmodified tables.
//    At a quality value of 100 no quantization takes place (all 1s).
//    A quality value of 1 will create a solid black image. Quality
//    values below around 25 are pretty useless.
//
//  Parameters:
//    quality:  The image quality (1-100)
//

void JpegEncoder::CreateQuantizationTables (unsigned int quality)
{
  // We need this declaration because MSVC++ does not support
  // standard scoping in for statements.
  unsigned int ii ;

  if (quality < 1 || quality > 100)
    throw EJpegValueOutOfRange () ;

  // Scale Factor
  double scale = exp ((6 * (50 - (int) quality)/50.0) * log (2.0)) ;

  for (ii = 0 ; ii < JpegSampleSize ; ++ ii)
  {
    unsigned int value =
         default_luminance_table [JpegZigZagInputOrder (ii)]
         * scale ;
    if (value < JpegMinQuantizationValue)
      (*luminance_quanttbl) [ii] = JpegMinQuantizationValue ;
    else if (value > JpegMax8BitQuantizationValue)
      (*luminance_quanttbl) [ii] = JpegMax8BitQuantizationValue ;
    else
      (*luminance_quanttbl) [ii] = value ;
  }

  for (ii = 0 ; ii < JpegSampleSize ; ++ ii)
  {
    unsigned int value =
        default_chrominance_table [JpegZigZagInputOrder (ii)]
        * scale ;
    if (value < 1)
      (*chrominance_quanttbl) [ii] = JpegMinQuantizationValue ;
    else if (value > JpegMax8BitQuantizationValue)
      (*chrominance_quanttbl) [ii] = JpegMax8BitQuantizationValue ;
    else
      (*chrominance_quanttbl) [ii] = value ;
  }

  // Here we generate the scaled tables used for the fast FDCT.
  chrominance_quanttbl->BuildScaledTables () ;
  luminance_quanttbl->BuildScaledTables () ;
  return ;

}

//
//  Description:
//
//    This function writes a single byte to the output stream.
//
//  Parameters:
//    value: The byte to be written to the output stream.
//
void JpegEncoder::OutputByte (UBYTE1 value)
{
  if (bit_count != 0)
    FlushBitBuffer () ;

  fwrite ((char *) &value, sizeof (value),1,output_stream) ;
  return ;
}

//
//  Description:
//
//    This function writes a 2-byte integer in system format to the output
//    stream in bit-endian order (most significant byte first).
//
//  Parameters:
//    value: The 2-byte data to be written to the output stream
//
void JpegEncoder::OutputWord (UBYTE2 value)
{
  if (bit_count != 0)
    FlushBitBuffer () ;

  UBYTE2 data = SystemToBigEndian (value) ;
  fwrite ((char *) &data, sizeof (data),1,output_stream) ;
  return ;
}

//
//  Description:
//
//    This function outputs a single bit to the output stream.
//
//    Individual its are buffered using the bit_buffer and bit_count member
//    variables.
//
//    This function throws an exception if it is called with a zero bit count.
//    We use this for error trapping since no place in the rest of the
//    code should attempt to write zero bits.
//
//  Parameters:
//    bits: The bit string to be output
//    count: The number of bits to output
//
void JpegEncoder::OutputBits (int bits, unsigned int count)
{
  if (count == 0)
    throw EJpegFatal ("Internal Error - Invalid Bit Count") ;

  for (unsigned int ii = 0 ; ii < count ; ++ ii)
  {
    bit_buffer <<= 1 ;
    ++ bit_count ;
    bit_buffer |= ((bits >> (count - ii - 1)) & 0x1) ;
    if (bit_count == 8)
    {
      fwrite ((char *) &bit_buffer, 1,1,output_stream) ;
      // The Sequence FF00 in the files is used to represent the value FF
      // in the stream to differentiate it from a marker.
      if (bit_buffer == SOB)
      {
        bit_buffer = 0 ;
        fwrite ((char *) &bit_buffer, 1,1,output_stream) ;
      }
      bit_count = 0 ;
    }
  }
  return ;
}

//
//  Description:
//
//    This function flushes the bit buffer. This causes buffered bit streams
//    of less than the bit buffer size to be written out.
//
void JpegEncoder::FlushBitBuffer ()
{
  if (bit_count != 0)
  {
    bit_buffer <<= (8 - bit_count) ;
    fwrite ((char *) &bit_buffer, 1,1,output_stream) ;
    bit_count = 0 ;
  }
  return ;
}

//
//  This function writes a marker to the output stream.
//
//  Parameters:
//    marker: The marker to be written to the output stream
//
void JpegEncoder::OutputMarker (UBYTE1 marker)
{
  OutputByte (SOB) ;
  OutputByte (marker) ;
  return ;
}

//
//  Description:
//
//    This function validates the user-set output parameters. If an error is
//    detected an EJpegBadOutputParameter exception is thrown.
//

void JpegEncoder::ValidateParameters ()
{
  // Ensure we do not have fractional sampling of pixels.
  if (! gray_scale)
  {
    if (image_components [YComponent].GetHorizontalFrequency () == 3
        || image_components [CbComponent].GetHorizontalFrequency () == 3
        || image_components [CrComponent].GetHorizontalFrequency () == 3)
    {
      if (image_components [YComponent].GetHorizontalFrequency () == 2
          || image_components [YComponent].GetHorizontalFrequency () == 4
          || image_components [CbComponent].GetHorizontalFrequency () == 2
          || image_components [CbComponent].GetHorizontalFrequency () == 4
          || image_components [CrComponent].GetHorizontalFrequency () == 2
          || image_components [CrComponent].GetHorizontalFrequency () == 4)
      {
        throw EJpegError ("Fractional Horizontal Sampling") ;
      }
    }

    if (image_components [YComponent].GetVerticalFrequency () == 3
        || image_components [CbComponent].GetVerticalFrequency () == 3
        || image_components [CrComponent].GetVerticalFrequency () == 3)
    {
      if (image_components [YComponent].GetVerticalFrequency () == 2
          || image_components [YComponent].GetVerticalFrequency () == 4
          || image_components [CbComponent].GetVerticalFrequency () == 2
          || image_components [CbComponent].GetVerticalFrequency () == 4
          || image_components [CrComponent].GetVerticalFrequency () == 2
          || image_components [CrComponent].GetVerticalFrequency () == 4)
      {
        throw EJpegError ("Fractional Vertical Sampling") ;
      }
    }
  }

  if (progressive_mode)
  {
    // For a progressive scan the following rules apply
    //
    // o The spectral selection start can be zero if and only if
    // the spectral selection end is zero.
    //
    // o For each component the zero spectral selection start must occur
    // before any other.
    //
    // o If the spectral selection start is not zero there may only be
    // one component in a scan.
    //
    // o The entire spectral range must be specified for each component.
    //
    // o There can be no overlapping spectral ranges in scans.
    //
    int lasty = -1 ;
    int lastcb = -1 ;
    int lastcr = -1 ;

    if (gray_scale)
    {
      // For a grayscale image the Cb and Cr components are not used.
      lastcb = 63 ;
      lastcr = 63 ;
    }

    for (scan_count = 0 ; scan_count < MaxScans ; ++ scan_count)
    {
      if (lasty == 63 && lastcb == 63 && lastcr == 63)
        break ;

      if (image_scans [scan_count].component_mask == 0)
        throw EJpegError ("Scan contains no components") ;

      if (image_scans [scan_count].successive_approximation
          > JpegMaxSuccessiveApproximation)
      {
        throw EJpegError ("Successive Approximation too large") ;
      }
        
      image_scans [scan_count].successive_approximation_low =
        image_scans [scan_count].successive_approximation ;
      image_scans [scan_count].successive_approximation_high = 0 ;

      // Y Component Validation
      if ((image_scans [scan_count].component_mask & (1<<YComponent)) != 0)
      {
        if (image_scans [scan_count].spectral_selection_end == 0)
        {
          if (lasty != -1)
            throw EJpegError ("Duplicate Y Component Scan") ;

          image_scans [scan_count].spectral_selection_start = 0 ;
          lasty = 0 ;
        }
        else
        {
          if (image_scans [scan_count].component_mask != (1<<YComponent))
            throw EJpegError ("Multiple Components in AC Scan") ;

          if (image_scans [scan_count].spectral_selection_end != 0
              && lasty == -1)
          {
            throw EJpegError (
                     "AC Scan specified before DC scan for Y Component") ;
          }

          if (image_scans [scan_count].spectral_selection_end <= lasty)
            throw EJpegError ("Duplicate or overlapping spectral selection for Y Component") ;

          image_scans [scan_count].spectral_selection_start = lasty + 1 ;
          lasty = image_scans [scan_count].spectral_selection_end ;
        }
      }

      if (! gray_scale)
      {
        // Cb Component Validation
        if ((image_scans [scan_count].component_mask & (1<<CbComponent)) != 0)
        {
          if (image_scans [scan_count].spectral_selection_end == 0)
          {
            if (lastcb != -1)
              throw EJpegError ("Duplicate Cb Component Scan") ;

            image_scans [scan_count].spectral_selection_start = 0 ;
            lastcb = 0 ;
          }
          else
          {
            if (image_scans [scan_count].component_mask != (1<<CbComponent))
              throw EJpegError ("Multiple Components in AC Scan") ;

            if (image_scans [scan_count].spectral_selection_end != 0 && lastcb == -1)
              throw EJpegError ("AC Scan specified before DC scan for cb Component") ;

            if (image_scans [scan_count].spectral_selection_end <= lastcb)
            {
              throw EJpegError (
              "Duplicate or overlapping spectral selection for Cb Component") ;
            }

            image_scans [scan_count].spectral_selection_start = lastcb + 1 ;
            lastcb = image_scans [scan_count].spectral_selection_end ;
          }
        }

        // Cr Component Validation
        if ((image_scans [scan_count].component_mask & (1<<CrComponent)) != 0)
        {
          if (image_scans [scan_count].spectral_selection_end == 0)
          {
            if (lastcr != -1)
              throw EJpegError ("Duplicate Cr Component Scan") ;

            image_scans [scan_count].spectral_selection_start = 0 ;
            lastcr = 0 ;
          }
          else
          {
            if (image_scans [scan_count].component_mask != (1<<CrComponent))
              throw EJpegError ("Multiple Components in AC Scan") ;

            if (image_scans [scan_count].spectral_selection_end != 0
                && lastcr== -1)
            {
              throw EJpegError (
                      "AC Scan specified before DC scan for Cr Component") ;
            }

            if (image_scans [scan_count].spectral_selection_end <= lastcr)
            {
              throw EJpegError (
             "Duplicate or overlapping spectral selection for Cr Component") ;
            }
            image_scans [scan_count].spectral_selection_start = lastcr + 1 ;
            lastcr = image_scans [scan_count].spectral_selection_end ;
          }
        }
      }
    }
    if (lasty != 63)
      throw EJpegError ("Y Component not completely defined by scans") ;
    if (! gray_scale)
    {
      if (lastcb != 63)
        throw EJpegError ("Cb Component not completely defined by scans") ;
      if (lastcr != 63)
        throw EJpegError ("Cr Component not completely defined by scans") ;
    }
  }
  else
  {
    if (! gray_scale)
    {
      if ((image_scans [0].component_mask
          |image_scans [1].component_mask|image_scans [2].component_mask)
          != ((1<<YComponent)|(1<<CbComponent)|(1<<CrComponent)))
      {
        throw EJpegError ("Not all components specified in scans") ;
      }
      if ((image_scans [0].component_mask
           +image_scans [1].component_mask
           +image_scans [2].component_mask)
          != ((1<<YComponent)|(1<<CbComponent)|(1<<CrComponent)))
      {
        throw EJpegError ("Component in more than one scan") ;
      }

      if (image_scans [2].component_mask != 0)
        scan_count = 3 ;
      else if (image_scans [1].component_mask != 0)
        scan_count = 2 ;
      else
        scan_count = 1 ;
    }
    else
    {
      scan_count = 1 ;
    }
  }

  // Enforce the MCU size limitation in Section B.2.3
  //
  // I am not certain why this limitation of 10 data units per MCU even
  // exists. It seems to be an silly arbitrary limit. Maybe its to set
  // a maximum upper size for an MCU decoding buffer. In any event we
  // must abide by it.
  if (! gray_scale)
  {
    for (unsigned int ii = 0 ; ii < scan_count ; ++ ii)
    {
      unsigned int dataunits = 0 ;
      unsigned int componentcount = 0 ;
      if ((image_scans [ii].component_mask & (1<<YComponent)) != 0)
      {
        dataunits += image_components [YComponent].GetHorizontalFrequency ()
                     * image_components [YComponent].GetVerticalFrequency () ;
        ++ componentcount ;
      }
      if ((image_scans [ii].component_mask & (1<<CbComponent)) != 0)
      {
        dataunits += image_components [CbComponent].GetHorizontalFrequency ()
                  * image_components [CbComponent].GetVerticalFrequency () ;
        ++ componentcount ;
      }
      if ((image_scans [ii].component_mask & (1<<CrComponent)) != 0)
      {
        dataunits += image_components [CrComponent].GetHorizontalFrequency ()
                  * image_components [CrComponent].GetVerticalFrequency () ;
        ++ componentcount ;
      }
      if (dataunits > JpegMaxDataUnitsPerMCU && componentcount > 1)
        throw EJpegBadOutputParameter ("Data units in MCU exceeds 10") ;
    }
  }
  return ;
}
//
//  Description:
//
//    This function writes a comment marker to the output stream.
//
//  Parameters:
//    str:  The comment string
//
void JpegEncoder::PrintComment (const char* str)
{
  unsigned int length = sizeof (UBYTE2) + strlen(str) + 1 ;
  OutputMarker (COM) ;
  OutputWord (length) ;
  for (unsigned int ii = 0 ; ii < strlen(str) ; ++ ii)
    OutputByte (str[ii]) ;

  OutputByte (0) ;
  return ;
}


//
//  Description:
//
//    This function writes the quantization tables to the output stream.
//
void JpegEncoder::PrintQuantizationTables ()
{
  // We need this declaration here because MSVC++ does not conform
  // to standard scoping in for statements.
  unsigned int ii ;

  const int precision = 0 ; // Byte Precision
  int index ;
  UBYTE1 pqtq ;

  // Section B.2.4.1

  if (gray_scale)
  {
    OutputMarker (DQT) ;
    OutputWord (sizeof(UBYTE2) + (sizeof (pqtq) + JpegSampleSize)) ;

    index = 0 ;
    pqtq = (precision << 4) | index ;
    OutputByte (pqtq) ;
    for (ii = 0 ; ii < JpegSampleSize ; ++ ii)
      OutputByte ((*luminance_quanttbl) [ii]) ;
  }
  else
  {
    OutputMarker (DQT) ;
    OutputWord (sizeof(UBYTE2) + 2 * (sizeof (pqtq) + JpegSampleSize)) ;

    index = 0 ;
    pqtq = (precision << 4) | index ;
    OutputByte (pqtq) ;
    for (ii = 0 ; ii < JpegSampleSize ; ++ ii)
      OutputByte ((*luminance_quanttbl) [ii]) ;

    index = 1 ;
    pqtq = (precision << 4) | index ;
    OutputByte (pqtq) ;
    for (ii = 0 ; ii < JpegSampleSize ; ++ ii)
      OutputByte ((*chrominance_quanttbl) [ii]) ;
  }
  return ;
}

//
//  Description:
//
//    This function is called by a scan to write a restart interval to the
//    output stream. We do not output a Define Restart Interval marker if
//    the restart interval is not changing.
//
//  Parameters:
//    restartinterval: The new restart interval
//
void JpegEncoder::OutputRestartInterval (unsigned int restartinterval)
{
  if (restartinterval == restart_interval)
    return ;
  restart_interval =  restartinterval ;

  // B.2.4.4
  OutputMarker (DRI) ;
  OutputWord (4) ;
  OutputWord (restartinterval) ;
  return ;
}


//
//  Description:
//
//    This function writes a sequential frame to the output stream. We create
//    frames with either one (black & white) or three (color) components.
//
//  Parameters:
//    image: The image to be written in the frame
//
void JpegEncoder::PrintSequentialFrame (BitmapImage &image)
{
  // Sample the components
  if (gray_scale)
  {
    ++ current_pass ;
    image_components [YComponent].SampleYComponent (
                                     *this,
                                     image,
                                     max_horizontal_frequency,
                                     max_vertical_frequency) ;
  }
  else
  {
    ++ current_pass ;
    image_components [YComponent].SampleYComponent (*this,
                                                    image,
                                                    max_horizontal_frequency,
                                                    max_vertical_frequency) ;
    ++ current_pass ;
    image_components [CbComponent].SampleCbComponent (
                                           *this,
                                           image,
                                           max_horizontal_frequency,
                                           max_vertical_frequency) ;
    ++ current_pass ;
    image_components [CrComponent].SampleCrComponent (
                                           *this,
                                           image,
                                           max_horizontal_frequency,
                                           max_vertical_frequency) ;
  }

  // Write the Frame Header
  // Section B.2.2

  OutputMarker (SOF0) ;
  if (gray_scale)  // Length
    OutputWord (8 + 3) ;
  else
    OutputWord (8 + 3 * 3) ;

  OutputByte (8) ; // Precision
  OutputWord (image.Height ()) ;
  OutputWord (image.Width ()) ;

  if (gray_scale)
  {
    OutputByte (1) ;  // Component Count
    OutputByte(YComponent) ;
    OutputByte (
        (image_components [YComponent].GetHorizontalFrequency () << 4)
        | image_components [YComponent].GetVerticalFrequency ()) ;
    OutputByte (0) ;  // Quantization Table


    PrintQuantizationTables () ;
    scan_component_count = 1 ;
    scan_components [0] = &image_components [YComponent] ;
    ++ current_pass ;
    PrintSequentialScan (image_scans [0]) ;
  }
  else
  {
    OutputByte (3) ;  // Component Count
    OutputByte(YComponent) ;
    OutputByte (
        (image_components [YComponent].GetHorizontalFrequency () << 4)
        | image_components [YComponent].GetVerticalFrequency ()) ;
    OutputByte (0) ;  // Quantization Table

    OutputByte(CbComponent) ;
    OutputByte (
        (image_components [CbComponent].GetHorizontalFrequency () << 4)
        | image_components [CbComponent].GetVerticalFrequency ()) ;
    OutputByte (1) ;  // Quantization Table

    OutputByte(CrComponent) ;
    OutputByte (
        (image_components [CrComponent].GetHorizontalFrequency () << 4)
        | image_components [CrComponent].GetVerticalFrequency ()) ;
    OutputByte (1) ;  // Quantization Table

    PrintQuantizationTables () ;

    // Print the scans that make up the frame.
    for (unsigned int ii = 0 ; ii < scan_count ; ++ ii)
    {
      ++ current_pass ;
      FindComponentsInScan (image_scans [ii]) ;
      PrintSequentialScan (image_scans [ii]) ;
    }
  }

  return ;
}

//
//  Description:
//
//    This function writes a progressive frame to the output stream.
//
//  Parameters:
//    image: The image to be written in the frame
//
void JpegEncoder::PrintProgressiveFrame (BitmapImage &image)
{
  if (gray_scale)
  {
    ++ current_pass ;
    image_components [YComponent].SampleYComponent (*this,
                                                    image,
                                                    max_horizontal_frequency,
                                                    max_vertical_frequency) ;

    // Output JPEG SOF Marker
    // Section B.2.2
    OutputMarker (SOF2) ;
    OutputWord (8 + 3) ;

    OutputByte (8) ; // Precision
    OutputWord (image.Height ()) ;
    OutputWord (image.Width ()) ;

    OutputByte (1) ;  // Component Count
    OutputByte(YComponent) ;
    OutputByte (
        (image_components [YComponent].GetHorizontalFrequency () << 4)
        | image_components [YComponent].GetVerticalFrequency ()) ;
    OutputByte (0) ;  // Quantization Table

    PrintQuantizationTables () ;

    scan_component_count = 1 ;
    scan_components [0] = &image_components [YComponent] ;

    bool doingwork ;
    do
    {
      doingwork = false ;
      for (unsigned int ii = 0 ; ii < scan_count ; ++ ii)
      {
        if ((image_scans [ii].component_mask & (1<<YComponent)) != 0
             && image_scans [ii].successive_approximation_low >= 0)
        {
          ++ current_pass ;
          PrintProgressiveScan (image_scans [ii]) ;
          doingwork = true ;
          image_scans [ii].successive_approximation_high
            = image_scans [ii].successive_approximation_low ;
          -- image_scans [ii].successive_approximation_low ;
        }
      }
    } while (doingwork) ;
  }
  else
  {
    ++ current_pass ;
    image_components [YComponent].SampleYComponent (*this,
                                                    image,
                                                    max_horizontal_frequency,
                                                    max_vertical_frequency) ;
    ++ current_pass ;
    image_components [CbComponent].SampleCbComponent (
                                        *this,
                                        image,
                                        max_horizontal_frequency,
                                        max_vertical_frequency) ;
    ++ current_pass ;
    image_components [CrComponent].SampleCrComponent (
                                        *this,
                                        image,
                                        max_horizontal_frequency,
                                        max_vertical_frequency) ;

    // Output JPEG SOS Marker
    // Section B.2.2
    OutputMarker (SOF2) ;
    OutputWord (8 + 3 * 3) ;

    OutputByte (8) ; // Precision
    OutputWord (image.Height ()) ;
    OutputWord (image.Width ()) ;

    OutputByte (3) ;  // Component Count
    OutputByte(YComponent) ;
    OutputByte (
        (image_components [YComponent].GetHorizontalFrequency () << 4)
        | image_components [YComponent].GetVerticalFrequency ()) ;
    OutputByte (0) ;  // Quantization Table

    OutputByte(CbComponent) ;
    OutputByte (
        (image_components [CbComponent].GetHorizontalFrequency () << 4)
         | image_components [CbComponent].GetVerticalFrequency ()) ;
    OutputByte (1) ;  // Quantization Table

    OutputByte(CrComponent) ;
    OutputByte (
        (image_components [CrComponent].GetHorizontalFrequency () << 4)
         | image_components [CrComponent].GetVerticalFrequency ()) ;
    OutputByte (1) ;  // Quantization Table

    PrintQuantizationTables () ;

    // Because of successive approximation we may have to go through the
    // scan list several times. This flag gets set each time a scan
    // is processed. The first time we search the scan list and do
    // nothing this flag will be false and we are all done.
    bool doingwork ;
    do
    {
      doingwork = false ;
      for (unsigned int ii = 0 ; ii < scan_count ; ++ ii)
      {
        FindComponentsInScan (image_scans [ii]) ;

        if (scan_component_count != 0
            && image_scans [ii].successive_approximation_low >= 0)
        {
          ++ current_pass ;
          PrintProgressiveScan (image_scans [ii]) ;
          doingwork = true ;
          image_scans [ii].successive_approximation_high
            = image_scans [ii].successive_approximation_low ;
          -- image_scans [ii].successive_approximation_low ;
        }
      }
    } while (doingwork) ;

  }
  return ;
}


//
//  Description:
//
//    This function enables or disables progressive output. When the mode is
//    changed the image_scans is initialized with a default set of values for
//    the new mode.
//
//  Parameters:
//    value: (true=>Progressive Mode, false=>Sequential Mode)
//
void JpegEncoder::SetProgressive (bool value)
{
  if (value == progressive_mode)
    return ;

  progressive_mode = value ;
  memset (image_scans, 0, sizeof (image_scans)) ;
  if (progressive_mode)
  {
    // For Progressive Scans our default is four scans.  The first
    // contains the DC coefficients for all three components. The next
    // three scans contain the AC coefficients for each component.

    image_scans [0].component_mask = (1<<YComponent)
                                    |(1<<CbComponent)
                                    |(1<<CrComponent) ;
    image_scans [1].component_mask = (1<<YComponent) ;
    image_scans [1].spectral_selection_end = 63 ;
    image_scans [2].component_mask = (1<<CbComponent) ;
    image_scans [2].spectral_selection_end = 63 ;
    image_scans [3].component_mask = (1<<CrComponent) ;
    image_scans [3].spectral_selection_end = 63 ;
  }
  else
  {
    // For sequential mode the default is to create one scan with
    // all components.
    image_scans [0].component_mask = (1<<YComponent)
                                   |(1<<CbComponent)
                                   |(1<<CrComponent) ;
  }
  return ;
}

//
//  Description:
//
//    This function writes a sequential scan to the output stream.
//
//  Parameters:
//    scan:   The scan descriptor
//
void JpegEncoder::PrintSequentialScan (const Scan &scan)
{
  if (scan_component_count == 0)
    throw EJpegFatal ("INTERNAL ERROR - No components defined for scan") ;

  // Output the restart interval and the Huffman tables. We must set the
  // restart interval *BEFORE* gathering statistics.
  if (scan_component_count != 1)
  {
    OutputRestartInterval (rows_per_restart * mcu_cols) ;
  }
  else
  {
    OutputRestartInterval (rows_per_restart
      * scan_components [0]->DataUnitCols ()) ;
  }

  // Gather Huffman Statistics
  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    dc_tables [0].Reset () ;
    ac_tables [0].Reset () ;
  }
  if ((scan.component_mask & (1<<YComponent)) == 0
      || scan_component_count != 1)
  {
    ac_tables [1].Reset () ;
    dc_tables [1].Reset () ;
  }


  if (scan_component_count != 1)
  {
    InterleavedPass (false,
          JpegEncoderComponent::EncodeSequential,
          JpegEncoderComponent::GatherDcData,
          JpegEncoderComponent::GatherAcData,
          0, JpegSampleSize - 1, 0) ;
  }
  else
  {
    NoninterleavedPass (false,
          JpegEncoderComponent::EncodeSequential,
          JpegEncoderComponent::GatherDcData,
          JpegEncoderComponent::GatherAcData,
          0, JpegSampleSize - 1, 0) ;
  }

  // Create the Huffman Tables
  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    dc_tables [0].BuildTable () ;
    ac_tables [0].BuildTable () ;
  }
  if ((scan.component_mask & (1<<YComponent)) == 0
       || scan_component_count != 1)
  {
    ac_tables [1].BuildTable () ;
    dc_tables [1].BuildTable () ;
  }

  // Output the Huffman tables
  PrintHuffmanTables (scan, true, true) ; // Output DC and AC tables.

  // Create the scan marker.
  // Section B.2.3
  OutputMarker (SOS) ;
  OutputWord (6 + 2 * scan_component_count) ;  // Length
  OutputByte (scan_component_count) ;  // Component Count

  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    OutputByte (0x01) ; // YComponent
    OutputByte (0x00) ; // Entropy Tables
  }
  if (! gray_scale)
  {
    if ((scan.component_mask & (1<<CbComponent)) != 0)
    {
      OutputByte (0x02) ; // CbComponent
      OutputByte (0x11) ; // Entropy Tables
    }
    if ((scan.component_mask & (1<<CrComponent)) != 0)
    {
      OutputByte (0x03) ; // CrComponent
      OutputByte (0x11) ; // Entropy Tables
    }
  }

  OutputByte (0) ; // Spectral Selection Start
  OutputByte (JpegSampleSize - 1) ; // Spectral Selection End
  OutputByte (0) ; // Successive Approximation

  // Output the Scan data.
  if (scan_component_count != 1)
  {
    InterleavedPass (true,
          JpegEncoderComponent::EncodeSequential,
          JpegEncoderComponent::PrintDcData,
          JpegEncoderComponent::PrintAcData,
          0, JpegSampleSize - 1, 0) ;
  }
  else
  {
    NoninterleavedPass (true,
          JpegEncoderComponent::EncodeSequential,
          JpegEncoderComponent::PrintDcData,
          JpegEncoderComponent::PrintAcData,
          0, JpegSampleSize - 1, 0) ;
  }

  return ;
}

//
//  Description:
//
//    This function makes a pass through the data units for a non-interleaved
//    Scan.
//
//    The reason for having a generic function that uses pointers to member
//    functions to do processing is that there are several places where the
//    control processing has to be identical in multiple passes where the
//    low level actions are different. For example, the processing here when
//    gathering Huffman statistics has to be identical to the processing when
//    writing values to the output file. Using separate functions turned out
//    to be error prone due to the difficulty of keeping multiple functions
//    exactly in synchronization.
//
//  Parameters:
//    writerestarts: false => This is a statistics gathering pass
//                   true => This pass is writing data to the output file.
//    passfunctions: Pointer to EncoderComponent member function used
//                   to process this pass.
//    dcfunction:    Pointer to the EncoderComponent member function used
//                   to process DC coefficient values.
//    acfunction:    Pointer to the EncoderComponent member function used
//                   to process AC coefficient values.
//    sss:           Spectral Selection Start (0-63)
//    sse:           Spectral Selection End (0-63)
//    ssa:           Successive Approximation (0-13)
//
void JpegEncoder::NoninterleavedPass (
                    bool writedata,
                    JpegEncoderComponent::COMPONENTPASSFUNCTION passfunction,
                    JpegEncoderComponent::DCOUTPUTFUNCTION dcfunction,
                    JpegEncoderComponent::ACOUTPUTFUNCTION acfunction,
                    unsigned int sss,
                    unsigned int sse,
                    unsigned int ssa)
{
  // We use a scale integer to keep trake of progress to lessen the
  // change that the progress increment will be zero.
  const int progressscale = 8 ;
  unsigned int progress ;
  unsigned int progressincrement ;
  if (writedata)
    progress = 50 << progressscale ;
  else
    progress = 0 ;
  progressincrement = (100 << progressscale)
                    / (2 * scan_components [0]->DataUnitRows ()) ;

  ResetDcValues () ;

  unsigned int intervalcount = 0 ;
  unsigned int restartmarker = 0 ;
  for (unsigned int row = 0 ;
       row < scan_components [0]->DataUnitRows () ;
       ++ row)
  {
    for (unsigned int col = 0 ;
         col < scan_components [0]->DataUnitCols () ;
         ++ col, ++ intervalcount)
    {
      if (restart_interval != 0)
      {
        if (intervalcount == restart_interval && restart_interval != 0)
        {
          // If there are any outstanding EOB runs we flush them before the
          // restart marker. This is not explicitly stated in the JPEG
          // standard. The definition of "Restart Interval" in section 4.1
          // states that restart markers separate independent sequences,
          // something we would not have if we did not output the EOB run
          // here.
          scan_components [0]->PrintEobRun (acfunction) ;
          // E.1.4
          if (writedata)
            OutputMarker (RST0 | restartmarker) ;
          ResetDcValues () ;
          ++ restartmarker ;
          restartmarker %= 8 ;
          intervalcount = 0 ;
        }
      }
      (scan_components [0]->*passfunction) (
                                row,
                                col,
                                dcfunction,
                                acfunction,
                                sss, sse,
                                ssa) ;

    }
    progress += progressincrement ;
    CallProgressFunction (progress >> progressscale) ;
  }
  CallProgressFunction (100) ;
  return ;
}

//
//  Description:
//
//    This function makes a pass through the data units for an interleaved
//    Scan.
//
//    The reason for having a generic function that uses pointers to member
//    functions to do processing is that there are several places where the
//    control processing has to be identical in multiple passes where the
//    low level actions are different. For example, the processing here when
//    gathering Huffman statistics has to be identical to the processing when
//    writing values to the output file. Using separate functions turned out
//    to be error prone due to the difficulty of keeping multiple functions
//    exactly in synchronization.
//
//  Parameters:
//     writerestarts: false => This is a statistics gathering pass
//                    true => This pass is writing data to the output file.
//     passfunctions: Pointer to EncoderComponent member function used
//                    to process this pass.
//     dcfunction:    Pointer to the EncoderComponent member function used
//                    to process DC coefficient values.
//     acfunction:    Pointer to the EncoderComponent member function used
//                    to process AC coefficient values.
//     sss:           Spectral Selection Start (0-63)
//     sse:           Spectral Selection End (0-63)
//     ssa:           Successive Approximation (0-13)
//
void JpegEncoder::InterleavedPass (
                    bool writedata,
                    JpegEncoderComponent::COMPONENTPASSFUNCTION passfunction,
                    JpegEncoderComponent::DCOUTPUTFUNCTION dcfunction,
                    JpegEncoderComponent::ACOUTPUTFUNCTION acfunction,
                    unsigned int sss,
                    unsigned int sse,
                    unsigned int ssa)
{
  const int progressscale = 8 ;
  unsigned int progress ;
  unsigned int progressincrement ;
  if (writedata)
    progress = 50 << progressscale ;
  else
    progress = 0 ;
  progressincrement = (100 << progressscale) / (2 * mcu_rows) ;

  ResetDcValues () ;

  unsigned int intervalcount = 0 ;
  unsigned int restartmarker = 0 ;
  for (unsigned int mcurow = 0 ;
       mcurow < mcu_rows ;
       ++ mcurow)
  {
    for (unsigned int mcucol = 0 ;
         mcucol < mcu_cols ;
          ++ mcucol, ++ intervalcount)
    {
      // Handle Restart Markers
      if (restart_interval != 0)
      {
        if (intervalcount == restart_interval && restart_interval != 0)
        {
          if (writedata)
            OutputMarker (RST0 | restartmarker) ;
          ResetDcValues () ;
          ++ restartmarker ;
          restartmarker %= 8 ;
          intervalcount = 0 ;
        }
      }

      for (unsigned int cc = 0 ; cc < scan_component_count ; ++ cc)
      {
        for (unsigned int cy = 0 ;
             cy < scan_components [cc]->GetVerticalFrequency () ;
             ++ cy)
        {
          unsigned int durow = scan_components [cc]->GetVerticalFrequency ()
                             * mcurow + cy ;
          for (unsigned int cx = 0 ;
               cx < scan_components [cc]->GetHorizontalFrequency () ;
               ++ cx)
          {
            unsigned int ducol
              = scan_components [cc]->GetHorizontalFrequency ()
              * mcucol + cx ;
            (scan_components [cc]->*passfunction) (durow, ducol,
                                              dcfunction, acfunction,
                                              sss, sse, ssa) ;
          }
        }
      }
    }
    progress += progressincrement ;
    CallProgressFunction (progress >> progressscale) ;
  }
  CallProgressFunction (100) ;
  return ;
}

//
//  Description:
//
//    This function writes the Huffman tables used by a scan to the output
//    stream.
//
//    We only use two DC and two AC tables to be compatible with baseline
//    JPEG.  In progressive JPEG there is really no reason for more than
//    two tables for RGB images. If we ever go to four colors then things
//    may need to change.
//
//    The Y component always uses table ID 0. The Cb and Cr components always
//    use table ID 1.
//
//  Parameters:
//    scan:     Here the scan structure is used to determine which components
//              are part of the scan (Y and/or Cb/Cr)
//    usedc:    Set to true if the scan includes DC components. (Sequential
//              Scan or Progressive Scan with the spectral selection 0)
//    useac:    Set to true if the scan includes AC components. (Sequential
//              Scan or Progressive Scan with the spectral selection start
//              not zero)
//
void JpegEncoder::PrintHuffmanTables (const Scan &scan,
                                      bool usedc,
                                      bool useac)
{
  // Section B.2.4.2

  if ((scan.component_mask & (1 << YComponent)) != 0)
  {
    // See if this is a color image.
    if (scan_component_count != 1)
    {
      // We have at least two components and the first is the Y component.
      // This means we need the both of the huffman tables.
      // B.2.4.2
      OutputMarker (DHT) ;
      unsigned int size = sizeof (UBYTE2) ;
      if (usedc)
      {
        size += dc_tables [0].OutputSize ()
                + dc_tables [1].OutputSize () ;
      }
      if (useac)
      {
        size += ac_tables [0].OutputSize ()
                + ac_tables [1].OutputSize () ;
      }
      OutputWord (size) ;
      if (usedc)
      {
        OutputByte (0x00) ;
        dc_tables [0].PrintTable (*this) ;
      }
      if (useac)
      {
        OutputByte (0x10) ;
        ac_tables [0].PrintTable (*this) ;
      }
      if (usedc)
      {
        OutputByte (0x01) ;
        dc_tables [1].PrintTable (*this) ;
      }
      if (useac)
      {
        OutputByte (0x11) ;
        ac_tables [1].PrintTable (*this) ;
      }
    }
    else
    {
      // The only component is Y
      unsigned int size = sizeof (UBYTE2) ;
      if (usedc)
      {
        size += dc_tables [0].OutputSize () ;
      }
      if (useac)
      {
        size += ac_tables [0].OutputSize () ;
      }

      OutputMarker (DHT) ;
      OutputWord (size) ;
      if (usedc)
      {
        OutputByte (0x00) ;
        dc_tables [0].PrintTable (*this) ;
      }
      if (useac)
      {
        OutputByte (0x10) ;
        ac_tables [0].PrintTable (*this) ;
      }
    }
  }
  else
  {
    // The Y component is not present. Output is the same for
    // Cb, Cr, or Cb & Cr.
    unsigned int size = sizeof (UBYTE2) ;
    if (usedc)
    {
      size += dc_tables [1].OutputSize () ;
    }
    if (useac)
    {
      size += ac_tables [1].OutputSize () ;
    }

    OutputMarker (DHT) ;
    OutputWord (size) ;
    if (usedc)
    {
      OutputByte (0x01) ;
      dc_tables [1].PrintTable (*this) ;
    }
    if (useac)
    {
      OutputByte (0x11) ;
      ac_tables [1].PrintTable (*this) ;
    }
  }
  return ;
}

//
//  Description:
//
//   This function resets the DC difference values for all
//   all the components in the scan.  We do this at the start of
//   each scan and whenever we output a restart marker.
//
void JpegEncoder::ResetDcValues ()
{
  for (unsigned int ii = 0 ; ii < scan_component_count ; ++ ii)
    scan_components [ii]->ResetDcDifference () ;
  return ;
}

//
//  Description:
//
//    This function determines the dimensions of an MCU using the maximum
//    sampling frequencies of the components.
//
void JpegEncoder::CalculateMcuDimensions ()
{
  max_horizontal_frequency = 1 ;
  max_vertical_frequency = 1 ;

  if (! gray_scale)
  {
    for (unsigned int ii = YComponent ; ii <= CrComponent ; ++ ii)
    {
      if (image_components [ii].GetHorizontalFrequency ()
          > max_horizontal_frequency)
      {
        max_horizontal_frequency
          = image_components [ii].GetHorizontalFrequency () ;
      }

      if (image_components [ii].GetVerticalFrequency ()
          > max_vertical_frequency)
      {
        max_vertical_frequency
          = image_components [ii].GetVerticalFrequency () ;
      }
    }
  }
  else
  {
    max_horizontal_frequency
      = image_components [YComponent].GetHorizontalFrequency () ;
    max_vertical_frequency
      = image_components [YComponent].GetVerticalFrequency () ;
  }

  unsigned int mcuheight = max_vertical_frequency * JpegSampleWidth ;
  unsigned int mcuwidth = max_horizontal_frequency * JpegSampleWidth ;
  mcu_rows = (frame_height + mcuheight - 1) / mcuheight ;
  mcu_cols = (frame_width + mcuwidth - 1) / mcuwidth ;
  return ;
}

//
//  Description:
//
//    This function writes a progressive scan to the output stream.
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//
void JpegEncoder::PrintProgressiveScan (const Scan &scan)
{
  if (scan_component_count == 0)
    throw EJpegFatal ("INTERNAL ERROR - No components in progressive scan") ;

  if (scan.spectral_selection_start == 0)
  {
    if (scan.successive_approximation_high == 0)
    {
      PrintDcFirst (scan) ;
    }
    else
    {
      PrintDcRefine (scan) ;
    }
  }
  else
  {
    if (scan_component_count != 1)
      throw EJpegFatal ("INTERNAL ERROR - AC Scan does not have 1 component") ;

    if (scan.successive_approximation_high == 0)
    {
      PrintAcFirst (scan) ;
    }
    else
    {
      PrintAcRefine (scan) ;
    }
  }
  return ;
}

//
//  Description:
//
//    This function handles scans containing the first pass for DC
//    coefficients.  If successive approximation is not used then
//    this would be the only DC coefficient pass. DC progressive
//    scans may be interleaved, unlike AC scans.
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//
void JpegEncoder::PrintDcFirst (const Scan &scan)
{
  // Reset the Huffman statistics counters.
  if ((scan.component_mask & (1 << YComponent)) != 0)
  {
    dc_tables [0].Reset () ;
  }
  if ((scan.component_mask & (1 << YComponent)) == 0
       || scan_component_count > 1)
  {
    dc_tables [1].Reset () ;
  }

  OutputRestartInterval (restart_interval) ;

  // Gather the Huffman statistics
  if (scan_component_count != 1)
  {
    InterleavedPass (false,
          JpegEncoderComponent::ProgressiveDcFirst,
          JpegEncoderComponent::GatherDcData, NULL,
          0, 0,
          scan.successive_approximation_low) ;
  }
  else
  {
    NoninterleavedPass (false,
          JpegEncoderComponent::ProgressiveDcFirst,
          JpegEncoderComponent::GatherDcData, NULL,
          0, 0,
          scan.successive_approximation_low) ;
  }

  // Create the Huffman tables from the statistics
  if ((scan.component_mask & (1 << YComponent)) != 0)
  {
    dc_tables [0].BuildTable () ;
  }
  if ((scan.component_mask & (1 << YComponent)) == 0
      || scan_component_count > 1)
  {
    dc_tables [1].BuildTable () ;
  }

  PrintHuffmanTables (scan, true, false) ;

  // Output the scan header.
  // Section B.2.3
  OutputMarker (SOS) ;
  OutputWord (6 + 2 * scan_component_count) ;  // Length
  OutputByte (scan_component_count) ;  // Component Count


  if ((scan.component_mask & (1 << YComponent)) != 0)
  {
    OutputByte (YComponent) ;
    OutputByte (0x00) ; // Entropy Tables
  }
  if (! gray_scale)
  {
    if ((scan.component_mask & (1 << CbComponent)) != 0)
    {
      OutputByte (CbComponent) ;
      OutputByte (0x11) ; // Entropy Tables
    }
    if ((scan.component_mask & (1 << CrComponent)) != 0)
    {
      OutputByte (CrComponent) ;
      OutputByte (0x11) ; // Entropy Tables
    }
  }

  OutputByte (0) ; // Spectral Selection Start
  OutputByte (0) ; // Spectral Selection End
  int value = (scan.successive_approximation_high << 4)
            | scan.successive_approximation_low ;
  OutputByte (value) ; // Successive Approximation

  // Output the scan data.
  if (scan_component_count != 1)
  {
    InterleavedPass (true,
          JpegEncoderComponent::ProgressiveDcFirst,
          JpegEncoderComponent::PrintDcData, NULL,
          0, 0,
          scan.successive_approximation_low) ;
  }
  else
  {
    NoninterleavedPass (true,
          JpegEncoderComponent::ProgressiveDcFirst,
          JpegEncoderComponent::PrintDcData, NULL,
          0, 0,
          scan.successive_approximation_low) ;
  }
  return ;
}

//
//  Description:
//
//    This function outputs the data for a refining DC scan. This type of
//    scan is unique in that it does use Huffman encoding.
//
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//
void JpegEncoder::PrintDcRefine (const Scan &scan)
{
  // Output the scan header.
  // Section B.2.3
  OutputMarker (SOS) ;
  OutputWord (6 + 2 * scan_component_count) ;  // Length
  OutputByte (scan_component_count) ;


  if ((scan.component_mask & (1 << YComponent)) != 0)
  {
    OutputByte (YComponent) ;
    OutputByte (0) ; // No Huffman table is used.
  }
  if (! gray_scale)
  {
    if ((scan.component_mask & (1 << CbComponent)) != 0)
    {
      OutputByte (CbComponent) ;
      OutputByte (0) ; // No Huffman table is used.
    }
    if ((scan.component_mask & (1 << CrComponent)) != 0)
    {
      OutputByte (CrComponent) ;
      OutputByte (0) ; // No Huffman table is used.
    }
  }

  OutputByte (0) ; // Spectral Selection Start
  OutputByte (0) ; // Spectral Selection End

  int value = (scan.successive_approximation_high << 4)
            | scan.successive_approximation_low ;
  OutputByte (value) ;

  // Output the scan data.
  if (scan_component_count != 1)
  {
    InterleavedPass (true,
          JpegEncoderComponent::ProgressiveDcRefine,
          NULL, NULL,
          0, 0,
          scan.successive_approximation_low) ;
  }
  else
  {
    NoninterleavedPass (true,
          JpegEncoderComponent::ProgressiveDcRefine,
          NULL, NULL,
          0, 0,
          scan.successive_approximation_low) ;
  }
  return ;
}

//
//  Description:
//
//    This function outputs a scan that is the first pass for a set of AC
//    coefficients.
//
//    Even though AC progressive scans cannot be interleaved, we follow the
//    convention of using Huffman Table #0 for the Y component and #1 for
//    the Cb and Cr components.
//
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//
void JpegEncoder::PrintAcFirst (const Scan &scan)
{
  // Reset the Huffman statistics counters.
  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    ac_tables [0].Reset () ;
  }
  else
  {
    ac_tables [1].Reset () ;
  }

  OutputRestartInterval (restart_interval) ;

  // Gather the Huffman statistics
  FirstAcData (scan, false, JpegEncoderComponent::GatherAcData) ;

  // Generate the Huffman statistics
  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    ac_tables [0].BuildTable () ;
  }
  else
  {
    ac_tables [1].BuildTable () ;
  }

  PrintHuffmanTables (scan, false, true) ;

  // Section B.2.3
  OutputMarker (SOS) ;
  OutputWord (6 + 2 * scan_component_count) ;  // Length
  OutputByte (scan_component_count) ;  // Component Count

  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    OutputByte (YComponent) ;
    OutputByte (0x00) ; // Entropy Table
  }
  else if ((scan.component_mask & (1<<CbComponent)) != 0)
  {
    OutputByte (CbComponent) ;
    OutputByte (0x11) ; // Entropy Tables
  }
  else if ((scan.component_mask & (1<<CrComponent)) != 0)
  {
    OutputByte (CrComponent) ;
    OutputByte (0x11) ; // Entropy Tables
  }

  OutputByte (scan.spectral_selection_start) ; // Spectral Selection Start
  OutputByte (scan.spectral_selection_end) ; // Spectral Selection End
  int value = (scan.successive_approximation_high << 4)
            | scan.successive_approximation_low ;
  OutputByte (value) ; // Successive Approximation

  FirstAcData (scan, true, JpegEncoderComponent::PrintAcData) ;
  return ;
}

//
//  Descriptio:
//
//    This function outputs the data for a scan that refines AC coefficients
//    through successive approximation.
//
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//
void JpegEncoder::PrintAcRefine (const Scan &scan)
{
  // Reset the Huffman statistics counters.
  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    ac_tables [0].Reset () ;
  }
  else
  {
    ac_tables [1].Reset () ;
  }

  OutputRestartInterval (restart_interval) ;
  // Gather the Huffman statistics.
  RefineAcData (scan, false, JpegEncoderComponent::GatherAcData) ;

  // Create the Huffman Table.
  if ((scan.component_mask & (1 <<YComponent)) != 0)
  {
    ac_tables [0].BuildTable () ;
  }
  else
  {
    ac_tables [1].BuildTable () ;
  }

  PrintHuffmanTables (scan, false, true) ;  // Only output the AC table.

  // Create the scan header.
  // Section B.2.3
  OutputMarker (SOS) ;
  OutputWord (6 + 2 * scan_component_count) ;  // Length
  OutputByte (scan_component_count) ;  // Component Count

  if ((scan.component_mask & (1<<YComponent)) != 0)
  {
    OutputByte (YComponent) ;
    OutputByte (0x00) ; // Entropy Tables
  }
  if ((scan.component_mask & (1<<CbComponent)) != 0)
  {
    OutputByte (CbComponent) ;
    OutputByte (0x11) ; // Entropy Tables
  }
  if ((scan.component_mask & (1<<CrComponent)) != 0)
  {
    OutputByte (CrComponent) ;
    OutputByte (0x11) ; // Entropy Tables
  }

  OutputByte (scan.spectral_selection_start) ; // Spectral Selection Start
  OutputByte (scan.spectral_selection_end) ; // Spectral Selection End
  int value = (scan.successive_approximation_high << 4)
            | scan.successive_approximation_low ;
  OutputByte (value) ; // Successive Approximation

  // Output the scan data.
  RefineAcData (scan, true, JpegEncoderComponent::PrintAcData) ;

  return ;
}

//
//  Description:
//
//    This function loops through the data in an initial AC scan. For
//    all scans other than AC progressive ones we use the same function for
//    this purpose. Due to the excessive complexity of AC progressive scans
//    we use a specialized function.
//
//    This function gets called twice for each scan. The first pass is used
//    to gather Huffman statistics. The second pass is to output the scan.
//    Having a common function ensures that Huffman statistics are gathered
//    in the exact same manner as the scan data is output.
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//    outputrestarts: flag to indicate if restart markers are to be output
//    acfunction: The member function to process a data unit
//
void JpegEncoder::FirstAcData (
                        const Scan &scan,
                        bool outputrestarts,
                        JpegEncoderComponent::ACOUTPUTFUNCTION acfunction)
{
  // We use a scale integer to keep trake of progress to lessen the
  // change that the progress increment will be zero.
  const int progressscale = 8 ;
  unsigned int progress ;
  unsigned int progressincrement ;
  if (outputrestarts)
    progress = 50 << progressscale ;
  else
    progress = 0 ;
  progressincrement = (100 << progressscale)
                    / (2 * scan_components [0]->DataUnitRows ()) ;

  scan_components [0]->ResetEobRun () ;

  unsigned int intervalcount = 0 ;  // Count between restarts
  unsigned int restartmarker = 0 ;  // Value 0..7
  for (unsigned int row = 0 ;
       row < scan_components [0]->DataUnitRows () ;
       ++ row)
  {
    for (unsigned int col = 0 ;
         col < scan_components [0]->DataUnitCols () ;
         ++ col, ++ intervalcount)
    {
      // See if we are using restart markers.
      if (restart_interval != 0)
      {
        // Is a restart marker needed.
        if (intervalcount == restart_interval)
        {
          // If there are any outstanding EOB runs we flush them before the
          // restart marker. This is not explicitly stated in the JPEG
          // standard. The definition of "Restart Interval" in section 4.1
          // states that restart markers separate independent sequences,
          // something we would not have if we did not output the EOB run
          // here.
          scan_components [0]->PrintEobRun (acfunction) ;
          // Here we rely on the relationship RST0|n = RSTn [n = 0..7]
          // Section E.1.4
          if (outputrestarts)
            OutputMarker (RST0 | restartmarker) ;
          ++ restartmarker ;
          restartmarker %= 8 ;
          intervalcount = 0 ;
        }
      }

      // Process the data unit
      scan_components [0]->ProgressiveAcFirst (
                                row,
                                col,
                                acfunction,
                                scan.spectral_selection_start,
                                scan.spectral_selection_end,
                                scan.successive_approximation_low) ;
    }
    progress += progressincrement ;
    CallProgressFunction (progress >> progressscale) ;
  }
  // If there is a final end of band run then write it out.
  scan_components [0]->PrintEobRun (acfunction) ;
  CallProgressFunction (100) ;
  return ;
}

//
//  Description
//
//    This function loops through the DC using in a refining AC scan. For
//    all scans other than AC progressive ones we use the same function for
//    this purpose. Due to the excessive complexity of AC progressive scans
//    we use a specialized function.
//
//    This function gets called twice for each scan. The first pass is used
//    to gather Huffman statistics. The second pass is to output the scan.
//    Having a common function ensures that Huffman statistics are gathered
//    in the exact same manner as the scan data is output.
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//    outputrestarts: flag to indicate if restart markers are to be output
//    acfunction: The member function to process a data unit
//
void JpegEncoder::RefineAcData (
                      const Scan &scan,
                      bool outputrestarts,
                      JpegEncoderComponent::ACOUTPUTFUNCTION acfunction)
{
  // We use a scale integer to keep trake of progress to lessen the
  // change that the progress increment will be zero.
  const int progressscale = 8 ;
  unsigned int progress ;
  unsigned int progressincrement ;
  if (outputrestarts)
    progress = 50 << progressscale ;
  else
    progress = 0 ;
  progressincrement = (100 << progressscale)
                    / (2 * scan_components [0]->DataUnitRows ()) ;

  scan_components [0]->ResetEobRun () ;

  unsigned int intervalcount = 0 ;  // Count between restart markers
  unsigned int restartmarker = 0 ;  // 0..7 => RST0..RST7
  for (unsigned int row = 0 ;
       row < scan_components [0]->DataUnitRows () ;
       ++ row)
  {
    for (unsigned int col = 0 ;
         col < scan_components [0]->DataUnitCols () ;
         ++ col, ++ intervalcount)
    {
      // Are we using restart markers?
      if (restart_interval != 0)
      {
        // Do we need to output a restart marker?
        if (intervalcount == restart_interval)
        {
          // If there are any outstanding EOB runs we flush them before the
          // restart marker. This is not explicitly stated in the JPEG
          // standard. The definition of "Restart Interval" in section 4.1
          // states that restart markers separate independent sequences,
          // something we would not have if we did not output the EOB run
          // here.
          scan_components [0]->PrintRefineEobRun (
                                    acfunction,
                                    scan.spectral_selection_start,
                                    scan.spectral_selection_end,
                                    scan.successive_approximation_low) ;
          // Section E.1.4
          if (outputrestarts)
            OutputMarker (RST0 | restartmarker) ;
          ++ restartmarker ;
          restartmarker %= 8 ;
          intervalcount = 0 ;
        }
      }
      scan_components [0]->ProgressiveAcRefine (
                                row,
                                col,
                                acfunction,
                                scan.spectral_selection_start,
                                scan.spectral_selection_end,
                                scan.successive_approximation_low) ;

    }
    progress += progressincrement ;
    CallProgressFunction (progress >> progressscale) ;
  }
  // If there is a final end of band run then write it out.
  scan_components [0]->PrintRefineEobRun (acfunction,
                                     scan.spectral_selection_start,
                                     scan.spectral_selection_end,
                                     scan.successive_approximation_low) ;

  CallProgressFunction (100) ;
  return ;
}

//
//  Description:
//
//    This function determines which components is in a given scan.
//
//  Parameters:
//    scan:  The structure that contains the scan parameters.
//
//  Implicit Outputs:
//    scan_component_count: The number of components in the scan.
//    scan_components:      The list of components in the scan.
//

void JpegEncoder::FindComponentsInScan (Scan &scan)
{
  scan_component_count = 0 ;
  if ((scan.component_mask & (1 <<YComponent)) != 0)
  {
    scan_components [scan_component_count] = &image_components [YComponent] ;
    ++ scan_component_count ;
  }
  if (! gray_scale)
  {
    if ((scan.component_mask & (1 <<CbComponent)) != 0)
    {
      scan_components [scan_component_count]
        = &image_components [CbComponent] ;
      ++ scan_component_count ;
    }
    if ((scan.component_mask & (1 <<CrComponent)) != 0)
    {
      scan_components [scan_component_count]
        = &image_components [CrComponent] ;
      ++ scan_component_count ;
    }
  }
  return ;
}

//
//  Description:
//
//    This function sets the sampling frequencies for a component
//
//  Parameters:
//    component:  The component ID
//    hf:         Horizontal Frequency
//    vf:         Vertical Frequency
//
void JpegEncoder::SetSamplingFrequency (unsigned int component,   
                                        unsigned int hf,
                                        unsigned int vf)
{
  if (component >= MaxComponents)
    throw EJpegError ("Invalid Component ID") ;

  if (hf > JpegMaxSamplingFrequency || hf < JpegMinSamplingFrequency)
    throw EJpegError ("Invalid Horizontal Sampling Frequency") ;

  if (vf > JpegMaxSamplingFrequency || vf < JpegMinSamplingFrequency)
    throw EJpegError ("Invalid Vertical Sampling Frequency") ;

  image_components [component].SetHorizontalFrequency (hf) ;
  image_components [component].SetVerticalFrequency (vf) ;
  return ;
}

//
//  Description:
//
//    This function gets the sampling frequencies for a component
//
//  Parameters:
//    component:  The component ID
//    hf:         Horizontal Frequency
//    vf:         Vertical Frequency
//
void JpegEncoder::GetSamplingFrequency (unsigned int component,
                                        unsigned int &hf,
                                        unsigned int &vf)
{
  if (component >= MaxComponents)
    throw EJpegError ("Invalid Component ID") ;

  hf = image_components [component].GetHorizontalFrequency () ;
  vf = image_components [component].GetVerticalFrequency () ;
  return ;
}

#pragma comment (exestr, "Copyright 1998 Colosseum Builders Inc.")

//
//  Description:
//
//    This function writes the JFIF header for the image.
//
void JpegEncoder::OutputJfifHeader ()
{
  // Create the JFIF header.
  struct JfifHeader jfif ;
  jfif.length = SystemToBigEndian ((UBYTE2) sizeof (jfif)) ;
  jfif.identifier [0] = 'J' ;
  jfif.identifier [1] = 'F' ;
  jfif.identifier [2] = 'I' ;
  jfif.identifier [3] = 'F' ;
  jfif.version [0] = 1 ;
  jfif.version [1] = 1 ;
  jfif.units = 0 ;
  jfif.xdensity = SystemToBigEndian ((UBYTE2) 1) ;
  jfif.ydensity = SystemToBigEndian ((UBYTE2) 1) ;
  jfif.xthumbnail = 0 ;
  jfif.ythumbnail = 0 ;
  fwrite ((char *) &jfif, sizeof (jfif),1,output_stream) ;
  return ;
}

//
//  Description:
//
//    This function sets the attributes for a scan.
//
//  Parameters:
//    scan:  Scan number (0..MaxScan - 1)
//    components: Bit mask of components to include inthe scan
//    sse:  The spectral selection end for the scan.  The
//          spectral selection start is the sse+1 of the previous
//          scan
//    ssa:  Initial successive approximation value for the scan.
//
void JpegEncoder::SetScanAttributes (unsigned int scan,
                                     unsigned long components,
                                     unsigned int sse,
                                     unsigned int ssa)
{
  if (scan >= MaxScans)
    throw EJpegError ("Scan Index Out of Range") ;

  image_scans [scan].component_mask = components ;
  image_scans [scan].spectral_selection_end = sse ;
  image_scans [scan].successive_approximation = ssa ;
  return ;
}

//
//  Description:
//
//    This fnction returns the attributes that have been defined
//    for a scan (revsere of SetScanAttributes).
//
//  Parameters:
//    scan: (in) The scan to retrieve information about
//    components: (out) Bitmask of component IDs in the scan
//    sse: (out) Spectral selection end
//    ssa: (out) Successive Approximation
//
void JpegEncoder::GetScanAttributes (unsigned int scan,
                                     unsigned long &components,
                                     unsigned int &sse,
                                     unsigned int &ssa)
{
  if (scan >= MaxScans)
    throw EJpegError ("Scan Index Out of Range") ;

  components = image_scans [scan].component_mask ;
  sse = image_scans [scan].spectral_selection_end ;
  ssa = image_scans [scan].successive_approximation ;
}

//
//  Description:
//
//    This function counts the number of passes through the data required
//    to write the image. This value is passed t the progress function.
//
void JpegEncoder::CountPassesForProgressReporting ()
{
  current_pass = 0 ;
  if (progressive_mode)
  {
    if (gray_scale)
    {
      total_passes = 1 ;
      for (unsigned int ii = 0 ; ii < scan_count ; ++ ii)
      {
        if ((image_scans [ii].component_mask & (1<<YComponent)) != 0)
        {
          total_passes += image_scans [ii].successive_approximation + 1 ;
        }
      }
    }
    else
    {
      total_passes = 3 ;
      for (unsigned int ii = 0 ; ii < scan_count ; ++ ii)
      {
        total_passes += image_scans [ii].successive_approximation + 1 ;
      }
    }
  }
  else
  {
    if (gray_scale)
      total_passes = 2 ;
    else
      total_passes = 3 + scan_count ;
  }
  return ;
}

//
//  Description:
//
//    This function returns the current quality value for the
//    encoder.
//
//  Return Value:
//    The quality value.
//
unsigned int JpegEncoder::GetQuality () const
{
  return image_quality ;
}

//
//  Description:
//
//    This function sets the image quality value.
//
//  Parameters:
//    value: The quality value (1..100)
//
void JpegEncoder::SetQuality (unsigned int value)
{
  if (value < 1 || value > 100)
    throw EJpegValueOutOfRange () ;

  image_quality = value ;
  return ;
}
