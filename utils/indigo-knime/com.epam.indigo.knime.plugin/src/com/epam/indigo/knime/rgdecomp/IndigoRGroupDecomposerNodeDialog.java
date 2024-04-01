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

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.border.Border;

import org.knime.core.data.DataTableSpec;
import org.knime.core.node.*;
import org.knime.core.node.util.ColumnSelectionComboxBox;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoRGroupDecomposerNodeDialog extends NodeDialogPane
{
   IndigoRGroupDecomposerSettings nodeSettings = new IndigoRGroupDecomposerSettings();
   
   private final ColumnSelectionComboxBox cmbMolColumn;
   private final ColumnSelectionComboxBox cmbScafColumn;
   
   private final JTextField txtNewColPrefix = new JTextField(10);
   private final JTextField txtNewScafColName = new JTextField(10);
   private final JCheckBox chbAromatize = new JCheckBox("Aromatize");
   
   private final JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat input scaffold string as SMARTS");
   
   
   
   /**
    * New pane for configuring the IndigoRGroupDecomposer node.
    */
   protected IndigoRGroupDecomposerNodeDialog(IndigoType[] types)
   {
      super();
      
      // note: there is no a listener for input type; add listeners if there are a few available values
      cmbInputType = new JComboBox<>(types);
      
      // set indigo column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      cmbMolColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      IndigoType queryType = IndigoType.MOLECULE.equals(type) ?
            IndigoType.QUERY_MOLECULE : IndigoType.QUERY_REACTION; 
      cmbScafColumn = new ColumnSelectionComboxBox((Border) null, 
            queryType.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target molecule column", cmbMolColumn);
      dialogPanel.addItem("Scaffold query column", cmbScafColumn);
      dialogPanel.addItemsPanel("R-Group Decomposer Settings");
      dialogPanel.addItem("R-Group column prefix", txtNewColPrefix);
      dialogPanel.addItem("Scaffold column name", txtNewScafColName);
      dialogPanel.addItem(chbAromatize);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      
      addTab("Standard settings", dialogPanel.getPanel());
   }

   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbMolColumn, IndigoRGroupDecomposerSettings.MOL_PORT, nodeSettings.molColumn);
      nodeSettings.registerDialogComponent(cmbScafColumn, IndigoRGroupDecomposerSettings.SCAF_PORT, nodeSettings.scaffoldColumn);
      nodeSettings.registerDialogComponent(txtNewColPrefix, nodeSettings.newColPrefix);
      nodeSettings.registerDialogComponent(txtNewScafColName, nodeSettings.newScafColName);
      nodeSettings.registerDialogComponent(chbAromatize, nodeSettings.aromatize);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      nodeSettings.registerDialogComponent(chbTreatStringAsSMARTS, nodeSettings.treatStringAsSMARTS);
   }
   
   @Override
   protected void loadSettingsFrom (final NodeSettingsRO settings,
         final DataTableSpec[] specs) throws NotConfigurableException
   {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
      
   }

   @Override
   protected void saveSettingsTo (NodeSettingsWO settings)
         throws InvalidSettingsException
   {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }   
}
