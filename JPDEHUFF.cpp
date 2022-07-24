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
// JPEG Decoder Huffman Table Class Implementation
//
// Author:  John M. Miano miano@colosseumbuilders.com
//


#include "jpdehuff.h"
#include "jpegdeco.h"
#include "jpgexcep.h"

using namespace std ;

//
//  Description:
//
//    Class defualt constructor
//

JpegHuffmanDecoder::JpegHuffmanDecoder ()
{
  table_defined = false ;
  return ;
}

//
//  Description:
//
//    This function reads a Huffman table from the input stream. A DHT
//    marker can define more than one Huffman table. This function reads
//    just one of those tables.
//
//  Parameters:
//    decoder:  The JPEG decoder that owns the Huffman table
//
//  Return Value:
//    The size of the Huffman table in the input stream
//    (the number of bytes read).
//
unsigned int JpegHuffmanDecoder::ReadTable (JpegDecoder &decoder)
{
  // We declare this here because MSVC++ does not handle for
  // statement scoping rules correctly.
  unsigned int jj ;

  // B.2.4.2
  UBYTE1 huffbits [JpegMaxHuffmanCodeLength] ;

  unsigned int count = 0 ; // Count of codes in the Huffman table.

  // Read the 16 1-byte length counts and count the number of
  // codes in the table.
  for (jj = 0 ; jj < JpegMaxHuffmanCodeLength ; ++ jj)
  {
    // These values are called Li in the standard.
    huffbits [jj] = decoder.ReadByte () ;
    count += huffbits [jj] ;
  }
  if (count > JpegMaxNumberOfHuffmanCodes)
     throw EJpegBadData ("Huffman count > 256") ;

  // Read the Huffman values.
  for (jj = 0 ; jj < count ; ++ jj)
  {
    // These values are called Vi in the standard.
    huff_values [jj] = decoder.ReadByte () ;
  }

  // Generate the Structures for Huffman Decoding.
  MakeTable (huffbits) ;

  table_defined = true ; // This table can now be used.

  return JpegMaxHuffmanCodeLength + count ;
}

//
//  Description:
//
//    This function generates the data used for Huffman decoding.
//
//    The implicit outputs are the member variables mincode [n],
//     maxcode [n] and valptr [n]. These are the minimum Huffman Code of
//    length n+1, the maximum Huffman Code of length n+1, and the index
//    into huff_values [] for the first value with a Huffman code of length
//    n+1.
//
//  Parameters:
//    huffibts: The count of Huffman codes of length n+1) 
//
void JpegHuffmanDecoder::MakeTable (UBYTE1 huffbits [JpegMaxHuffmanCodeLength])
{
  // We have to declare the loop indices here because MSVC++ does not
  // handle scoping in for statements correctly.
  unsigned int ii, jj, kk ;

  // These values in these arrays correspond to the elements of the
  // "values" array. The Huffman code for values [N] is huffcodes [N]
  // and the length of the code is huffsizes [N].

  UBYTE2 huffcodes [JpegMaxNumberOfHuffmanCodes] ;
  unsigned int huffsizes [JpegMaxNumberOfHuffmanCodes + 1] ;

  // Section C.2 Figure C.1
  // Convert the array "huff_bits" containing the count of codes
  // for each length 1..16 into an array containing the length for each
  // code.
  for (ii = 0, kk = 0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
     for (int jj = 0 ; jj < huffbits [ii] ; ++ jj)
     {
        huffsizes [kk] = ii + 1 ;
        ++ kk ;
     }
     huffsizes [kk] = 0 ;
  }

  // Section C.2 Figure C.2
  // Calculate the Huffman code for each Huffman value.
  UBYTE2 code = 0 ;
  unsigned int si ;
  for (kk = 0, si = huffsizes [0] ;
       huffsizes [kk] != 0  ;
       ++ si, code <<= 1)
  {
     for ( ; huffsizes [kk] == si ; ++ code, ++ kk)
     {
        huffcodes [kk] = code ;
     }
  }

  // Section F.2.2. Figure F.15
  // Create three arrays.
  // mincode [n] : The smallest Huffman code of length n + 1.
  // maxcode [n] : The largest Huffman code of length n + 1.
  // valptr [n] : Index into the values array. First value with a code
  //                    of length n + 1.
  for (ii=0, jj=0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
     // ii is the index into Huffman code lengths
     // jj is the index into Huffman code values
     if (huffbits [ii] != 0)
     {
        // The jj'th Huffman value is the first with a Huffman code
        // of length ii.
        valptr [ii] = jj ;
        mincode [ii] = huffcodes [jj] ;
        jj += huffbits [ii] ;
        maxcode [ii] = huffcodes [jj - 1] ;
     }
     else
     {
        // There are no Huffman codes of length (ii + 1).
        maxcode [ii] = -1 ;
        // An illegal value > maxcode[]
        mincode [ii] = JpegMaxNumberOfHuffmanCodes + 1 ;
        valptr [ii] = 0 ;
     }
  }
  return ;
}

