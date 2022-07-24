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
// JPEG Decoder Component Class Implementation
//
// Author:  John M. Miano  miano@colosseumbuilders.com
//


#include "jpdecomp.h"
#include "jpgexcep.h"
#include "jpdequan.h"
#include "jpegdeco.h"
#include "jpdecobk.h"
#include "jpeg.h"
#include "bitimage.h"


//
//  Description:
//
//    SequentialOnly
//
//    This function extends the sign bit of a decoded value.
//
//  Parameters:
//    vv: The bit value
//    tt: The length of the bit value
//

static inline int Extend (int vv, int tt)
{
  // Extend function defined in Section F.2.2.1 Figure F.12
   // The tt'th bit of vv is the sign bit. One is for
   // positive values and zero is for negative values.
   int vt = 1 << (tt - 1) ;
   if (vv < vt)
   {
      vt = (-1 << tt) + 1 ;
      return vv + vt ;
   }
   else
   {
      return vv ;
   }
}

//
//  Description:
//
//    Class default constructor
//
JpegDecoderComponent::JpegDecoderComponent ()
{
  component_id = 0 ;
  horizontal_frequency = 0 ;
  vertical_frequency = 0 ;
  v_sampling = 0 ;
  h_sampling = 0 ;
  last_dc_value = 0 ;
  ac_table = NULL ;
  dc_table = NULL ;
  quantization_table = NULL ;
  eob_run = 0 ;
  noninterleaved_rows = 0 ;
  noninterleaved_cols = 0 ;
  data_units = NULL ;
  coefficient_blocks = NULL ;
  upsample_data = NULL ;
  return ;
}

//
//  Description:
//
//    Class Destructor
//
JpegDecoderComponent::~JpegDecoderComponent ()
{
  delete [] data_units ; data_units = NULL ;
  delete [] upsample_data ; upsample_data = NULL ;
  return ;
}

//
//  Description:
//
//    This function sets the horizontal sampling frequency
//    for the component.
//
//  Parameters:
//    value: The sampling factor (1..4)
//
void JpegDecoderComponent::HorizontalFrequency (unsigned int value)
{
  if (value < JpegMinSamplingFrequency || value > JpegMaxSamplingFrequency)
    throw EJpegValueOutOfRange () ;

  horizontal_frequency = value ;
  return ;
}

//
//  Description:
//
//    This function sets the vertical sampling frequency
//    for the component.
//
//  Parameters:
//    value: The sampling factor (1..4)
//
void JpegDecoderComponent::VerticalFrequency (unsigned int value)
{
  if (value < JpegMinSamplingFrequency || value > JpegMaxSamplingFrequency)
    throw EJpegValueOutOfRange () ;

  vertical_frequency = value ;

  return ;
}

//
//  Description:
//
//    This function associates a quantization table with the component.
//
//  Parameters:
//    table:  The quantization table
//
void JpegDecoderComponent::SetQuantizationTable (
                                     JpegDecoderQuantizationTable &table)
{
  quantization_table = &table ;
  return ;
}

//
//  Description:
//
//    This function determines the dimensions for the component and allocates
//    the storage to hold the component's data.
//
//  Parameters:
//    decoder:  The jpeg decoder this component belongs to.
//
void JpegDecoderComponent::AllocateComponentBuffers (
                               const JpegDecoder &decoder)
{
  if (data_units == NULL)
  {
    // Determine sampling for the component. This is the amount of
    // stretching needed for the component.
    v_sampling = decoder.MaxVFrequency () / vertical_frequency ;
    h_sampling = decoder.MaxHFrequency () / horizontal_frequency ;

    // Determine the component's dimensions in a non-interleaved scan.
    noninterleaved_rows = (decoder.FrameHeight ()
                           + v_sampling * JpegSampleWidth - 1)
                          / (v_sampling * JpegSampleWidth) ;
    noninterleaved_cols = (decoder.FrameWidth ()
                           + h_sampling * JpegSampleWidth - 1)
                          / (h_sampling * JpegSampleWidth) ;

    du_rows = decoder.McuRows () * vertical_frequency ;
    du_cols = decoder.McuCols () * horizontal_frequency ;

    data_units = new JpegDecoderDataUnit [du_rows * du_cols] ;
  }

  if (decoder.IsProgressive () && coefficient_blocks == NULL)
  {
    unsigned int count = du_rows * du_cols ;
    coefficient_blocks = new JpegDecoderCoefficientBlock [count] ;
    memset (coefficient_blocks,
            0,
            sizeof (JpegDecoderCoefficientBlock) * count) ;
  }
  return ;
}

