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

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.data.*;
import org.knime.core.node.*;
import org.knime.core.node.util.*;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.submatchcounter.IndigoSubstructureMatchCounterSettings.Uniqueness;

public class IndigoSubstructureMatchCounterNodeDialog extends NodeDialogPane {

   private final IndigoSubstructureMatchCounterSettings nodeSettings = new IndigoSubstructureMatchCounterSettings();
   
   private final ColumnSelectionComboxBox cmbTargetColumn;
   private final ColumnSelectionComboxBox cmbQueryColumn;

   private final JTextField txtNewColName = new JTextField(20);
   private final JComboBox<Object> cmbUniqueness = new JComboBox<Object>(new Object[] { Uniqueness.Atoms, Uniqueness.Bonds, Uniqueness.None });
   private final JCheckBox chbHighlight = new JCheckBox("Highlight all matches");
   private final JCheckBox chbAppendColumn = new JCheckBox("Append column");
   private final JTextField txtAppendColumnName = new JTextField(20);
   private final JRadioButton rbUseNewColumnName = new JRadioButton("New column name");
   private final JRadioButton rbUseQueryColumnName = new JRadioButton("Use name from the column");

   @SuppressWarnings("unchecked")
   private final ColumnSelectionComboxBox cmbQueryCounterColumn = new ColumnSelectionComboxBox((Border) null, StringValue.class);
   
   private final JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat input query string as SMARTS");

   private DataTableSpec targetSpec;
   private DataTableSpec querySpec;
   
   private final ChangeListener changeListener = new ChangeListener() {
      public void stateChanged (ChangeEvent e)
      {
         boolean enabled = chbHighlight.isSelected();
         chbAppendColumn.setEnabled(enabled);
         
         if (!enabled)
            chbAppendColumn.setSelected(false);
         
         if (chbAppendColumn.isEnabled())
            txtAppendColumnName.setEnabled(chbAppendColumn.isSelected());
         
         if (txtAppendColumnName.isEnabled() && txtAppendColumnName.getText().length() < 1)
            txtAppendColumnName.setText(cmbTargetColumn.getSelectedColumn() + " (highlihghted)");

         txtNewColName.setEnabled(rbUseNewColumnName.isSelected());
         cmbQueryCounterColumn.setEnabled(rbUseQueryColumnName.isSelected());
      }
   };
   
   /** listens to _inputType item's state and updates comboboxes in case of changes */
   
   private final ItemListener inputTypeChangeListener = new ItemListener() {
      
      @Override
      public void itemStateChanged(ItemEvent e) {
         
         String prevType = nodeSettings.inputType.getStringValue();
         String newType = cmbInputType.getSelectedItem().toString();
         // should update if only input type value has being changed
         boolean typeChanged = !newType.equals(prevType);
         IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
         
         if (targetSpec != null && typeChanged) {
 
            Class<? extends DataValue>[] valueClasses = 
                  type.getClassesConvertableToIndigoDataClass();
            DataValueColumnFilter columnFilter = new DataValueColumnFilter(valueClasses);
 
            // update columns to pick with new column filter
            try {
               cmbTargetColumn.update(targetSpec, null, false, columnFilter);
            } catch (NotConfigurableException ex) {
               ex.printStackTrace();
            }
         }
         
         // should update if only input type value has being changed
         
         if (querySpec != null && typeChanged) {
            
            IndigoType queryType = IndigoType.QUERY_MOLECULE;
            Class<? extends DataValue>[] valueClasses = 
                  queryType.getClassesConvertableToIndigoDataClass();
            
            DataValueColumnFilter columnFilter = new DataValueColumnFilter(valueClasses);
            
            // update columns to pick with new column filter
            try {
               cmbQueryColumn.update(querySpec, null, false, columnFilter);
            } catch (NotConfigurableException ex) {
               ex.printStackTrace();
            }
         }
         
      }
   };
   
