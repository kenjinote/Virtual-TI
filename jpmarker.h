#ifndef __MARKERS_H
#define __MARKERS_H
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
//  JPEG Markers as defined in Section B.1.1.3 of the JPEG standard.
//

// These definitions do not include the preceding 0xFF byte.
enum JpegMarkers
{
  // Start of Frame Markers, Non-Differential Huffman Coding
  SOF0 = 0xC0,    // Baseline DCT
  SOF1 = 0xC1,    // Sequential DCT
  SOF2 = 0xC2,    // Progressive DCT
  SOF3 = 0xC3,    // Spatial (sequential) lossless
  // Start of Frame Markers, Differential Huffman Coding
  SOF5 = 0xC5,    // Differential Sequential DCT
  SOF6 = 0xC6,    // Differential Progressive DCT
  SOF7 = 0xC7,    // Differential Spatial
  // Start of Frame Markers, Non-Differential Arithmetic Coding
  SOF9 = 0xC9,    // Extended Sequential DCT
  SOFA = 0xCA,    // Progressive DCT
  SOFB = 0xCB,    // Spacial (sequential) Lossless
  // Start of Frame Markers, Differential Arithmetic Coding
  SOFD = 0xCD,    // Differential Sequential DCT
  SOFE = 0xCE,    // Differential Progressive DCT
  SOFF = 0xCF,    // Differential Spatial
  // Other Markers
  DHT = 0xC4,     // Define Huffman Tables
  DAC = 0xCC,     // Define Arithmetic Coding Conditions
  RST0 = 0xD0,    // Restart Marker
  RST1 = 0xD1,    // Restart Marker
  RST2 = 0xD2,    // Restart Marker
  RST3 = 0xD3,    // Restart Marker
  RST4 = 0xD4,    // Restart Marker
  RST5 = 0xD5,    // Restart Marker
  RST6 = 0xD6,    // Restart Marker
  RST7 = 0xD7,    // Restart Marker
  SOI = 0xD8,     // Start of Image
  EOI = 0xD9,     // End of Image
  SOS = 0xDA,     // Start of Scan
  DQT = 0xDB,     // Define Quantization Table
  DNL = 0xDC,     // Define Number of Lines
  DRI = 0xDD,     // Define Restart Intervale
  DHP = 0xDE,     // Define Hierarchical Progression
  EXP = 0xDF,     // Expand Reference Components
  APP0 = 0xE0,    // Application Segments
  APP1 = 0xE1,    // Application Segments
  APP2 = 0xE2,    // Application Segments
  APP3 = 0xE3,    // Application Segments
  APP4 = 0xE4,    // Application Segments
  APP5 = 0xE5,    // Application Segments
  APP6 = 0xE6,    // Application Segments
  APP7 = 0xE7,    // Application Segments
  APP8 = 0xE8,    // Application Segments
  APP9 = 0xE9,    // Application Segments
  APPA = 0xEA,    // Application Segments
  APPB = 0xEB,    // Application Segments
  APPC = 0xEC,    // Application Segments
  APPD = 0xED,    // Application Segments
  APPE = 0xEE,    // Application Segments
  APPF = 0xEF,    // Application Segments
  // C8, F0-FD, 01, 02-BF reserved
  COM = 0xFE,     // Comment
  SOB = 0xFF,     // Start of Block - Byte that precedes all others - not in the standard.
} ;

#endif
