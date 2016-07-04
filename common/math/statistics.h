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

#ifndef _statistics_h_
#define _statistics_h_

#include "base_c/defs.h"

namespace indigo {

class DLLEXPORT MeanEstimator
{
public:
   MeanEstimator ();

   void addValue (float value);

   int getCount () const;
   // Manual change of the count parameter to avoid a lot addValue(0)
   void setCount (int count);

   float mean () const;
   float meanEsimationError () const;

private:
   int _count;
   float _sum, _sum_sq;
};

}

#endif // _statistics_h_
