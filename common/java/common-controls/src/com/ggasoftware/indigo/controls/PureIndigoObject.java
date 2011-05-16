package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.IndigoObject;

public class PureIndigoObject implements IndigoObjectWrapper
{
   private final IndigoObject _object;
   
   public PureIndigoObject (IndigoObject object)
   {
      _object = object;
   }

   public IndigoObject getObjectCopy ()
   {
      return _object.clone();
   }
}
