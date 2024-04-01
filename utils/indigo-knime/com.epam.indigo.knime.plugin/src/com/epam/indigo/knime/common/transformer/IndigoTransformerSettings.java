package com.epam.indigo.knime.common.transformer;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoTransformerSettings extends IndigoGeneralNodeSettings {
   
   public static final int INPUT_PORT = 0;
   
   public final SettingsModelString colName = new SettingsModelString("colName", null);
   public final SettingsModelBoolean appendColumn = new SettingsModelBoolean("replaceColumn", true);
   public final SettingsModelString newColName = new SettingsModelString("newColName", null);
   
   public IndigoTransformerSettings() {
      addSettingsParameter(colName);
      addSettingsParameter(appendColumn);
      addSettingsParameter(newColName);
   }
   
}
