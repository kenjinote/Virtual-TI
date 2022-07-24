#ifndef __BITIMAGE_H
#define __BITIMAGE_H

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
//  Title: BitmapImage Class Definitions
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//    The BitmapImage class is intended to be a neutral intermediate format
//    for storing decompressed images. This class can manage 1, 2, 4, 8 or
//    24-bit images. For 24-bit images the data is stored as RGB triples
//    within the main data buffer. For all other types a color map is used
//    and the image data contains indices into the color map. Sample values are
//    assumed to be in the range 0..255.
//
//  Windows Notes:
//
//    For the sake of "efficiency" this class has been optimized for use on
//    the "Windows" family. The folling oddities are a result of
//    "windowsisms":
//
//    o The data for 24-bitmaps is stored in BGR order rather than RGB.
//      To change this for your system redefine "RedOffset", "GreenOffset",
//      and "BlueOffset".
//
//    o For whatever reason, Windows expects bitmaps to be stored bottom
//      up rather than top down. The first row in the bitmap data is the
//      bottom row in the image. To change behavoir this for your system
//      redefine the implementation of the [] operator.
//
//    o Windows expects the length of all image rows to be rounded up to the
//      nearest four bytes. To change this behavior redefine the value for
//     "RowRounding".
//
//  Debugging Notes:
//
//    Two methods for accessing pixel data within the image are implemented
//    by default range checking is only performed on rows. If the
//    preprocessor symbol CHECK_RANGE is defined then range check is
//    performed on columns as well.
//
//    While the abandonment of range checking here is contrary to the
//    principles followed elsewhere, this is a place where the
//    performance benefit is worth the lack of safety.
//

#include <stdio.h>
#include "datatype.h"

class BitmapImage ;
class BitmapImageCoder ;


typedef void (*PROGRESSFUNCTION)(BitmapImageCoder &coder,
                                 void *data,
                                 unsigned int currentpass,
                                 unsigned int passcount,
                                 unsigned int progress,
                                 bool &cancel) ;

typedef void (*IMAGEPROGRESSFUNCTION)(BitmapImage &image,
                                      void *data,
                                      unsigned int currentpass,
                                      unsigned int passcount,
                                      unsigned int progress,
                                      bool &cancel) ;

class BitmapImageCoder
{
public:
  BitmapImageCoder () : progress_function (NULL), progress_data (NULL) {}
  BitmapImageCoder (const BitmapImageCoder &source) ;
  virtual ~BitmapImageCoder () {}
  BitmapImageCoder &operator=(const BitmapImageCoder &source) ;

  void SetProgressFunction (PROGRESSFUNCTION, void *) ;
  // Function to force an update of image data.
  virtual void UpdateImage () {}
protected:
  void Initialize () ;
  void DoCopy (const BitmapImageCoder &source) ;
  PROGRESSFUNCTION progress_function ;
  void *progress_data ;
} ;

inline BitmapImageCoder::BitmapImageCoder (const BitmapImageCoder &source)
{
  DoCopy (source) ;
  return ;
}

inline BitmapImageCoder &BitmapImageCoder::operator=(
                                const BitmapImageCoder &source)
{
  DoCopy (source) ;
  return *this ;
}

inline void BitmapImageCoder::DoCopy (const BitmapImageCoder &source)
{
  progress_function = source.progress_function ;
  progress_data = source.progress_data ;
  return ;
}

inline void BitmapImageCoder::SetProgressFunction (PROGRESSFUNCTION func,
                                                   void *data)
{
  progress_function = func ;
  progress_data = data ;
  return ;
}

class BitmapImageDecoder : public BitmapImageCoder
{
public:
  virtual ~BitmapImageDecoder () {}
  virtual void ReadImage (FILE *, BitmapImage &) = 0 ;
} ;

class BitmapImageEncoder : public BitmapImageCoder
{
public:
  virtual ~BitmapImageEncoder () {}
  virtual void WriteImage (FILE *, BitmapImage &) = 0 ;
} ;

class BitmapImage
{
public:
#if defined (CHECK_RANGE)
  // The Row class is used to implement range checking on columns. A Row
  // object represents a row if image data.
  class Row
  {
  public:
    UBYTE1 &operator[](unsigned int) ;
  private:
    Row (UBYTE1 *data, unsigned int length) ;
    UBYTE1 *row_data ;
    unsigned int row_length ;
    friend class BitmapImage ;
  } ;
#endif

  // Definition of the color map used by bitmaps of other than 24-bits.
  struct ColorMapEntry
  {
    UBYTE1 blue ;
    UBYTE1 green ;
    UBYTE1 red ;
  } ;

  enum { RedOffset=2, GreenOffset=1, BlueOffset=0 } ;

