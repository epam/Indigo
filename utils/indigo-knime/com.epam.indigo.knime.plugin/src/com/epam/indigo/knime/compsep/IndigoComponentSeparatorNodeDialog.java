package com.epam.indigo.knime.compsep;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.data.DataTableSpec;
import org.knime.core.node.*;
import org.knime.core.node.util.ColumnSelectionComboxBox;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoComponentSeparatorNodeDialog extends NodeDialogPane
{
   private IndigoComponentSeparatorSettings nodeSettings = new IndigoComponentSeparatorSettings();
   
   private final ColumnSelectionComboxBox cmbColumn;
   
   private final JTextField txtNewColPrefix = new JTextField(10);
   private final JCheckBox chbLimitComponentNumber = new JCheckBox("Limit component number");
   private final JSpinner spnComponentNumber = new JSpinner(new SpinnerNumberModel(1, 0, Integer.MAX_VALUE, 1));
   
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");

   private ChangeListener limitListener = new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
         if(chbLimitComponentNumber.isSelected())
            spnComponentNumber.setEnabled(true);
         else
            spnComponentNumber.setEnabled(false);
      }
   };
   
   protected IndigoComponentSeparatorNodeDialog (IndigoType[] types)
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
      dialogPanel.addItem("Molecule column", cmbColumn);
      dialogPanel.addItem("New column prefix", txtNewColPrefix);
      dialogPanel.addItemsPanel("Component Separator Settings");
      dialogPanel.addItem(chbLimitComponentNumber, spnComponentNumber);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      chbLimitComponentNumber.addChangeListener(limitListener);
      
      addTab("Standard settings", dialogPanel.getPanel());
   }
   
   private void registerDialogComponents() {

      nodeSettings.registerDialogComponent(cmbColumn, IndigoComponentSeparatorSettings.INPUT_PORT, nodeSettings.colName);
      nodeSettings.registerDialogComponent(txtNewColPrefix, nodeSettings.newColPrefix);
      nodeSettings.registerDialogComponent(chbLimitComponentNumber, nodeSettings.limitComponentNumber);
      nodeSettings.registerDialogComponent(spnComponentNumber, nodeSettings.componentNumber);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
   }

   @Override
   protected void loadSettingsFrom (final NodeSettingsRO settings,
         final DataTableSpec[] specs) throws NotConfigurableException
   {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         limitListener.stateChanged(null);
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
