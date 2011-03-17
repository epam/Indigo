/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.MolData;
import com.ggasoftware.indigo.controls.RenderableObject;

public class RenderableMolData extends MolData implements RenderableObject {
   public RenderableMolData( MolData mol_data )
   {
      if (mol_data != null)
      {
         mol_iterator = mol_data.mol_iterator;
         index = mol_data.index;
      }
      else
      {
         mol_iterator = null;
         index = -1;
      }
   }

   public IndigoObject getObject()
   {
      return mol_iterator.at(index).clone();
   }
}
