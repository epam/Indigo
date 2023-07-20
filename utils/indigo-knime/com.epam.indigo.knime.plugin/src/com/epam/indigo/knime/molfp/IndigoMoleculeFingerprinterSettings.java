/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems, Inc.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 ***************************************************************************/

package com.epam.indigo.knime.molfp;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelIntegerBounded;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoMoleculeFingerprinterSettings extends IndigoGeneralNodeSettings
{
   public static final int INPUT_PORT = 0;
   
   public static final int FP_SIM_DEFAULT = 8;
   public static final int FP_ORD_DEFAULT = 0;
   public static final int FP_TAU_DEFAULT = 0;
   public static final int FP_ANY_DEFAULT = 0;
   
   public static final int FP_MIN = 0;
   public static final int FP_MAX = 1000000;

   public static final boolean FP_EXT_ENABLED_DEFAULT = false;
   
   public final SettingsModelString colName = new SettingsModelString("colName", null);
   public final SettingsModelString newColName = new SettingsModelString("newColName", null);
   
   
   public final SettingsModelIntegerBounded fpSimQWords = new SettingsModelIntegerBounded("fpSimQWords", FP_SIM_DEFAULT, FP_MIN, FP_MAX);
   public final SettingsModelIntegerBounded fpOrdQWords = new SettingsModelIntegerBounded("fpOrdQWords", FP_ORD_DEFAULT, FP_MIN, FP_MAX);
   public final SettingsModelIntegerBounded fpTauQWords = new SettingsModelIntegerBounded("fpTauQWords", FP_TAU_DEFAULT, FP_MIN, FP_MAX);
   public final SettingsModelIntegerBounded fpAnyQWords = new SettingsModelIntegerBounded("fpAnyQWords", FP_ANY_DEFAULT, FP_MIN, FP_MAX);
   
   // radio buttons group
   public final SettingsModelBoolean similarityFp = new SettingsModelBoolean("similarityFp", true);
   public final SettingsModelBoolean substructureFp  = new SettingsModelBoolean("substructureFp", false);
   public final SettingsModelBoolean substructureResonanceFp  = new SettingsModelBoolean("substructureResonanceFp", false);
   public final SettingsModelBoolean substructureTautomerFp  = new SettingsModelBoolean("substructureTautomerFp", false);
   public final SettingsModelBoolean fullFp  = new SettingsModelBoolean("fullFp", false);
   
   public final SettingsModelBoolean includeEXTPart = new SettingsModelBoolean("includeEXTPart", FP_EXT_ENABLED_DEFAULT);
   public final SettingsModelBoolean denseFormat = new SettingsModelBoolean("denseFormat", false);
   
   public IndigoMoleculeFingerprinterSettings() {
      addSettingsParameter(colName);
      addSettingsParameter(newColName);
      addSettingsParameter(fpSimQWords);
      addSettingsParameter(fpOrdQWords);
      addSettingsParameter(fpTauQWords);
      addSettingsParameter(fpAnyQWords);
      addSettingsParameter(similarityFp);
      addSettingsParameter(substructureFp);
      addSettingsParameter(substructureResonanceFp);
      addSettingsParameter(substructureTautomerFp);
      addSettingsParameter(fullFp);
      addSettingsParameter(includeEXTPart);
      addSettingsParameter(denseFormat);
   }
}
