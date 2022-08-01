/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __indigo_render__
#define __indigo_render__

#include "indigo.h"

CEXPORT int indigoRendererInit(qword id);
CEXPORT int indigoRendererDispose(qword id);

/* Rendering */

// Returns an 'output' object for the given HDC
CEXPORT int indigoRenderWriteHDC(void* hdc, int printingHdc);

// output is either a file output obtained via indigoWriteFile(), or
//        a buffer obtained via indigoWriteBuffer(), or
//        an HDC obtained via indigoRenderWriteHDC
CEXPORT int indigoRender(int object, int output);

// objects  is an array of molecules created with indigoCreateArray)
// refAtoms is an array of integers, whose size must be equal to the number
//          of molecules if the array
// nColumns is the number of columns in the grid
// output -- see the comment for indigoRender
CEXPORT int indigoRenderGrid(int objects, int* refAtoms, int nColumns, int output);

// Works like indigoRender(), but renders directly to file
CEXPORT int indigoRenderToFile(int object, const char* filename);

// Works like indigoRenderGrid(), but renders directly to file
CEXPORT int indigoRenderGridToFile(int objects, int* refAtoms, int nColumns, const char* filename);

// Resets all the rendering settings
CEXPORT int indigoRenderReset();

#endif
