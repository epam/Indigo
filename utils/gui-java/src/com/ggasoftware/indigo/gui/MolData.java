package com.ggasoftware.indigo.gui;
import com.ggasoftware.indigo.IndigoObject;

public class MolData
{
   public int index;
   public String csmiles;
   public IndigoObject mol_iterator;

   public MolData()
   {
      mol_iterator = null;
      index = -1;
      csmiles = null;
   }

   public MolData( IndigoObject iterator, int idx, String csmiles )
   {
      mol_iterator = iterator;
      index = idx;
      this.csmiles = new String(csmiles);
   }

   public MolData( MolData mol_data )
   {
      mol_iterator = mol_data.mol_iterator;
      index = mol_data.index;
      if (mol_data.csmiles != null)
         csmiles = new String(mol_data.csmiles);
      else
         csmiles = null;
   }

   public IndigoObject getMolecule()
   {
      return mol_iterator.at(index).clone();
   }
}