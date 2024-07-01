package com.epam.indigo.knime.rsplitter;

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
import com.epam.indigo.knime.common.types.IndigoType;


/**
 * <code>NodeDialog</code> for the "IndigoReactionSplitter" Node.
 * 
 * 
 */
public class IndigoReactionSplitterNodeDialog extends NodeDialogPane {

   private final IndigoReactionSplitterSettings nodeSettings = new IndigoReactionSplitterSettings();
   
   private final ColumnSelectionComboxBox cmbReactionColumn;
   
   private final JTextField txtReactantColName = new JTextField(20);
   private final JTextField txtProductColName = new JTextField(20);
   private final JTextField txtCatalystColName = new JTextField(20);
   
   private final JCheckBox chbExtractReactants = new JCheckBox("Extract reactants");
   private final JCheckBox chbExtractProducts = new JCheckBox("Extract products");
   private final JCheckBox chbExtractCatalysts = new JCheckBox("Extract catalysts");
   
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat query reaction as SMARTS");
   
   private DataTableSpec inputTableSpec;
   
   private final ArrayList<ChangeListener> changeListeners = new ArrayList<ChangeListener>();
   
   /*
    *  listens to inputType item's state and updates reactionColumn combobox in case of changes 
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
               cmbReactionColumn.update(inputTableSpec, null, false, columnFilter);
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
         boolean query = IndigoType.QUERY_REACTION.equals(type);
         chbTreatStringAsSMARTS.setEnabled(query);
         if (!chbTreatStringAsSMARTS.isEnabled()) {
            chbTreatStringAsSMARTS.setSelected(false);
         }
      }
   };
   
   protected IndigoReactionSplitterNodeDialog(IndigoType[] types) {
      super();
      
      cmbInputType = new JComboBox<>(types);
      
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      
      cmbReactionColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target reaction column", cmbReactionColumn);
      dialogPanel.addItemsPanel("Output Column Names");
      dialogPanel.addItem(chbExtractReactants, txtReactantColName);
      dialogPanel.addItem(chbExtractProducts, txtProductColName);
      dialogPanel.addItem(chbExtractCatalysts, txtCatalystColName);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      
      /*
       * Add change listeners
       */
      ChangeListener reactListener = new ChangeListener() {
         public void stateChanged(ChangeEvent e) {
            txtReactantColName.setEnabled(chbExtractReactants.isSelected());
         }
      };
      ChangeListener productListener = new ChangeListener() {
         public void stateChanged(ChangeEvent e) {
            txtProductColName.setEnabled(chbExtractProducts.isSelected());
         }
      };
      ChangeListener catalystListener = new ChangeListener() {
         public void stateChanged(ChangeEvent e) {
            txtCatalystColName.setEnabled(chbExtractCatalysts.isSelected());
         }
      };
      changeListeners.add(reactListener);
      changeListeners.add(productListener);
      changeListeners.add(catalystListener);
      
      chbExtractReactants.addChangeListener(reactListener);
      chbExtractProducts.addChangeListener(productListener);
      chbExtractCatalysts.addChangeListener(catalystListener);
      
      cmbInputType.addItemListener(inputTypeChangeListener);
      cmbInputType.addActionListener(inputTypeActionListener);
      
      addTab("Standard Settings", dialogPanel.getPanel());

   }
   
   private void registerDialogComponents() {
      
      nodeSettings.registerDialogComponent(cmbReactionColumn, IndigoReactionSplitterSettings.INPUT_PORT, nodeSettings.reactionColumn);
      
      nodeSettings.registerDialogComponent(txtReactantColName, nodeSettings.reactantColName);
      nodeSettings.registerDialogComponent(txtProductColName, nodeSettings.productColName);
      nodeSettings.registerDialogComponent(txtCatalystColName, nodeSettings.catalystColName);
      
      nodeSettings.registerDialogComponent(chbExtractReactants, nodeSettings.extractReactants);
      nodeSettings.registerDialogComponent(chbExtractProducts, nodeSettings.extractProducts);
      nodeSettings.registerDialogComponent(chbExtractCatalysts, nodeSettings.extractCatalysts);
      
      /*
       * Treating input group
       */
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      nodeSettings.registerDialogComponent(chbTreatStringAsSMARTS, nodeSettings.treatStringAsSMARTS);
   }

   @Override
   protected void saveSettingsTo(NodeSettingsWO settings)
         throws InvalidSettingsException {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }
   
   @Override
   protected void loadSettingsFrom(NodeSettingsRO settings,
         DataTableSpec[] specs) throws NotConfigurableException {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         inputTableSpec = specs[IndigoReactionSplitterSettings.INPUT_PORT];
         
         for(ChangeListener cL : changeListeners) {
            cL.stateChanged(null);
         }
         
         inputTypeChangeListener.itemStateChanged(null);
         inputTypeActionListener.actionPerformed(null);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }
}
