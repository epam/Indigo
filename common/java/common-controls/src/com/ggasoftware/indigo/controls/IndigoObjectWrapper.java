package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.IndigoObject;

public interface IndigoObjectWrapper
{
   // This method can throw an exception because it might load molecule from file
   IndigoObject getObjectCopy() throws IndigoCheckedException;
}