  // Required Member Functions
  BitmapImage () ;
  BitmapImage (const BitmapImage &) ;
  virtual ~BitmapImage () ;
  BitmapImage &operator=(const BitmapImage &) ;

  // Function to allocate space for to store an image.
  void SetSize (unsigned int cc,    // Color Count
                unsigned int bits,  // Bit Count
                unsigned int ww,    // Width
                unsigned int hh) ;  // Height

  // Function to retrieve entries in the color map.
  ColorMapEntry &ColorMap (unsigned int index) ;
  ColorMapEntry ColorMap (unsigned int index) const ;

  // [] returns image data bytes.
#if defined (CHECK_RANGE)
  Row operator[](unsigned int) const ;
#else
  UBYTE1 *operator[](unsigned int) const ;
#endif
  // Function to reset the image to empty
  void Clear () ;

  // Function to return information about the image.
  UBYTE1 *ImageData() ;
  unsigned int Width () const ;
  unsigned int Height () const ;
  unsigned int BitCount () const ;
  unsigned int ColorCount () const ;
  void GetRGB (unsigned int row, unsigned int col,
               UBYTE1 &red, UBYTE1 &green, UBYTE1 &blue) const ;

  void EightBitQuantization (const BitmapImage &) ;

  void SetProgressFunction (IMAGEPROGRESSFUNCTION, void *) ;

  // Number of bytes to round each row to. On Windows this should be 4.
  enum { RowRounding = 4 } ;

  unsigned int BytesPerRow () const ;
protected:
  void Initialize () ;
  void DoCopy (const BitmapImage &) ;

private:
  // Width in bytes of each row. For 24-bit images this value will
  // always be larger than the width of the image. For 8-bit images
  // it may be larger if the row width is rounded up.
  unsigned int row_width ;
  unsigned int bit_count ;     // Number of bits (1, 2, 4, 8, or 24)
  unsigned int image_width ;
  unsigned int image_height ;
  // Image data arranged left to right, top to bottom. For 24-bit images
  // there are 3 bytes per pixel representing the color to display in RGB order.
  // For all others the data is in index into the color map. 1, 2, and 4-bit
  // image row widths are rounded up to the nearest byte.
  unsigned char *image_data ;

  unsigned int color_count ;   // Number of entries in the color map.
  ColorMapEntry *color_map ;   // Color map for 8-bit image

  // Interal function to state class variables to a known state.
  void ClearData () ;

  struct ColorUsage
  {
    UBYTE1 colors [3] ;
    UBYTE4 usage ;
    ColorUsage *next [3] ;
  } ;

  struct ColorUsageTable
  {
    ColorUsage *lists [256][3] ;
    unsigned int color_count ;
  } ;

  struct ColorArea
  {
    struct
    {
      UBYTE1 low ;
      UBYTE1 high ;
    } color_values [3] ;
    unsigned int color_count ;
    unsigned int pixel_count ;
  }  ;

  int LargestColorRange (ColorArea &area) ;
  void AddColor (UBYTE1 red, UBYTE1 green, UBYTE1 blue) ;
  void SplitAreaInHalf (unsigned int depth, unsigned int retrydepth, unsigned int area, unsigned int splitcolor) ;
  void CreateColor (unsigned int color) ;
  ColorUsage *FindColor (UBYTE1 red, UBYTE1 green, UBYTE1 blue) ;
  void FindColorUsage (const BitmapImage &image) ;
  void FreeColorQuantizationData () ;
  unsigned int QuantizedColor (UBYTE1 red, UBYTE1 green, UBYTE1 blue) ;
  void QuantizeSourceImage (const BitmapImage &image) ;

  ColorUsageTable *color_usage ;
  ColorArea *color_areas ;
  unsigned int color_area_count ;

  void CallProgressFunction (unsigned int, unsigned int, unsigned int) ;
  IMAGEPROGRESSFUNCTION progress_function ;
  void *progress_data ;
} ;

inline UBYTE1 *BitmapImage::ImageData()
{
  return image_data ;
}

inline unsigned int BitmapImage::Width () const
{
  return image_width ;
}

inline unsigned int BitmapImage::Height () const
{
 return image_height ;
}

inline unsigned int BitmapImage::BitCount () const
{
  return bit_count ;
}

inline unsigned int BitmapImage::ColorCount () const
{
  return color_count ;
}

#if ! defined (CHECK_RANGE)
inline UBYTE1 *BitmapImage::operator[](unsigned int xx)const
{
  return &image_data [(image_height - xx - 1) * row_width] ;
}
#endif

inline unsigned int BitmapImage::BytesPerRow () const
{
  return row_width ;
}


#endif
