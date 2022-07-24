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
// Title:   EncoderHuffmanTable Class Implementation
//
// Author: John M. Miano  miano@colosseumbuilders.com
//
//

#include <string.h>
#include "jpenhuff.h"
#include "jpegenco.h"
#include "jpgexcep.h"

//
//  Description:
//
//    Class default constructor
//
JpegEncoderHuffmanTable::JpegEncoderHuffmanTable ()
{
  Reset () ;
  return ;
}

//
//  Description:
//
//    After Huffman codes have been generated the object is in a state
//    where it cannot be used to create a new set of code. This function
//    places the object in a state where it can be reused to generate a
//    new set of Huffman Codes.
//

void JpegEncoderHuffmanTable::Reset ()
{
  memset (frequencies, 0, sizeof (frequencies)) ;
  // We add a dummy Huffman value at the end of the table with a minimumal
  // frequency. Since we create the Huffman codes using the in the frequency
  // table this dummy value will be assigned the longest all one huffman code.
  // This ensures that no Huffman code consists entirely of 1s.
  frequencies [256] = 1 ;
  sizes_found = false ;
  return ;
}

//
//  Description:
//
//    Function to increment the frequency for a value.
//
//  Parameters:
//    value:  The value whose frequency is to be incremented
//

void JpegEncoderHuffmanTable::IncrementFrequency (unsigned int value)
{
  // Once the Huffman codes have been generated for this object, the Reset()
  // function must be called before we can gather data again.
  if (sizes_found)
    throw EJpegFatal ("INTERNAL ERROR - Huffman Code sizes already found") ;

  if (value >= JpegMaxNumberOfHuffmanCodes)
    throw EJpegIndexOutOfRange () ;

  ++ frequencies [value] ;
  return ;
}

