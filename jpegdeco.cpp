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
// JPEG Decoder Class Implementation
//
// Author:  John M. Miano miano@colosseumbuilders.com
//

#include "jpegdeco.h"
#include "jfif.h"
#include "jpdehuff.h"
#include "bitimage.h"
#include "jpdequan.h"
#include "jpdecomp.h"

//using namespace std ;

//
//  Description:
//
//    Class Default Constructor
//
JpegDecoder::JpegDecoder ()
{
  Initialize () ;
  return ;
}
//
//  Description:
//
//    Class copy constructor
//
JpegDecoder::JpegDecoder (const JpegDecoder &source)
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
JpegDecoder::~JpegDecoder ()
{
  delete [] ac_tables ; ac_tables = NULL ;
  delete [] dc_tables ; dc_tables = NULL ;

  delete [] quantization_tables ; quantization_tables = NULL ;
  delete [] components ; components = NULL ;
  delete [] component_indices ; component_indices = NULL ;
  delete [] scan_components ; scan_components = NULL ;
  return ;
}
//
//  Description:
//
JpegDecoder &JpegDecoder::operator=(const JpegDecoder &source)
{
  DoCopy (source) ;
  return *this ;
}

//
//  Description:
//
//    Class initialization procudure for use by constructors.
//
void JpegDecoder::Initialize ()
{
  verbose_flag = false ;
  strict_jfif = false ;

  ac_tables = new JpegHuffmanDecoder [JpegMaxHuffmanTables] ;
  dc_tables = new JpegHuffmanDecoder [JpegMaxHuffmanTables] ;

  quantization_tables = new JpegDecoderQuantizationTable [MaxQuantizationTables] ;
  components = new JpegDecoderComponent [JpegMaxComponentsPerFrame] ;
  component_indices = new unsigned int [JpegMaxComponentsPerFrame] ;

  scan_components = new JpegDecoderComponent * [JpegMaxComponentsPerScan] ;
  return ;
}
//
//  Description:
//
//    Copy function. This function is called by the assignment operator
//    and copy constructor to copy the state of one object to another.
//    we only copy user configurable parameters. We do not support
//    copying while a decode operation is in progress.
//
void JpegDecoder::DoCopy (const JpegDecoder &source)
{
  verbose_flag = source.verbose_flag ;
  strict_jfif = source.strict_jfif ;
  BitmapImageCoder::operator=(source) ;
  return ;
}
//
//  Description:
//
//    This function returns the next byte from the input stream.
//
UBYTE1 JpegDecoder::ReadByte ()
{
  UBYTE1 value ;
  fread ((char *) &value, sizeof (value),1,input_stream) ;
  bit_position = 0 ;

  if (feof (input_stream))
    throw EJpegBadData ("Premature end of file") ;
  return value ;
}

//
//  Description:
//
//    This function reads the next 2-byte integer from the input stream
//    and returns the value in the correct format for the system.
//

UBYTE2 JpegDecoder::ReadWord ()
{
  bit_position = 0 ;
  UBYTE2 value ;
  fread ((char *) &value, sizeof (value),1,input_stream) ;
  return BigEndianToSystem (value) ;
}

//
//  Description:
//
//    Section F.2.2.4
//
//    Extracts the next "count" bits from the input stream.
//
int JpegDecoder::Receive (unsigned int count)
{
  int result = 0 ;
  for (unsigned int ii = 0 ; ii < count ; ++ii)
  {
    result <<= 1 ;
    result |= NextBit () ;
  }
  return result ;
}

//
//  Description:
//
//    This function reads the Start of Image Marker and the JFIF APP0
//    marker that begin a JPEG file.
//
//    The JFIF standard states "The JPEG FIF APP0 marker is mandatory
//     right after the SOI marker."
//
//    I have come across some JPEG files that have a COM marker between
//    the SOI marker and APP0. This code will reject these non-conforming
//    files.
//

void JpegDecoder::ReadStreamHeader ()
{
  if (ReadByte () != SOB)
    throw EJpegBadData ("Missing SOI Marker") ;
  if (ReadByte () != SOI)
    throw EJpegBadData ("Missing SOI Marker") ;
  if (ReadByte () != SOB)
    throw EJpegBadData ("Missing JFIF APP0 Marker") ;
  if (ReadByte () != APP0)
    throw EJpegBadData ("Missing JFIF APP0 Marker") ;

  JfifHeader header ;
  fread ((char *) &header, sizeof (header),1,input_stream) ;
  if (memcmp ("JFIF", (char *) header.identifier, 4) != 0)
    throw EJpegBadData ("Not a JFIF file") ;

  if (verbose_flag)
  {
/*    cout << "{ Start Of Image }" << endl ;
    cout << "{ JFIF APP0 Marker" << endl ;
    cout << "  Length: " << dec << BigEndianToSystem (header.length) << endl ;
    cout << "  Version: " << dec << (unsigned int) header.version [0]
         << "." << (unsigned int) header.version [0] << endl ;
    // density unit = 0 => Only the aspect ratio is specified.
    // density unit = 1 => Density in pixels per inch.
    // density unit = 2 => Density in pixels per centimeter.
    cout << "  Density Unit: " ;
    switch (header.units)
    {
    case 0:
      cout << " (aspect ratio)" << endl ;
      break ;
    case 1:
      cout << " (pixels per inch)" << endl ;
      break ;
    case 2:
      cout << " (pixels/cm)" << endl ;
      break ;
    default:
      cout << " (????)" << endl ;
      break ;
    }
    cout << "  X Density: " << dec << BigEndianToSystem (header.xdensity) << endl ;
    cout << "  Y Density: " << dec << BigEndianToSystem (header.xdensity) << endl ;
    cout << "  Thumbnail Width: " << dec << (unsigned int) header.xthumbnail << endl ;
    cout << "  Thumbnail Height: " << dec << (unsigned int) header.xthumbnail << endl ;
    cout << "}" << endl ;*/
  }

  // Skip over any thumbnail data.
  for (int ii = sizeof (header) ; ii < BigEndianToSystem (header.length) ; ++ ii)
    (void) ReadByte () ;

  return ;
}

