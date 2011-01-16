/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __bingo_profiling_h__
#define __bingo_profiling_h__

// Print current profiling information to the debug output
// Parameters:
//   get_all - true if print all counters, 
//             false - print only counters with nonzero values
void bingoProfilingPrintStatistics (bool print_all);

#endif // __bingo_profiling_h__

