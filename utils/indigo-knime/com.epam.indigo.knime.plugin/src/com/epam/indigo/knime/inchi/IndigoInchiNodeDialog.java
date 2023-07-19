package com.epam.indigo.knime.inchi;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.data.DataTableSpec;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.NotConfigurableException;
import org.knime.core.node.util.ColumnSelectionComboxBox;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoInchiNodeDialog extends NodeDialogPane {

   private final IndigoInchiNodeSettings nodeSettings = 
         new IndigoInchiNodeSettings();
   
   private final ColumnSelectionComboxBox cmbColumn;
   private final JTextField txtInchiColName = new JTextField("InChi column", 20);
   private final JCheckBox chbAppendInchiKeyColumn = new JCheckBox("Append InChiKey column");
   private final JTextField txtInchiKeyColName = new JTextField("InChiKey column", 20);
   
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbtTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   
   /** Listens to checkbox state's changing */
   private ChangeListener checkboxListener = new ChangeListener() {
      public void stateChanged (final ChangeEvent e) {
         txtInchiKeyColName.setEnabled(chbAppendInchiKeyColumn.isSelected());
      }
   };
   
   
   public IndigoInchiNodeDialog(IndigoType[] types) {
      
      super();
      
      cmbInputType = new JComboBox<>(types);
      
      // set column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      cmbColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      // bound component and corresponding setting
      registerDialogComponents();
      
      // create dialog panel and place components on it
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target column", cmbColumn);
      dialogPanel.addItem("InChi column name", txtInchiColName);
      dialogPanel.addItem(chbAppendInchiKeyColumn, txtInchiKeyColName);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbtTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      // assign listeners
      chbAppendInchiKeyColumn.addChangeListener(checkboxListener);
      
      // add dialog panel as a tab
      addTab("Standard settings", dialogPanel.getPanel());
      
   }
   
   private void registerDialogComponents() {
      
      nodeSettings.registerDialogComponent(cmbColumn, IndigoInchiNodeSettings.INPUT_PORT, nodeSettings.colName);
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(txtInchiColName, nodeSettings.inchiColName);
      nodeSettings.registerDialogComponent(chbAppendInchiKeyColumn, nodeSettings.appendInchiKeyColumn);
      nodeSettings.registerDialogComponent(txtInchiKeyColName, nodeSettings.inchiKeyColName);
      nodeSettings.registerDialogComponent(chbtTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      
   }

   @Override
   protected void loadSettingsFrom(NodeSettingsRO settings,
         DataTableSpec[] specs) throws NotConfigurableException {
      
      try {
         // load settings
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         // fire event
         checkboxListener.stateChanged(null);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
      
      
      
   }
   
   @Override
   protected void saveSettingsTo(NodeSettingsWO settings)
         throws InvalidSettingsException {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }

}
