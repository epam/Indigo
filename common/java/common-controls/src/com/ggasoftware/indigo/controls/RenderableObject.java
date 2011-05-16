package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoRenderer;

public abstract class RenderableObject
{
   private String _error_mesage;
   
   public abstract Indigo getIndigo ();
   public abstract IndigoRenderer getIndigoRenderer ();
   
   // Can return null if there is no renderable object or if it is not valid
   // If this method returns null then setErrorMessageToRender should contain
   // error message
   public abstract IndigoObject getRenderableObject();
   
   public String getErrorMessageToRender ()
   {
      return _error_mesage;
   }
   
   public void setErrorMessageToRender (String message)
   {
      _error_mesage = message;
   }
}
