package com.epam.indigo.knime.formulafilter;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelInteger;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoFormulaFilterNodeSettings extends IndigoGeneralNodeSettings {

   public final SettingsModelString targetColName = new SettingsModelString("colName", null);
   public final SettingsModelString queryColName = new SettingsModelString("colName2", null);
   
   public final SettingsModelBoolean appendQueryKeyColumn = new SettingsModelBoolean("appendQueryKeyColumn", false);
   public final SettingsModelString queryKeyColumn = new SettingsModelString("queryKeyColumn", null);

   public final SettingsModelBoolean appendQueryMatchCountKeyColumn = new SettingsModelBoolean("appendQueryMatchCountKeyColumn", false);
   public final SettingsModelString queryMatchCountKeyColumn = new SettingsModelString("queryMatchCountKeyColumn", null);
   
   public final SettingsModelBoolean lessThanOrEqualComparison = new SettingsModelBoolean("lessThanOrEqualComparison", false);
   public final SettingsModelBoolean equalComparison = new SettingsModelBoolean("equalComparison", true);
   public final SettingsModelBoolean greaterThanOrEqualComparison = new SettingsModelBoolean("greaterThanOrEqualComparison", false);
   
   public final SettingsModelBoolean matchAllSelected = new SettingsModelBoolean("matchAllSelected", false);
   public final SettingsModelBoolean matchAnyAtLeastSelected = new SettingsModelBoolean("matchAnyAtLeastSelected", true);
   public final SettingsModelInteger matchAnyAtLeast = new SettingsModelInteger("matchAnyAtLeast", 1);
   
   public IndigoFormulaFilterNodeSettings() {

      addSettingsParameter(targetColName);
      addSettingsParameter(queryColName);
      addSettingsParameter(appendQueryKeyColumn);
      addSettingsParameter(queryKeyColumn);
      addSettingsParameter(appendQueryMatchCountKeyColumn);
      addSettingsParameter(queryMatchCountKeyColumn);
      addSettingsParameter(lessThanOrEqualComparison);
      addSettingsParameter(equalComparison);
      addSettingsParameter(greaterThanOrEqualComparison);
      addSettingsParameter(matchAllSelected);
      addSettingsParameter(matchAnyAtLeastSelected);
      addSettingsParameter(matchAnyAtLeast);
      
   }
   
}
