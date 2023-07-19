package com.epam.indigo.knime.canonsmiles;

import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoCanonicalSmilesNodeSettings extends IndigoGeneralNodeSettings {

   public static final int INPUT_PORT = 0;
   
   public final SettingsModelString colName = new SettingsModelString("colName", null);
   public final SettingsModelString newColName = new SettingsModelString("newColName", null);
   
   public IndigoCanonicalSmilesNodeSettings() {
      addSettingsParameter(colName);
      addSettingsParameter(newColName);
   }
   
}