//
//  Description:
//
//    This function generates the Huffman Codes using the frequency data. The code
//    generation process is taken directly from the JPEG standard.
//
//    The outputs from this function are the following member variables:
//   
//     ehufsi [n] : The length of the Huffman Code for value "n"
//     ehufco [n] : The Huffman Code for value "n"
//     huff_bits [n] : The number of Huffman codes of length "n+1"
//     huff_values [n] : The Huffman Values sorted by code length.
//
//    The first two arrays are used to encode Huffman values. The last two
//    are for writing the table to the output file.
//
//    The code generation process is:
//
//    1. Arrange the Huffman Values into a binary tree so that the most
//       frequently used codes are closest to the root of the tree. At the end
//       of this process the temporary array codesize [n] contains the length
//       of the pure Huffman code for the value "n"
//
//    2. Determine the number of Huffman Codes of for each code length. This
//       step places the number of codes of length "n+1" in huff_bits[].
//
//    3. The JPEG standard only allows Huffman codes of up to 16-bits. If any
//       codes longer than 16-bits were generated in the previous steps then
//       we need to reduce the maximum depth of the tree created in step 1.
//       The input and output to this step is the array huff_bits[] created in
//       the previous step.
//
//    4. Remove the dummy all 1-bit code (See the Reset() function).
//
//    5. Sort the Huffman values in code length order. codesize [n] is the
//       input to this step and huff_values [n] is the output. At this point
//       all the information needed to write the Huffman Table to the output
//       stream has been found.
//
//    6. Determine the code size for each value. At the end of this step
//       the temporary array huffsizes [n] is the Huffman code length for
//       huff_values [n].
//
//    7. Determine the Huffman code for each value. The temporary array
//       huffcodes [n] is the Huffman Code of length huffsizes [n] for
//       the value huff_value [n].
//
//    8. Using huffsizes [] and huffcodes created in steps 6 and 7 create
//       the arrays ehufco [n] and ehufsi [n] giving the Huffman Code and
//       Code Size for n.
//
void JpegEncoderHuffmanTable::BuildTable ()
{
  // We need this because MSVC++ does not support standard
  // scoping in for statements.
  int ii, kk ;

  // See if we have already calculated the Huffman codes.
  if (sizes_found)
    return ;

  // The tmp array is used for validating the integrity of the Huffman code
  // table. We need a temporary array since frequencies [] gets trashed
  // during the code generation process.
	unsigned int tmp [JpegMaxNumberOfHuffmanCodes + 1] ;
  for (ii = 0 ; ii < JpegMaxNumberOfHuffmanCodes + 1 ; ++ ii)
   	tmp [ii] = frequencies [ii] ;

  // Figure K.1
  // Build the Huffman Code Length Lists
  int others [JpegMaxNumberOfHuffmanCodes + 1] ;
  for (ii = 0 ; ii < JpegMaxNumberOfHuffmanCodes + 1 ; ++ ii)
    others [ii] = -1 ;

  unsigned int codesize [JpegMaxNumberOfHuffmanCodes + 1] ;
  memset (codesize, 0, sizeof (codesize)) ;
  while (true)
  {
    // Find the two smallest non-zero values
    int v1 = -1 ;
    int v2 = -1 ;
    for (unsigned int ii = 0 ; ii < JpegMaxNumberOfHuffmanCodes + 1 ; ++ ii)
    {
      if (frequencies [ii] != 0)
      {
        // K.2 says to take the highest value value for v1 and v2
        // in case of a tie. This ensures the dummy value gets
        // the last Huffman code.
        if (v1 < 0 || frequencies [ii] <= frequencies [v1])
        {
          v2 = v1 ;
          v1 = ii ;
        }
        else if (v2 < 0 || frequencies [ii] <= frequencies [v2])
        {
          v2 = ii ;
        }
      }
    }
    if (v2 < 0)
      break ;

    // Join the two tree nodes.
    frequencies [v1] = frequencies [v1] + frequencies [v2] ;
    frequencies [v2] = 0 ;

    for (++ codesize [v1] ; others [v1] >= 0 ; ++ codesize [v1])
      v1 = others [v1] ;

    others [v1] = v2 ;

    for (++ codesize [v2] ; others [v2] >= 0 ; ++ codesize [v2])
      v2 = others [v2] ;
  }

  // Figure K.2
  // Determine the number of codes of length [n]
  memset (huff_bits, 0, sizeof (huff_bits)) ;
  for (ii = 0 ; ii < JpegMaxNumberOfHuffmanCodes + 1 ; ++ ii)
  {
    if (codesize [ii] != 0)
    {
      ++ huff_bits [codesize [ii] - 1] ;
    }
  }

  // Figure K.3
  // Ensure that no code is longer than 16-bits.
  for (ii = 2 * JpegMaxHuffmanCodeLength -  1 ;
       ii >= JpegMaxHuffmanCodeLength ;
       -- ii)
  {
    while (huff_bits [ii] != 0)
    {
      unsigned int jj = ii - 1 ;
      do
      {
        -- jj ;
      }
      while (huff_bits [jj] == 0) ;

      huff_bits [ii] -= 2 ;
      ++ huff_bits [ii - 1] ;
      huff_bits [jj + 1] += 2 ;
      -- huff_bits [jj] ;
    }
  }

  // Remove the reserved code from the end of the list.
  for (ii = JpegMaxHuffmanCodeLength - 1 ; ii >= 0 ; -- ii)
  {
    if (huff_bits [ii] != 0)
    {
      -- huff_bits [ii] ;
      break ;
    }
  }

  // Make sure that the total number of symbols is correct.
  unsigned int count = 0 ;
  for (ii = 0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
    count += huff_bits [ii] ;
  }
  if (count >= JpegMaxNumberOfHuffmanCodes)
    throw EJpegFatal ("INTERNAL ERROR - Too many codes defined") ;

  // Figure K.4
  // Sort the values in order of code length
  memset (huff_values, 0, sizeof (huff_values)) ;
  for (ii = 1, kk = 0 ; ii < 2 * JpegMaxHuffmanCodeLength ; ++ ii)
  {
    for (unsigned int jj = 0 ; jj < JpegMaxNumberOfHuffmanCodes ; ++ jj)
    {
      if (codesize [jj]  == ii)
      {
        huff_values [kk] = jj ;
        ++ kk ;
      }
    }
  }

  // Section C.2 Figure C.1
  // Convert the array "huff_bits" containing the count of codes for each
  // length 1..16 into an array containing the length for each code.
  unsigned int huffsizes [JpegMaxNumberOfHuffmanCodes] ;
  memset (huffsizes, 0, sizeof (huffsizes)) ;
  for (ii = 0, kk = 0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
     for (int jj = 0 ; jj < huff_bits [ii] ; ++ jj)
     {
        huffsizes [kk] = ii + 1 ;
        ++ kk ;
     }
     huffsizes [kk] = 0 ;
  }

  // Section C.2 Figure C.2
  // Calculate the Huffman code for each Huffman value.
  UBYTE2 code = 0 ;
  unsigned int huffcodes [JpegMaxNumberOfHuffmanCodes] ;
  memset (huffcodes, 0, sizeof (huffcodes)) ;
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

  // Section C.2 Figure C.3
  memset (ehufco, 0, sizeof (ehufco)) ;
  memset (ehufsi, 0, sizeof (ehufsi)) ;
  for (kk = 0 ; kk < JpegMaxNumberOfHuffmanCodes ; ++ kk)
  {
    if (huffsizes [kk] != 0)
    {
      unsigned int ii = huff_values [kk] ;
      ehufco [ii] = huffcodes [kk] ;
      ehufsi [ii] = huffsizes [kk] ;
    }
  }

  // Validations
  // This remaining code is not necessary other than to ensure the
  // integrity of the Huffman table that is generated.

  // Make sure each value is used exactly once.
  for (ii = 0 ; ii < JpegMaxNumberOfHuffmanCodes ; ++ ii)
  {
    int count = 0 ;
    if (tmp [ii] != 0)
    {
      if (ehufsi [ii] == 0)
        throw EJpegFatal ("INTERNAL ERROR - Missing Huffman Code Size") ;

      for (unsigned int jj = 0 ; jj < JpegMaxNumberOfHuffmanCodes ; ++ jj)
      {
        if (ii == huff_values [jj] && huffsizes [jj] != 0)
          ++ count ;
        if (count > 1)
          throw EJpegFatal ("INTERNAL ERROR - Duplicate Huffman Value") ;
      }
      if (count == 0)
        throw EJpegFatal ("INTERNAL ERROR - Missing Huffman Value") ;
    }
  }

  // Ensure that each huffman code is used once annd that the values
  // are in range.
  for (ii = 0 ; ii < JpegMaxNumberOfHuffmanCodes ; ++ ii)
  {
    if (ehufsi [ii] != 0)
    {
      if (ehufsi [ii] > JpegMaxHuffmanCodeLength)
        throw EJpegFatal ("INTERNAL ERROR - Invalid Huffman Range") ;

      for (int jj = ii + 1 ; jj < JpegMaxNumberOfHuffmanCodes ; ++ jj)
      {
        if (ehufco [ii] == ehufco [jj] && ehufsi [jj] != 0)
          throw EJpegFatal ("INTERNAL ERROR - Duplicate Huffman Code") ;
      }
    }
  }

  sizes_found = true ;
  return ;
}

