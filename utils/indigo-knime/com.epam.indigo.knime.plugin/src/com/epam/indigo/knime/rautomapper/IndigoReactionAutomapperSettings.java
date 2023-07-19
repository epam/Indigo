package com.epam.indigo.knime.rautomapper;

import java.util.HashMap;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelColumnName;
import org.knime.core.node.defaultnodesettings.SettingsModelInteger;
import org.knime.core.node.defaultnodesettings.SettingsModelIntegerBounded;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;
import com.epam.indigo.knime.common.IndigoNodeModel.Format;

public class IndigoReactionAutomapperSettings extends IndigoGeneralNodeSettings {
   
   public static enum AAMode {
      Discard, Keep, Alter, Clear
   }
   
   static final String CFGKEY_IGNORE_CHARGES = "ignore_charges";
   static final String CFGKEY_IGNORE_ISOTOPES = "ignore_isotopes";
   static final String CFGKEY_IGNORE_RADICALS = "ignore_radicals";
   static final String CFGKEY_IGNORE_VALENCE = "ignore_valence";

   public final SettingsModelColumnName reactionColumn = new SettingsModelColumnName("column", null);
   public final DeprecatedSettingsModelBooleanInverse appendColumn = new DeprecatedSettingsModelBooleanInverse("replaceColumn", false);
   public final SettingsModelColumnName newColName = new SettingsModelColumnName("newColumn", null);
   public final SettingsModelInteger mode = new SettingsModelInteger("mode", AAMode.Discard.ordinal());
   
   public final SettingsModelBoolean ignoreCharges = new SettingsModelBoolean(CFGKEY_IGNORE_CHARGES, false);
   public final SettingsModelBoolean ignoreIsotopes = new SettingsModelBoolean(CFGKEY_IGNORE_ISOTOPES, false);
   public final SettingsModelBoolean ignoreRadicals = new SettingsModelBoolean(CFGKEY_IGNORE_RADICALS, false);
   public final SettingsModelBoolean ignoreValence = new SettingsModelBoolean(CFGKEY_IGNORE_VALENCE, false);
   public final SettingsModelBoolean highlightReactingCenters = new SettingsModelBoolean("highlightReactingCenters", false);
   public final SettingsModelBoolean useAamTimeout = new SettingsModelBoolean("useAamTimeout", false);
   public final SettingsModelIntegerBounded aamTimeout = new SettingsModelIntegerBounded("aamTimeout", 0, 0, Integer.MAX_VALUE);
   public final SettingsModelBoolean treatStringAsSMARTS = new SettingsModelBoolean("treatStringAsSMARTS", false);
   public final SettingsModelString outputType = new SettingsModelString("outputType", Format.Rxn.toString());
   
   private final HashMap<String, SettingsModelBoolean> ignoreFlags = new HashMap<String, SettingsModelBoolean>();

   public IndigoReactionAutomapperSettings() {
      addSettingsParameter(reactionColumn);
      addSettingsParameter(appendColumn);
      addSettingsParameter(newColName);
      addSettingsParameter(mode);
      addSettingsParameter(ignoreCharges);
      addSettingsParameter(ignoreIsotopes);
      addSettingsParameter(ignoreRadicals);
      addSettingsParameter(ignoreValence);
      addSettingsParameter(highlightReactingCenters);
      addSettingsParameter(useAamTimeout);
      addSettingsParameter(aamTimeout);
      addSettingsParameter(outputType);
      
      addSettingsParameter(treatStringAsSMARTS);
      
      ignoreFlags.put(CFGKEY_IGNORE_CHARGES, ignoreCharges);
      ignoreFlags.put(CFGKEY_IGNORE_ISOTOPES, ignoreIsotopes);
      ignoreFlags.put(CFGKEY_IGNORE_RADICALS, ignoreRadicals);
      ignoreFlags.put(CFGKEY_IGNORE_VALENCE, ignoreValence);
   }
   
   public String getAAMParameters() {
      StringBuilder result = new StringBuilder();
      /*
       * Append mode
       */
      result.append(AAMode.values()[mode.getIntValue()].name().toLowerCase());
      /*
       * Append ignore flags
       */
      for (String ignoreFlagKey : ignoreFlags.keySet()) {
         SettingsModelBoolean ignoreFlag = ignoreFlags.get(ignoreFlagKey);
         
         if(ignoreFlag.getBooleanValue()) {
            result.append(" ");
            result.append(ignoreFlagKey);
         }
      }
      return result.toString();
   }

}
