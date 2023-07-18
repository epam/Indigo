package com.epam.indigo.knime.standardizer;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeLogger;
import org.knime.core.node.NodeSettingsRO;

import com.epam.indigo.knime.common.transformer.IndigoTransformerBase;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoStandardizerNodeModel extends IndigoTransformerNodeModel {

   private static final NodeLogger LOGGER = NodeLogger
         .getLogger(IndigoStandardizerNodeModel.class);
   
   public IndigoStandardizerNodeModel(String message, IndigoTransformerSettings settings,
                                      IndigoTransformerBase transformer) {
      super(message, settings, transformer);
   }
   
   @Override
   protected void validateSettings(NodeSettingsRO settings)
         throws InvalidSettingsException {

      IndigoStandardizerNodeSettings s = new IndigoStandardizerNodeSettings();
      s.loadSettingsFrom(settings);
      
      String[] pickedOptions = s.pickedOptions.getStringArrayValue();
      
      super.validateSettings(settings);
      
      // No options picked
      if (pickedOptions == null || pickedOptions.length < 1) {
         LOGGER.warn("No options set to apply.");
      }
      
      // Clearing coordinates option is in front of a coordinate-dependent option
      if (pickedOptions != null && pickedOptions.length > 0) {
         final String CLEAR_COORD = "standardize-clear-coordinates";
         List<String> lstOptions = new ArrayList<String>(Arrays.asList(pickedOptions));
         if (lstOptions.contains(CLEAR_COORD)) {
            int idx = lstOptions.indexOf(CLEAR_COORD);
            List<String> coordDependent = new ArrayList<String>();
            for (int i = idx; i < lstOptions.size(); i++) {
               if (IndigoStandardizerUtils.optionsMap.get(lstOptions.get(i)).getScope() == 3) {
                  coordDependent.add(lstOptions.get(i));
               }
            }
            if (coordDependent != null && coordDependent.size() > 0) {
               throw new InvalidSettingsException("The following options cannot be replaced after "
                     + "the standardize-clear-coordinates option: " + String.join(", ", coordDependent));
            }
         }
      }
      
      List<String> picked = new ArrayList<String>(Arrays.asList(pickedOptions));
      IndigoType inputType = IndigoType.findByString(s.inputType.getStringValue());
      
      // Find out inappropriate options for the input data
      if (IndigoType.MOLECULE.equals(inputType)) {
         List<String> inappropriate  = IndigoStandardizerUtils.getOptionsByScope(2);
         inappropriate.retainAll(picked);
         // Throw exception in case of there is at least one an inappropriate option
         if (inappropriate.size() > 0) {
            String options = String.join(", ", inappropriate);
            throw new InvalidSettingsException("Input data has molecule type. "
                  + "The following options are inappropriate: " + options + ".");
         }
      } else if (IndigoType.QUERY_MOLECULE.equals(inputType)) {
         List<String> inappropriate  = IndigoStandardizerUtils.getOptionsByScope(1);
         inappropriate.retainAll(picked);
         // Throw exception in case of there is at least one an inappropriate option
         if (inappropriate.size() > 0) {
            String options = String.join(", ", inappropriate);
            throw new InvalidSettingsException("Input data has query molecule type. "
                  + "The following options are inappropriate: " + options + ".");
         }
      }
      
   }
   
}
