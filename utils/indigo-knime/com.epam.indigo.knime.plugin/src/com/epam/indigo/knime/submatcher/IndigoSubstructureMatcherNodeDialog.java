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

import java.awt.*;
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
import com.epam.indigo.knime.submatcher.IndigoSubstructureMatcherSettings.MoleculeMode;
import com.epam.indigo.knime.submatcher.IndigoSubstructureMatcherSettings.ReactionMode;

public class IndigoSubstructureMatcherNodeDialog extends NodeDialogPane
{
   private final IndigoSubstructureMatcherSettings nodeSettings = new IndigoSubstructureMatcherSettings();
   
   private final ColumnSelectionComboxBox cmbTargetColumn;
   private final ColumnSelectionComboxBox cmbQueryColumn;
   
   private final JComboBox<Object> cmbMode = new JComboBox<Object>(new Object[] {MoleculeMode.Normal, MoleculeMode.Tautomer, MoleculeMode.Resonance});
   private final JCheckBox chbExact = new JCheckBox("Allow only exact matches");
   private final JCheckBox chbHighlight = new JCheckBox("Highlight matched structures");
   private final JCheckBox chbAlign = new JCheckBox("Align matched structures");
   private final JCheckBox chbAlignByQuery = new JCheckBox("Align by query");
   private final JCheckBox chbAppendColumn = new JCheckBox("Append column");
   private final JTextField chbNewColName = new JTextField(20);
   private final JCheckBox chbAppendQueryKeyColumn = new JCheckBox("Append queries row ID column");
   private final JTextField chbQueryKeyColumnName = new JTextField(20);
   
   private final JCheckBox chbAppendQueryMatchCountKeyColumn = new JCheckBox("Append match count column");
   private final JTextField txtQueryMatchCountKeyColumn = new JTextField(20);
   
   private final JRadioButton rbMatchAllExceptSelected = new JRadioButton("All queries");
   private final JRadioButton rbMatchAnyAtLeastSelected = new JRadioButton("At least ");
   private final JSpinner spnMatchAnyAtLeast = new JSpinner(new SpinnerNumberModel(1, 0, Integer.MAX_VALUE, 1));
   
   private final JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat input query string as SMARTS");
   
   private DataTableSpec targetSpec;
   private DataTableSpec querySpec;

   private static void updateNullableEdit (JTextField field, String defValue)
   {
      if (field.isEnabled() && field.getText().length() < 1)
         field.setText(defValue);
      if (!field.isEnabled() && field.getText().length() > 0 && field.getText().equals(defValue))
         field.setText("");
   }
   
   private final ChangeListener changeListener = new ChangeListener() {
      public void stateChanged (ChangeEvent e)
      {
         boolean enabled = chbHighlight.isSelected() || chbAlign.isSelected();
         chbNewColName.setEnabled(enabled);
         chbAppendColumn.setEnabled(enabled);
        
         chbAlignByQuery.setEnabled(chbAlign.isSelected());
         
         if (!enabled)
            chbAppendColumn.setSelected(false);
         
         chbQueryKeyColumnName.setEnabled(chbAppendQueryKeyColumn.isSelected());
         txtQueryMatchCountKeyColumn.setEnabled(chbAppendQueryMatchCountKeyColumn.isSelected());
         
         spnMatchAnyAtLeast.setEnabled(rbMatchAnyAtLeastSelected.isSelected());
         
         if (chbAppendColumn.isEnabled())
            chbNewColName.setEnabled(chbAppendColumn.isSelected());
         

         updateNullableEdit(chbQueryKeyColumnName, cmbTargetColumn.getSelectedColumn() + " (query row ID)");
         updateNullableEdit(txtQueryMatchCountKeyColumn, cmbTargetColumn.getSelectedColumn() + " (queries matched)");
         updateNullableEdit(chbNewColName, cmbTargetColumn.getSelectedColumn() + " (matched)");
      }
   };
   
   /** listens to _inputType item's state and updates comboboxes in case of changes */
   
