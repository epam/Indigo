package com.ggasoftware.indigo.legio;

import java.awt.image.BufferedImage;

public class MolImage {
   public BufferedImage image;
   public String string;

   public MolImage( String mol_string )
   {
      string = mol_string;
      image = null;
   }
}
