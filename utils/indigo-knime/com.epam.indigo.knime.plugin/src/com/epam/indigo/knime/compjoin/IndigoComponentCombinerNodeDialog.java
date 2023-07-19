package com.epam.indigo.knime.compjoin;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JTextField;

import org.knime.core.data.DataTableSpec;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.NotConfigurableException;
import org.knime.core.node.defaultnodesettings.DialogComponentColumnFilter;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.IndigoNodeModel.Format;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoComponentCombinerNodeDialog extends NodeDialogPane {
   
   private final IndigoComponentCombinerSettings nodeSettings = new IndigoComponentCombinerSettings();
   
   private final Format[] molFormats = {Format.Mol, Format.Smiles, Format.Sdf, Format.CML, Format.String};
   private final Format[] qmolFormatsWithSMARTS = {Format.Mol, Format.Smiles, Format.SMARTS, Format.Sdf, Format.String};
   private final Format[] qmolFormatsWithoutSMARTS = {Format.Mol, Format.Smiles, Format.Sdf, Format.String};
   
   private final DialogComponentColumnFilter filterPanel;
   private final JTextField txtNewColName = new JTextField(20);
   private JComboBox<Format> cmbOutputType = new JComboBox<>(molFormats);

   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat query as SMARTS");
   
   /** listens to inputType item's state and updates indigoColumn combobox in case of changes */
   
   private final ItemListener inputTypeChangeListener = new ItemListener() {
      
      @Override
      public void itemStateChanged(ItemEvent e) {
         
         IndigoType type = IndigoType.findByString(cmbInputType.getSelectedItem().toString());
         
         chbTreatStringAsSMARTS.setEnabled(IndigoType.QUERY_MOLECULE.equals(type));

         // update output types list
         cmbOutputType.removeAllItems();
         Format[] qformats  = chbTreatStringAsSMARTS.isSelected() ? qmolFormatsWithSMARTS : 
            qmolFormatsWithoutSMARTS;
         Format[] formats = IndigoType.MOLECULE.equals(type) ? molFormats : qformats;
         for (Format format : formats) {
            cmbOutputType.addItem(format);
         }
         
      }
   };
   
   private final ActionListener chbTreatAsSmarts = new ActionListener() {
      
      @Override
      public void actionPerformed(ActionEvent e) {
         
         IndigoType type = IndigoType.findByString(cmbInputType.getSelectedItem().toString());
         
         if (IndigoType.QUERY_MOLECULE.equals(type)){
            Format[] formats = chbTreatStringAsSMARTS.isSelected() ? qmolFormatsWithSMARTS : 
               qmolFormatsWithoutSMARTS;
            cmbOutputType.removeAllItems();
            for (Format format : formats) {
               cmbOutputType.addItem(format);
            }
         }
         
      }
   };
   
   protected IndigoComponentCombinerNodeDialog(IndigoType[] types) {
        super();
        
        cmbInputType = new JComboBox<>(types);
        
        // bound component and corresponding setting
        registerDialogComponents();
        
        IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
        dialogPanel.addItemsPanel("Include Columns");
        
        /*
         * Add filter panel
         */
        filterPanel = new DialogComponentColumnFilter(nodeSettings.colNames, 
              IndigoComponentCombinerSettings.INPUT_PORT, 
              true, nodeSettings.columnFilter);
        
        dialogPanel.addItem(filterPanel.getComponentPanel());
        dialogPanel.addItemsPanel("Output Column Settings");
        dialogPanel.addItem("Result molecule column name", txtNewColName);
        dialogPanel.addItem("Output combined molecules type", cmbOutputType);
        
        // load treating settings
        dialogPanel.addItemsPanel("Treating Settings");
        dialogPanel.addItem("Input type", cmbInputType);
        dialogPanel.addItem(chbTreatXAsPseudoatom);
        dialogPanel.addItem(chbIgnoreStereochemistryErrors);
        dialogPanel.addItem(chbTreatStringAsSMARTS);
        
        // assign listeners
        cmbInputType.addItemListener(inputTypeChangeListener);
        chbTreatStringAsSMARTS.addActionListener(chbTreatAsSmarts);
        
        addTab("Standard Settings", dialogPanel.getPanel());
        
    }

   private void registerDialogComponents() {
      
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(cmbOutputType, nodeSettings.outputType);
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      nodeSettings.registerDialogComponent(chbTreatStringAsSMARTS, nodeSettings.treatStringAsSMARTS);
      
   }

   @Override
   protected void saveSettingsTo(NodeSettingsWO settings)
         throws InvalidSettingsException {
      
      if(nodeSettings.colNames.getIncludeList().isEmpty())
         throw new InvalidSettingsException("selected column list can not be empty");
      
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
      filterPanel.saveSettingsTo(settings);
   }
   
   @Override
   protected void loadSettingsFrom(NodeSettingsRO settings,
         DataTableSpec[] specs) throws NotConfigurableException {
      try {
         
         inputTypeChangeListener.itemStateChanged(null);
         cmbOutputType.actionPerformed(null);
         
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         filterPanel.loadSettingsFrom(settings, specs);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }
}

