package com.epam.indigo.knime.isomers;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.border.Border;
import org.knime.core.data.DataTableSpec;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.NotConfigurableException;
import org.knime.core.node.util.ColumnSelectionComboxBox;

import com.epam.indigo.knime.common.IndigoColumnControlListener;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.molfp.IndigoMoleculeFingerprinterSettings;

public class IndigoIsomersEnumeratorNodeDialog extends NodeDialogPane {
   private final IndigoIsomersEnumeratorSettings nodeSettings = new IndigoIsomersEnumeratorSettings();

   private final ColumnSelectionComboxBox cmbColumn;

   private final JCheckBox chbAppendColumn = new JCheckBox("Append column");
   private final JTextField txtNewColName = new JTextField(20);

   private final JCheckBox chbAppendRowidColumn = new JCheckBox(
         "Append rowid column");
   private final JTextField txtNewRowidColName = new JTextField(20);

   private final JCheckBox chbCisTransIsomers = new JCheckBox("Cis-trans isomers");
   private final JCheckBox chbTetrahedralIsomers = new JCheckBox("Enantiomers and Diastereomers");
   
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");

   protected IndigoIsomersEnumeratorNodeDialog(IndigoType[] types) {
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
      dialogPanel.addItem(chbAppendColumn, txtNewColName);
      dialogPanel.addItem(chbAppendRowidColumn, txtNewRowidColName);

      dialogPanel.addItemsPanel("Isomers enumeration settings");
      dialogPanel.addItem(chbCisTransIsomers);
      dialogPanel.addItem(chbTetrahedralIsomers);

      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      chbAppendColumn.addChangeListener(new IndigoColumnControlListener(
            cmbColumn, txtNewColName, "(isomer)"));
      chbAppendRowidColumn.addChangeListener(new IndigoColumnControlListener(
            cmbColumn, txtNewRowidColName, "(row ID)"));

      addTab("Standard Settings", dialogPanel.getPanel());

   }

   private void registerDialogComponents() 
   {
      nodeSettings.registerDialogComponent(cmbColumn, IndigoMoleculeFingerprinterSettings.INPUT_PORT, nodeSettings.colName);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(chbAppendColumn, nodeSettings.appendColumn);
      nodeSettings.registerDialogComponent(txtNewRowidColName, nodeSettings.newRowidColName);
      nodeSettings.registerDialogComponent(chbAppendRowidColumn, nodeSettings.appendRowidColumn);
      nodeSettings.registerDialogComponent(txtNewRowidColName, nodeSettings.newRowidColName);
      nodeSettings.registerDialogComponent(chbCisTransIsomers, nodeSettings.cisTransIsomers);
      nodeSettings.registerDialogComponent(chbTetrahedralIsomers, nodeSettings.tetrahedralIsomers);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
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
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }
}
