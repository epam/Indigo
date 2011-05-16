package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.IndigoException;
import com.ggasoftware.indigo.IndigoObject;

public class IndigoIteratorItem implements IndigoObjectWrapper
{
   private final int _index;
   private final IndigoObject _iterator;

   public IndigoIteratorItem(IndigoObject iterator, int idx)
   {
      _iterator = iterator;
      _index = idx;
   }

   private IndigoObject _getIndigoObjectCopy()
   {
      synchronized (_iterator.getIndigo())
      {
         return _iterator.at(_index).clone();
      }
   }

   public IndigoObject getObjectCopy() throws IndigoCheckedException
   {
      try 
      {
         return _getIndigoObjectCopy();
      }
      catch (IndigoException ex)
      {
         throw new IndigoCheckedException(ex.getMessage(), ex);
      }
   }
}
