package com.epam.indigo.knime.transform;

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

public class IndigoMoleculeTransformNodeDialog extends NodeDialogPane {
   
   private final IndigoMoleculeTransformSettings nodeSettings = new IndigoMoleculeTransformSettings();
   
   private final ColumnSelectionComboxBox cmbMolColumn;
   private final ColumnSelectionComboxBox cmbReactionColumn;
   private final JCheckBox chbAppendColumn = new JCheckBox("Append column");
   private final JTextField txtNewColName = new JTextField(20);
   
   private JComboBox<IndigoType> cmbMolType;
   private JComboBox<IndigoType> cmbReactionType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat query reaction as SMARTS");
   
   
   ChangeListener changeListener = new ChangeListener() {
      public void stateChanged(final ChangeEvent e) {
         if (chbAppendColumn.isSelected()) {
            txtNewColName.setEnabled(true);
            if ("".equals(txtNewColName.getText())) {
               txtNewColName.setText(cmbMolColumn.getSelectedColumn()
                     + " (transformed)");
            }
         } else {
            txtNewColName.setEnabled(false);
         }
      }
   };

   /**
    */
   protected IndigoMoleculeTransformNodeDialog(IndigoType[] types, IndigoType[] reactionTypes) {
      
      super();
      
      cmbMolType = new JComboBox<>(types);
      cmbReactionType = new JComboBox<>(reactionTypes);
      
      // set column combobox taking into account selected type
      IndigoType mType = (IndigoType) cmbMolType.getSelectedItem();
      cmbMolColumn = new ColumnSelectionComboxBox((Border) null, 
            mType.getClassesConvertableToIndigoDataClass());

      IndigoType rType = (IndigoType) cmbReactionType.getSelectedItem();
      cmbReactionColumn = new ColumnSelectionComboxBox((Border) null, 
            rType.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();

      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target molecule column", cmbMolColumn);
      dialogPanel.addItem("Query Reaction column", cmbReactionColumn);
      dialogPanel.addItem(chbAppendColumn, txtNewColName);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Molecule type", cmbMolType);
      dialogPanel.addItem("Reaction type", cmbReactionType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      
      chbAppendColumn.addChangeListener(changeListener);
      
      addTab("Standard Settings", dialogPanel.getPanel());
   }
   
   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbMolColumn, IndigoMoleculeTransformSettings.MOL_PORT,  nodeSettings.molColumn);
      nodeSettings.registerDialogComponent(cmbReactionColumn, IndigoMoleculeTransformSettings.REACTION_PORT,  nodeSettings.reactionColumn);
      nodeSettings.registerDialogComponent(chbAppendColumn, nodeSettings.appendColumn);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      
      nodeSettings.registerDialogComponent(cmbMolType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(cmbReactionType, nodeSettings.rectionType);
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
