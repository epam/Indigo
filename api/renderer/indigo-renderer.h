/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

CEXPORT int indigoRenderWriteHDC (void* hdc, int printingHdc);
CEXPORT int indigoRender (int object, int output);
CEXPORT int indigoRenderGrid (int objects, int* refAtoms, int nColumns, int output);
CEXPORT int indigoRenderToFile (int object, const char *filename);
CEXPORT int indigoRenderReset (int render);

#endif
