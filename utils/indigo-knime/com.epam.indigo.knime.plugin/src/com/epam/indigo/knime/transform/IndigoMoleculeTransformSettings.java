package com.epam.indigo.knime.transform;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoMoleculeTransformSettings extends IndigoGeneralNodeSettings {
   public static final int MOL_PORT = 0;
   public static final int REACTION_PORT = 1;
   
   public final SettingsModelString molColumn = new SettingsModelString("colName", null);
   public final SettingsModelString reactionColumn = new SettingsModelString("colName2", null);
   public final SettingsModelBoolean appendColumn = new SettingsModelBoolean("appendColumn", false);
   public final SettingsModelString newColName = new SettingsModelString("newColName", null);
   public final SettingsModelBoolean treatStringAsSMARTS = 
         new SettingsModelBoolean("treatStringAsSMARTS", false);
   public final SettingsModelString rectionType = new SettingsModelString("reactionType", 
         IndigoType.QUERY_REACTION.toString());
   
   public IndigoMoleculeTransformSettings() {
      addSettingsParameter(molColumn);
      addSettingsParameter(reactionColumn);
      addSettingsParameter(appendColumn);
      addSettingsParameter(newColName);
      addSettingsParameter(treatStringAsSMARTS);
      addSettingsParameter(rectionType);
   }
}
