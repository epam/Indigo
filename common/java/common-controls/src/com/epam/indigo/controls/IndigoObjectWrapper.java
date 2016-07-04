package com.epam.indigo.controls;

import com.epam.indigo.IndigoObject;

public interface IndigoObjectWrapper
{
   // This method can throw an exception because it might load molecule from file
   IndigoObject getObjectCopy() throws IndigoCheckedException;
}
