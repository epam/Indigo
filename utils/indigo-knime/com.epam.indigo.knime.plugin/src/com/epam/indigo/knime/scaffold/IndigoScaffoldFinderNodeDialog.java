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

package com.epam.indigo.knime.scaffold;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataValue;
import org.knime.core.node.*;
import org.knime.core.node.util.ColumnSelectionComboxBox;
import org.knime.core.node.util.DataValueColumnFilter;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoScaffoldFinderNodeDialog extends NodeDialogPane {

   private final ColumnSelectionComboxBox cmbColumn;

   private final JCheckBox chbTryExact = new JCheckBox("Try exact method");
   private final JSpinner spnExactIterations = new JSpinner();
   private final JSpinner spnApproxIterations = new JSpinner();

   private final JTextField txtNewColName = new JTextField(20);

   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   
   private DataTableSpec inputTableSpec; // set in constructor and at loading settings
   
   private final IndigoScaffoldFinderSettings nodeSettings = new IndigoScaffoldFinderSettings();

   private ChangeListener changeListener = new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent arg0) {
         spnExactIterations.setEnabled(chbTryExact.isSelected());
      }
   };
   
   /** listens to inputType item's state and updates indigoColumn combobox in case of changes */
   
   private final ItemListener inputTypeChangeListener = new ItemListener() {
      
      @Override
      public void itemStateChanged(ItemEvent e) {
         
         String prevType = nodeSettings.inputType.getStringValue();
         String newType = cmbInputType.getSelectedItem().toString();
         // should update if only input type value has being changed
         boolean typeChanged = !newType.equals(prevType);
         
         if (inputTableSpec != null && typeChanged) {
 
            IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
            Class<? extends DataValue>[] valueClasses = 
                  type.getClassesConvertableToIndigoDataClass();
            DataValueColumnFilter columnFilter = new DataValueColumnFilter(valueClasses);
 
            // update columns to pick with new column filter
            try {
               cmbColumn.update(inputTableSpec, null, false, columnFilter);
            } catch (NotConfigurableException ex) {
               ex.printStackTrace();
            }
         }
         
      }
   };

   /**
    * New pane for configuring the IndigoScaffoldFinder node.
    */
   protected IndigoScaffoldFinderNodeDialog(IndigoType[] types) {

      super();

      cmbInputType = new JComboBox<>(types);
      
      // set indigo column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      cmbColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target column", cmbColumn);
      dialogPanel.addItem("New column with scaffolds", txtNewColName);
      dialogPanel.addItemsPanel("Scaffold Finder Settings");
      dialogPanel.addItem(chbTryExact);
      dialogPanel.addItem("Maximum number of iterations for exact method (0 to no limit)", spnExactIterations);
      dialogPanel.addItem("Number of iterations for approximate method", spnApproxIterations);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);

      ((JSpinner.DefaultEditor) spnExactIterations.getEditor()).getTextField().setColumns(8);
      ((JSpinner.DefaultEditor) spnApproxIterations.getEditor()).getTextField().setColumns(8);
      
      chbTryExact.addChangeListener(changeListener);
      cmbInputType.addItemListener(inputTypeChangeListener);

      addTab("Standard settings", dialogPanel.getPanel());
   }

   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbColumn, IndigoScaffoldFinderSettings.INPUT_PORT, nodeSettings.colName);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(chbTryExact, nodeSettings.tryExactMethod);
      nodeSettings.registerDialogComponent(spnExactIterations, nodeSettings.maxIterExact);
      nodeSettings.registerDialogComponent(spnApproxIterations, nodeSettings.maxIterApprox);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
   }

   @Override
   protected void saveSettingsTo(NodeSettingsWO settings) throws InvalidSettingsException {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }

   @Override
   protected void loadSettingsFrom(final NodeSettingsRO settings, final DataTableSpec[] specs) throws NotConfigurableException {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);

         // fire events
         inputTypeChangeListener.itemStateChanged(null);
         changeListener.stateChanged(null);
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }
}
