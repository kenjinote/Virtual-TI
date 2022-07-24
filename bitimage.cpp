//
// Copyright (c) 1997 Colosseum Builders, Inc.
// All rights reserved.
//
// Colosseum Builders, Inc. makes no warranty, expressed or implied
// with regards to this software. It is provided as is.
//
// Permission to use, redistribute, and copy this file is granted
// without a fee so long as as the following conditions are adhered to:
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
//  Title:  Bitmap Image Class Implementation
//
//  Author:  John M. Miano miano@colosseumbuilders.com
//
#include <string.h>
#include "bitimage.h"
#include "grexcept.h"

// Function to round a row width up to the nearest multiple of
// RowRounding. Windows expects rows to be a length that is a multiple of
// four.

static inline int SQUARE (int xx)
{
  return xx * xx ;
}

static inline unsigned int RoundRow (unsigned int width)
{
  unsigned int result = (width + BitmapImage::RowRounding - 1)
                      & ~(BitmapImage::RowRounding - 1) ;
  return result ;
}

//
//  Description:
//
//    Default Constructor
//
BitmapImage::BitmapImage ()
{
  Initialize () ;
  return ;
}

//
//  Description:
//
//    Copy Constructor
//
BitmapImage::BitmapImage (const BitmapImage &source)
{
  Initialize () ;
  DoCopy (source) ;
  return ;
}

//
//  Description:
//
//    Assignment Operator
//
//  Parameters:
//    source:  The object to copy
//
BitmapImage &BitmapImage::operator=(const BitmapImage &source)
{
  delete [] color_map ;
  delete [] image_data ;
  DoCopy (source) ;
  return *this ;
}

//
//  Description:
//
//    Common initialization function.
//
void BitmapImage::Initialize ()
{
  ClearData () ;
  progress_function = NULL ;
  progress_data = NULL ;
  return ;
}

//
//  Description:
//
//    Class Destructor
//
BitmapImage::~BitmapImage ()
{
  delete [] color_map ;
  delete [] image_data ;
  return ;
}

//
//  Description:
//
//    Common function for resetting an object to a known state.
//
void BitmapImage::ClearData ()
{
  bit_count = 0 ;
  image_width = 0 ;
  image_height = 0 ;
  color_count = 0 ;
  color_map = NULL ;
  image_data = NULL ;
  color_usage = NULL ;
  color_areas = NULL ;
  color_area_count = 0 ;

  return ;
}

//
//  Description:
//
//    Common copy function for use by the copy constructor and assignment
//    operator.
//
//  Parameters:
//    source:  The object to copy
//
void BitmapImage::DoCopy (const BitmapImage &source)
{
  progress_function = source.progress_function ;
  progress_data = source.progress_function ;

  bit_count = source.bit_count ;
  image_width = source.image_width ;
  image_height = source.image_height ;
  color_count = source.color_count ;
  color_map = NULL ;
  image_data = NULL ;

  color_usage = NULL ;
  color_areas = NULL ;
  color_area_count = 0 ;

  // Only copy the image data if the size values are valid.
  if (image_width > 0 && image_height > 0 && bit_count > 0
      && (bit_count == 24 || color_count != 0))
  {
    unsigned int bitwidth ;
    unsigned int bytecount ;
    switch (bit_count)
    {
    case 1:
    case 2:
    case 4:
    case 8:
      color_map = new ColorMapEntry [color_count] ;
      memcpy (color_map,
              source.color_map,
              sizeof (ColorMapEntry) * color_count) ;
      bitwidth = bit_count * image_width ;
      row_width = RoundRow ((bitwidth + 7)/8) ;

      bytecount = row_width * image_height ;
      image_data = new UBYTE1 [bytecount] ;
      memcpy (image_data, source.image_data, bytecount) ;
      break ;

    case 24:
      row_width = RoundRow (3 * image_width) ;
      image_data = new UBYTE1 [row_width * image_height] ;
      memcpy (image_data, source.image_data, row_width * image_height) ;
      break ;
    default:
      if (image_width != 0 ||image_height != 0)
        throw EInvalidBitCount () ;
    }
  }
  return ;
}

