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

package com.epam.indigo.knime.submatchcounter;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelInteger;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoSubstructureMatchCounterSettings extends IndigoGeneralNodeSettings
{
   public enum Uniqueness
   {
      Atoms, Bonds, None
   }

   public final SettingsModelString targetColName = new SettingsModelString("colName", null);
   public final SettingsModelString queryColName = new SettingsModelString("colName2", null);
   public final SettingsModelString queryCounterColName = new SettingsModelString("counterColName", null);
   public final SettingsModelString newColName = new SettingsModelString("newColName", "Number of matches");
   public final SettingsModelInteger uniqueness = new SettingsModelInteger("uniqueness", Uniqueness.Atoms.ordinal());
   public final SettingsModelBoolean highlight = new SettingsModelBoolean("highlight", false);
   public final SettingsModelBoolean appendColumn = new SettingsModelBoolean("appendColumn", false);
   public final SettingsModelString appendColumnName = new SettingsModelString("newColName2", null);
   
   public final SettingsModelBoolean useNewColumnName = new SettingsModelBoolean("useNewColumnName", true);
   public final SettingsModelBoolean useQueryCoumnName = new SettingsModelBoolean("useQueryColumnName", false);
   
   public final SettingsModelBoolean treatStringAsSMARTS = new SettingsModelBoolean("treatStringAsSMARTS", false);
   
   public IndigoSubstructureMatchCounterSettings() {
      addSettingsParameter(targetColName);
      addSettingsParameter(queryColName);
      addSettingsParameter(queryCounterColName);
      addSettingsParameter(newColName);
      addSettingsParameter(uniqueness);
      addSettingsParameter(highlight);
      addSettingsParameter(appendColumn);
      addSettingsParameter(appendColumnName);
      addSettingsParameter(useNewColumnName);
      addSettingsParameter(useQueryCoumnName);
      addSettingsParameter(treatStringAsSMARTS);
   }

}
