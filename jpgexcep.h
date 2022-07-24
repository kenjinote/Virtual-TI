#ifndef __JPGEXCEP_H
#define __JPGEXCEP_H
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
//  Title:  JPEG Exception Definitions
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//

#include "grexcept.h"

// Base class for trappable JPEG errors.
class EJpegError : public EGraphicsException
{
public:
  EJpegError (const char* msg) : EGraphicsException (msg) {}
} ;

// Exception that makes reading the image impossible.
class EJpegFatal : public EGraphicsException
{
public:
  EJpegFatal (const char* msg) : EGraphicsException (msg) {}
} ;

// Exception thrown when the callback function returns a values
// indicating the caller wants to abort the decode.
class EJpegAbort : public EGraphicsException
{
public:
  EJpegAbort  () : EGraphicsException ("JPEG Aborted") {}
} ;

class EJpegValueOutOfRange : public EJpegError
{
public:
  EJpegValueOutOfRange () : EJpegError ("Value out of range") {}
} ;

class EJpegIndexOutOfRange : public EJpegError
{
public:
  EJpegIndexOutOfRange () : EJpegError ("Index out of range") {}
} ;

class EJpegBadOutputParameter : public EJpegError
{
public:
  EJpegBadOutputParameter (char * msg) : EJpegError (msg) {}
} ;

class EJpegBadData : public EJpegError
{
public:
  EJpegBadData (char* msg = "Bad input") : EJpegError (msg) {}
} ;

#endif

