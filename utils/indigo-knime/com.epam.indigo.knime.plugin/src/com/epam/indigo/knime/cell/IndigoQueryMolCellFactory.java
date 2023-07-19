package com.epam.indigo.knime.cell;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataCellFactory;

import org.knime.core.data.DataType;

/**
 * This class implements DataCellFactory itself (not its sub-interfaces) as there is no an appropriate 
 * interface among them. The problem is all the sub-interfaces do not have a custom create method allowing
 * arbitrary parameters count. Query cells require SMARTS format flag.
 */
public class IndigoQueryMolCellFactory implements DataCellFactory {

   /** Type for indigo query mol cell */
   public static final DataType TYPE = IndigoQueryMolCell.TYPE;
   
   public DataCell createCell(String input, boolean smarts) {
      return createAdapterCell(input, smarts);
   }

   @Override
   public DataType getDataType() {
      return TYPE;
   }
   
   /**
    * Creates indigo mol adapter cell
    * @param io indigo object (Molecule)
    * @return IndigoQueryMolAdapterCell instance if io != null. MissingCell - otherwise.
    */
   public static DataCell createAdapterCell(final String input, final boolean smarts) {

      if (input == null) {
         throw new NullPointerException("query mol value must not be null");
      }
      
      IndigoQueryMolCell queryMolCell = smarts ? IndigoQueryMolCell.fromSmarts(input) :
         IndigoQueryMolCell.fromString(input);
      
      return new IndigoQueryMolAdapterCell(queryMolCell);
   }

}
