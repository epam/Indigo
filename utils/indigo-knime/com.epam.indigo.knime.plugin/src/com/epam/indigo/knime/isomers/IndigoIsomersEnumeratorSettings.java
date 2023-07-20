package com.epam.indigo.knime.isomers;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoIsomersEnumeratorSettings extends IndigoGeneralNodeSettings {

   public static final int INPUT_PORT = 0;

   public final SettingsModelString colName = new SettingsModelString("colName", null);
   public final SettingsModelBoolean appendColumn = new SettingsModelBoolean("appendColumn", false);
   public final SettingsModelString newColName = new SettingsModelString("newColName", null);
   public final SettingsModelBoolean appendRowidColumn = new SettingsModelBoolean("appendRowidColumn", false);
   public final SettingsModelString newRowidColName = new SettingsModelString("newRowidColName", null);
   public final SettingsModelBoolean cisTransIsomers = new SettingsModelBoolean("cisTransIsomers", true);
   public final SettingsModelBoolean tetrahedralIsomers = new SettingsModelBoolean("tetrahedralIsomers", false);
   
   public IndigoIsomersEnumeratorSettings() {
      addSettingsParameter(colName);
      addSettingsParameter(appendColumn);
      addSettingsParameter(newColName);
      addSettingsParameter(appendRowidColumn);
      addSettingsParameter(newRowidColName);
      addSettingsParameter(cisTransIsomers);
      addSettingsParameter(tetrahedralIsomers);
   }

}
