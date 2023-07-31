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

import java.awt.*;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Map;
import java.util.List;
import java.util.Set;

import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import org.knime.core.data.DataTableSpec;
import org.knime.core.node.*;
import org.knime.core.node.util.*;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.IndigoItemsFilterPanel;
import com.epam.indigo.knime.common.IndigoListPanel;
import com.epam.indigo.knime.common.IndigoNodeSettings;
import com.epam.indigo.knime.common.components.FilteredList;
import com.epam.indigo.knime.common.components.FilteredListModel;
import com.epam.indigo.knime.common.components.FilteredListPanel;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.molprop.IndigoMoleculePropertiesUtils.PropertyCalc;

public class IndigoMoleculePropertiesNodeDialog extends NodeDialogPane
{

   private final IndigoMoleculePropertiesSettings nodeSettings = new IndigoMoleculePropertiesSettings();

   private final ColumnSelectionComboxBox cmbIndigoColumn;
   private final IndigoItemsFilterPanel<PropertyCalc> filterPanel = new IndigoItemsFilterPanel<PropertyCalc>(
               new FilteredListPanel<PropertyCalc>(new LeftListModel(), new ColumnCellRenderer()), 
               new RightPanel());
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");

   protected IndigoMoleculePropertiesNodeDialog(IndigoType[] types)
   {
      super();

      cmbInputType = new JComboBox<>(types);
      
      // set indigo column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      cmbIndigoColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      filterPanel.uploadItems(getCalculators(), filterPanel.getRightPanel());
      
      dialogPanel.addItemsPanel("Column settings");
      dialogPanel.addItem("Target column", cmbIndigoColumn);
      
      dialogPanel.addItemsPanel("Append columns");
      dialogPanel.addItem(filterPanel);

      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      addTab("Properties and target column", dialogPanel.getPanel());
   }

  private void registerDialogComponents() {
     nodeSettings.registerDialogComponent(cmbIndigoColumn, 0, nodeSettings.colName);
     nodeSettings.registerDialogComponent(((RightPanel) filterPanel.getRightPanel()).getUserSpecifiedAtomsField(), 
                                       nodeSettings.userSpecifiedAtoms);
     nodeSettings.registerDialogComponent(((RightList)filterPanel.getRightPanel().getList()).getFilteredField(),
                                       nodeSettings.searchEditField);    
     nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
     nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
     nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
   }

 /**
    * @return array of calculators from static field
    */
   private PropertyCalc[] getCalculators() {
      Map<String, PropertyCalc> map = IndigoMoleculePropertiesUtils.calculators;
      return map.values().toArray(new PropertyCalc[map.size()]);
   }
   
   /** A panel to be put on IndigoItemsFilterPanel, right panel. */
   @SuppressWarnings("serial")
   private class RightPanel extends IndigoListPanel<PropertyCalc> {
      
      private final JTextField userSpecifiedAtomsField;

      public RightPanel() {
         
         super(new RightList(new RightListModel()), new ColumnCellRenderer());

         final RightList list = ((RightList) this.getList());
         
         final JTextField searchField = list.getFilteredField();
         final JTextField atomsField = list.getAtomsField();
         final JLabel lblAtoms = list.getAtomsLabel();
         atomsField.setEnabled(false); // as user-specified calculator is not selected at the init state
         lblAtoms.setEnabled(false); // as user-specified calculator is not selected at the init state
         
         userSpecifiedAtomsField = atomsField;
         
         JScrollPane avScroller = new JScrollPane(list);
         avScroller.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

         searchField.setToolTipText("Search");
         atomsField.setToolTipText("List here comma-separated elements for the 'User-specified atoms count'");
       
         // fill panel up
         GridBagLayout gbl = new GridBagLayout();
         gbl.columnWidths = new int[]{0, 0, 0};
         gbl.rowHeights = new int[]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
         gbl.columnWeights = new double[]{0.0, 1.0, Double.MIN_VALUE};
         gbl.rowWeights = new double[]{0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
         this.setLayout(gbl);
          
         GridBagConstraints gbcSearch = new GridBagConstraints();
         gbcSearch.insets = new Insets(0, 0, 5, 0);
         gbcSearch.fill = GridBagConstraints.HORIZONTAL;
         gbcSearch.gridx = 0;
         gbcSearch.gridy = 0;
         gbcSearch.gridwidth = 2;
         this.add(searchField, gbcSearch);
         searchField.setColumns(10);
          
         GridBagConstraints gbcList = new GridBagConstraints();
         gbcList.fill = GridBagConstraints.BOTH;
         gbcList.gridx = 0;
         gbcList.gridy = 1;
         gbcList.gridwidth = 2;
         gbcList.gridheight = 8;
         this.add(avScroller, gbcList);
         
         lblAtoms.setHorizontalAlignment(SwingConstants.LEFT);
         lblAtoms.setVerticalAlignment(SwingConstants.BOTTOM);
         GridBagConstraints gbcLblAtoms = new GridBagConstraints();
         gbcLblAtoms.insets = new Insets(5, 0, 1, 0);
         gbcLblAtoms.gridx = 0;
         gbcLblAtoms.gridy = 9;
         add(lblAtoms, gbcLblAtoms);
         
         GridBagConstraints gbcAtoms = new GridBagConstraints();
         gbcAtoms.fill = GridBagConstraints.HORIZONTAL;
         gbcAtoms.insets = new Insets(5, 0, 1, 0);
         gbcAtoms.gridx = 1;
         gbcAtoms.gridy = 9;
         atomsField.setColumns(10);
         add(atomsField, gbcAtoms);
         
         
         /**
          * Listener to en(dis)able text edit field intended to list user-specified atoms 
          */
         list.addListSelectionListener(new ListSelectionListener() {
            
            private final String calc = "User-specified atoms count";

            /**
             * @return true if selected 'User-specified atoms count' calculator only to set atoms list
             */
            private boolean selectedUserSpecifiedCalcOnly() {
               List<PropertyCalc> selected = list.getSelectedValuesList();
               boolean result = selected.contains(IndigoMoleculePropertiesUtils.calculators.get(calc)) 
                                 && selected.size() == 1;
               return result;
            }
            
            @Override
            public void valueChanged(ListSelectionEvent e) {
               boolean enabled = selectedUserSpecifiedCalcOnly();
               atomsField.setEnabled(enabled);
               lblAtoms.setEnabled(enabled);
            }
         });
         
      }

      public JTextField getUserSpecifiedAtomsField() {
         return userSpecifiedAtomsField;
      }
      
   }
   
