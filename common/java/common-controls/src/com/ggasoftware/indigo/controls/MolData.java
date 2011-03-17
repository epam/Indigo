package com.ggasoftware.indigo.controls;
import com.ggasoftware.indigo.IndigoObject;
import java.awt.image.renderable.RenderableImage;

public class MolData implements CanonicalizableObject
{
   public int index;
   public IndigoObject mol_iterator;

   public MolData()
   {
      mol_iterator = null;
      index = -1;
   }

   public MolData( IndigoObject iterator, int idx )
   {
      mol_iterator = iterator;
      index = idx;
   }

   public MolData( MolData mol_data )
   {
      mol_iterator = mol_data.mol_iterator;
      index = mol_data.index;
   }

   public String getCanonicalCode()
   {
      IndigoObject mol = mol_iterator.at(index).clone();
      return mol.canonicalSmiles();
   }
}