//
//  Description:
//
//    This function reads the next marker in the input
//    stream. If the marker is followed by a data block
//    this function dispatches a routine to read the
//    data.
//
void JpegDecoder::ReadMarker ()
{
  while (! feof (input_stream))
  {
    UBYTE1 type = ReadByte () ;
    switch (type)
    {
    case SOB:
      // According to E.1.2, 0xFF is allowed as fill when a
      // marker is expected.
      break ;
    case SOI:
//      if (verbose_flag)
//        cout << "{ Start Of Image }" << endl ;
      return ; // SOI has no data.
    case DQT:
      ReadQuantization () ;
      return ;
    case DHP:
      throw EJpegBadData ("DHP marker not supported") ;

    // The only difference between a Sequential DCT Frame
    // (SOF0) and an extended Sequential DCT Frame (SOF1)
    // is that a baseline frame may only have 2 DC and 2 AC
    // Huffman tables per scan (F.1.2) and and extended
    // Sequential Frame may have up to 4 DC and 4 AC Huffman
    // tables per scan (F.1.3). Both are decoded identically
    // for 8-bit precision. Extended Sequential frames may
    // use 12-bit precision (F, Table B.2) which we do not
    // support.
    case SOF0:
    case SOF1:
    case SOF2:
      ReadStartOfFrame (type) ;
      return ;
    case SOF3:
      throw EJpegBadData ("Lossless Huffman Coding Not Supported") ;
    case SOF5:
      throw EJpegBadData (
        "Differential Sequential Huffman Coding Not Supported") ;
    case SOF6:
      throw EJpegBadData (
        "Differential Progressive Huffman Coding Not Supported") ;
    case SOF7:
      throw EJpegBadData (
        "Differential Lossless Huffman Coding Not Supported") ;

    // These are markers for frame using arithmetic coding.
    // Arithmetic coding is covered by patents so we ignore
    // this type.
    case SOF9:
    case SOFA:
    case SOFB:
    case SOFD:
    case SOFE:
    case SOFF:
      throw EJpegBadData (
        "Cannot read image - Arithmetic Coding covered by patents") ;
    case DHT:
      ReadHuffmanTable () ;
      return ;
    case SOS:
      ReadStartOfScan () ;
      return ;
    case DRI:
      ReadRestartInterval () ;
      return ;
    case EOI:
      eoi_found = true ;
//      if (verbose_flag)
//        cout << "{ End Of Image }" << endl ;
      return ; // End of Image marker has no data
    case APP0:
    case APP1:
    case APP2:
    case APP3:
    case APP4:
    case APP5:
    case APP6:
    case APP7:
    case APP8:
    case APP9:
    case APPA:
    case APPB:
    case APPC:
    case APPD:
    case APPE:
    case APPF:
    case COM:
      ReadApplication (type) ;
      return ;
    default:
      // We call ReadByte to make sure the problem
      // is not a premature EOF.
      (void) ReadByte () ; 
      throw EJpegBadData (
      "Unknown, unsupported, or reserved marker encountered") ;
    }
  }
  throw EJpegBadData ("Premature end of file") ;
}

//
//  Description:
//
//    This method reads an application or comment marker
//    from the input stream.
//
//  Parameters:
//    type:  The marker type
//
void JpegDecoder::ReadApplication (UBYTE1 type)
{
  unsigned int length = ReadWord () ;
  char id [512] ;
  int ii = 0 ;

  id [ii] = ReadByte () ;
  for (ii = 1 ; id [ii - 1] != '\000'
                && ii < sizeof (id)
                && ii < length - sizeof (UBYTE2) ; ++ ii)
  {
    id [ii] = ReadByte () ;
  }

  for ( ; ii < length - sizeof (UBYTE2) ; ++ ii)
    (void) ReadByte () ;

  if (verbose_flag)
  {
/*    if (type == COM)
      cout << "( Comment Marker" << endl ;
    else
    cout << "{ APP" << hex << (UBYTE2) (type & 0x0F) << " Marker" << endl ;
    cout << "Length: " << dec << length << endl ;
    cout << "ID: " << id << endl ;
    cout << "}" << endl ;*/
  }
  return ;
}

