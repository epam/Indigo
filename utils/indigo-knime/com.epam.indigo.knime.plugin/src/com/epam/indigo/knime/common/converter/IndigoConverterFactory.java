package com.epam.indigo.knime.common.converter;

import org.knime.chem.types.CMLAdapterCell;
import org.knime.chem.types.CMLValue;
import org.knime.chem.types.InchiAdapterCell;
import org.knime.chem.types.InchiValue;
import org.knime.chem.types.MolAdapterCell;
import org.knime.chem.types.MolValue;
import org.knime.chem.types.RxnAdapterCell;
import org.knime.chem.types.RxnValue;
import org.knime.chem.types.SdfAdapterCell;
import org.knime.chem.types.SdfValue;
import org.knime.chem.types.SmartsAdapterCell;
import org.knime.chem.types.SmartsValue;
import org.knime.chem.types.SmilesAdapterCell;
import org.knime.chem.types.SmilesValue;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;
import org.knime.core.data.RWAdapterValue;
import org.knime.core.data.StringValue;
import org.knime.core.data.def.StringCell;

import com.epam.indigo.IndigoException;
import com.epam.indigo.knime.cell.IndigoDataCell;
import com.epam.indigo.knime.common.types.IndigoType;

/**
 * 
 * This factory services to provide a converter to get cells containing standard 
 * as well as Indigo representations. 
 */
public class IndigoConverterFactory {

   /**
    * This method provides converter.
    * @param inSpec {@link DataTableSpec} containing column to convert
    * @param colIdx Index of column to convert
    * @param indigoType Expected {@link INDIGO_TYPE} for Indigo representation
    * @return {@link IndigoConverter}
    */
   @SuppressWarnings("unchecked")
   public static IndigoConverter createConverter(DataTableSpec inSpec, 
         int colIdx, IndigoType indigoType, boolean stringAsSMARTS) {
      
      // column type
      DataType type = inSpec.getColumnSpec(colIdx).getType();
      // indigo value class 
      Class<? extends DataValue> indigoValueClass = indigoType.getIndigoDataValueClass();
      // array of classes indigo converted from
      Class<? extends DataValue>[] classes = indigoType.getClassesConvertableToIndigoDataClass();
      // any valueClass compatible to the type
      Class<? extends DataValue> valueCls = findAnyCompatibleValueClass(type, classes);
      
      IndigoConverter converter = null;
      
      if (type.isAdaptable(indigoValueClass)) {
         // if input is already adaptable to an indigo type (cell has indigo representation)

         converter = new IndigoConverter(type) {
            
            @Override
            public DataCell convert(DataCell source) throws IndigoException {
               return source;
            }
         };
         
      } else if (type.isCompatible(indigoValueClass)) {
         // if input is compatible to an indigo type
         
         converter = new IndigoConverter(indigoType.getIndigoDataType()) {
            
            @Override
            public DataCell convert(DataCell source) throws IndigoException {
               return indigoType.createAdapterFromIndigoDataCell((IndigoDataCell) source);
            }
         };
         
      } else if (type.isCompatible(RWAdapterValue.class) && type.isAdaptableToAny(classes)) {
         // if input is a writable adapter that contains types indigo representation can be converted from.
         
         DataType resultType = type.createNewWithAdapter(indigoValueClass);
         converter = new IndigoConverter(resultType) {
            
            @Override
            public DataCell convert(DataCell source) throws IndigoException {
               return indigoType.createAdapterContainingIndigoRepr(source, true, stringAsSMARTS);
            }

         };
         
      } else if (valueCls != null) {
         // if  input is not an adapter cell but can be converted into indigo representation
         
         DataType resultType = adapterDataTypeForValue(valueCls, stringAsSMARTS);
         converter = new IndigoConverter(resultType) {
            
            @Override
            public DataCell convert(DataCell source) throws IndigoException {
               return indigoType.createAdapterContainingIndigoRepr(source, false, stringAsSMARTS);
            }
         };
      } 
      
      return converter;
   }

   
   public static IndigoConverter createConverter(DataTableSpec inSpec, 
         int colIdx, IndigoType indigoType) {
      return createConverter(inSpec, colIdx, indigoType, false);
   }
   
   
   @SuppressWarnings("unchecked")
   private static Class<? extends DataValue> findAnyCompatibleValueClass(DataType type, 
         Class<? extends DataValue>... valueClasses) {
      
      for (Class<? extends DataValue> cls : valueClasses) {
         if (type.isCompatible(cls)) 
            return cls;
      }
      
      return null;
   }
   
   private static DataType adapterDataTypeForValue(Class<? extends DataValue> valueClass, boolean stringAsSMARTS) { 
      DataType type = null;
      
      if (SmartsValue.class.equals(valueClass))
         type = SmartsAdapterCell.RAW_TYPE;
      else if (MolValue.class.equals(valueClass))
         type = MolAdapterCell.RAW_TYPE;
      else if (SdfValue.class.equals(valueClass))
         type = SdfAdapterCell.RAW_TYPE;
      else if (SmilesValue.class.equals(valueClass))
         type = SmilesAdapterCell.RAW_TYPE;
      else if (RxnValue.class.equals(valueClass))
         type = RxnAdapterCell.RAW_TYPE;
      else if (CMLValue.class.equals(valueClass))
         type = CMLAdapterCell.RAW_TYPE;
      else if (InchiValue.class.equals(valueClass))
         type = InchiAdapterCell.RAW_TYPE;
      else if (StringValue.class.equals(valueClass))         
         type = stringAsSMARTS ? SmartsAdapterCell.RAW_TYPE :
            StringCell.TYPE;
      
      return type;
      
   }
   
}
