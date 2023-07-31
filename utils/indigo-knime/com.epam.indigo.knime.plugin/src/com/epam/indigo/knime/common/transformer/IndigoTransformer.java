package com.epam.indigo.knime.common.transformer;

import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;

import com.epam.indigo.IndigoObject;

public abstract class IndigoTransformer implements IndigoTransformerBase 
{
   public abstract void transform (IndigoObject io, boolean reaction);
   
   public void transformWithRow (IndigoObject io, boolean reaction, final DataRow row)
   {
      transform(io, reaction);
   }
   
   public void initialize (final DataTableSpec inSpec)
   {
      
   }
}