#if defined (CHECK_RANGE)
//
//  Description:
//
//    Row Class Constructor.  The row class represents a single
//    row of image data.
//
//  Parameters:
//    data: A pointer to the row's data.
//    length: The row length
//
BitmapImage::Row::Row (UBYTE1 *data, unsigned int length)
{
  row_data = data ;
  row_length = length ;
  return ;
}

//
//  Description:
//
//    Row class [] operator. This operator returns the data
//    value at a given point offset in the row.
//
//  Parameters:
//    index:  The row offset
//
//  Return Value:
//    The data value in the row at the specified offset
//
UBYTE1 &BitmapImage::Row::operator[](unsigned int index)
{
  if (index >= row_length)
    throw ESubscriptOutOfRange () ;

  return row_data [index] ;
}

//
//  Description:
//
//    This function returns a pointer to the Nth row of the image's data.
//    It is set up to return the rows in reverse order as Windows expects
//    them so that other software does not have to deal with such wierdness.
//
//  Parameters:
//    xx: The row index
//
//  Return Value:
//    A Row object that describes the image row.
//
BitmapImage::Row BitmapImage::operator[](unsigned int xx)const
{
  // In Windows bitmaps are stored bass ackwards.
  if (xx >= image_height)
    throw ESubscriptOutOfRange () ;
  return Row (&image_data [(image_height - xx - 1) * row_width],
              row_width) ;
}
#endif

//
//  Description:
//
//    This function allocates space to hold an image of the specified size.
//    The colormap (if used) and the image data are all set to zeros.
//
//  Parameters:
//    cc: Number of colors. Ignored for 24-bit bitmaps
//    bits: Number of bits per pixel.
//    ww, hh: Bitmap size
//
void BitmapImage::SetSize (unsigned int cc,     // Color Count
                           unsigned int bits,   // Data Size in Bits
                           unsigned int ww,     // Width
                           unsigned int hh)     // Height
{
  // Get rid of any existing image.
  delete [] color_map ;
  delete [] image_data ;
  ClearData () ;

  switch (bits)
  {
  case 1:
  case 2:
  case 4:
  case 8:
    {
      bit_count = bits ;
      color_count = cc ;
      image_width = ww ;
      image_height = hh ;

      color_map = new ColorMapEntry [color_count] ;
      memset (color_map, 0, sizeof (ColorMapEntry) * color_count) ;
      unsigned int bitsize = bit_count * image_width ;
      row_width = RoundRow ((bitsize + 7)/8) ;
      unsigned int bytecount = row_width * image_height ;
      image_data = new UBYTE1 [bytecount] ;
      memset (image_data, 0, bytecount) ;
    }
    break ;

  case 24:
    {
      bit_count = bits ;
      color_count = cc ;
      image_width = ww ;
      image_height = hh ;
      row_width = RoundRow (3 * image_width) ;
      image_data = new UBYTE1 [row_width * image_height] ;
      memset (image_data, 0, row_width * image_height) ;
    }
    break ;
  default:
    throw EInvalidBitCount () ;
  }
  return ;
}

//
//  Description:
//
//    This function returns a reference to the Nth colormap entry.
//
//  Parameters:
//    index:  The index of the color map entry 0..ColorCount () -1.
//
//  Return Value:
//    The color map entry.
//
BitmapImage::ColorMapEntry &BitmapImage::ColorMap (unsigned int index)
{
  if (index >= color_count)
    throw ESubscriptOutOfRange () ;

  return color_map [index] ;
}

//
//  Description:
//
//    This function returns a reference to the Nth colormap entry.
//
//    This is a const version of the previous function.
//
//  Parameters:
//    index:  The index of the color map entry 0..ColorCount () -1.
//
//  Return Value:
//    The color map entry.
//
BitmapImage::ColorMapEntry BitmapImage::ColorMap (unsigned int index) const
{
  if (index >= color_count)
    throw ESubscriptOutOfRange () ;

  return color_map [index] ;
}

