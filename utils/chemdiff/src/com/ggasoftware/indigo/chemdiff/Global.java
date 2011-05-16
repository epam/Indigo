package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoRenderer;

public class Global
{
   public static final Indigo indigo;
   public static final IndigoRenderer indigo_renderer;
   
   static
   {
      indigo = new Indigo();
      indigo_renderer = new IndigoRenderer(indigo);
   }
}
