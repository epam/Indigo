/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __auto_iter_h__
#define __auto_iter_h__

namespace indigo
{

class AutoIterator
{
public:
   AutoIterator( int idx );

   int operator* () const;

   bool operator!= ( const AutoIterator &other ) const;

protected:
   int _idx;
};


}

#endif // __auto_iter_h__
