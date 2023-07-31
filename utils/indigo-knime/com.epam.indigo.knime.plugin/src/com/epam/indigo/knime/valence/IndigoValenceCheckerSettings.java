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

package com.epam.indigo.knime.valence;

import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoGeneralNodeSettings;

public class IndigoValenceCheckerSettings extends IndigoGeneralNodeSettings
{
   public static final int INPUT_PORT = 0;
   
   public final SettingsModelString colName = new SettingsModelString("colName", null);

   public IndigoValenceCheckerSettings() {
      addSettingsParameter(colName);
   }
}