   /*
    * Not working for reactions at the moment
    */
   
//   private final ItemListener _columnChangeListener = new ItemListener() {
//      @Override
//      public void itemStateChanged(ItemEvent e) {
//         STRUCTURE_TYPE stype = _getStructureType();
//         switch(stype) {
//            case Unknown:
//               _structureType.setText("Unknown");
//               break;
//            case Reaction:
//               _structureType.setText("Reaction");
//               _newColName.setEnabled(false);
//               break;
//            case Molecule:
//               _structureType.setText("Molecule");
//               _newColName.setEnabled(true);
//               break;
//         }
//      }
//   };
   
   
   /**
    * New pane for configuring the IndigoSmartsMatcher node.
    */
   protected IndigoSubstructureMatchCounterNodeDialog(IndigoType[] types)
   {
      super();
      
      cmbInputType = new JComboBox<>(types);
      
      // set indigo column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      
      cmbTargetColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      IndigoType queryType = IndigoType.QUERY_MOLECULE;
      
      cmbQueryColumn = new ColumnSelectionComboxBox((Border) null, 
            queryType.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Input Column Settings");
//      dialogPanel.addItem("Structure type", _structureType);
      dialogPanel.addItem("Target molecule column", cmbTargetColumn);
      dialogPanel.addItem("Query column", cmbQueryColumn);
      
      dialogPanel.addItemsPanel("Result Counter Column Settings");
      dialogPanel.addItem(rbUseNewColumnName, txtNewColName);
      dialogPanel.addItem(rbUseQueryColumnName, cmbQueryCounterColumn);
      
      ButtonGroup bg = new ButtonGroup();
      bg.add(rbUseNewColumnName);
      bg.add(rbUseQueryColumnName);
      
      dialogPanel.addItemsPanel("Substructure Settings");
      dialogPanel.addItem("Uniqueness", cmbUniqueness);
      dialogPanel.addItem(chbHighlight);
      dialogPanel.addItem(chbAppendColumn, txtAppendColumnName);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      /*
       * Add change listeners
       */
      chbHighlight.addChangeListener(changeListener);
      chbAppendColumn.addChangeListener(changeListener);
      rbUseNewColumnName.addChangeListener(changeListener);
      rbUseQueryColumnName.addChangeListener(changeListener);
      
      cmbInputType.addItemListener(inputTypeChangeListener);
      
      chbHighlight.setSelected(false);
      chbAppendColumn.setEnabled(false);
      txtAppendColumnName.setEnabled(false);
      
      addTab("Standard settings", dialogPanel.getPanel());
   }

   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbTargetColumn, IndigoSubstructureMatchCounterNodeModel.TARGET_PORT, nodeSettings.targetColName);
      nodeSettings.registerDialogComponent(cmbQueryColumn, IndigoSubstructureMatchCounterNodeModel.QUERY_PORT, nodeSettings.queryColName);
      nodeSettings.registerDialogComponent(cmbQueryCounterColumn, IndigoSubstructureMatchCounterNodeModel.QUERY_PORT, nodeSettings.queryCounterColName);
      
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(cmbUniqueness, nodeSettings.uniqueness);
      nodeSettings.registerDialogComponent(chbHighlight, nodeSettings.highlight);
      nodeSettings.registerDialogComponent(chbAppendColumn, nodeSettings.appendColumn);
      nodeSettings.registerDialogComponent(txtAppendColumnName, nodeSettings.appendColumnName);

      nodeSettings.registerDialogComponent(rbUseNewColumnName, nodeSettings.useNewColumnName);
      nodeSettings.registerDialogComponent(rbUseQueryColumnName, nodeSettings.useQueryCoumnName);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      nodeSettings.registerDialogComponent(chbTreatStringAsSMARTS, nodeSettings.treatStringAsSMARTS);
   }
   
   @Override
   protected void loadSettingsFrom(final NodeSettingsRO settings, final DataTableSpec[] specs) throws NotConfigurableException {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);

         changeListener.stateChanged(null);
         
         targetSpec = specs[IndigoSubstructureMatchCounterNodeModel.TARGET_PORT];
         querySpec = specs[IndigoSubstructureMatchCounterNodeModel.QUERY_PORT];
         /*
          * Update mode
          */
//         _columnChangeListener.itemStateChanged(null);
         inputTypeChangeListener.itemStateChanged(null);
         
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