//
//  Description:
//
//    This function clears out the image.
//
void BitmapImage::Clear ()
{
  delete [] color_map ;
  delete [] image_data ;
  ClearData () ;
  return ;
}

//
//  Description:
//
//   This function returns the RGB values for a pixel in the bitmap at the
//   point [row,col] where row=[0..height-1] and col=[0..width-1].
//
//   This function allows a caller to get the RGB values for 1, 2, 4, 6,
//   and 24-bit bitmaps using the same method.
//
//  Parameters:
//    row, col: The position in the image to return data from
//    red, green, blue:  The color value at the specified position
//
void BitmapImage::GetRGB (unsigned int row, unsigned int col,
                          UBYTE1 &red, UBYTE1 &green, UBYTE1 &blue) const
{
  if (row >= image_height && col >= image_width)
    throw ESubscriptOutOfRange () ;

  switch (bit_count)
  {
    unsigned int index ;
    unsigned int offset ;
  case 1:
    offset = col / 8  ;
    index = (((*this)[row][offset] >> (7 - (col % 8))) & 0x1) ;
    red = color_map [index].red ;
    green = color_map [index].green ;
    blue = color_map [index].blue ;
    break ;

  case 2:
    offset = col / 4 ;
    index = (((*this)[row][offset] >> (2 * (3 - (col % 4)))) & 0x3) ;
    red = color_map [index].red ;
    green = color_map [index].green ;
    blue = color_map [index].blue ;
    break ;

  case 4:
    offset = col / 2 ;
    if (col % 2 == 0)
      index = ((*this)[row][offset] & 0xF0) >> 4 ;
    else
      index = ((*this)[row][offset] & 0x0F) ;

    red = color_map [index].red ;
    green = color_map [index].green ;
    blue = color_map [index].blue ;
    break ;

  case 8:
    red = color_map [(*this)[row][col]].red ;
    green = color_map [(*this)[row][col]].green ;
    blue = color_map [(*this)[row][col]].blue ;
    break ;

  case 24:
    red = (*this)[row][3 * col + RedOffset] ;
    green = (*this)[row][3 * col + GreenOffset] ;
    blue = (*this)[row][3 * col + BlueOffset] ;
    break ;
  default:
    throw EInvalidBitCount () ;
  }
  return ;
}

//
// Description:
//    This sets the progress function for the image.
//
// Parameters:
//    function:   The progress function
//    data:       The call progress data
//
void BitmapImage::SetProgressFunction (
                              IMAGEPROGRESSFUNCTION function,
                              void *data)
{
  progress_function = function ;
  progress_data = data ;
  return ;
}

//
// Description:
//    This function calls the progression function.
//
// Parameters:
//    percent:    % complete
//    pass:       Current Pass
//    passcount:  Number of passes
//
void BitmapImage::CallProgressFunction (unsigned int percent,
                                        unsigned int pass,
                                        unsigned int passcount)
{
  if (progress_function == NULL)
    return ;
  if (percent > 100)
    percent = 100 ;

  bool cancel = false ;
  progress_function (*this, progress_data, pass, passcount, percent, cancel) ;
  if (cancel)
    throw EGraphicsAbort () ;
  return ;
}

//
// Color Quantization Routines
//
//  Since I have received many requests for color quantization I have
//  whipped this up. I have to admit to knowing nothing about color
//  quantization (I have always had systems that did not need it). The
//  only techniques for quantization that I had been familiar with are
//  scaling color values and having a fixed color space. However, I thought
//  1-pass methods such as those would be too cude for someone of my
//  programming skill.
//
//  What I have tried instead is to use two passes. What we do is create
//  a 3-dimensional hash table of color values. The we keep dividing the
//  colorspace in half until we have set of color ranges.
//
//  Hey, it's probably not all that efficient but it's the best I could come
//  up with in an evening.
//
//

