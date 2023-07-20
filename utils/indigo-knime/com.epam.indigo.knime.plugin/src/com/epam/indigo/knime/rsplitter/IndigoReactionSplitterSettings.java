package com.epam.indigo.knime.rsplitter;

import java.util.HashMap;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoReactionSplitterSettings extends IndigoGeneralNodeSettings {
   
   public static final int INPUT_PORT = 0;
   
   public final SettingsModelString reactionColumn = new SettingsModelString("reactionColumn", null);
   
   public final SettingsModelString reactantColName = new SettingsModelString("reactantColName", "Reactants");
   public final SettingsModelString productColName = new SettingsModelString("productColName", "Products");
   public final SettingsModelString catalystColName = new SettingsModelString("catalystColName", "Catalysts");
   
   public final SettingsModelBoolean extractReactants = new SettingsModelBoolean("extractReactants", true);
   public final SettingsModelBoolean extractProducts = new SettingsModelBoolean("extractProducts", true);
   public final SettingsModelBoolean extractCatalysts = new SettingsModelBoolean("extractCatalysts", false);
   
   public final SettingsModelBoolean treatStringAsSMARTS = new SettingsModelBoolean("treatStringAsSMARTS", false);
   
   private final HashMap<SettingsModelBoolean, SettingsModelString> settingsColumnMap = new HashMap<SettingsModelBoolean, SettingsModelString>();;
   
   public IndigoReactionSplitterSettings() {
      addSettingsParameter(reactionColumn);
      addSettingsParameter(reactantColName);
      addSettingsParameter(productColName);
      addSettingsParameter(catalystColName);
      addSettingsParameter(extractReactants);
      addSettingsParameter(extractProducts);
      addSettingsParameter(extractCatalysts);
      
      addSettingsParameter(treatStringAsSMARTS);
      
      settingsColumnMap.put(extractReactants, reactantColName);
      settingsColumnMap.put(extractProducts, productColName);
      settingsColumnMap.put(extractCatalysts, catalystColName);
   }
   
   public HashMap<SettingsModelBoolean, SettingsModelString> getSettingsColumnMap() {
      return settingsColumnMap;
   }
   
}
