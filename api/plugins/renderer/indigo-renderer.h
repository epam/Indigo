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

#ifndef __indigo_render__
#define __indigo_render__

#include "indigo.h"
  
/* Rendering */

// Returns an 'output' object for the given HDC
CEXPORT int indigoRenderWriteHDC (void* hdc, int printingHdc);

// output is either a file output obtained via indigoWriteFile(), or
//        a buffer obtained via indigoWriteBuffer(), or
//        an HDC obtained via indigoRenderWriteHDC
CEXPORT int indigoRender (int object, int output);

// objects  is an array of molecules created with indigoCreateArray)
// refAtoms is an array of integers, whose size must be equal to the number
//          of molecules if the array
// nColumns is the number of columns in the grid
// output -- see the comment for indigoRender
CEXPORT int indigoRenderGrid (int objects, int* refAtoms, int nColumns, int output);

// Works like indigoRender(), but renders directly to file
CEXPORT int indigoRenderToFile (int object, const char *filename);

// Works like indigoRenderGrid(), but renders directly to file
CEXPORT int indigoRenderGridToFile (int objects, int* refAtoms, int nColumns, const char *filename);

// Resets all the rendering settings
CEXPORT int indigoRenderReset ();

#endif