//
//  Description:
//
//    This function frees the memory allocated by the component
//    during the decompression process.
//
void JpegDecoderComponent::FreeComponentBuffers ()
{
  delete [] data_units ; data_units = NULL ;
  delete [] coefficient_blocks ; coefficient_blocks = NULL ;
  delete [] upsample_data ; upsample_data = NULL ;
  return ;
}

//
//  Description:
//
//    This function asigned Huffman tables to the component.
//
//  Parameters:
//    dc:  The DC Huffman table
//    ac:  The AC Huffman table
//
void JpegDecoderComponent::SetHuffmanTables (JpegHuffmanDecoder &dc,
                                             JpegHuffmanDecoder &ac)
{
  dc_table = &dc ;
  ac_table = &ac ;
  return ;
}

//
//  Description:
//
//    This function ensures that this component has a defined
//    AC table assigned to it. If not, it throws an exception.
//
void JpegDecoderComponent::CheckAcTable ()
{
  // If this occurs then we have a programming error.
  if (ac_table == NULL)
    throw EJpegFatal ("INTERNAL ERROR - AC Table Not Assigned") ;

  if (! ac_table->Defined ())
    throw EJpegError ("AC Table Not Defined") ;

  return ;
}

//
//  Sequential and Progressive
//
//  This function is called before processing a scan. It ensures that the
//  DC Huffman table used by the component has actually been defined.
//
void JpegDecoderComponent::CheckDcTable ()
{
  if (dc_table == NULL)
    throw EJpegFatal ("INTERNAL ERROR - DC Table Not Assigned") ;

  // This condition could be caused by a corrupt JPEG stream.
  if (! dc_table->Defined ())
    throw EJpegFatal ("INTERNAL ERROR - DC Table Not Defined") ;

  return ;
}

//
//  Description:
//
//    Sequential and Progressive
//
//    This function is called before processing a scan. It ensures that the
//    Quantization table used by the component has actually been defined.
//
void JpegDecoderComponent::CheckQuantizationTable ()
{
  if (quantization_table == NULL)
    throw EJpegFatal ("INTERNAL ERROR - Quantization Table Not Assigned") ;

  if (! quantization_table->Defined ())
    throw EJpegError ("Quantization Table Not Defined") ;

  return ;
}

//
//  Description:
//
//    This function decodes a data unit in a sequential scan.
//
//  Parameters:
//    decoder: The decoder that owns this component
//    mcurow, mcucol:  The row and column for this data unit.
//
void JpegDecoderComponent::DecodeSequential (JpegDecoder &decoder,
                                             unsigned int mcurow,
                                             unsigned int mcucol)
{
  JpegDecoderCoefficientBlock data ;
  memset (&data, 0, sizeof (data)) ;

  // Decode the DC differce value.
  // Section F.2.2.1
  unsigned int count ; // called T in F.2.2.1
  count = dc_table->Decode (decoder) ;
  int bits = decoder.Receive (count) ;
  int diff = Extend (bits, count) ;

  // Create the DC value from the difference and the previous DC value.
  int dc = diff + last_dc_value ;
  last_dc_value = dc ;
  data [0][0] = dc ;

  // Decode the AC coefficients.
  // Section F.2.2.2 Figure F.13
  for (unsigned int kk = 1 ; kk < JpegSampleSize ; ++ kk)
  {
    UBYTE2 rs = ac_table->Decode (decoder) ;
    UBYTE2 ssss = (UBYTE2) (rs & 0xF) ;
    UBYTE2 rrrr = (UBYTE2) (rs >> 0x4) ;

    if (ssss == 0)
    {
      // ssss is zero then rrrr should either be 15 or zero according to
      // Figure F.1. 0 means that the rest of the coefficients are zero
      // while 15 means the next 16 coefficients are zero. We are not checking
      // for other values because Figure F.13 shows values other than 15
      // as being treated as zero.
      if (rrrr  != 15)
        break ;
      kk += 15 ; // Actually 16 since one more gets added by the loop.
    }
    else
    {
      // If ssss is non-zero then rrrr gives the number of zero coefficients
      // to skip.

      kk += rrrr ;
      if (kk >= JpegSampleSize)
        throw EJpegFatal ("Value out of range") ;

      // Receive and extend the additional bits.
      // Section F2.2.2 Figure F.14
      int bits = decoder.Receive (ssss) ;
      int value = Extend (bits, ssss) ;
      (&data [0][0])[JpegZigZagInputOrder (kk)] = value ;
    }
  }
  data_units [mcurow * du_cols + mcucol].InverseDCT (data,
                                                     *quantization_table) ;
  return ;
}

