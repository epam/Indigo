package com.ggasoftware.indigo.controls;

// Checked exception that must be handled explicitly
public class IndigoCheckedException extends Exception
{
   public IndigoCheckedException (String message)
   {
      super(message);
   }
   public IndigoCheckedException (String message, Throwable cause)
   {
      super(message, cause);
   }
}