   private final ItemListener columnChangeListener = new ItemListener() {
      
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
            
            IndigoType queryType = defineAppropriateQueryType(type);
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
   
   private final ItemListener inputTypeChangeListener = new ItemListener() {
      @Override
      public void itemStateChanged(ItemEvent e) {
         IndigoType stype = (IndigoType)cmbInputType.getSelectedItem();
         switch(stype) {
            case REACTION:
               cmbMode.setEnabled(true);
               cmbMode.removeAllItems();
               for(ReactionMode mode : ReactionMode.values())
                  cmbMode.addItem(mode);
               break;
            case MOLECULE:
               cmbMode.setEnabled(true);
               cmbMode.removeAllItems();
               for(MoleculeMode mode : MoleculeMode.values())
                  cmbMode.addItem(mode);
               break;
         default:
            // this in as impossible case
            break;
         }
      }
   };
   
   
   /**
    * New pane for configuring the IndigoSmartsMatcher node.
    */
   protected IndigoSubstructureMatcherNodeDialog(IndigoType[] types)
   {
      super();
      
      cmbInputType = new JComboBox<>(types);
      
      // set indigo column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      
      cmbTargetColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      IndigoType queryType = defineAppropriateQueryType(type);
      
      cmbQueryColumn = new ColumnSelectionComboxBox((Border) null, 
            queryType.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target column", cmbTargetColumn);
      dialogPanel.addItem("Query column", cmbQueryColumn);
      dialogPanel.addItem(chbAppendColumn, chbNewColName);
      dialogPanel.addItemsPanel("Substructure Settings");
      dialogPanel.addItem("Mode: ", cmbMode);
      dialogPanel.addItem(chbExact);
      dialogPanel.addItem(chbHighlight);
      dialogPanel.addItem(chbAlign, chbAlignByQuery);
      
      ((JSpinner.DefaultEditor)spnMatchAnyAtLeast.getEditor()).getTextField().setColumns(4);

      JPanel p2 = new JPanel(new GridBagLayout());
      GridBagConstraints c2 = new GridBagConstraints();
      c2.anchor = GridBagConstraints.WEST;
      c2.insets = new Insets(2, 2, 2, 2);
      c2.gridy = 0;
      c2.gridx = 0;
      
      JLabel matchLabel = new JLabel("Match");
      IndigoDialogPanel.setDefaultComponentFont(matchLabel);
      p2.add(matchLabel, c2);
      c2.gridx++;
      p2.add(rbMatchAnyAtLeastSelected, c2);
      c2.gridx++;
      p2.add(spnMatchAnyAtLeast, c2);
      c2.gridx++;
      JLabel queriesLabel = new JLabel(" queries");
      IndigoDialogPanel.setDefaultComponentFont(queriesLabel);
      p2.add(queriesLabel, c2);
     
      c2.gridy++;
      c2.gridx = 1;
      c2.gridwidth = 3;
      p2.add(rbMatchAllExceptSelected, c2);
      
      ButtonGroup bg = new ButtonGroup();
      bg.add(rbMatchAllExceptSelected);
      bg.add(rbMatchAnyAtLeastSelected);
      
      
      dialogPanel.addItem(p2, new JPanel());
      
      dialogPanel.addItemsPanel("Column Key Settings");
      dialogPanel.addItem(chbAppendQueryKeyColumn, chbQueryKeyColumnName);
      dialogPanel.addItem(chbAppendQueryMatchCountKeyColumn, txtQueryMatchCountKeyColumn);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      
      /*
       * Add all change listeners
       */
      chbAppendQueryKeyColumn.addChangeListener(changeListener);
      chbAppendQueryMatchCountKeyColumn.addChangeListener(changeListener);
      
      chbAppendColumn.addChangeListener(changeListener);
      chbAlign.addChangeListener(changeListener);
      chbHighlight.addChangeListener(changeListener);
      
      rbMatchAllExceptSelected.addChangeListener(changeListener);
      rbMatchAnyAtLeastSelected.addChangeListener(changeListener);
      /*
       * Add reaction molecule change listener
       */
      cmbTargetColumn.addItemListener(columnChangeListener);
      cmbQueryColumn.addItemListener(columnChangeListener);
      
      cmbInputType.addItemListener(inputTypeChangeListener);
      
      chbAlign.setSelected(false);
      chbHighlight.setSelected(false);
      chbAppendColumn.setEnabled(false);
      chbNewColName.setEnabled(false);
      
      addTab("Standard settings", dialogPanel.getPanel());
   }

   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbTargetColumn, IndigoSubstructureMatcherNodeModel.TARGET_PORT, nodeSettings.targetColName);
      nodeSettings.registerDialogComponent(cmbQueryColumn, IndigoSubstructureMatcherNodeModel.QUERY_PORT, nodeSettings.queryColName);
      
      nodeSettings.registerDialogComponent(chbNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(chbAlign, nodeSettings.align);
      nodeSettings.registerDialogComponent(chbAlignByQuery,nodeSettings.alignByQuery);
      nodeSettings.registerDialogComponent(chbExact, nodeSettings.exact);
      nodeSettings.registerDialogComponent(chbHighlight, nodeSettings.highlight);
      nodeSettings.registerDialogComponent(chbAppendColumn, nodeSettings.appendColumn);
      nodeSettings.registerDialogComponent(chbAppendQueryKeyColumn, nodeSettings.appendQueryKeyColumn);

      nodeSettings.registerDialogComponent(chbAppendQueryKeyColumn, nodeSettings.appendQueryKeyColumn);
      nodeSettings.registerDialogComponent(chbQueryKeyColumnName, nodeSettings.queryKeyColumn);

      nodeSettings.registerDialogComponent(chbAppendQueryMatchCountKeyColumn, nodeSettings.appendQueryMatchCountKeyColumn);
      nodeSettings.registerDialogComponent(txtQueryMatchCountKeyColumn, nodeSettings.queryMatchCountKeyColumn);

      nodeSettings.registerDialogComponent(rbMatchAllExceptSelected, nodeSettings.matchAllSelected);
      nodeSettings.registerDialogComponent(rbMatchAnyAtLeastSelected, nodeSettings.matchAnyAtLeastSelected);
      nodeSettings.registerDialogComponent(spnMatchAnyAtLeast, nodeSettings.matchAnyAtLeast);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      nodeSettings.registerDialogComponent(chbTreatStringAsSMARTS, nodeSettings.treatStringAsSMARTS);
      
   }

   private IndigoType defineAppropriateQueryType(IndigoType type) {
      if (IndigoType.MOLECULE.equals(type))
         return IndigoType.QUERY_MOLECULE;
      else
         return IndigoType.QUERY_REACTION;
   }
   
   @Override
   protected void loadSettingsFrom(final NodeSettingsRO settings, final DataTableSpec[] specs) throws NotConfigurableException {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         changeListener.stateChanged(null);
         
         targetSpec = specs[IndigoSubstructureMatcherNodeModel.TARGET_PORT];
         querySpec = specs[IndigoSubstructureMatcherNodeModel.QUERY_PORT];
         /*
          * Update mode
          */
         columnChangeListener.itemStateChanged(null);
         inputTypeChangeListener.itemStateChanged(null);
         
         String selectedMode = nodeSettings.mode.getStringValue();
         IndigoType stype = IndigoType.findByString(nodeSettings.inputType.getStringValue());
         if(selectedMode != null && selectedMode.length() > 0) {
            if(IndigoType.REACTION.equals(stype))
               cmbMode.setSelectedItem(ReactionMode.valueOf(selectedMode));
            else if (IndigoType.MOLECULE.equals(stype))
               cmbMode.setSelectedItem(MoleculeMode.valueOf(selectedMode));
         }
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }

   @Override
   protected void saveSettingsTo (NodeSettingsWO settings)
         throws InvalidSettingsException
   {
      
      nodeSettings.mode.setStringValue(cmbMode.getSelectedItem().toString());
      
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }
}