//
//  Description:
//    This function looks up the color entry in the color hash table
//    for a specified color value.
//
//  Parameters:
//    red, green, blue:  The color value to search for.
//
BitmapImage::ColorUsage *BitmapImage::FindColor (UBYTE1 red,
                                                 UBYTE1 green,
                                                 UBYTE1 blue)
{
  if (color_usage->lists [red][RedOffset] == NULL
      || color_usage->lists [green][GreenOffset] == NULL
      || color_usage->lists [blue][BlueOffset] == NULL)
  {
    return NULL ;
  }

  for (ColorUsage *entry = color_usage->lists [red][RedOffset] ;
       entry != NULL ;
       entry = entry->next [RedOffset])
  {
    if (entry->colors [BlueOffset] == blue
        && entry->colors [GreenOffset] == green)
    {
      return entry ;
    }
  }
  return NULL ;
}

//
// Description:
//  This function adds a new color value to the color hash table.
//
//  Parameters:
//    red, green, blue:  The color value to search for.
//

void BitmapImage::AddColor (UBYTE1 red, UBYTE1 green, UBYTE1 blue)
{
  // Create the new color entry.
  ColorUsage *entry = new ColorUsage ;
  memset (entry, 0, sizeof (*entry)) ;
  entry->usage = 1 ;
  entry->colors [RedOffset] = red ;
  entry->colors [GreenOffset] = green ;
  entry->colors [BlueOffset] = blue ;

  // Add the new entry to each hash chain.
  if (color_usage->lists [red][RedOffset] == NULL)
  {
    color_usage->lists [red][RedOffset] = entry ;
  }
  else
  {
    entry->next [RedOffset] = color_usage->lists [red][RedOffset] ;
    color_usage->lists [red][RedOffset] = entry ;
  }
  if (color_usage->lists[green][GreenOffset] == NULL)
  {
    color_usage->lists [green][GreenOffset] = entry ;
  }
  else
  {
    entry->next [GreenOffset] = color_usage->lists [green][GreenOffset] ;
    color_usage->lists [green][GreenOffset] = entry ;
  }
  if (color_usage->lists [blue][BlueOffset] == NULL)
  {
    color_usage->lists [blue][BlueOffset] = entry ;
  }
  else
  {
    entry->next [BlueOffset] = color_usage->lists [blue][BlueOffset] ;
    color_usage->lists [blue][BlueOffset] = entry ;
  }

  ++ color_usage->color_count ;
  return ;
}

