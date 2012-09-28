/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __lzw_encoder_h__
#define __lzw_encoder_h__

#include "base_cpp/bitoutworker.h"
#include "base_cpp/output.h"
#include "lzw/lzw_dictionary.h"

namespace indigo {

class LzwEncoder
{
public:

   DECL_ERROR;

   LzwEncoder( LzwDict &NewDict, Output &NewOut );

   void start( void );
   
   void send( int NextSymbol );
   
   void finish( void );

   ~LzwEncoder( void );

private:

   LzwDict &_dict;            

   BitOutWorker _bitout;

   int _string;

   byte _char;

   bool _isFinished;

   // no implicit copy
   LzwEncoder( const LzwEncoder & );

};

class LzwOutput : public Output
{
public:
   LzwOutput (LzwEncoder &encoder);

   virtual void write (const void *data, int size);
   virtual void writeByte (byte value);
   virtual void seek  (int offset, int from);
   virtual int  tell  ();
   virtual void flush ();
private:
   LzwEncoder &_encoder; 
};

}

#endif 

