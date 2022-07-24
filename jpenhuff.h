#ifndef __JPENHUFF_H
#define __JPENHUFF_H
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
//  Title:  JpegEncoderHuffmanTable class definition
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//
//  Description:
//
//    This class represents a Huffman Table used for compression
//    by the JpegEncoder class.
//

#include "jpeg.h"

class JpegEncoder ;

class JpegEncoderHuffmanTable
{
public:
  JpegEncoderHuffmanTable () ;
  virtual ~JpegEncoderHuffmanTable () {}

  // ENCODING FUNCTIONS

  // This function resets the table so that the object can be used
  // over again.
  void Reset () ;

  // This function increases the frequency for a huffman value.
  void IncrementFrequency (unsigned int value) ;

  // This function creates the Huffman codes from the code frequencies.
  void BuildTable () ;

  // This function returns the Huffman code and code length to encode the
  // specified value.
  void Encode (unsigned int value, UBYTE2 &code, UBYTE1 &size) const ;

  // Returns the number of bytes required to write the table to the output
  // file.
  unsigned int OutputSize () const ;

  // Function to print the table definition to the output stream.
  void PrintTable (JpegEncoder &encoder) const ;

private:
  JpegEncoderHuffmanTable (const JpegEncoderHuffmanTable &) ;
  JpegEncoderHuffmanTable &operator=(const JpegEncoderHuffmanTable &) ;

  // This function builds the structures needed for Huffman
  // decoding after the table data has been read.
  void MakeTable () ;

  // frequencies [n] is the number of times the value "n" needs to
  // be encoded.
  unsigned int frequencies [JpegMaxNumberOfHuffmanCodes + 1] ;

  // Values used to represent Huffman tables in a JPEG stream
  //  huff_bits [n] is the number of codes of length "n+1"
  //  huff_values is the list of Huffman values sorted in order
  //   of code length.
  UBYTE1 huff_bits [2 * JpegMaxHuffmanCodeLength] ; // 2x needed for encoding only.
  UBYTE1 huff_values [JpegMaxNumberOfHuffmanCodes] ;

  // Values used to encode values.
  //   ehufsi [n] is the number of bits required to code "n"
  //   ehufco [n] is the Huffman code for "n"
  UBYTE1 ehufsi [JpegMaxNumberOfHuffmanCodes+ 1] ;
  UBYTE2 ehufco [JpegMaxNumberOfHuffmanCodes+1] ;

  // The flag is set to true when the Huffman code sizes has been determined.
  // It is cleared when the object is Reset().
  bool sizes_found ;
} ;

#endif