//
// Description:
//  This function converts a 24-bit image to 8-bit.
//
// Parameters:
//  image: The image to convert.
//
void BitmapImage::EightBitQuantization (const BitmapImage &image)
{
  // If this is not a 24-bit image then there is no need to quantize.
  // Instead, we make this a copy operation.
  if (image.bit_count != 24)
  {
    *this = image ;
    return ;
  }

  progress_function = image.progress_function ;
  progress_data = image.progress_function ;

  // Allocate space for the image.
  SetSize (256, 8, image.image_width, image.image_height) ;

  // Allocate temporary structures used for color quantization.
  color_usage = new ColorUsageTable ;
  memset (color_usage, 0, sizeof (*color_usage)) ;
  color_areas = new ColorArea [256] ;

  try
  {
    FindColorUsage (image) ;
  }
  catch (EGraphicsAbort &)
  {
    FreeColorQuantizationData () ;
    return ;
  }
  catch (...)
  {
    FreeColorQuantizationData () ;
    throw ;
  }

  // Set the first (zero'th) area to the entire color space.
  color_areas [0].color_values [RedOffset].low = 0 ;
  color_areas [0].color_values [RedOffset].high = 255 ;
  color_areas [0].color_values [GreenOffset].low = 0 ;
  color_areas [0].color_values [GreenOffset].high = 255 ;
  color_areas [0].color_values [BlueOffset].low = 0 ;
  color_areas [0].color_values [BlueOffset].high = 255 ;
  color_areas [0].pixel_count = image_height * image_width ;
  color_areas [0].color_count = color_usage->color_count ;
  color_area_count = 1 ;

  // Divide the color area in half.
  SplitAreaInHalf (7, // Depth
                  0, // Retry Count
                  0, // Area Number
                  RedOffset) ; // Split Color

  // It is possible that some of the areas could not be divided using the
  // previous process. If we have some remaining colors we try to assign them
  // to the blocks with the largest size in the colorspace.
  while (color_area_count < 256)
  {
    int cb = 0 ;
    // Search for the largest colorspace area.
    unsigned int value
                    = SQUARE (color_areas [cb].color_values [RedOffset].high -
                              color_areas [cb].color_values [RedOffset].low)
                    + SQUARE (color_areas [cb].color_values [GreenOffset].high -
                              color_areas [cb].color_values [GreenOffset].low)
                    + SQUARE (color_areas [cb].color_values [BlueOffset].high -
                              color_areas [cb].color_values [BlueOffset].low)
                    * color_areas [cb].color_count ;
    for (unsigned int ii = 1 ; ii < color_area_count ; ++ ii)
    {
      if (color_areas [ii].color_count > 1)
      {
        unsigned int newvalue
                    = SQUARE (color_areas [ii].color_values [RedOffset].high -
                              color_areas [ii].color_values [RedOffset].low)
                    + SQUARE (color_areas [ii].color_values [GreenOffset].high -
                              color_areas [ii].color_values [GreenOffset].low)
                    + SQUARE (color_areas [ii].color_values [BlueOffset].high -
                              color_areas [ii].color_values [BlueOffset].low)
                    * color_areas [ii].color_count ;
        if (newvalue > value)
        {
          value = newvalue ;
          cb = ii ;
        }
      }
    }
    // If we have not colors to divide then stop.
    if (color_areas [cb].color_count == 1)
      break ;

    // Split this color block in half.
    SplitAreaInHalf (0, // Depth
                    0, // Retry Count
                    cb, // Area Number
                    LargestColorRange (color_areas [cb])) ; // Split Color
  }

  for (unsigned int ii = 0 ; ii < color_area_count ; ++ ii)
  {
    CreateColor (ii) ;
  }
  // Assign colors to the image.
  try
  {
    QuantizeSourceImage (image) ;
  }
  catch (EGraphicsAbort &)
  {
    FreeColorQuantizationData () ;
    return ;
  }
  catch (...)
  {
    FreeColorQuantizationData () ;
    throw ;
  }

  FreeColorQuantizationData () ;
  return ;
}

//
// Description:
//    This function finds the colors that are used and their frequency
//    within a source image.
//
// Parameters:
//    image:   The source image.
//
void BitmapImage::FindColorUsage (const BitmapImage &image)
{
  // Create a color entry for each distinct color.
  const unsigned int climit = image_width * 3 ;
  for (unsigned int rr = 0 ; rr < image_height ; ++ rr)
  {
    UBYTE1 *rowdata = &image.image_data [rr * image.row_width] ;
    for (unsigned int cc = 0 ; cc < climit ; cc += 3 )
    {
      UBYTE1 red, green, blue ;
      red = rowdata [cc + RedOffset] ;
      green = rowdata [cc + GreenOffset]  ;
      blue = rowdata [cc + BlueOffset] ;

      // If the color already exists in the table just increment
      // its usage count. Otherwise add a new color entry for it.
      ColorUsage *entry = FindColor (red, green, blue) ;
      if (entry == NULL)
      {
        AddColor (red, green, blue) ;
      }
      else
      {
        ++ entry->usage ;
      }
    }
    CallProgressFunction (100 * rr/image_height, 1, 2) ;
  }
  CallProgressFunction (100, 1, 2) ;

  return ;
}

//
//  Description:
//
//    This function frees all the dynamic data allocated during
//    color quantization.
//
void BitmapImage::FreeColorQuantizationData ()
{
  // Get rid of al the temporary storage.
  for (unsigned int ii = 0 ; ii < 256 ; ++ ii)
  {
    ColorUsage *next ;
    for (ColorUsage *entry = color_usage->lists [ii][RedOffset] ;
         entry != NULL ;
         entry = next)
    {
       next = entry->next [RedOffset] ;
       delete entry ;
    }
  }

  delete color_usage ; color_usage = NULL ;
  delete [] color_areas ;
  return ;
}

