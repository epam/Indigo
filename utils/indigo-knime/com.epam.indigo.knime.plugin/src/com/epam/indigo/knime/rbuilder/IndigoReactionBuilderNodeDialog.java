package com.epam.indigo.knime.rbuilder;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.ArrayList;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataValue;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.NotConfigurableException;
import org.knime.core.node.util.ColumnSelectionComboxBox;
import org.knime.core.node.util.DataValueColumnFilter;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.IndigoNodeModel.Format;
import com.epam.indigo.knime.common.types.IndigoType;


/**
 * 
 */
public class IndigoReactionBuilderNodeDialog extends NodeDialogPane {
   private final IndigoReactionBuilderSettings nodeSettings = new IndigoReactionBuilderSettings();
   
   private final Format[] reacFormats = new Format[] {Format.Rxn, Format.Smiles, Format.String};
   private final Format[] qreacFormatsWithSMARTS = new Format[] {Format.Rxn, Format.Smiles, Format.SMARTS, Format.String};
   
   private final ColumnSelectionComboxBox cmbReactantColName;
   private final ColumnSelectionComboxBox cmbProductColName;
   private final ColumnSelectionComboxBox cmbCatalystColName;
   
   private final JCheckBox chbAddReactants = new JCheckBox("Add reactants");
   private final JCheckBox chbAddProducts = new JCheckBox("Add products");
   private final JCheckBox chbAddCatalysts = new JCheckBox("Add catalysts");
   
   private final JTextField txtNewColName = new JTextField(20);
   private JComboBox<Format> cmbOutputType = new JComboBox<>(reacFormats);
   
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat input string as SMARTS");
   
   private DataTableSpec inputTableSpec;
   
   private final ArrayList<ChangeListener> changeListeners = new ArrayList<ChangeListener>();
   
   /*
    *  listens to _inputType item's state and updates comboboxes in case of changes 
    */
   private final ItemListener inputTypeChangeListener = new ItemListener() {
      
      @Override
      public void itemStateChanged(ItemEvent e) {
         
         String prevType = nodeSettings.inputType.getStringValue();
         String newType = cmbInputType.getSelectedItem().toString();
         // should update if only input type value has being changed
         boolean typeChanged = !newType.equals(prevType);
         
         if (inputTableSpec != null && typeChanged) {
 
            IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
            Class<? extends DataValue>[] valueClasses = type.getClassesConvertableToIndigoDataClass();
            DataValueColumnFilter columnFilter = new DataValueColumnFilter(valueClasses);
 
            // update columns to pick with new column filter
            try {
               cmbReactantColName.update(inputTableSpec, null, false, columnFilter);
               cmbProductColName.update(inputTableSpec, null, false, columnFilter);
               cmbCatalystColName.update(inputTableSpec, null, false, columnFilter);
            } catch (NotConfigurableException ex) {
               ex.printStackTrace();
            }
         }
         
      }
   };
   
   private final ActionListener inputTypeActionListener = new ActionListener() {
      
      @Override
      public void actionPerformed(ActionEvent e) {
         IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
         boolean query = IndigoType.QUERY_MOLECULE.equals(type);
         chbTreatStringAsSMARTS.setEnabled(query);
         if (!chbTreatStringAsSMARTS.isEnabled()) {
            chbTreatStringAsSMARTS.setSelected(false);
         }
      }
   };
   
   private final ActionListener chbTreatAsSmarts = new ActionListener() {
      
      @Override
      public void actionPerformed(ActionEvent e) {
         
         IndigoType type = IndigoType.findByString(cmbInputType.getSelectedItem().toString());
         
         if (IndigoType.QUERY_MOLECULE.equals(type)){
            Format[] formats = chbTreatStringAsSMARTS.isSelected() ? qreacFormatsWithSMARTS : 
               reacFormats;
            cmbOutputType.removeAllItems();
            for (Format format : formats) {
               cmbOutputType.addItem(format);
            }
         }
         
      }
   };

   protected IndigoReactionBuilderNodeDialog(IndigoType[] types) {
      
      super();
      
      cmbInputType = new JComboBox<>(types);
      
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      
      // define column's comboboxes
      cmbReactantColName = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      cmbProductColName = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      cmbCatalystColName = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem(chbAddReactants, cmbReactantColName);
      dialogPanel.addItem(chbAddProducts, cmbProductColName);
      dialogPanel.addItem(chbAddCatalysts, cmbCatalystColName);
      dialogPanel.addItem("Result column name", txtNewColName);

      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem("Output type", cmbOutputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      
      /*
       * Add change listeners
       */
      ChangeListener reactListener = new ChangeListener() {
         public void stateChanged(ChangeEvent e) {
            cmbReactantColName.setEnabled(chbAddReactants.isSelected());
         }
      };
      ChangeListener productListener = new ChangeListener() {
         public void stateChanged(ChangeEvent e) {
            cmbProductColName.setEnabled(chbAddProducts.isSelected());
         }
      };
      ChangeListener catalystListener = new ChangeListener() {
         public void stateChanged(ChangeEvent e) {
            cmbCatalystColName.setEnabled(chbAddCatalysts.isSelected());
         }
      };
      changeListeners.add(reactListener);
      changeListeners.add(productListener);
      changeListeners.add(catalystListener);
      
      chbAddReactants.addChangeListener(reactListener);
      chbAddProducts.addChangeListener(productListener);
      chbAddCatalysts.addChangeListener(catalystListener);
      
      cmbInputType.addItemListener(inputTypeChangeListener);
      cmbInputType.addActionListener(inputTypeActionListener);
      chbTreatStringAsSMARTS.addActionListener(chbTreatAsSmarts);
      
      addTab("Standard Settings", dialogPanel.getPanel());
   }
   
   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbReactantColName, 
            IndigoReactionBuilderSettings.INPUT_PORT, nodeSettings.reactantColName);
      nodeSettings.registerDialogComponent(cmbProductColName, 
            IndigoReactionBuilderSettings.INPUT_PORT, nodeSettings.productColName);
      nodeSettings.registerDialogComponent(cmbCatalystColName, 
            IndigoReactionBuilderSettings.INPUT_PORT, nodeSettings.catalystColName);
      
      nodeSettings.registerDialogComponent(chbAddReactants, nodeSettings.addReactants);
      nodeSettings.registerDialogComponent(chbAddProducts, nodeSettings.addProducts);
      nodeSettings.registerDialogComponent(chbAddCatalysts, nodeSettings.addCatalysts);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      
      /*
       * Treating input group
       */
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(cmbOutputType, nodeSettings.outputType);
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
         
         inputTableSpec = specs[IndigoReactionBuilderSettings.INPUT_PORT];
         
         for(ChangeListener cL : changeListeners) {
            cL.stateChanged(null);
         }
         
         inputTypeChangeListener.itemStateChanged(null);
         inputTypeActionListener.actionPerformed(null);
         
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }

   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveSettingsTo (final NodeSettingsWO settings)
         throws InvalidSettingsException
   {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }
}