//
//  Description:
//
//    The function reads a Define Huffman Table marker from the input
//    stream.
//
void JpegDecoder::ReadHuffmanTable ()
{
  // Section B.2.4.2

/*  if (verbose_flag)
    cout << "{ Define Huffman Table" << endl ;*/

  UBYTE2 length = ReadWord () ;
  unsigned int remaining = length - sizeof (length) ;
  while (remaining > 0)
  {
    UBYTE1 data = ReadByte () ; -- remaining ;

    // Tc in standard 0=>DC, 1=>AC
    unsigned int tableclass = data >> 4 ; 
    unsigned int id = data & 0x0F ; // Th in standard
    if (id > 3)
    {
      throw EJpegBadData (
        "Huffman Table Index outside range [0..3]") ;
    }
    if (verbose_flag)
    {
/*      cout << "   Table Index " << (int) id << endl ;
      if (tableclass == 0)
        cout << "   Table Class: DC" << endl ;
      else
        cout << "   Table Class: AC" << endl ;*/
    }
    if (id > 3)
    {
//      cout << "Bad index " << id << endl ;
      return ;
    }

    JpegHuffmanDecoder *table ;
    if (tableclass != 0)
      table = &ac_tables [id] ;
    else
       table = &dc_tables [id] ;

    // Read the table data into the table object
    remaining -= table->ReadTable (*this) ;

    if (verbose_flag)
    {
      table->Dump (stdout) ;
    }
  }

//  if (verbose_flag)
//    cout << "}" << endl ;
  return ;
}

//
//  Description:
//
//    This function reads a DQT marker from the input stream.
//
void JpegDecoder::ReadQuantization ()
{
  // Defined in Section B.2.4.1

  UBYTE2 length = ReadWord () ;
  UBYTE1 data ;

  // Maintain a counter for the number of bytes remaining to be read in
  // the quantization table.
  int remaining = length - sizeof (length) ;

  if (verbose_flag)
  {
/*    cout << "{ Define Quantization Table" << endl ;
    cout << "  Length: " << length << endl ;*/
  }
  while (remaining > 0)
  {
    data = ReadByte () ; -- remaining ;
    unsigned int precision = data >> 4 ;    // Pq in standard
    unsigned int index = data & 0x0F ;      // Tq in standard

    if (index >= MaxQuantizationTables)
      throw EJpegBadData ("Quantization Table Index Too Large") ;

    if (verbose_flag)
    {
/*      cout << "  Table Index: " << dec << index << endl ;
      cout << "  Table Precision: " << dec << precision << endl ;*/
    }

    switch (precision)
    {
    case 1:
      remaining -= sizeof(UBYTE2) * JpegSampleSize ;
      break ;
    case 0:
      remaining -= sizeof (UBYTE1) * JpegSampleSize ;
      break ;
    }

    // Read the table data into the table object
    quantization_tables [index].ReadTable (*this, precision) ;

    if (verbose_flag)
    {
/*      cout << "  Table Values: " ;
      quantization_tables [index].Print (stdout) ;
      cout << endl << "}" << endl ;*/
    }
  }
  return ;
}

//
//  Description:
//
//    This function reads a define restart interval marker
//    from the input stream.
//
void JpegDecoder::ReadRestartInterval ()
{
  // Section B.2.4.4

  UBYTE2 length = ReadWord () ;
  restart_interval = ReadWord () ;
  if (verbose_flag)
  {
/*    cout << "{ Define Restart Interval" << endl ;
    cout << "  Length:  " << dec << length << endl ; // Should be 4
    cout << "  Interval: " << dec << restart_interval << endl ;
    cout << "}" << endl ;*/
   }
   return ;
}

