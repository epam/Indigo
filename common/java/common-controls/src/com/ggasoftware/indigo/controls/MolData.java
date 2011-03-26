package com.ggasoftware.indigo.controls;
import com.ggasoftware.indigo.IndigoObject;

public class MolData
{
   protected int index;
   protected IndigoObject mol_iterator;

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

   public IndigoObject getIterator()
   {
      return mol_iterator;
   }

   public int getIndex()
   {
      return index;
   }

   protected synchronized IndigoObject _getIndigoObject() {
      synchronized (mol_iterator) {
         return mol_iterator.at(index);
      }
   }
}