//
//  Description:
//
//    This function returns the Huffman Code and Code Size for a given value.
//
//  Parameters:
//    value:  The value to encode
//    code:   The Huffman Code
//    size:   The Huffman Code Length
//
void JpegEncoderHuffmanTable::Encode (unsigned int value,
                                      UBYTE2 &code,
                                      UBYTE1 &size) const
{
  if (value >= JpegMaxNumberOfHuffmanCodes)
    throw EJpegIndexOutOfRange () ;

  if (ehufsi [value] == 0)
    throw EJpegFatal ("INTERNAL ERROR - Missing Huffman Code") ;

  code = ehufco [value] ;
  size = ehufsi [value] ;
  return ;
}

//
//  Description:
//
//    This function writes the Huffman table data to the output stream in the
//    format specified by the JPEG standard.
//
//  Parameters:
//    encoder:    The JpegEncoder that defines the output stream.
//
void JpegEncoderHuffmanTable::PrintTable (JpegEncoder &encoder) const
{
  // We need this declaration here because MSVC++ does not support
  // standard scoping in for statements.
  unsigned int ii ;

  // B.2.4.2
  UBYTE1 data ;
  unsigned int count = 0 ; // Number of codes in the table.

  // Write 16 1-byte values containing the count of codes for
  // each possible Huffman code length and count the number of
  // codes used.
  for (ii = 0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
    count += huff_bits [ii] ;
    data = huff_bits [ii] ;
    encoder.OutputByte (data) ;
  }

  // Write the variable length part of the table, the Huffman values
  // sorted by Huffman Code.
  for (ii = 0 ; ii < count ; ++ ii)
  {
    data = huff_values [ii] ;
    encoder.OutputByte (data) ;
  }
  return ;
}
//
//  Description:
//
//    This function determines the size of the Huffman table when it is
//    written to a DHT marker. This function is used to calculate the
//    2-byte marker length that comes right after the FF-DHT sequence in
//    the output stream. Therefore we need to find the length before we
//    actually write the table.
//
unsigned int JpegEncoderHuffmanTable::OutputSize () const
{
  unsigned int count = 0 ;
  for (unsigned int ii = 0 ; ii < JpegMaxHuffmanCodeLength ; ++ ii)
  {
    count += huff_bits [ii] ;
  }

  // 1 byte for each value + one byte for each of 16 code lengths +
  // 1 byte for the table class and ID.
  return count + JpegMaxHuffmanCodeLength + 1 ;
}