//
//  Description:
//
//    This function divides an area of the colorspace in half.
//
//  Parameters:
//    depth:        The search depth
//    retrydepth:   The number of retries
//    areaid:       The area to split
//    splitcolor:   The color to split on.
//
void BitmapImage::SplitAreaInHalf (unsigned int depth,
                                   unsigned int retrydepth,
                                   unsigned int areaid,
                                   unsigned int splitcolor)
{

  if (color_areas [areaid].color_count == 1)
  {
    return ;
  }
  else if (color_areas [areaid].color_values [splitcolor].high
      == color_areas [areaid].color_values [splitcolor].low)
  {
    if (retrydepth < 2)
    {
      SplitAreaInHalf (depth, retrydepth + 1, areaid, (splitcolor + 1) % 3) ;
    }
    return ;
  }

  unsigned int c1 = (splitcolor + 1) % 3 ;
  unsigned int c2 = (splitcolor + 2) % 3 ;

  unsigned int splitsize = color_areas [areaid].pixel_count / 2 ;
  unsigned int splitpixelcount = 0 ;
  unsigned int splitcolorcount = 0 ;
  unsigned int newlimit ;
  unsigned int newpixelcount ;
  unsigned int newcolorcount ;

  for (newlimit = color_areas [areaid].color_values [splitcolor].low ;
       newlimit <= color_areas [areaid].color_values [splitcolor].high ;
       ++ newlimit)
  {
    newpixelcount = 0 ;
    newcolorcount = 0 ;
    for (ColorUsage *entry = color_usage->lists [newlimit][splitcolor] ;
         entry != NULL ;
         entry = entry->next [splitcolor])
    {
      if (entry->colors [c1] >= color_areas [areaid].color_values [c1].low
          && entry->colors [c1] <= color_areas [areaid].color_values [c1].high
          && entry->colors [c2] >= color_areas [areaid].color_values [c2].low
          && entry->colors [c2] <= color_areas [areaid].color_values [c2].high)
      {
        newpixelcount += entry->usage ;
        ++ newcolorcount ;
      }
    }

    if (newcolorcount == color_areas [areaid].color_count)
    {
      // There is no way to split using this color.
      if (retrydepth < 2)
      {
        SplitAreaInHalf (depth, retrydepth + 1, areaid, (splitcolor + 1) % 3) ;
      }
      return ;
    }
    else if (newcolorcount > color_areas [areaid].color_count)
    {
      throw EGraphicsException ("INTERNAL ERROR - Quantization area color count invalid") ;
    }

    if (splitpixelcount + newpixelcount >= splitsize)
    {
      if (splitpixelcount + newpixelcount != color_areas [areaid].pixel_count)
      {
        splitpixelcount += newpixelcount ;
        splitcolorcount += newcolorcount ;
      }
      else
      {
        -- newlimit ;
      }
      color_areas [color_area_count] = color_areas [areaid] ;
      color_areas [color_area_count].pixel_count = color_areas [areaid].pixel_count - splitpixelcount ;
      color_areas [color_area_count].color_count = color_areas [areaid].color_count - splitcolorcount ;
      color_areas [color_area_count].color_values [splitcolor].low = newlimit + 1 ;
      ++ color_area_count ;

      color_areas [areaid].color_values [splitcolor].high = newlimit ;
      color_areas [areaid].pixel_count = splitpixelcount ;
      color_areas [areaid].color_count = splitcolorcount ;

      if (depth > 0)
      {
        SplitAreaInHalf (depth - 1, 0, color_area_count - 1, LargestColorRange (color_areas [color_area_count-1])) ;
        SplitAreaInHalf (depth - 1, 0, areaid, LargestColorRange (color_areas [areaid])) ;
      }
      return ;
    }
    else
    {
      splitpixelcount += newpixelcount ;
      splitcolorcount += newcolorcount ;
    }
  }
  throw EGraphicsException ("INTERNAL ERROR - Quantization area pixel count invalid") ;
}