//
//  Description:
//
//    The function reads a start of frame marker from the input stream.
//
//  Parameters:
//    type:  The marker type for the frame
//
void JpegDecoder::ReadStartOfFrame (UBYTE1 type)
{
  if (type == SOF2)
    progressive_frame = true ;
  else
    progressive_frame = false ;

  // Section B.2.2
  // Read in the image dimensions
  unsigned int length = ReadWord () ;
  unsigned int dataprecision = ReadByte () ;  // P in standard
  if (dataprecision != 8)
    throw EJpegBadData ("Only 8-bit data supported") ;

  frame_height = ReadWord () ;            // Y in standard
  frame_width = ReadWord () ;             // X in standard
  component_count = ReadByte () ;   // Nf in standard

  // JFIF only allows 1 or 3 components.
  if (component_count != 1 && component_count != 3)
    throw EJpegBadData ("JFIF only supports 1 and 3 component streams") ;

  frame_type = type ;

  if (verbose_flag)
  {
/*    cout << "{ Start Of Frame " << endl ;
    cout << "  Length: " << dec << length << endl ;
    cout << "  Precision: " << dec << dataprecision << endl ;
    cout << "  Height: " << dec << frame_height << endl ;
    cout << "  Width: " << dec << frame_width << endl ;
    cout << "  Component Count: " << component_count << endl ;*/
  }

  if (length != (component_count * 3 + 8))
    throw EJpegBadData ("Invalid Frame Size") ;

  // Rread the component descriptions
  max_horizontal_frequency = 0 ;
  max_vertical_frequency = 0 ;
  for (unsigned int ii = 0 ; ii < component_count ; ++ ii)
  {
    unsigned int ID = ReadByte () ;  // Ci in standard
    unsigned int qtable ;

    // While JPEG does not put these restrictions on component IDs
    // the JFIF standard does.
    if (strict_jfif)
    {
    if (component_count == 1 && ID != 1)
      throw EJpegBadData ("Component ID not 1") ;
    else if (ID != ii + 1)
      throw EJpegBadData ("Invalid Component ID or ID out of order") ;
    }
    
    component_indices [ii] = ID ;

    UBYTE1 data = ReadByte () ;
    components [ID].HorizontalFrequency (data >> 4) ; // Hi in standard
    components [ID].VerticalFrequency (data & 0xF) ;  // Vi in standard
    qtable= ReadByte () ;  // Tqi in standard
    if (qtable >= MaxQuantizationTables)
      throw EJpegBadData ("Bad Quantization Table Index") ;
    components [ID].SetQuantizationTable (quantization_tables [qtable]) ;

    // Keep track of the largest values for horizontal and vertical
    // frequency.
    if (components [ID].HorizontalFrequency ()
        > max_horizontal_frequency)
    {
      max_horizontal_frequency =
        components [ID].HorizontalFrequency () ;
    }

    if (components [ID].VerticalFrequency ()
          > max_vertical_frequency)
    {
      max_vertical_frequency =
        components [ID].VerticalFrequency () ;
    }

    if (verbose_flag)
    {
/*      cout << "   Component " << ID << endl ;
      cout << "   Horizontal Frequency: "
           << components [ID].HorizontalFrequency () << endl ;
      cout << "   Vertical Frequency: "
           << components [ID].VerticalFrequency () << endl ;
      cout << "   Quantization Table: "
           << qtable << endl ;*/
    }
  }

  CalculateMcuDimensions () ;

  // Allocate storage for the image.
  if (component_count == 1)
  {
    current_image->SetSize (1 << 8, 8, frame_width, frame_height) ;
    // For a grey scale image we need to create the color map. Each color
    // has equal amounts of red, green, and blue.
    for (unsigned int ii = 0 ; ii < (1 << 8) ; ++ ii)
    {
      current_image->ColorMap (ii).red = (UBYTE1) ii ;
      current_image->ColorMap (ii).green = (UBYTE1) ii ;
      current_image->ColorMap (ii).blue = (UBYTE1) ii ;
    }
  }
  else
  {
    current_image->SetSize (0,     // Color Table Entries
                            8 * component_count,  // Bits
                            frame_width,
                            frame_height) ;

  }

//  if (verbose_flag)
//    cout << "}" << endl ;

  sof_found = true ;
  return ;
}

//
//  Description:
//
//    This function reads a start of scan marker and the scan data
//    following the marker.
//
void JpegDecoder::ReadStartOfScan ()
{
  unsigned int ii ;

  if (! sof_found)
    throw EJpegBadData ("Scan found before frame defined") ;

  // Section B.2.3

  UBYTE2 length = ReadWord () ;
  if (verbose_flag)
  {
/*    cout << "{ Start Of Scan " << endl ;
    cout << "  Length:  " << dec << length << endl ;*/
  }

  scan_component_count = ReadByte () ;  // Ns in standard
  // Right now we can only handle up to three components.
  if (scan_component_count > 3 || scan_component_count < 1)
    throw EJpegBadData ("Invalid component count in scan") ;

  for (ii = 0 ; ii < scan_component_count ; ++ ii)
  {
    UBYTE1 componentID = ReadByte () ;  // Csi in standard

    scan_components [ii] = &components [componentID] ;
    // If the horizontal frequency is zero then the component was not
    // defined in the SOFx marker.
    if (scan_components [ii]->HorizontalFrequency () == 0)
      throw EJpegBadData ("Component Not Defined") ;

    UBYTE1 rb = ReadByte () ;
    unsigned int actable = rb & 0x0F ;
    unsigned int dctable = rb >> 4 ;

    scan_components [ii]->SetHuffmanTables (
                              dc_tables [dctable],
                              ac_tables [actable]) ;
    if (verbose_flag)
    {
/*      cout << "  Component ID: "
           << dec << (unsigned int) componentID << endl ;
      cout << "  DC Entropy Table: "
           << dec << dctable << endl  ;
      cout << "  AC Entropy Table: "
           << dec << actable << endl  ;*/
    }
  }

  unsigned int spectralselectionstart = ReadByte () ; // Ss in standard
  unsigned int spectralselectionend = ReadByte ()  ;  // Se in standard

  UBYTE1 ssa = ReadByte () ;
  unsigned int successiveapproximationhigh = ssa >> 4 ;  // Ah in standard
  unsigned int successiveapproximationlow = ssa & 0x0F ; // Al in standard

  if (verbose_flag)
  {
/*    cout << " Spectral Selection Start: "
         << dec << (unsigned int) spectralselectionstart << endl ;
    cout << " Spectral Selection End: "
         << dec << (unsigned int) spectralselectionend << endl ;
    cout << " Successive Approximation High: "
         << dec << successiveapproximationhigh << endl ;
    cout << " Successive Approximation Low: "
         << dec << successiveapproximationlow << endl  ;
    cout << "}" << endl ;*/
  }

  for (ii = 0 ; ii < scan_component_count ; ++ ii)
  {
    if (progressive_frame)
    {
      scan_components [ii]->CheckQuantizationTable () ;
      if (spectralselectionstart == 0)
        scan_components [ii]->CheckDcTable () ;
      else
        scan_components [ii]->CheckAcTable () ;
    }
    else
    {
      scan_components [ii]->CheckQuantizationTable () ;
      scan_components [ii]->CheckDcTable () ;
      scan_components [ii]->CheckAcTable () ;
    }
    scan_components [ii]->AllocateComponentBuffers (*this) ;

  }

  ++ current_scan ;
  if (progressive_frame)
  {
    ReadProgressiveScanData (
                          spectralselectionstart,
                          spectralselectionend,
                          successiveapproximationhigh,
                          successiveapproximationlow) ;
  }
  else
  {
    ReadSequentialScanData () ;
  }

  CallProgressFunction (100) ;
  return ;
}

