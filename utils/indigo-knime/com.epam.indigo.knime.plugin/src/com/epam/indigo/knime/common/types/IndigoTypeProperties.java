package com.epam.indigo.knime.common.types;

import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;

import com.epam.indigo.knime.cell.IndigoDataCell;
import com.epam.indigo.knime.cell.IndigoDataValue;

/**
 * This interface keeps all the required and often used queries to indigo types.
 * The Indigo type should be interpreted as one of the four: Molecule,
 * Reaction, Query molecule and Query reaction.
 *
 */
public interface IndigoTypeProperties {

	static String STRING_MOLECULE = "Molecule";
	static String STRING_REACTION = "Reaction";
	static String STRING_QUERY_MOLECULE = "Query molecule";
	static String STRING_QUERY_REACTION	 = "Query reaction";
	
   /**
    * Returns Indigo adapter cell's type.
    * @return One of Indigo's {@link DataType}
    */
   DataType getIndigoDataType(); 
   
   /**
    * Returns Indigo data value.
    * @return {@link IndigoDataValue}'s child
    */
   Class<? extends DataValue> getIndigoDataValueClass();
   
   /**
    * Returns array of classes Indigo data classes can be converted from.
    * @return Standard types {@link DataValue}'s childs.
    */
   Class<? extends DataValue>[] getClassesConvertableToIndigoDataClass();
   
   /**
    * Creates an adapter cell containing Indigo representation based on {@link IndigoDataCell}.
    * @param source {@link IndigoDataCell} instance
    * @return {@link AdapterCell}
    */
   AdapterCell createAdapterFromIndigoDataCell(IndigoDataCell source);
   
   /**
    * Creates an adapter cell containing both standard and Indigo representations. NOTE: It returns original NOT adapter cell
    * if it's a string cell.
    * @param source {@link DataCell} instance
    * @param isAdapter If source cell is adapter or not
    * @return{@link AdapterCell}
    */
   DataCell createAdapterContainingIndigoRepr(DataCell source, boolean isAdapter, boolean stringAsSMARTS);
   
}
