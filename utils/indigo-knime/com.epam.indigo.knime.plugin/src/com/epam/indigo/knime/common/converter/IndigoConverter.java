package com.epam.indigo.knime.common.converter;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataType;

import com.epam.indigo.IndigoException;

/** 
 * 
 * Class to convert {@link DataCell} into {@link AdapterCell} containing both
 * standard and Indigo representation.
 */
public abstract class IndigoConverter {

   private DataType resultType;
   
   public IndigoConverter(DataType resultType) {
      this.resultType = resultType;
   }
   
   /**
    * Converts input {@link DataCell}. The method usage should be inside try/catch block.
    * @param source A {@link DataCell} to convert
    * @return Converted cell
    * @throws IndigoException
    */
   public abstract DataCell convert(final DataCell source) throws IndigoException;
   
   public DataType getConvertedType() {
      return resultType;
   }
   
}