//
//  Description:
//
//    This function determines for non-interlaced scans:
//
//     o The dimensions in pixels for an MCU
//     o The number of MCU rows and columns needed to encode the scan.
//
void JpegDecoder::CalculateMcuDimensions ()
{
  mcu_height = max_vertical_frequency * JpegSampleWidth ;
  mcu_width = max_horizontal_frequency * JpegSampleWidth ;
  mcu_rows = (frame_height + mcu_height - 1) / mcu_height ;
  mcu_cols = (frame_width + mcu_width - 1) / mcu_width ;
  return ;
}

//
//  Dimensions:
//
//    This function calls the progress function if it has
//    been supplied by the user.
//
//  Parameters:
//    progress: The progress percentage.
//
void JpegDecoder::CallProgressFunction (unsigned int progress)
{
  if (progress_function == NULL)
    return ;

  bool abort = false ;
  unsigned int percent = progress ;
  if (percent > 100)
    percent = 100 ;
  if (progress_function != NULL)
  {
    progress_function (*this,
                       progress_data,
                       current_scan,
                       scan_count,
                       percent,
                       abort) ;
  }

  if (abort)
    throw EJpegAbort () ;
  return ;
}

//
//  Description:
//
//    This function reads the scan data for progressive scans.
//
//    All we do here is determine if we are processing a DC
//    scan (sss==sse==0) or AC scan and if we are processing
//    the first scan for the spectral selection (sah==0) or
//    subsequent scan.
//
//  Parameters:
//    sss: Spectral Selection Start (0..63)
//    sse: Spectral Selection End (sse..63)
//    sah: Successive Approximation High
//    sal: Successive Approximation Low
//
void JpegDecoder::ReadProgressiveScanData (unsigned int sss, unsigned int sse,
                                           unsigned int sah, unsigned int sal)
{
  if (sss == 0)
  {
    if (sse != 0)
      throw EJpegBadData ("Progressive scan contains DC and AC data") ;

    if (sah == 0)
    {
      ReadDcFirst (sal) ;
    }
    else
    {
      ReadDcRefine (sal) ;
    }
  }
  else
  {
    if (sah == 0)
    {
      ReadAcFirst (sss, sse, sal) ;
    }
    else
    {
      ReadAcRefine (sss, sse, sal) ;
    }
  }
  return ;
}

//
//  Description:
//
//    This funtion reads the scan data for the first DC scan for
//    one or more components.
//
//  Parameters:
//
//    ssa:  Successive Approximation
//
void JpegDecoder::ReadDcFirst (unsigned int ssa)
{
  ResetDcDifferences () ;
  unsigned int restartcount = 0 ;

  if (ScanIsInterleaved ())
  {
    for (unsigned int mcurow = 0 ;
         mcurow < mcu_rows ;
         ++ mcurow)
    {
      CallProgressFunction (mcurow * 100 / mcu_rows) ;
      for (unsigned int mcucol = 0 ;
           mcucol < mcu_cols ;
           ++ mcucol, ++restartcount)
      {
        if (restart_interval != 0
            && restart_interval == restartcount)
        {
          ResetDcDifferences () ;
          ProcessRestartMarker () ;
          restartcount = 0 ;
        }
        for (unsigned int cc = 0 ; cc < scan_component_count ; ++ cc)
        {
          for (unsigned int cy = 0 ;
               cy < scan_components [cc]->VerticalFrequency () ;
               ++ cy)
          {
            unsigned int durow = scan_components [cc]->VerticalFrequency ()
                                 * mcurow + cy ;
            for (unsigned int cx = 0 ;
                 cx < scan_components [cc]->HorizontalFrequency () ;
                 ++ cx)
            {
              unsigned int ducol = scan_components [cc]->HorizontalFrequency ()
                                   * mcucol + cx ;

              scan_components [cc]->DecodeDcFirst (*this, durow, ducol, ssa) ;
            }
          }
        }
      }
    }
  }
  else
  {
    for (unsigned int row = 0 ;
         row < scan_components [0]->NoninterleavedRows () ;
         ++ row)
    {
      CallProgressFunction (row * 100 / scan_components [0]->NoninterleavedRows ()) ;
      for (unsigned int col = 0 ;
           col < scan_components [0]->NoninterleavedCols () ;
           ++ col, ++restartcount)
      {
        if (restart_interval != 0 && restart_interval == restartcount)
        {
          ResetDcDifferences () ;
          ProcessRestartMarker () ;
          restartcount = 0 ;
        }
        scan_components [0]->DecodeDcFirst (*this, row, col, ssa) ;
      }
    }
  }
  return ;
}

