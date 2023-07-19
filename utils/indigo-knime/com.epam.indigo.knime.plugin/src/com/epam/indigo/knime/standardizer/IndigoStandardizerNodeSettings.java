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

package com.epam.indigo.knime.standardizer;

import org.knime.core.node.defaultnodesettings.SettingsModelStringArray;

import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;

public class IndigoStandardizerNodeSettings extends IndigoTransformerSettings {
   
   public final SettingsModelStringArray pickedOptions = 
         new SettingsModelStringArray("options", new String[0]);
   
   public IndigoStandardizerNodeSettings() {

      super();
      addSettingsParameter(pickedOptions);
      
   }
   
}
