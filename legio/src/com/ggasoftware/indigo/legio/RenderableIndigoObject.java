package com.ggasoftware.indigo.legio;

import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.RenderableObject;


public class RenderableIndigoObject implements RenderableObject {
   private IndigoObject _object;

   public RenderableIndigoObject( IndigoObject object )
   {
      this._object = object;
   }

   public IndigoObject getObject() {
      return _object;
   }
}
