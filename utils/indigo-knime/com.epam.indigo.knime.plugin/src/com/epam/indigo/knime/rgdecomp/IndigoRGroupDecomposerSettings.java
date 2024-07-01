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

package com.epam.indigo.knime.rgdecomp;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoRGroupDecomposerSettings extends IndigoGeneralNodeSettings
{
   public static final int MOL_PORT = 0;
   public static final int SCAF_PORT = 1;
   
   public final SettingsModelString molColumn = new SettingsModelString("colName", null);
   public final SettingsModelString scaffoldColumn = new SettingsModelString("colName2", null);
   public final SettingsModelString newColPrefix = new SettingsModelString("newColPrefix", "R-Group #");
   public final SettingsModelString newScafColName = new SettingsModelString("newScafColName", "Scaffold");
   public final SettingsModelBoolean aromatize = new SettingsModelBoolean("aromatize", true);
   
   public final SettingsModelBoolean treatStringAsSMARTS = new SettingsModelBoolean("treatStringAsSMARTS", false);
   
   public IndigoRGroupDecomposerSettings() {
      addSettingsParameter(molColumn);
      addSettingsParameter(scaffoldColumn);
      addSettingsParameter(newColPrefix);
      addSettingsParameter(newScafColName);
      addSettingsParameter(aromatize);
      addSettingsParameter(treatStringAsSMARTS);
   }
}
