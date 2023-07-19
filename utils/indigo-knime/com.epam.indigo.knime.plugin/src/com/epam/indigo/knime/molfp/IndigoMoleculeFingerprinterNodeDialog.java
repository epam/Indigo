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

package com.epam.indigo.knime.molfp;

import java.awt.event.*;

import javax.swing.*;
import javax.swing.border.*;

import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataValue;
import org.knime.core.node.*;
import org.knime.core.node.util.*;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoMoleculeFingerprinterNodeDialog extends NodeDialogPane
{
   private final ColumnSelectionComboxBox cmbColumn;
   private final JTextField txtNewColName = new JTextField(16);

   private final JSpinner spnSimSize = new JSpinner(new SpinnerNumberModel(IndigoMoleculeFingerprinterSettings.FP_SIM_DEFAULT, 
		   IndigoMoleculeFingerprinterSettings.FP_MIN, 
		   IndigoMoleculeFingerprinterSettings.FP_MAX, 1));
   private final JSpinner spnOrdSize = new JSpinner(new SpinnerNumberModel(IndigoMoleculeFingerprinterSettings.FP_ORD_DEFAULT, 
		   IndigoMoleculeFingerprinterSettings.FP_MIN, 
		   IndigoMoleculeFingerprinterSettings.FP_MAX, 1));
   private final JSpinner spnTauSize = new JSpinner(new SpinnerNumberModel(IndigoMoleculeFingerprinterSettings.FP_TAU_DEFAULT, 
		   IndigoMoleculeFingerprinterSettings.FP_MIN, 
		   IndigoMoleculeFingerprinterSettings.FP_MAX, 1));
   private final JSpinner spnAnySize = new JSpinner(new SpinnerNumberModel(IndigoMoleculeFingerprinterSettings.FP_ANY_DEFAULT, 
		   IndigoMoleculeFingerprinterSettings.FP_MIN, 
		   IndigoMoleculeFingerprinterSettings.FP_MAX, 1));
   
   private final JRadioButton rbSimilarityFP = new JRadioButton("Similarity fingerprint");
   private final JRadioButton rbSubstructureFP = new JRadioButton("Substructure fingerprint");
   private final JRadioButton rbResonanceSubstructureFP = new JRadioButton("Resonance substructure fingerprint");
   private final JRadioButton rbTautomerSubstructureFP = new JRadioButton("Tautomer substructure fingerprint");
   private final JRadioButton rbFullFP = new JRadioButton("Full fingerprint");
   
   private DataTableSpec indigoSpec;
   
   private final JCheckBox chbIncludEXTPart = new JCheckBox("include 3-byte \"EXT\" part");
   private final JCheckBox chbDenseOutputFormat = new JCheckBox("Dense representation");
   
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   
   private final IndigoMoleculeFingerprinterSettings nodeSettings = new IndigoMoleculeFingerprinterSettings();
   
   private ItemListener changeListener = new ItemListener() {
      @Override
      public void itemStateChanged(ItemEvent arg0) {
         if ("".equals(txtNewColName.getText()))
            txtNewColName.setText(cmbColumn.getSelectedColumn() + " (fingerprint)");
      }
   };

   private final ItemListener inputTypeChangeListener = new ItemListener() {
      
      @Override
      public void itemStateChanged(ItemEvent e) {
         
         String prevType = nodeSettings.inputType.getStringValue();
         String newType = cmbInputType.getSelectedItem().toString();
         // should update if only input type value has being changed
         boolean typeChanged = !newType.equals(prevType);
         
         if (indigoSpec != null && typeChanged) {
 
            IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
            Class<? extends DataValue>[] valueClasses = 
                  type.getClassesConvertableToIndigoDataClass();
            DataValueColumnFilter columnFilter = new DataValueColumnFilter(valueClasses);
 
            // update columns to pick with new column filter
            try {
               cmbColumn.update(indigoSpec, null, false, columnFilter);
            } catch (NotConfigurableException ex) {
               ex.printStackTrace();
            }
         }
         
      }
   };
   
   /**
    * New pane for configuring the IndigoMoleculeFingerprinter node.
    */
   protected IndigoMoleculeFingerprinterNodeDialog(IndigoType[] types)
   {
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
      dialogPanel.addItem("New column name", txtNewColName);
      
      dialogPanel.addItemsPanel("Fingerprint Dense Settings");
      dialogPanel.addItem(chbDenseOutputFormat);
      
      dialogPanel.addItemsPanel("Fingerprint Size Settings");
      dialogPanel.addItem("\"Similarity\" part size in qwords:", spnSimSize);
      dialogPanel.addItem("\"Ordinary\" part size in qwords:", spnOrdSize);
      dialogPanel.addItem("\"Tautomer\" part size in qwords:", spnTauSize);
      dialogPanel.addItem("\"Any\" part size in qwords:", spnAnySize);
      dialogPanel.addItem(chbIncludEXTPart);

      
      dialogPanel.addItemsPanel("Fingerprint Type Settings");
      dialogPanel.addItem(rbSimilarityFP);
      dialogPanel.addItem(rbSubstructureFP);
      dialogPanel.addItem(rbResonanceSubstructureFP);
      dialogPanel.addItem(rbTautomerSubstructureFP);
      dialogPanel.addItem(rbFullFP);
      
      ButtonGroup bg = new ButtonGroup();
      bg.add(rbSimilarityFP);
      bg.add(rbSubstructureFP);
      bg.add(rbResonanceSubstructureFP);
      bg.add(rbTautomerSubstructureFP);
      bg.add(rbFullFP);
      
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      ((JSpinner.DefaultEditor)spnSimSize.getEditor()).getTextField().setColumns(2);
      ((JSpinner.DefaultEditor)spnOrdSize.getEditor()).getTextField().setColumns(2);
      ((JSpinner.DefaultEditor)spnTauSize.getEditor()).getTextField().setColumns(2);
      ((JSpinner.DefaultEditor)spnAnySize.getEditor()).getTextField().setColumns(2);

      cmbColumn.addItemListener(changeListener);
      cmbInputType.addItemListener(inputTypeChangeListener);
      
      addTab("Standard settings", dialogPanel.getPanel());
   }
   
   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbColumn, IndigoMoleculeFingerprinterSettings.INPUT_PORT, nodeSettings.colName);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(spnSimSize, nodeSettings.fpSimQWords);
      nodeSettings.registerDialogComponent(spnOrdSize, nodeSettings.fpOrdQWords);
      nodeSettings.registerDialogComponent(spnTauSize, nodeSettings.fpTauQWords);
      nodeSettings.registerDialogComponent(spnAnySize, nodeSettings.fpAnyQWords);

      nodeSettings.registerDialogComponent(rbSimilarityFP, nodeSettings.similarityFp);
      nodeSettings.registerDialogComponent(rbSubstructureFP, nodeSettings.substructureFp);
      nodeSettings.registerDialogComponent(rbResonanceSubstructureFP, nodeSettings.substructureResonanceFp);
      nodeSettings.registerDialogComponent(rbTautomerSubstructureFP, nodeSettings.substructureTautomerFp);
      nodeSettings.registerDialogComponent(rbFullFP, nodeSettings.fullFp);
      
      nodeSettings.registerDialogComponent(chbIncludEXTPart, nodeSettings.includeEXTPart);
      nodeSettings.registerDialogComponent(chbDenseOutputFormat, nodeSettings.denseFormat);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
   }
   

   @Override
   protected void saveSettingsTo (NodeSettingsWO settings)
         throws InvalidSettingsException
   {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadSettingsFrom (final NodeSettingsRO settings,
         final DataTableSpec[] specs) throws NotConfigurableException
   {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         indigoSpec = specs[IndigoMoleculeFingerprinterSettings.INPUT_PORT];
         changeListener.itemStateChanged(null);
         inputTypeChangeListener.itemStateChanged(null);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }
}
