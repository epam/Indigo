package com.epam.indigo.knime.rautomapper;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
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
import com.epam.indigo.knime.rautomapper.IndigoReactionAutomapperSettings.AAMode;

public class IndigoReactionAutomapperNodeDialog extends NodeDialogPane {

   private final IndigoReactionAutomapperSettings nodeSettings = new IndigoReactionAutomapperSettings();

   private final Format[] reacFormats = new Format[] {Format.Rxn, Format.Smiles, Format.String};
   
   private final ColumnSelectionComboxBox cmbColName;
   private final JCheckBox chbAppendColumn = new JCheckBox("Append column");
   private final JTextField txtNewColName = new JTextField(20);
   private JComboBox<Format> chbOutputType = new JComboBox<>(reacFormats);

   private final JComboBox<String> cmbMode = new JComboBox<String>(new String[] { AAMode.Discard.toString(), 
         AAMode.Keep.toString(), AAMode.Alter.toString(), AAMode.Clear.toString() });
   
   private final JCheckBox chbIgnoreCharges = new JCheckBox("Ignore charges");
   private final JCheckBox chbIgnoreIsotopes = new JCheckBox("Ignore isotopes");
   private final JCheckBox chbIgnoreRadicals = new JCheckBox("Ignore radicals");
   private final JCheckBox chbIgnoreValence = new JCheckBox("Ignore valence");
   private final JCheckBox chbHighlightReactingCenters = new JCheckBox("Highlight Reacting Centers");
   private final JSpinner spnAamTimeout = new JSpinner(new SpinnerNumberModel(0, 0, Integer.MAX_VALUE, 1));
   private final JCheckBox chbUseAamTimeout = new JCheckBox("AAM Timeout (milliseconds)");
   
   private JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   private final JCheckBox chbTreatStringAsSMARTS = new JCheckBox("Treat query reaction as SMARTS");

   private DataTableSpec inputTableSpec;
   
   private ChangeListener aamChangeListener = new ChangeListener() {
      
      @Override
      public void stateChanged(ChangeEvent e) {
         if(chbUseAamTimeout.isSelected())
            spnAamTimeout.setEnabled(true);
         else
            spnAamTimeout.setEnabled(false);
      }
   };

   /*
    *  listens to inputType item's state and updates colName combobox in case of changes 
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
               cmbColName.update(inputTableSpec, null, false, columnFilter);
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
   
   protected IndigoReactionAutomapperNodeDialog(IndigoType[] types) {
     
      super();
      
      cmbInputType = new JComboBox<>(types);
      
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      
      cmbColName = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      registerDialogComponents();
      
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column settings");
      dialogPanel.addItem("Target column", cmbColName);
      dialogPanel.addItem(chbAppendColumn, txtNewColName);
      
      dialogPanel.addItemsPanel("Automapping settings");
      dialogPanel.addItem("Reaction AAM mode", cmbMode);
      dialogPanel.addItem(chbHighlightReactingCenters);
      dialogPanel.addItem(chbUseAamTimeout, spnAamTimeout);
      
      dialogPanel.addItemsPanel("Match rules settings");
      dialogPanel.addItem(chbIgnoreCharges);
      dialogPanel.addItem(chbIgnoreIsotopes);
      dialogPanel.addItem(chbIgnoreRadicals);
      dialogPanel.addItem(chbIgnoreValence);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem("Output reaction type", chbOutputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      dialogPanel.addItem(chbTreatStringAsSMARTS);
      
      IndigoDialogPanel.addColumnChangeListener(chbAppendColumn, cmbColName, txtNewColName, " (mapped)");
      
      chbUseAamTimeout.addChangeListener(aamChangeListener);
      cmbInputType.addItemListener(inputTypeChangeListener);
      cmbInputType.addActionListener(inputTypeActionListener);
      
      addTab("Standard settings", dialogPanel.getPanel());
   }

   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbColName, IndigoReactionAutomapperNodeModel.INPUT_PORT, nodeSettings.reactionColumn);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(chbAppendColumn, nodeSettings.appendColumn);
      nodeSettings.registerDialogComponent(cmbMode, nodeSettings.mode);
      /*
       * Ignore flags
       */
      nodeSettings.registerDialogComponent(chbIgnoreCharges, nodeSettings.ignoreCharges);
      nodeSettings.registerDialogComponent(chbIgnoreIsotopes, nodeSettings.ignoreIsotopes);
      nodeSettings.registerDialogComponent(chbIgnoreRadicals, nodeSettings.ignoreRadicals);
      nodeSettings.registerDialogComponent(chbIgnoreValence, nodeSettings.ignoreValence);
      
      nodeSettings.registerDialogComponent(chbHighlightReactingCenters, nodeSettings.highlightReactingCenters);
      nodeSettings.registerDialogComponent(chbUseAamTimeout, nodeSettings.useAamTimeout);
      nodeSettings.registerDialogComponent(spnAamTimeout, nodeSettings.aamTimeout);
      
      /*
       * Treating input group
       */
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbOutputType, nodeSettings.outputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      nodeSettings.registerDialogComponent(chbTreatStringAsSMARTS, nodeSettings.treatStringAsSMARTS);
      
   }

   @Override
   protected void loadSettingsFrom(NodeSettingsRO settings, DataTableSpec[] specs) 
         throws NotConfigurableException {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         inputTableSpec = specs[IndigoReactionAutomapperNodeModel.INPUT_PORT];
         
         aamChangeListener.stateChanged(null);
         inputTypeChangeListener.itemStateChanged(null);
         inputTypeActionListener.actionPerformed(null);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage(), e);
      }

   }

   @Override
   protected void saveSettingsTo(NodeSettingsWO settings) throws InvalidSettingsException {
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }
}
