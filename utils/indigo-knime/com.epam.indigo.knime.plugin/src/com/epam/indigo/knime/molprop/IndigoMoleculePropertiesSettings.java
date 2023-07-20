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

package com.epam.indigo.knime.molprop;

import org.knime.core.node.defaultnodesettings.SettingsModelString;
import org.knime.core.node.defaultnodesettings.SettingsModelStringArray;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoMoleculePropertiesSettings extends IndigoGeneralNodeSettings
{
   public final SettingsModelString colName = new SettingsModelString("colName", null);
   public final SettingsModelStringArray selectedProps = new SettingsModelStringArray("selectedProps", null);
   public final SettingsModelString userSpecifiedAtoms = new SettingsModelString("userSpecifiedAtoms", null);
   
   /** The searchEditField is required just to watch that the search edit field of the right list panel is empty.
    *  This setting is saved but not loaded.*/
   public final SettingsModelString searchEditField = new SettingsModelString("searchEditField", null);

   public IndigoMoleculePropertiesSettings() {
      addSettingsParameter(colName);
      addSettingsParameter(selectedProps);
      addSettingsParameter(userSpecifiedAtoms);
      addSettingsParameter(searchEditField);
   }
   
}
