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
