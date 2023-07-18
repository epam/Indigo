package com.epam.indigo.knime.rbuilder;

import java.util.HashMap;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;
import com.epam.indigo.knime.common.IndigoNodeModel.Format;

public class IndigoReactionBuilderSettings extends IndigoGeneralNodeSettings{
   public static final int INPUT_PORT = 0;
   
   public final SettingsModelString reactantColName = new SettingsModelString("reactantColName", null);
   public final SettingsModelString productColName = new SettingsModelString("productColName", null);
   public final SettingsModelString catalystColName = new SettingsModelString("catalystColName", null);
   
   public final SettingsModelBoolean addReactants = new SettingsModelBoolean("addReactants", true);
   public final SettingsModelBoolean addProducts = new SettingsModelBoolean("addProducts", true);
   public final SettingsModelBoolean addCatalysts = new SettingsModelBoolean("addCatalysts", false);
   public final SettingsModelString outputType = new SettingsModelString("outputType", Format.Rxn.toString());
   
   public final SettingsModelString newColName = new SettingsModelString("newColName", "Result reaction");
   
   public final SettingsModelBoolean treatStringAsSMARTS = new SettingsModelBoolean("treatStringAsSMARTS", false);
   
   private final HashMap<SettingsModelBoolean, SettingsModelString> settingsColumnMap = new HashMap<SettingsModelBoolean, SettingsModelString>();
   
   public IndigoReactionBuilderSettings() {
      addSettingsParameter(reactantColName);
      addSettingsParameter(productColName);
      addSettingsParameter(catalystColName);
      addSettingsParameter(addReactants);
      addSettingsParameter(addProducts);
      addSettingsParameter(addCatalysts);
      addSettingsParameter(newColName);
      addSettingsParameter(outputType);
      
      addSettingsParameter(treatStringAsSMARTS);
      
      settingsColumnMap.put(addReactants, reactantColName);
      settingsColumnMap.put(addProducts, productColName);
      settingsColumnMap.put(addCatalysts, catalystColName);
   }

   public HashMap<SettingsModelBoolean, SettingsModelString> getSettingsColumnMap() {
      return settingsColumnMap;
   }
   
}
