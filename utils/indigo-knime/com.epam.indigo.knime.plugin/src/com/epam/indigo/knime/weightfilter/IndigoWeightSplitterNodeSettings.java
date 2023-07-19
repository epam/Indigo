package com.epam.indigo.knime.weightfilter;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelDoubleBounded;

import com.epam.indigo.knime.common.splitter.IndigoSplitterSettings;

public class IndigoWeightSplitterNodeSettings extends IndigoSplitterSettings {
   
   public final SettingsModelBoolean molecularWeight = new SettingsModelBoolean("Molecular weight", true);
   public final SettingsModelBoolean mostAbundantMass = new SettingsModelBoolean("Most abundant mass", false);
   public final SettingsModelBoolean monoisotopicMass = new SettingsModelBoolean("Monoisotopic mass", false);
   public final SettingsModelDoubleBounded maxMolWeight = new SettingsModelDoubleBounded("maxMolWeight", 0, 0, Double.MAX_VALUE);
   public final SettingsModelDoubleBounded minMolWeight = new SettingsModelDoubleBounded("minMolWeight", 0, 0, Double.MAX_VALUE);
   public final SettingsModelBoolean maxMolWeightActive = new SettingsModelBoolean("maxMolWeightActive", false);
   public final SettingsModelBoolean minMolWeightActive = new SettingsModelBoolean("minMolWeightActive", false);
   
   public IndigoWeightSplitterNodeSettings() {

      super();
      
      addSettingsParameter(molecularWeight);
      addSettingsParameter(mostAbundantMass);
      addSettingsParameter(monoisotopicMass);
      addSettingsParameter(minMolWeight);
      addSettingsParameter(maxMolWeight);
      addSettingsParameter(minMolWeightActive);
      addSettingsParameter(maxMolWeightActive);
   }
   
}