//
//  Description:
//
//    This function decodes the next Huffman-encoded value in the input
//    stream.
//
//  Parameters:
//    decoder:  The JPEG decoder that owns the Huffman table.
//
int JpegHuffmanDecoder::Decode (JpegDecoder &decoder)
{
   // This function decodes the next byte in the input stream using this
   // Huffman table.

   // Section A F.2.2.3 Figure F.16

   UBYTE2 code = decoder.NextBit () ;
   int codelength ; // Called I in the standard.

   // Here we are taking advantage of the fact that 1 bits are used as
   // a prefix to the longer codes.
   for (codelength = 0 ;
        (code > maxcode [codelength] && codelength <
         JpegMaxHuffmanCodeLength) ;
        ++ codelength)
   {
      code = (UBYTE2) ((code << 1) | decoder.NextBit ()) ;
   }

   if (codelength >= JpegMaxHuffmanCodeLength)
    throw EJpegBadData ("Bad Huffman Code Length") ;

   // Now we have a Huffman code of length (codelength + 1) that
   // is somewhere in the range
   // mincode [codelength]..maxcode [codelength].

   // This code is the (offset + 1)'th code of (codelength + 1) ;
   int offset = code - mincode [codelength] ;

   // valptr [codelength] is the first code of length (codelength + 1)
   // so now we can look up the value for the Huffman code in the table.
   int index = valptr [codelength] + offset ;
   return huff_values [index] ;
}

//
//  Description:
//
//    This is a debugging function for writing the contents of the Huffman
//    table to a stream.
//
//  Parameters:
//    strm:  The output stream
//
void JpegHuffmanDecoder::Dump (FILE *strm) const
{
/*  // We have the loop index here because MSVC++ does not handle
  // scoping in for statements correctly.

  unsigned int ii ;

  int count ;
  for (ii = 0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
    if (valptr [ii] < 255)
      count = valptr [ii] ;
  }

  strm << "   Code Values: " ;
  for (ii = 0 ; ii < count ; ++ ii)
    strm << dec << ii << hex << (UBYTE2) huff_values [ii] << " " ;
  strm << endl ;
  strm << "Length" << "\t\t" << "Mincode" << "\t\t"
       << "Maxcode" << "\t\t" << "Valptr" << endl ;
  strm << "-------------------------------------------------------" << endl ;
  for (ii = 0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
    strm << dec << ii << "\t\t" << (int) mincode [ii] << "\t\t"
         << (int)maxcode [ii] << "\t\t"<< (int) valptr [ii] << endl ;
  }
  return ;*/
}

//
//  Description:
//
//   This function tells if the Huffman table has been defined
//   by the JPEG input stream.  It is used to detect corrupt
//   streams that have scans that use a Huffman table before
//   it has been defined.
//
//  Return Value:
//
//    true => The table has been defind
//    false => The table has not been defined
//

bool JpegHuffmanDecoder::Defined () const
{
  return table_defined ;
}
