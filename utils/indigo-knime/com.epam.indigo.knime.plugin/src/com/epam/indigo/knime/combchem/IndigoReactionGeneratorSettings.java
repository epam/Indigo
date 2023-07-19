package com.epam.indigo.knime.combchem;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelInteger;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;
import com.epam.indigo.knime.common.IndigoNodeModel.Format;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoReactionGeneratorSettings extends IndigoGeneralNodeSettings {
   
   public static final int REACTION_PORT = 0;
   public static final int MOL_PORT1 = 1;
   public static final int MOL_PORT2 = 2;
   
   public final SettingsModelString molColumn1 = new SettingsModelString("molColName1", null);
   public final SettingsModelString molColumn2 = new SettingsModelString("molColName2", null);
   public final SettingsModelString reactionColumn = new SettingsModelString("reactionColName", null);
   public final SettingsModelString newColName = new SettingsModelString("newColName", "Reaction");
   public final SettingsModelInteger productsCountLimit = new SettingsModelInteger("productsCountLimit", 1000);
   
   public final SettingsModelString reactionType = 
         new SettingsModelString("reactionType", IndigoType.QUERY_REACTION.toString());
   public final SettingsModelString outputType = 
         new SettingsModelString("outputType", Format.Smiles.toString());
   public final SettingsModelBoolean treatStringAsSMARTS = new SettingsModelBoolean("treatStringAsSMARTS", false);
   
   public IndigoReactionGeneratorSettings() {
      addSettingsParameter(reactionColumn);
      addSettingsParameter(molColumn1);
      addSettingsParameter(molColumn2);
      addSettingsParameter(newColName);
      addSettingsParameter(productsCountLimit);
      
      addSettingsParameter(outputType);
      addSettingsParameter(reactionType);
      addSettingsParameter(treatStringAsSMARTS);
   }
}