//
// Description:
//    This function creates a color from a color area then maps the colors
//    in the source image to the new color map.
//
// Parameters:
//    color:   The new color index value
//
void BitmapImage::CreateColor (unsigned int color)
{
  unsigned int red = 0 ;
  unsigned int green = 0 ;
  unsigned int blue = 0 ;

  const int c0 = RedOffset ;
  const int c1 = GreenOffset ;
  const int c2 = BlueOffset ;

  unsigned int itemcount = 0 ;
  for (unsigned int cc = color_areas [color].color_values [c0].low ;
       cc <= color_areas [color].color_values [c0].high ;
       ++ cc)
  {
    for (ColorUsage *entry = color_usage->lists [cc][c0] ;
         entry != NULL ;
         entry = entry->next [c0])
    {
      if (entry->colors [c1] >= color_areas [color].color_values [c1].low
          && entry->colors [c1] <= color_areas [color].color_values [c1].high
          && entry->colors [c2] >= color_areas [color].color_values [c2].low
          && entry->colors [c2] <= color_areas [color].color_values [c2].high)
      {
        red += entry->colors [RedOffset] * entry->usage ;
        green += entry->colors [GreenOffset] * entry->usage ;
        blue += entry->colors [BlueOffset] * entry->usage ;
        itemcount += entry->usage ;
      }
    }
  }

  if (itemcount == 0)
    return ;

  color_map [color].red = (red + itemcount/2) / itemcount ;
  color_map [color].green = (green + itemcount/2) / itemcount ;
  color_map [color].blue = (blue + itemcount/2) / itemcount ;

  return ;
}

//
// Description:
//    This function finds the largest dimension of a color area.
//
// Parameters:
//    area:    The color area to use
//
int BitmapImage::LargestColorRange (ColorArea &area)
{
  unsigned int deltared = area.color_values [RedOffset].high
                        - area.color_values [RedOffset].low ;
  unsigned int deltagreen = area.color_values [GreenOffset].high
                        - area.color_values [GreenOffset].low ;
  unsigned int deltablue = area.color_values [BlueOffset].high
                        - area.color_values [BlueOffset].low ;

  if (deltared >= deltagreen && deltared >= deltablue)
    return RedOffset ;

  if (deltablue >= deltagreen && deltablue >= deltared)
    return BlueOffset ;

  return GreenOffset ;
}

//
// Description:
//    This function returns the 8-bit quantized color value for and RGB color.
//
// Parameters:
//    red, green, blue: The RGB value to convert
//
unsigned int BitmapImage::QuantizedColor (UBYTE1 red, UBYTE1 green, UBYTE1 blue)
{
  for (unsigned int color = 0 ; color < color_area_count ; ++ color)
  {
    if (red >= color_areas [color].color_values [RedOffset].low
        && red <= color_areas [color].color_values [RedOffset].high
        && green >= color_areas [color].color_values [GreenOffset].low
        && green <= color_areas [color].color_values [GreenOffset].high
        && blue >= color_areas [color].color_values [BlueOffset].low
        && blue <= color_areas [color].color_values [BlueOffset].high)
    {
      return color ;
    }
  }
  throw EGraphicsException ("INTERNAL ERROR - color not quantized") ;

  DUMMY_RETURN // MSVC++ is too stupid to realize we can't get here.
}

//
// Description:
//    This function converts the RGB color values in the source image
//    to 8-bit quantized color values.
//
// Parameters:
//    src:  The source image
//
void BitmapImage::QuantizeSourceImage (const BitmapImage &src)
{
  for (unsigned int rr = 0 ; rr < image_height ; ++ rr)
  {
    CallProgressFunction (rr * 100 / image_height, 2, 2) ;

    UBYTE1 *srcdata = &src.image_data [rr * src.row_width] ;
    UBYTE1 *dstdata = &image_data [rr * row_width] ;
    for (unsigned int cc = 0 ; cc < image_width ; ++ cc)
    {
      UBYTE1 red = srcdata [3 * cc + RedOffset] ;
      UBYTE1 green = srcdata [3 * cc + GreenOffset]  ;
      UBYTE1 blue = srcdata [3 * cc + BlueOffset] ;
      dstdata [cc] = QuantizedColor (red, green, blue) ;
    }
  }
  CallProgressFunction (100, 2, 2) ;
  return ;
}
