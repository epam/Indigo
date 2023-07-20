package com.epam.indigo.knime.common;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.types.IndigoType;


/**
 * This class contains settings which are general almost for every node
 *
 */
public class IndigoGeneralNodeSettings extends IndigoNodeSettings {

   public final SettingsModelString inputType = new SettingsModelString("inputType", IndigoType.MOLECULE.toString());
   public final SettingsModelBoolean ignoreStereochemistryErrors = new SettingsModelBoolean("ignoreStereochemistryErrors", true);
   public final SettingsModelBoolean treatXAsPseudoatom = new SettingsModelBoolean("treatXAsPseudoatom", true);
   
   public IndigoGeneralNodeSettings() {
      addSettingsParameter(inputType);
      addSettingsParameter(ignoreStereochemistryErrors);
      addSettingsParameter(treatXAsPseudoatom);
   }
   
}