//
//  Description:
//
//    This function upsamples the data for the component. Here we take
//    the values from the data_units array and copy it to the
//    upsample_data. If the horizontal or vertical sampling frequencies
//    are less than the maximum for the image then we need to
//    stretch the data during the copy.
//
void JpegDecoderComponent::Upsample ()
{
  unsigned int imagesize = du_rows * v_sampling * du_cols
                              * h_sampling * JpegSampleSize ;
  if (imagesize == 0)
    return ;  // No data for this component yet.

  if (upsample_data == NULL)
    upsample_data = new JPEGSAMPLE [imagesize] ;

  // Simple case where component does not need to be upsampled.
  if (v_sampling == 1 && h_sampling == 1)
  {
    unsigned output = 0 ;
    unsigned int startdu = 0 ;
    for (unsigned int durow = 0 ; durow < du_rows ; ++ durow)
    {
      for (unsigned int ii = 0 ; ii < JpegSampleWidth ; ++ ii)
      {
         unsigned int du = startdu ;
	      for (unsigned int ducol = 0 ; ducol < du_cols ; ++ ducol)
        {
      	  upsample_data [output] = data_units [du][ii][0] ; ++ output ;
      	  upsample_data [output] = data_units [du][ii][1] ; ++ output ;
      	  upsample_data [output] = data_units [du][ii][2] ; ++ output ;
      	  upsample_data [output] = data_units [du][ii][3] ; ++ output ;
      	  upsample_data [output] = data_units [du][ii][4] ; ++ output ;
      	  upsample_data [output] = data_units [du][ii][5] ; ++ output ;
      	  upsample_data [output] = data_units [du][ii][6] ; ++ output ;
      	  upsample_data [output] = data_units [du][ii][7] ; ++ output ;
          ++ du ;
      	}
      }
      startdu += du_cols ;
    }
  }
  else
  {
    unsigned output = 0 ;
    unsigned int startdu = 0 ;
    for (unsigned int durow = 0 ; durow < du_rows ; ++ durow)
    {
      for (unsigned int ii = 0 ; ii < JpegSampleWidth ; ++ ii)
      {
         for (unsigned int vv = 0 ; vv < v_sampling ; ++ vv)
         {
           unsigned int du = startdu ;
    	      for (unsigned int ducol = 0 ; ducol < du_cols ; ++ ducol)
           {
             unsigned int jj ;
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][0] ; ++ output ;
             }
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][1] ; ++ output ;
             }
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][2] ; ++ output ;
             }
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][3] ; ++ output ;
             }
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][4] ; ++ output ;
             }
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][5] ; ++ output ;
             }
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][6] ; ++ output ;
             }
             for (jj = 0 ; jj < h_sampling ; ++ jj)
             {
               upsample_data [output] = data_units [du][ii][7] ; ++ output ;
             }
             ++ du ;
           }
      	}
      }
      startdu += du_cols ;
    }
  }
  return ;
}

//
//  Description:
//
//    This static member function grayscale converts component
//    image data in the upsample_data array and writes it to the
//    the output image.  Actually for a grayscale conversion all
//    we do is copy.
//
//  Parameters:
//    cc:  The component
//    image:  The output image
//
void JpegDecoderComponent::GrayscaleConvert (JpegDecoderComponent &cc,
                                             BitmapImage &image)
{
  unsigned int rowstart = 0 ;
  for (unsigned int ii = 0 ; ii < image.Height () ; ++ ii)
  {
    unsigned int offset = rowstart ;
    UBYTE1 *outrow = image [ii] ;
    for (unsigned int jj = 0 ; jj < image.Width () ; ++ jj)
    {
      outrow [jj] = cc.upsample_data [offset] ;
      ++ offset ;
    }
    rowstart += cc.du_cols * cc.h_sampling * JpegSampleWidth ;
  }
  return ;
}

