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

package com.epam.indigo.knime.plugin;

import org.eclipse.core.runtime.preferences.*;
import org.eclipse.jface.preference.*;

public class IndigoPreferenceInitializer extends AbstractPreferenceInitializer
{
   public static final String PREF_BOND_LENGTH = "knime.indigo.bondLength";
   public static final String PREF_SHOW_IMPLICIT_HYDROGENS = "knime.indigo.implicitHydrogensVisible";
   public static final String PREF_COLORING = "knime.indigo.coloring";
   public static final String PREF_MOL_IMAGE_WIDTH = "knime.indigo.moleculeImageWidth";
   public static final String PREF_MOL_IMAGE_HEIGHT = "knime.indigo.moleculeImageHeight";
   public static final String PREF_LABEL_MODE = "knime.indigo.labelMode";
   public static final String PREF_ENABLE_RENDERER = "knime.indigo.enableRenderer";
   public static final String PREF_BACKGROUND_COLOR = "knime.indigo.backgroundColor";
   public static final String PREF_BASE_COLOR = "knime.indigo.baseColor";
   public static final String PREF_VALENCES_VISIBLE = "knime.indigo.valencesVisible";
   public static final String PREF_SUPERATOM_MODE = "knime.indigo.superatomMode";

   @Override
   public void initializeDefaultPreferences ()
   {
      IPreferenceStore store = IndigoPlugin.getDefault().getPreferenceStore();

      store.setDefault(PREF_BOND_LENGTH, 40);
      store.setDefault(PREF_SHOW_IMPLICIT_HYDROGENS, true);
      store.setDefault(PREF_COLORING, true);
      store.setDefault(PREF_MOL_IMAGE_WIDTH, 250);
      store.setDefault(PREF_MOL_IMAGE_HEIGHT, 150);
      store.setDefault(PREF_ENABLE_RENDERER, true);
      store.setDefault(PREF_LABEL_MODE, "terminal-hetero");
      store.setDefault(PREF_BACKGROUND_COLOR, "255,255,255");
      store.setDefault(PREF_BASE_COLOR, "0,0,0");
      store.setDefault(PREF_VALENCES_VISIBLE, false);
      store.setDefault(PREF_SUPERATOM_MODE, "expand");
   }
}