//
//  Description:
//
//    This function reads the scan data for a refining DC scan.
//
//  Parameters:
//    ssa:  The successive approximation value for this scan.
//
void JpegDecoder::ReadDcRefine (unsigned int ssa)
{
  ResetDcDifferences () ;
  unsigned int restartcount = 0 ;

  if (ScanIsInterleaved ())
  {
    for (unsigned int mcurow = 0 ; mcurow < mcu_rows ; ++ mcurow)
    {
      CallProgressFunction (mcurow * 100 / mcu_rows) ;
      for (unsigned int mcucol = 0 ;
           mcucol < mcu_cols ;
           ++ mcucol, ++restartcount)
      {
        if (restart_interval != 0 && restart_interval == restartcount)
        {
          ResetDcDifferences () ;
          ProcessRestartMarker () ;
          restartcount = 0 ;
        }
        for (unsigned int cc = 0 ; cc < scan_component_count ; ++ cc)
        {
          for (unsigned int cy = 0 ;
               cy < scan_components [cc]->VerticalFrequency () ;
               ++ cy)
          {
            unsigned int durow = scan_components [cc]->VerticalFrequency ()
                                 * mcurow + cy ;
            for (unsigned int cx = 0 ;
                 cx < scan_components [cc]->HorizontalFrequency () ;
                 ++ cx)
            {
              unsigned int ducol = scan_components [cc]->HorizontalFrequency ()
                                   * mcucol + cx ;

              scan_components [cc]->DecodeDcRefine (*this, durow, ducol, ssa) ;
            }
          }
        }
      }
    }
  }
  else
  {
    for (unsigned int row = 0 ;
         row < scan_components [0]->NoninterleavedRows () ;
         ++ row)
    {
      CallProgressFunction (row * 100 / scan_components [0]->NoninterleavedRows ()) ;
      for (unsigned int col = 0 ;
           col < scan_components [0]->NoninterleavedCols () ;
           ++ col, ++restartcount)
      {
        if (restart_interval != 0 && restart_interval == restartcount)
        {
          ResetDcDifferences () ;
          ProcessRestartMarker () ;
          restartcount = 0 ;
        }
        scan_components [0]->DecodeDcRefine (*this, row, col, ssa) ;
      }
    }
  }
  return ;
}

//
//  Description:
//
//    This function reads the scan data for the first AC scan for a
//    component. Progressive scans that read AC data cannot be
//    interleaved.
//
//  Parameters:
//    sss:  Spectral Selection Start
//    sse:  Spectral Selection End
//    ssa:  Spectral Selection
//

void JpegDecoder::ReadAcFirst (unsigned int sss,
                               unsigned int sse,
                               unsigned int ssa)
{
  ResetDcDifferences () ;

  unsigned int restartcount = 0 ;
  for (unsigned int row = 0 ;
       row < scan_components [0]->NoninterleavedRows () ;
       ++ row)
  {
    CallProgressFunction (row * 100 / scan_components [0]->NoninterleavedRows ()) ;
    for (unsigned int col = 0 ;
         col < scan_components [0]->NoninterleavedCols () ;
         ++ col, ++restartcount)
    {
      if (restart_interval != 0 && restart_interval == restartcount)
      {
        ResetDcDifferences () ;
        ProcessRestartMarker () ;
        restartcount = 0 ;
      }
      scan_components [0]->DecodeAcFirst (*this,
                                          row, col,
                                          sss, sse,
                                          ssa) ;
    }
  }
  return ;
}

//
//  Description:
//
//    This function reads the scan data for a refining AC scan for a
//    component. Progressive scans that read AC data cannot be
//    interleaved.
//
//  Parameters:
//    sss:  Spectral Selection Start
//    sse:  Spectral Selection End
//    ssa:  Spectral Selection
//

void JpegDecoder::ReadAcRefine (unsigned int sss,
                                unsigned int sse,
                                unsigned int ssa)
{
  ResetDcDifferences () ;

  unsigned int restartcount = 0 ;
  for (unsigned int row = 0 ;
       row < scan_components [0]->NoninterleavedRows () ;
       ++ row)
  {
    CallProgressFunction (row * 100 / scan_components [0]->NoninterleavedRows ()) ;
    for (unsigned int col = 0 ;
         col < scan_components [0]->NoninterleavedCols () ;
         ++ col, ++restartcount)
    {
      if (restart_interval != 0 && restart_interval == restartcount)
      {
        ResetDcDifferences () ;
        ProcessRestartMarker () ;
        restartcount = 0 ;
      }
      scan_components [0]->DecodeAcRefine (*this,
                                           row, col,
                                           sss, sse,
                                           ssa) ;
    }
  }
  return ;
}

//
//  Parameters:
//
//    The function reads the scan data for a sequential scan. All
//    we do here is determine whether or not we have an interleaved
//    or non-interleaved scan then call a function that handles
//    the scan type.

void JpegDecoder::ReadSequentialScanData ()
{
  expected_restart = 0 ;
  if (ScanIsInterleaved ())
  {
    ReadSequentialInterleavedScan () ;
  }
  else
  {
    ReadSequentialNonInterleavedScan () ;
  }
  return ;
}