//
//  Description:
//
//    This static member function converts the upsample_data in three
//    components from YCbCr to RGB and writes it to an image.
//
//  Parameters:
//    c1: The component containing the Y data
//    c2: Ditto for Cb
//    c3: Ditto for Cr
//    image: The output image
//
void JpegDecoderComponent::RGBConvert (JpegDecoderComponent &c1,
                                       JpegDecoderComponent &c2,
                                       JpegDecoderComponent &c3,
                                       BitmapImage &image)
{
  if (c1.upsample_data == NULL
      || c2.upsample_data == NULL
      || c3.upsample_data == NULL)
  {
    // If we get here then do do not yet have data for all components.
    return ;
  }

  unsigned int rowstart = 0 ;
  for (unsigned int ii = 0 ; ii < image.Height () ; ++ ii)
  {
    unsigned int offset = rowstart ;
    UBYTE1 *outrow = image [ii] ;
    for (unsigned int jj = 0 ; jj < 3 * image.Width () ; jj += 3)
    {
      JPEGSAMPLE red = YCbCrToR (c1.upsample_data [offset],
                                 c2.upsample_data [offset],
                                 c3.upsample_data [offset]) ;
      JPEGSAMPLE green = YCbCrToG (c1.upsample_data [offset],
                                  c2.upsample_data [offset],
                                  c3.upsample_data [offset]) ;
      JPEGSAMPLE blue = YCbCrToB (c1.upsample_data [offset],
                                  c2.upsample_data [offset],
                                  c3.upsample_data [offset]) ;
      outrow [jj + BitmapImage::RedOffset] = red ;
      outrow [jj + BitmapImage::GreenOffset] = green ;
      outrow [jj + BitmapImage::BlueOffset] = blue ;
      ++ offset ;
    }
    rowstart += c1.du_cols * c1.h_sampling * JpegSampleWidth ;
  }
  return ;
}

//
//  Description:
//
//    Progressive Only
//
//    This function decodes the DC coefficient for a data unit in the first
//    DC scan for the component.
//
//    According to G.2 "In order to avoid repetition, detail flow diagrams
//    of progressive decoder operation are not included. Decoder operation is
//    defined by reversing the function of each stop described in the encoder
//    flow charts, and performing the steps in reverse order."
//
//  Parameters:
//    decoder:  The JPEG decoder
//    row:  The data unit row
//    col:  The data unit column
//    ssa:  Successive Approximation
//
void JpegDecoderComponent::DecodeDcFirst (JpegDecoder &decoder,
                                          unsigned int row,
                                          unsigned int col,
                                          unsigned int ssa)
{
  // We decode the first DC coeffient that same way as in a sequential
  // scan except for the point transform according to G.1.2.1

  // Section F.2.2.1
  unsigned int count ; // called T in F.2.2.1
  count = dc_table->Decode (decoder) ;
  int bits = decoder.Receive (count) ;
  int diff = Extend (bits, count) ;
  int value = diff + last_dc_value ;
  last_dc_value = value ;
  coefficient_blocks [row * du_cols + col][0][0] = (value << ssa) ;
  return ;
}

//
//  Description:
//
//    Progressive Only
//    
//    This function decodes the DC coefficient for a data unit in refining
//    DC scans for the component.
//
//    According to G.2 "In order to avoid repetition, detail flow diagrams
//    of progressive decoder operation are not included. Decoder operation is
//    defined by reversing the function of each stop described in the encoder
//    flow charts, and performing the steps in reverse order."
//
//  Parameters:
//    decoder:  The JPEG decoder
//    row:  The data unit row
//    col:  The data unit column
//    ssa:  Successive Approximation
//
void JpegDecoderComponent::DecodeDcRefine (JpegDecoder &decoder,
                                           unsigned int row,
                                           unsigned int col,
                                           unsigned int ssa)
{
  // Reversing G.1.2.1
  if (decoder.Receive (1) != 0)
  {
    coefficient_blocks [row * du_cols + col][0][0] |= (1 << ssa) ;
  }
  return ;
}

