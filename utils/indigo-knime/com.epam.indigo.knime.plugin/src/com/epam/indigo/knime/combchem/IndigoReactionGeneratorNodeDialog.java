package com.epam.indigo.knime.combchem;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.border.Border;
import org.knime.core.data.DataTableSpec;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.NotConfigurableException;
import org.knime.core.node.util.ColumnSelectionComboxBox;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.IndigoNodeModel.Format;
import com.epam.indigo.knime.common.types.IndigoType;



/**
 * 
 */
public class IndigoReactionGeneratorNodeDialog extends NodeDialogPane {
   private final IndigoReactionGeneratorSettings nodeSettings = new IndigoReactionGeneratorSettings();
   
   private final ColumnSelectionComboxBox cmbReactionColumn;
   private final ColumnSelectionComboxBox cmbMolColumn1;
   private final ColumnSelectionComboxBox cmbMolColumn2;

   private final JTextField txtNewColName = new JTextField(20);
   private final JSpinner spnProductsCountLimt = new JSpinner();
   
   private JComboBox<IndigoType> cmbMoleculeType;
   private JComboBox<IndigoType> cmbReactionType;
   private final JComboBox<Format> cmbPutputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIignoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat query reaction as SMARTS");
   

   protected IndigoReactionGeneratorNodeDialog(IndigoType[] reactionTypes, IndigoType[] moleculeTypes) {
      super();
      
      cmbMoleculeType = new JComboBox<>(moleculeTypes);
      cmbReactionType = new JComboBox<>(reactionTypes);
      
      cmbPutputType = new JComboBox<Format>(new Format[] {Format.Smiles, Format.Rxn});
      
      IndigoType molIndigoType = (IndigoType) cmbMoleculeType.getSelectedItem();
      IndigoType reacIndigoType = (IndigoType) cmbReactionType.getSelectedItem();
      
      cmbReactionColumn = new ColumnSelectionComboxBox((Border) null, 
            reacIndigoType.getClassesConvertableToIndigoDataClass());
      cmbMolColumn1 = new ColumnSelectionComboxBox((Border) null, 
            molIndigoType.getClassesConvertableToIndigoDataClass());
      cmbMolColumn2 = new ColumnSelectionComboxBox((Border) null, 
            molIndigoType.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();

      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target reaction column", cmbReactionColumn);
      dialogPanel.addItem("Reactans 1 column", cmbMolColumn1);
      dialogPanel.addItem("Reactans 2 column", cmbMolColumn2);
      dialogPanel.addItem("New column name", txtNewColName);
      
      dialogPanel.addItemsPanel("Enumeration settings");
      dialogPanel.addItem("Products count limit", spnProductsCountLimt);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Reaction type", cmbReactionType);
      dialogPanel.addItem("Molecule type", cmbMoleculeType);
      dialogPanel.addItem("Output column type", cmbPutputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIignoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      
      addTab("Standard Settings", dialogPanel.getPanel());
   }
   
   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbMolColumn1, IndigoReactionGeneratorSettings.MOL_PORT1,  nodeSettings.molColumn1);
      nodeSettings.registerDialogComponent(cmbMolColumn2, IndigoReactionGeneratorSettings.MOL_PORT2,  nodeSettings.molColumn2, true);
      nodeSettings.registerDialogComponent(cmbReactionColumn, IndigoReactionGeneratorSettings.REACTION_PORT, nodeSettings.reactionColumn);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(spnProductsCountLimt, nodeSettings.productsCountLimit);
      
      /*
       * Treating input group
       */
      nodeSettings.registerDialogComponent(cmbReactionType, nodeSettings.reactionType);
      nodeSettings.registerDialogComponent(cmbMoleculeType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(cmbPutputType, nodeSettings.outputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIignoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      nodeSettings.registerDialogComponent(chbTreatStringAsSMARTS, nodeSettings.treatStringAsSMARTS);
      
   }

   @Override
   protected void loadSettingsFrom(final NodeSettingsRO settings, final DataTableSpec[] specs) 
         throws NotConfigurableException {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         DataTableSpec molSpec2 = specs[IndigoReactionGeneratorSettings.MOL_PORT2];
         cmbMolColumn2.setVisible(molSpec2 != null);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }

   @Override
   protected void saveSettingsTo (NodeSettingsWO settings)
         throws InvalidSettingsException {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }
}