//
//  Description:
//
//    This function reads the scan data for an interleaved scan.
//

void JpegDecoder::ReadSequentialInterleavedScan ()
{
  ResetDcDifferences () ;

  unsigned int restartcount = 0 ;
  for (unsigned int mcurow = 0 ; mcurow < mcu_rows ; ++ mcurow)
  {
    CallProgressFunction (mcurow * 100 / mcu_rows) ;
    for (unsigned int mcucol = 0 ; mcucol < mcu_cols ;
         ++ mcucol, ++restartcount)
    {
      if (restart_interval != 0 && restart_interval == restartcount)
      {
        ProcessRestartMarker () ;
        restartcount = 0 ;
      }
      for (unsigned int cc = 0 ; cc < scan_component_count ; ++ cc)
      {
        for (unsigned int cy = 0 ;
             cy < scan_components [cc]->VerticalFrequency () ;
             ++ cy)
        {
          unsigned int durow = scan_components [cc]->VerticalFrequency ()
                               * mcurow + cy ;
          for (unsigned int cx = 0 ;
               cx < scan_components [cc]->HorizontalFrequency () ;
               ++ cx)
          {
            unsigned int ducol =
                   scan_components [cc]->HorizontalFrequency ()
                   * mcucol + cx ;

            scan_components [cc]->DecodeSequential (
                                      *this,
                                      durow,
                                      ducol) ;
          }
        }
      }
    }
  }
  return ;
}

//
//  Description:
//
//    This function reads the scan data for a non-interleaved scan.
//

void JpegDecoder::ReadSequentialNonInterleavedScan ()
{
  ResetDcDifferences () ;

  unsigned int restartcount = 0 ;
  for (unsigned int row = 0 ;
       row < scan_components [0]->NoninterleavedRows () ;
       ++ row)
  {
    CallProgressFunction (row * 100 / scan_components [0]->NoninterleavedRows ()) ;
    for (unsigned int col = 0 ;
         col < scan_components [0]->NoninterleavedCols () ;
         ++ col, ++ restartcount)
    {
      if (restart_interval != 0
          && restart_interval == restartcount)
      {
        ProcessRestartMarker () ;
        restartcount = 0 ;
      }
      scan_components [0]->DecodeSequential (*this, row, col) ;
    }
  }
  return ;
}

//
//  Description:
//
//    This function resets the DC difference values for all components
//    for the current scan.
//
//    This function gets called before each scan is processed and
//    whenever a restart marker is read.
//

void JpegDecoder::ResetDcDifferences ()
{
  for (unsigned int ii = 0 ; ii < scan_component_count ; ++ ii)
    scan_components [ii]->ResetDcDifference () ;
  return ;
}

//
//  Description:
//
//    This function reads a restart marker from the input stream.
//    It gets called byte functions that read scan data whenever
//    a restart marker is expected. An exception is raise if the
//    correct restart marker is not next in the input stream.
//
void JpegDecoder::ProcessRestartMarker ()
{
  UBYTE1 data = ReadByte () ;
  if (data != 0xFF)
    throw EJpegBadData ("Missing Restart Marker") ;
  // According to E.1.2 0xFF can be used as a fill character
  // before the marker.
  while (data == 0xFF)
    data = ReadByte () ;
  if (data < RST0 || data > RST7)
    throw EJpegBadData ("Missing Restart Marker") ;

  // Restart markers RST0..RST7 should come in sequence.
  if ((0x0F & data) != expected_restart)
    throw EJpegBadData ("Incorrect Restart Marker") ;

  // Move the counter to the next restart marker
  ++ expected_restart ;
  expected_restart %= 8 ;

  // Reset the DC coefficent differences to zero.
  ResetDcDifferences () ;
  return ;
}

//
//  Description:
//
//    This function reads an image from a JPEG stream. The
//    stream needs to have been opened in binary mode.
//
//  Parameters:
//    istrm: Input stream
//    image: The output image
//

void JpegDecoder::ReadImage (FILE *istrm,
                             BitmapImage &image)
{
  unsigned char data ;

  current_scan = 0 ;
  scan_count = 0 ;
  input_stream = istrm ;
  current_image = &image ;

  if (progress_function != NULL)
    GetScanCount () ;

  restart_interval = 0 ;  // Clear the restart interval ;
  try
  {
    current_image->Clear () ;
    eoi_found = false ;
    sof_found = false ;

    // Read the required SOI and APP0 markers at the start of the image.
    ReadStreamHeader () ;

    data = ReadByte () ;
    while (! feof (input_stream) && ! eoi_found)
    {
      if (data == SOB)
      {
        ReadMarker () ;
        if (eoi_found)
          break ;
      }
      data = ReadByte () ;
      if (feof (input_stream))
        throw EJpegBadData ("Premature end of file") ;
    }
  }
  catch (EJpegAbort)
  {
    FreeAllocatedResources () ;
    current_image = NULL ;
  }
  catch (...)
  {
    UpdateImage () ;
    FreeAllocatedResources () ;
    current_image = NULL ;
    throw ;
  }
  UpdateImage () ;

  if (! eoi_found)
  {
    throw EJpegBadData("End of Image Marker Not Found") ;
  }
  // We do no want an exception so do not call ReadByte ()
  // Sometimes there can be trailing end of record markers.
  fread ((char *) &data, sizeof (data),1,input_stream) ;
  while ((data == '\r' || data == '\n') && ! feof (input_stream))
    fread ((char *) &data, sizeof (data),1,input_stream) ;
  if (! feof (input_stream))
  {
    throw EJpegBadData ("Extra Data After End of Image Marker") ;
  }

  FreeAllocatedResources () ;
  current_image = NULL ;
  return ;
}