//
//  Description:
//
//    Progressive Only
//
//    This function decodes the AC coefficients for a data unit in the first
//    AC scans for a spectral range within the component.
//
//    According to G.2 "In order to avoid repetition, detail flow diagrams
//    of progressive decoder operation are not included. Decoder operation is
//    defined by reversing the function of each stop described in the encoder
//    flow charts, and performing the steps in reverse order."
//
//    This function comes from reversing the steps in Figures G.3-G.5.
//
//  Parameters:
//    decoder:  The JPEG decoder
//    row:  The data unit row
//    col:  The data unit column
//    sss:  Spectral Selection Start
//    sse:  Spectral Selection End
//    ssa:  Successive Approximation
//
void JpegDecoderComponent::DecodeAcFirst (JpegDecoder &decoder,
                                          unsigned int row,
                                          unsigned int col,
                                          unsigned int sss,
                                          unsigned int sse,
                                          unsigned int ssa)
{
  JpegDecoderCoefficientBlock *data =
  				&coefficient_blocks [row * du_cols + col] ;
  if (eob_run > 0)
  {
    // If a previous call created a nonzero EOB run then we decrement the
    // counter and return.
    -- eob_run ;
  }
  else
  {
    for (unsigned int kk = sss ; kk <= sse ; )
    {
      // Decode the next value in the input stream.
      UBYTE2 rs = ac_table->Decode (decoder) ;
      UBYTE1 ssss = (UBYTE1) (rs & 0xF) ;
      UBYTE1 rrrr = (UBYTE1) (rs >> 0x4) ;

      if (ssss == 0)
      {
        if (rrrr == 15)
        {
          // A zero value ssss with rrrr == 15 means to skip
          // 16 zero coefficients.
            kk += 16 ;
        }
        else
        {
          // A zero value ssss with rrrr != 15 means to create
          // End of Band run.

          // The EOB run includes the current block. This is why we
          // do no processing for rrrr = 0 and substract one when
          // rrrrr != 0.
          if (rrrr != 0)
          {
            int bits = decoder.Receive (rrrr) ;
            eob_run = (1 << rrrr) + bits - 1 ;
          }
          break ;
        }
      }
      else
      {
        // When ssss != 0, rrrr gives the number of zero elements to skip
        // before the next non-zero coefficient.
        kk += rrrr ;
        if (kk >= JpegSampleSize)
          throw EJpegBadData ("Data out of range") ;

        // Extend the value and store.
        int bits = decoder.Receive (ssss) ;
        int value = Extend (bits, ssss) ;
        (&((*data)[0][0]))[JpegZigZagInputOrder (kk)] = (value << ssa) ;
        ++ kk ;
      }
    }
  }
  return ;
}

