#ifndef __DATATYPE_H
#define __DATATYPE_H

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

#if defined (__BORLANDC__)
#pragma warn -csu
#pragma warn -sig
#pragma warn -pch
#pragma warn -inl
#endif

//
// For some reason MSVC++ gives errors about missing return statements
// in non-void functions for execution paths that can never be reached
// due to throw statements.
//
#if defined (_MSC_VER)
# define DUMMY_RETURN return 0 ;
#else
# define DUMMY_RETURN
#endif

typedef char BYTE1 ;
typedef unsigned char UBYTE1 ;

typedef short BYTE2 ;
typedef unsigned short UBYTE2 ;
typedef long BYTE4 ;
typedef unsigned long UBYTE4 ;

#define BIGENDIAN_SYSTEM 0

#if BIGENDIDIAN_SYSTEM!=0
inline UBYTE2 SystemToLittleEndian (UBYTE2 value)
{
  return ((value&0xFF) << 8)|((value&0xFF00)>>8) ;
}
inline UBYTE4 SystemToLittleEndian (UBYTE4 value)
{
  return ((value&0xFF000000)>>24)|((value&0xFF0000)>>8)
         | ((value&0xFF0)<<8) | ((value&0xFF)<<24) ;
}
inline UBYTE2 SystemToBigEndian (UBYTE2 value)
{
  return value ;
}
inline UBYTE4 SystemToBigEndian (UBYTE4 value)
{
  return value ;
}

inline UBYTE2 LittleEndianToSystem (UBYTE2 value)
{
  return ((value&0xFF) << 8)|((value&0xFF00)>>8) ;
}
inline UBYTE4 LittleEndianToSystem (UBYTE4 value)
{
  return ((value&0xFF000000)>>24)|((value&0xFF0000)>>8)
         | ((value&0xFF0)<<8) | ((value&0xFF)<<24) ;
}
inline UBYTE2 BigEndianToSystem (UBYTE2 value)
{
  return value ;
}
inline UBYTE4 BigEndianToSystem (UBYTE4 value)
{
  return value ;
}

#else
inline UBYTE2 SystemToLittleEndian (UBYTE2 value)
{
  return value ;
}
inline UBYTE4 SystemToLittleEndian (UBYTE4 value)
{
  return value ;
}
inline UBYTE2 SystemToBigEndian (UBYTE2 value)
{
  return (UBYTE2) ((value&0xFF) << 8)|((value&0xFF00)>>8) ;
}
inline UBYTE4 SystemToBigEndian (UBYTE4 value)
{
  return ((value&0xFF000000)>>24)|((value&0xFF0000)>>8)
         | ((value&0xFF00)<<8) | ((value&0xFF)<<24) ;
}

inline UBYTE2 LittleEndianToSystem (UBYTE2 value)
{
  return value ;
}
inline UBYTE4 LittleEndianToSystem (UBYTE4 value)
{
  return value ;
}
inline UBYTE2 BigEndianToSystem (UBYTE2 value)
{
  return (UBYTE2) ((value&0xFF) << 8)|((value&0xFF00)>>8) ;
}
inline UBYTE4 BigEndianToSystem (UBYTE4 value)
{
  return ((value&0xFF000000)>>24)|((value&0xFF0000)>>8)
         | ((value&0xFF00)<<8) | ((value&0xFF)<<24) ;
}
#endif

#endif
