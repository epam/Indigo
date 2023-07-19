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

import org.eclipse.jface.preference.*;
import org.eclipse.ui.*;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.widgets.*;

public class IndigoPreferencePage extends FieldEditorPreferencePage implements
      IWorkbenchPreferencePage
{

   public IndigoPreferencePage()
   {
      super(GRID);

      setPreferenceStore(IndigoPlugin.getDefault().getPreferenceStore());
      setDescription("KNIME Indigo preferences");
   }

   @Override
   public void init (IWorkbench workbench)
   {
   }

   @Override
   protected void createFieldEditors () {
	   
      final Composite parent = getFieldEditorParent();

      final BooleanFieldEditor enableRenderer = new BooleanFieldEditor(IndigoPreferenceInitializer.PREF_ENABLE_RENDERER, "Enable rendering", parent);
      final BooleanFieldEditor coloredRendering = new BooleanFieldEditor(IndigoPreferenceInitializer.PREF_COLORING, "Colored rendering", parent);
      final BooleanFieldEditor showImplH = new BooleanFieldEditor(IndigoPreferenceInitializer.PREF_SHOW_IMPLICIT_HYDROGENS, "Show implicit hydrogens", parent);
      final BooleanFieldEditor valencesVisible = new BooleanFieldEditor(IndigoPreferenceInitializer.PREF_VALENCES_VISIBLE, "Render explicit valences", parent);
      final IntegerFieldEditor bondLength = new IntegerFieldEditor(IndigoPreferenceInitializer.PREF_BOND_LENGTH, "Desired bond length in pixels", parent);
      final IntegerFieldEditor imageWidth = new IntegerFieldEditor(IndigoPreferenceInitializer.PREF_MOL_IMAGE_WIDTH, "Molecule image width in pixels", parent);
      final IntegerFieldEditor imageHeight = new IntegerFieldEditor(IndigoPreferenceInitializer.PREF_MOL_IMAGE_HEIGHT, "Molecule image height in pixels", parent);
      
      String[][] labelModes = {{"none", "none"}, {"terminal-hetero", "terminal-hetero"}, {"hetero", "hetero"}, {"all", "all"}};
      final ComboFieldEditor labelMode = new ComboFieldEditor(IndigoPreferenceInitializer.PREF_LABEL_MODE, "Atom label rendering mode", labelModes, parent);
      String[][] superatomModes = {{"expand", "expand"},{"collapse", "collapse"}};
      final ComboFieldEditor superatomMode = new ComboFieldEditor(IndigoPreferenceInitializer.PREF_SUPERATOM_MODE, "Superatoms rendering mode", superatomModes, parent);
      
      final ColorFieldEditor backgroudColor = new ColorFieldEditor(IndigoPreferenceInitializer.PREF_BACKGROUND_COLOR, "Background color", parent);
      final ColorFieldEditor baseColor = new ColorFieldEditor(IndigoPreferenceInitializer.PREF_BASE_COLOR, "Default color for atoms and bonds", parent);
      
      
      ((Button)enableRenderer.getDescriptionControl(parent)).addSelectionListener(new SelectionListener()
      {
         @Override
         public void widgetSelected(SelectionEvent e)
         {
            boolean enabled = enableRenderer.getBooleanValue();
            
            coloredRendering.setEnabled(enabled, parent);
            showImplH.setEnabled(enabled, parent);
            bondLength.setEnabled(enabled, parent);
            valencesVisible.setEnabled(enabled, parent);
            imageWidth.setEnabled(enabled, parent);
            imageHeight.setEnabled(enabled, parent);     
            labelMode.setEnabled(enabled, parent);
            backgroudColor.setEnabled(enabled, parent);
            baseColor.setEnabled(enabled, parent);
            superatomMode.setEnabled(enabled, parent);
         }

         @Override
         public void widgetDefaultSelected(SelectionEvent e)
         {
         }
      });
      
      addField(enableRenderer);
      
      boolean enabled = IndigoPlugin.getDefault().getPreferenceStore().getBoolean(IndigoPreferenceInitializer.PREF_ENABLE_RENDERER);
      
      addField(coloredRendering);
      addField(showImplH);
      addField(bondLength);
      addField(valencesVisible);
      addField(imageWidth);
      addField(imageHeight);
      addField(labelMode);
      addField(superatomMode);
      addField(backgroudColor);
      addField(baseColor);

      coloredRendering.setEnabled(enabled, parent);
      showImplH.setEnabled(enabled, parent);
      bondLength.setEnabled(enabled, parent);
      valencesVisible.setEnabled(enabled, parent);
      imageWidth.setEnabled(enabled, parent);
      imageHeight.setEnabled(enabled, parent);
      labelMode.setEnabled(enabled, parent);
      superatomMode.setEnabled(enabled, parent);
      backgroudColor.setEnabled(enabled, parent);
      baseColor.setEnabled(enabled, parent);
   }
}
