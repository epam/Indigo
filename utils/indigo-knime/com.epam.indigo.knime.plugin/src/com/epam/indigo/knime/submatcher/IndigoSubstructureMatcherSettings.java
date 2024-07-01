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

package com.epam.indigo.knime.submatcher;

import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelInteger;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoSubstructureMatcherSettings extends IndigoGeneralNodeSettings {
   
   enum MoleculeMode {
      Normal, Tautomer, Resonance
   }
   enum ReactionMode {
      Standard, DaylightAAM
   }
   
   public final SettingsModelString targetColName = new SettingsModelString("colName", null);
   public final SettingsModelString queryColName = new SettingsModelString("colName2", null);
   public final SettingsModelString mode = new SettingsModelString("mode", MoleculeMode.Normal.toString());
   public final SettingsModelBoolean exact = new SettingsModelBoolean("exact", false);
   public final SettingsModelBoolean align = new SettingsModelBoolean("align", false);
   public final SettingsModelBoolean alignByQuery = new SettingsModelBoolean("alignByQuery", false);
   public final SettingsModelBoolean highlight = new SettingsModelBoolean("highlight", false);
   public final SettingsModelBoolean appendColumn = new SettingsModelBoolean("appendColumn", false);
   public final SettingsModelString newColName = new SettingsModelString("newColName", null);
   
   public final SettingsModelBoolean appendQueryKeyColumn = new SettingsModelBoolean("appendQueryKeyColumn", false);
   public final SettingsModelString queryKeyColumn = new SettingsModelString("queryKeyColumn", null);

   public final SettingsModelBoolean appendQueryMatchCountKeyColumn = new SettingsModelBoolean("appendQueryMatchCountKeyColumn", false);
   public final SettingsModelString queryMatchCountKeyColumn = new SettingsModelString("queryMatchCountKeyColumn", null);
   
   public final SettingsModelBoolean matchAllSelected = new SettingsModelBoolean("matchAllExceptSelected", false);
   public final SettingsModelBoolean matchAnyAtLeastSelected = new SettingsModelBoolean("matchAnyAtLeastSelected", true);
   public final SettingsModelInteger matchAnyAtLeast = new SettingsModelInteger("matchAnyAtLeast", 1);
   
   public final SettingsModelBoolean treatStringAsSMARTS = new SettingsModelBoolean("treatStringAsSMARTS", false);
   /*
    * Parameter is not saved
    */
   
   public IndigoSubstructureMatcherSettings() {
      addSettingsParameter(targetColName);
      addSettingsParameter(queryColName);
      addSettingsParameter(mode);
      addSettingsParameter(exact);
      addSettingsParameter(align);
      addSettingsParameter(alignByQuery);
      addSettingsParameter(highlight);
      addSettingsParameter(appendColumn);
      addSettingsParameter(newColName);
      addSettingsParameter(appendQueryKeyColumn);
      addSettingsParameter(queryKeyColumn);
      addSettingsParameter(appendQueryMatchCountKeyColumn);
      addSettingsParameter(queryMatchCountKeyColumn);
      addSettingsParameter(matchAllSelected);
      addSettingsParameter(matchAnyAtLeastSelected);
      addSettingsParameter(matchAnyAtLeast);
      addSettingsParameter(treatStringAsSMARTS);
   }

}