//
//  Description:
//
//    Progressive Only
//
//    This function decodes the AC coefficients for a data unit in the
//    refining AC scans for a spectral range within the component.
//
//    According to G.2 "In order to avoid repetition, detail flow diagrams
//    of progressive decoder operation are not included. Decoder operation is
//    defined by reversing the function of each stop described in the encoder
//    flow charts, and performing the steps in reverse order."
//
//    Section G.1.2.3 defines how to encode refining scans for AC
//    coefficients. Unfortunately this section is vague and
//    undecipherable. Reversing an undecipherable process results
//    in something unimaginable. This is a "best-guess" interpretation
//    that seems to work.
//
//    The basic process at work is that zero counts do not include nonzero
//    values. Whenever we skip a value due to zero count or End of Band runs
//    we have to read one bit to refine each non-zero value we skip. The
//    process is ugly and it means that data is encoding out of order.
//
//  Parameters:
//    decoder:  The JPEG decoder
//    row:  The data unit row
//    col:  The data unit column
//    sss:  Spectral Selection Start
//    sse:  Spectral Selection End
//    ssa:  Successive Approximation
//                                                            4
void JpegDecoderComponent::DecodeAcRefine (JpegDecoder &decoder,
                                           unsigned int row,
                                           unsigned int col,
                                           unsigned int sss,
                                           unsigned int sse,
                                           unsigned int ssa)
{
  JpegDecoderCoefficientBlock *data =
  				&coefficient_blocks [row * du_cols + col] ;
  // kk is incremented within the loop.
  for (unsigned int kk = sss ; kk <= sse ;)
  {
    if (eob_run != 0)
    {
      // An EOB run has caused us to skip entire data units. We need
      // to refine any previously non-zero coefficients.
      // Notice that we do not initialize kk here. We could be using
      // an EOB run to skip all the remaining coefficients in the current
      // one.

      for ( ; kk <= sse ; ++ kk)
      {
        if ((&((*data)[0][0]))[JpegZigZagInputOrder (kk)] != 0)
        {
          decoder.RefineAcCoefficient (
                      (&((*data)[0][0]))[JpegZigZagInputOrder (kk)],
                      ssa) ;
        }
      }
      -- eob_run ;
    }
    else
    {
      UBYTE2 rs = ac_table->Decode (decoder) ;
      UBYTE1 ssss = (UBYTE1) (rs & 0xF) ;
      UBYTE1 rrrr = (UBYTE1) (rs >> 0x4) ;

      if (ssss == 0)
      {
        // ssss == 0 means that we either have an EOB run or we need to
        // 16 non-zero coefficients.

        if (rrrr == 15)
        {
          // ssss == 0 and rrrr == 15 => Skip over 16 zero coefficients
          for (unsigned int ii = 0 ;
              kk <= sse
              && (ii < 16 
                  || (&((*data)[0][0]))[JpegZigZagInputOrder (kk)] != 0) ;
              ++ kk)
          {
            if (kk >  sse)
              throw EJpegBadData ("Corrupt Scan Data") ;

            if ((&((*data)[0][0]))[JpegZigZagInputOrder (kk)] != 0)
            {
              decoder.RefineAcCoefficient (
                            (&((*data)[0][0]))[JpegZigZagInputOrder (kk)],
                            ssa) ;
            }
            else
            {
              ++ ii ;
            }
          }
        }
        else
        {
          // We are reading an EOB run.
          if (rrrr == 0)
          {
            eob_run = 1 ;
          }
          else
          {
            int bits = decoder.Receive (rrrr) ;
            eob_run = (1 << rrrr) + bits ;
          }
        }
      }
      else if (ssss == 1)
      {
        // ssss == 1 means that we are creating a new non-zero
        // coefficient. rrrr gives the number of zero coefficients to
        // skip before we reach this one.
        // Save the value for the new coefficient. Unfortunately the data
        // is stored out of order.
        int newvalue = decoder.Receive (1) ;

        // Skip the zero coefficients.
        for (unsigned int zerocount = 0 ;
             kk <  JpegSampleSize
              && (zerocount < rrrr
                  || (&((*data)[0][0]))[JpegZigZagInputOrder (kk)] != 0) ;
             ++ kk)
        {
          if (kk >  sse)
            throw EJpegBadData ("Error in progressive scan") ;

          if ((&((*data)[0][0]))[JpegZigZagInputOrder (kk)] != 0)
          {
            decoder.RefineAcCoefficient ((&((*data)[0][0]))[JpegZigZagInputOrder (kk)],
                                 ssa) ;
          }
          else
          {
            ++ zerocount ;
          }
        }

        if (kk >  sse)
          throw EJpegBadData ("Error in progressive scan") ;

        if (newvalue)
        {
          (&((*data)[0][0]))[JpegZigZagInputOrder (kk)] = (1 << ssa) ;
        }
        else
        {
          (&((*data)[0][0]))[JpegZigZagInputOrder (kk)] = (-1 << ssa) ;
        }
        ++ kk ;
      }
      else
      {
        // The value of SSSS must be zero or one. Since we add data one
        // bit at at a time data values larger than 1 make no sense.
        throw EJpegBadData ("Invalid value in input stream") ;
      }
    }
  }
  return ;
}

//
//  Description:
//
//    Progressive Only
//
//    This function performs the IDCT on all the data in
//    coefficient_blocks and stores the result in data_units.
//
//    This function gets called whenever the image data is written
//    to the image.  For a sequential image the IDCT only needs to
//    performed once no matter how often the data gets updated in the
//    image but to continuously update the progressive image data
//    an update after each scan gives new results.
//
void JpegDecoderComponent::ProgressiveInverseDct ()
{
  // If UpdateImage gets called before the image is completely
  // decoded the these values may be NULL.
  if (data_units == NULL || coefficient_blocks == NULL)
    return ;

  unsigned int limit = du_cols * du_rows ;
  for (unsigned int ii = 0 ; ii < limit ; ++ ii)
  {
    data_units [ii].InverseDCT (coefficient_blocks [ii], *quantization_table) ;
  }
  return ;
}


