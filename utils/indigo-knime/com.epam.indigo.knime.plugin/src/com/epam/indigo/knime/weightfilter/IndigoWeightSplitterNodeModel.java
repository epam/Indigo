package com.epam.indigo.knime.weightfilter;

import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeSettingsRO;

import com.epam.indigo.knime.common.splitter.IndigoSplitter;
import com.epam.indigo.knime.common.splitter.IndigoSplitterNodeModel;

public class IndigoWeightSplitterNodeModel extends IndigoSplitterNodeModel {

   
   public IndigoWeightSplitterNodeModel(IndigoWeightSplitterNodeSettings settings,
         IndigoSplitter splitter) {
      super(settings, splitter);
   }

   @Override
   protected void validateSettings(NodeSettingsRO settings)
         throws InvalidSettingsException {
      
      IndigoWeightSplitterNodeSettings s = new IndigoWeightSplitterNodeSettings();
      s.loadSettingsFrom(settings);
      
      // first default validation
      super.validateSettings(settings);

      // second
      boolean minActive = s.minMolWeightActive.getBooleanValue();
      boolean maxActive = s.maxMolWeightActive.getBooleanValue();
      
      double min = s.minMolWeight.getDoubleValue();
      double max = s.maxMolWeight.getDoubleValue();
      
      if ((minActive && min < 0) || (maxActive && max < 0)) 
         throw new InvalidSettingsException("Neither min nor max value can be less than 0."); 

      if ((minActive && maxActive) && ( max < min)) 
         throw new InvalidSettingsException("Max value must be more than min value."); 
         
   }
   
}
