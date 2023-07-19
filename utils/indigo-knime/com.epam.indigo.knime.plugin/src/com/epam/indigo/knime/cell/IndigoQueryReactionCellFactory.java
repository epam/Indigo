package com.epam.indigo.knime.cell;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataCellFactory;
import org.knime.core.data.DataType;

/**
 * This class implements DataCellFactory itself (not its sub-interfaces) as there is no an appropriate 
 * interface among them. The problem is all the sub-interfaces do not have a custom create method allowing
 * arbitrary parameters count. Query cells require SMARTS format flag.
 */
public class IndigoQueryReactionCellFactory implements DataCellFactory {

   /** Type for indigo query reaction cell */
   public static final DataType TYPE = IndigoQueryReactionCell.TYPE;
   
   @Override
   public DataType getDataType() {
      return TYPE;
   }

   public DataCell createCell(String input, boolean smarts) {
      return createAdapterCell(input, smarts);
   }
   
   
   /**
    * Creates indigo mol adapter cell
    * @param io indigo object (Molecule)
    * @return IndigoQueryReactionAdapterCell instance if io != null. MissingCell - otherwise.
    */
   public static DataCell createAdapterCell(final String input, final boolean smarts) {

      if (input == null) {
         throw new NullPointerException("query mol value must not be null");
      }
      
      IndigoQueryReactionCell queryMolCell = smarts ? IndigoQueryReactionCell.fromSmarts(input) :
         IndigoQueryReactionCell.fromString(input);
      
      return new IndigoQueryReactionAdapterCell(queryMolCell);
   }
}
