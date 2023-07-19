package com.epam.indigo.knime.common.transformer;

import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;

import com.epam.indigo.IndigoObject;

/*
 * Interface for all indigo transformers
 */
public interface IndigoTransformerBase
{
   public void initialize (final DataTableSpec inSpec);
   
   public void transformWithRow (IndigoObject io, boolean reaction, final DataRow row);
}
