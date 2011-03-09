using System;
using System.Collections.Generic;
using System.Text;

namespace com.ggasoftware.indigo
{
   public unsafe interface IndigoRendererLib
   {
      int indigoRenderWriteHDC (void* hdc, int printingHdc);
      int indigoRender (int item, int output);
      int indigoRenderToFile (int item, string filename);
      int indigoRenderGrid (int items, int[] refAtoms, int nColumns, int output);
      int indigoRenderGridToFile (int items, int[] refAtoms, int nColumns, string filename);
   }
}