   /** A list to be put on the RightPanel */
   @SuppressWarnings("serial")
   private class RightList extends FilteredList<PropertyCalc> {

      private AtomsField atomsField;
      private JLabel atomsLabel;
      
      public RightList(RightListModel model) {
         super(model);
         atomsField = new AtomsField(model);
         atomsLabel = new JLabel("Atoms: ");
      }

      public JTextField getAtomsField() {
         return atomsField;
      }
      
      public JLabel getAtomsLabel() {
         return atomsLabel;
      }
      
      /** EditField to enter user-specified atoms for correspondent calculator */
      private class AtomsField extends JTextField implements DocumentListener {

         RightListModel model;
         
         public AtomsField(RightListModel model) {
            super();
            this.model = model;
            getDocument().addDocumentListener(this);
         }
         
         private void updateList() {
            model.setUserSpecifiedAtoms(this.getText());
         }
         
         @Override
         public void changedUpdate(DocumentEvent arg0) {
            updateList();            
         }

         @Override
         public void insertUpdate(DocumentEvent arg0) {
            updateList();            
         }

         @Override
         public void removeUpdate(DocumentEvent arg0) {
            updateList();            
         }
         
      }
      
   }
   
   /**
    * List model class for the left list on the panel.
    */
   @SuppressWarnings("serial")
   class LeftListModel extends FilteredListModel<PropertyCalc> {
      
      @Override
      public void saveSettingsTo(IndigoNodeSettings settings) {
      }

      @Override
      public void loadSettingsFrom(IndigoNodeSettings settings) {
         String[] props = ((IndigoMoleculePropertiesSettings)settings)
               .selectedProps.getStringArrayValue();
         Set<String> selected = new HashSet<String>(Arrays.asList(props));
         Set<String> allCalculators = IndigoMoleculePropertiesUtils.calculators.keySet(); 
         // add all the unselected calculators to the left list
         for (String calculator: allCalculators) {
            if (!selected.contains(calculator))
            addItem(IndigoMoleculePropertiesUtils.calculators.get(calculator));
         }
      }
   }

   /**
    * List model class for the right list on the panel.
    */
   @SuppressWarnings("serial")
   class RightListModel extends FilteredListModel<PropertyCalc> {

      private String userSpecifiedAtoms;
      
      public RightListModel() {
         super();
         setUserSpecifiedAtoms("");
      }
      
      @Override
      public void saveSettingsTo(IndigoNodeSettings settings) {
         /**
         * Saves settings from the dialog to the settings object
         */
        String[] selected = new String[getSize()];
        for (int i = 0; i < selected.length; i++) {
           selected[i] = (getElementAt(i)).getName();
        }
        ((IndigoMoleculePropertiesSettings)settings).
                          selectedProps.setStringArrayValue(selected);  
        ((IndigoMoleculePropertiesSettings)settings).
                          userSpecifiedAtoms.setStringValue(userSpecifiedAtoms);
      }

      @Override
      public void loadSettingsFrom(IndigoNodeSettings settings) {
         String[] selected = ((IndigoMoleculePropertiesSettings)settings)
               .selectedProps.getStringArrayValue();
         for (int i = 0; i < selected.length; i++) {
            addItem(IndigoMoleculePropertiesUtils.calculators.get(selected[i]));
         }         
      }

      public String getUserSpecifiedAtoms() {
         return userSpecifiedAtoms;
      }

      public void setUserSpecifiedAtoms(String userSpecifiedAtoms) {
         this.userSpecifiedAtoms = userSpecifiedAtoms;
      }
   }
   
   /** Cell renderer to  display a cell with calculator object inside */
   @SuppressWarnings("serial")
   class ColumnCellRenderer extends JLabel implements ListCellRenderer<PropertyCalc> {

      
      public ColumnCellRenderer() {
         setOpaque(true);
      }
      
      @Override
      public Component getListCellRendererComponent(
            JList<? extends PropertyCalc> list, PropertyCalc value,
            int index, boolean isSelected, boolean cellHasFocus) {
         PropertyCalc calc = (PropertyCalc) value;
         // define items view
         setText(calc.getName());
         if (isSelected) {
            setBackground(Color.getHSBColor(0.56f, 0.9f, 0.8f)); 
            setForeground(Color.white);
         } else {
            setBackground(Color.white);
            setForeground(Color.black);
         }
         return this;
      }
      
   }
   
   @Override
   protected void saveSettingsTo (NodeSettingsWO settings)
         throws InvalidSettingsException
   {
      filterPanel.saveSettingsTo(nodeSettings);
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
         if (nodeSettings.selectedProps.getStringArrayValue() != null) {
            filterPanel.loadSettingsFrom(nodeSettings);
         }
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
      
   }
}
