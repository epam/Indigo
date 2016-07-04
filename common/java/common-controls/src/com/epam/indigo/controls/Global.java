package com.epam.indigo.controls;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoRenderer;

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
