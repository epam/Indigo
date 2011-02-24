/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
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

package com.ggasoftware.indigo;

import com.sun.jna.*;

public interface IndigoRendererLib extends Library
{
   int indigoRenderWriteHDC (Pointer hdc, int printingHdc);
   int indigoRender (int object, int output);
   int indigoRenderGrid (int objects, int[] refAtoms, int nColumns, int output);
   int indigoRenderToFile (int object, String filename);
   int indigoRenderGridToFile (int objects, int[] refAtoms, int nColumns, String filename);
}
