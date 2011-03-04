package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.IndigoObject;
import java.awt.image.BufferedImage;

public class MolCell extends MolData
{
   public BufferedImage image;
   //public IndigoObject object;
   public boolean is_reaction_mode;
   public boolean is_query;

   int image_w;
   int image_h;

   public MolCell( IndigoObject new_iterator, int new_mol_idx, boolean is_reaction )
   {
      mol_iterator = new_iterator;
      index = new_mol_idx;
      image = null;
      is_reaction_mode = is_reaction;
      is_query = true;
   }

   public MolCell( MolData mol_data, boolean is_reaction )
   {
      if (mol_data != null)
      {
         mol_iterator = mol_data.mol_iterator;
         index = mol_data.index;
         csmiles = mol_data.csmiles;
      }
      else
      {
         mol_iterator = null;
         index = -1;
         csmiles = null;
      }

      image = null;
      is_reaction_mode = is_reaction;
      is_query = true;
   }
}