//
//  Description:
//
//    This function scans a stream and counts the number of scans.  This
//    allows an exact count for a progress function.
//
void JpegDecoder::GetScanCount ()
{
  // Save the stream position so we can go back
  // when we are finished.
//  long startpos = input_stream->tellg () ;

  // Count the number of SOS markers.
  scan_count = 1 ;
/*  while (! input_stream->eof ())
  {
    UBYTE1 data ;
    input_stream->read ((char *) &data, 1) ;
    if (data == SOB)
    {
      while (data == SOB)
      {
        input_stream->read ((char *) &data, 1) ;
      }
      if (data == SOS)
        ++ scan_count ;
      else if (data == EOI)
        break ;
    }
  }
  // Go back to where we were in the stream.
  input_stream->seekg (startpos) ;
  input_stream->clear () ;  // Clear the EOF flag.
  return ;*/
}

//
//  Description:
//
//    This function writes the image data that has been read so
//    far to the image. This function gets called after reading
//    the entire image stream.  The user can also call this function
//    from a progress function to display progressive images,
//    multi-scan sequential images, or just to display the image
//    as it is being read (slow).
//

void JpegDecoder::UpdateImage ()
{
  if (current_image == NULL)
    throw EJpegError ("Not reading an image") ;

  if (progressive_frame)
  {
    for (unsigned int ii = 0 ; ii < component_count ; ++ ii)
    {
      components [component_indices [ii]].ProgressiveInverseDct () ;
      components [component_indices [ii]].Upsample () ;
    }
  }
  else
  {
    for (int ii = 0 ; ii < component_count ; ++ ii)
    {
      components [component_indices [ii]].Upsample () ;
    }
  }

  switch (component_count)
  {
  case 3:
    JpegDecoderComponent::RGBConvert (components [component_indices [0]],
                                      components [component_indices [1]],
                                      components [component_indices [2]],
                                      *current_image) ;
    break ;
  case 1:
    JpegDecoderComponent::GrayscaleConvert (
                             components [component_indices [0]],
                             *current_image) ;
    break ;
  }
  return ;
}

//
//  Description:
//
//    This function frees all the memory dynamically allocated
//    during the image decoding process.  
//
void JpegDecoder::FreeAllocatedResources ()
{
  for (unsigned int ii = 0 ; ii < component_count ; ++ ii)
  {
    components [component_indices [ii]].FreeComponentBuffers () ;
  }
  return ;
}

//
//  Description:
//
//    This function is only used in progressive images. It refines
//    a non-zero AC coefficient. It is called at various times while
//    processing a progressive scan that refines AC coefficients.
//
//  Parameters:
//    value: (in/out) The value to refine
//    ssa: The succesive approximation for the scan.
//    decoder: The JPEG decoder
//

void JpegDecoder::RefineAcCoefficient (BYTE2 &value, unsigned int ssa)
{
  // Section G.1.2.3
  if (value > 0)
  {
    if (Receive (1) != 0)
    {
      value += (1 << ssa) ;
    }
  }
  else if (value < 0)
  {
    if (Receive (1) != 0)
    {
      value += (-1 << ssa) ;
    }
  }
  return ;
}

//
//  Description:
//
//    This function returns the next raw bit in the input stream.
//
//    Bits are read from high order to low.
//
//  Return Value:
//
//    The next bit (0, 1)
//

// This function returns the next bit in the input stream.
int JpegDecoder::NextBit ()
{
  // Section F.2.2.5 Figure F.18.
  // CNT is called bitposition
  // B is called bitdata
  if (bit_position == 0)
  {
    // We are out of data so read the next byte from the input stream.
    fread ((char *) &bit_data, sizeof (bit_data),1,input_stream) ;
    if (feof (input_stream))
      throw EJpegBadData ("Premature end of file") ;
    // Reset the bit read position starting with the highest order bit. We
    // read high to low.
    bit_position = CHAR_BIT * sizeof (bit_data) ;

    if (bit_data == 0xFF)
    {
      // 0xFF could start a marker. The sequence 0xFF, 0x00 is used to
      // to represent the value 0xFF. The only other marker that is legal
      // at this point is a DNL marker.
      UBYTE1 b2 ;
      fread ((char *) &b2, 1,1,input_stream) ;
      if (feof (input_stream))
        throw EJpegBadData ("Premature end of file") ;
      if (b2 != 0)
      {
        if (b2 == DNL)
        {
          // DNL markers should not occur within the supported frame types.
          throw EJpegBadData ("Unexpected Marker DNL") ;
        }
        else
        {
          throw EJpegBadData ("Unexpected Marker") ;
        }
      }
    }
  }

  // Consume one bit of the input.
  -- bit_position ;
  // Shift the value to the low order bit position.
  UBYTE1 result = (UBYTE1) ((bit_data >> bit_position) & 1) ;
  return result ;
}

