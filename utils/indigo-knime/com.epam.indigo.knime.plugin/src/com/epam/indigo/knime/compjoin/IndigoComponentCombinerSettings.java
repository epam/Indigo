package com.epam.indigo.knime.compjoin;

import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelFilterString;
import org.knime.core.node.defaultnodesettings.SettingsModelString;
import org.knime.core.node.util.ColumnFilter;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;
import com.epam.indigo.knime.common.IndigoNodeModel.Format;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoComponentCombinerSettings extends IndigoGeneralNodeSettings {

   public static final int INPUT_PORT = 0;

   public final SettingsModelFilterString colNames = new SettingsModelFilterString(
         "colNames");
   public final SettingsModelString newColName = new SettingsModelString(
         "newColName", "Joined molecule");
   public final SettingsModelString outputType = new SettingsModelString("outputType", 
         Format.Mol.toString());
   
   public final SettingsModelBoolean treatStringAsSMARTS = 
         new SettingsModelBoolean("treatStringAsSMARTS", false);
   
   public final ColumnFilter columnFilter = new ColumnFilter() {
      
      @Override
      public boolean includeColumn(DataColumnSpec colSpec) {
         
         boolean result = false;
         
         if (compatibleToAny(colSpec, IndigoType.MOLECULE.getClassesConvertableToIndigoDataClass()))
            result = true;

         if (compatibleToAny(colSpec, IndigoType.QUERY_MOLECULE.getClassesConvertableToIndigoDataClass()))
            result = true;
         
         return result;
      }
      
      @Override
      public String allFilteredMsg() {
         return "no convertable to indigo types column was found";
      }
      
      private boolean compatibleToAny(DataColumnSpec columnSpec, Class<? extends DataValue>[] valueClasses) {
         DataType type = columnSpec.getType();
         for (Class<? extends DataValue> cls : valueClasses) {
            if (type.isCompatible(cls)) {
               return true;
            }
         }
         return false;
      }
   };

   
   public IndigoComponentCombinerSettings() {
      addSettingsParameter(colNames);
      addSettingsParameter(newColName);
      addSettingsParameter(outputType);
      addSettingsParameter(treatStringAsSMARTS);
   }

}
