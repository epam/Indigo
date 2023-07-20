package com.epam.indigo.knime.common.splitter;

import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoSplitterSettings extends IndigoGeneralNodeSettings {

   public static final int INPUT_PORT = 0;
   
   public final SettingsModelString colName = new SettingsModelString("colName", null);
   
   public IndigoSplitterSettings() {
      addSettingsParameter(colName);
   }
   
}
