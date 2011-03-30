/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.RenderableObject;

public class RenderableMolData implements RenderableObject {
   private int _index;
   private IndigoObject _mol_iterator;

   public RenderableMolData()
   {
      _mol_iterator = null;
      _index = -1;
   }

   public RenderableMolData( IndigoObject iterator, int idx )
   {
      _mol_iterator = iterator;
      _index = idx;
   }

   public RenderableMolData( RenderableMolData mol_data )
   {
      _mol_iterator = mol_data._mol_iterator;
      _index = mol_data._index;
   }

   public IndigoObject getIterator()
   {
      return _mol_iterator;
   }

   public int getIndex()
   {
      return _index;
   }

   protected synchronized IndigoObject _getIndigoObject() {
      synchronized (_mol_iterator) {
         return _mol_iterator.at(_index);
      }
   }

   public IndigoObject getObject()
   {
      return _getIndigoObject().clone();
   }
}
