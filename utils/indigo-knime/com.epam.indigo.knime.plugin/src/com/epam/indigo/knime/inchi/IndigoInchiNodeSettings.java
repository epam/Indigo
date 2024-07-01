package com.epam.indigo.knime.inchi;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoInchiNodeSettings extends IndigoGeneralNodeSettings {

   public static final int INPUT_PORT = 0;
   
   public final SettingsModelString colName = new SettingsModelString("colName", null);
   public final SettingsModelString inchiColName = new SettingsModelString("inchiColName", null);
   public final SettingsModelBoolean appendInchiKeyColumn = new SettingsModelBoolean("appendInchiKeyColumn", false);
   public final SettingsModelString inchiKeyColName = new SettingsModelString("inchiKeyColName", null);
   
   public IndigoInchiNodeSettings() {
      addSettingsParameter(colName);
      addSettingsParameter(inchiColName);
      addSettingsParameter(appendInchiKeyColumn);
      addSettingsParameter(inchiKeyColName);
   }
   
}
