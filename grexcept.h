#ifndef __GREXCEPT_H
#define __GREXCEPT_H
//
// Copyright (c) 1997,1998 Colosseum Builders, Inc.
// All rights reserved.
//
// Colosseum Builders, Inc. makes no warranty, expressed or implied
// with regards to this software. It is provided as is.
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
// o This GIF decoder may be distributed and used without license fee
//   to the authors. It is our understanding that GIF decoding is not
//   covered by patents (which is why there are non-LZW GIF encoders).
//   However, we are not lawyers. If you wish to encorporate this software
//   then you should get your own legal advice.
//

//
//  Title:  Graphics Exception Classes
//
//  Author:  John M. Miano  miano@colosseumbuilders.com
//

//#include <stdexcept>

class EGraphicsException
{
public:
	EGraphicsException (const char* msg) {}
} ;

class ESubscriptOutOfRange : public EGraphicsException
{
public:
  ESubscriptOutOfRange () : EGraphicsException ("Subscript out of range") {}
} ;

class EInvalidBitCount : public EGraphicsException
{
public:
  EInvalidBitCount () : EGraphicsException ("Invalid bit count") {}
} ;

class EGraphicsAbort : EGraphicsException
{
public:
  EGraphicsAbort () : EGraphicsException ("Abort") {}
} ;

#endif
