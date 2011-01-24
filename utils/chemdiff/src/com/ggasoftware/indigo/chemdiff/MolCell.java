package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.IndigoObject;
import java.awt.image.BufferedImage;

public class MolCell {
   public BufferedImage image;
   public IndigoObject object;
   public boolean is_reaction_mode;
   public boolean is_query;

   int image_w;
   int image_h;

   public MolCell( IndigoObject mol_object, boolean is_reaction )
   {
      object = mol_object;
      image = null;
      is_reaction_mode = is_reaction;
      is_query = true;
   }
}