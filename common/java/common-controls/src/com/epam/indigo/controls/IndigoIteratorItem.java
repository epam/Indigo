package com.epam.indigo.controls;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;

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
