package com.epam.indigo.knime.common.splitter;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.border.Border;

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

public class IndigoSplitterNodeDialog extends NodeDialogPane {

   public static interface DialogComponents {
      public void loadDialogComponents(IndigoDialogPanel dialogPanel, IndigoSplitterSettings settings);
   }
   
   private final IndigoSplitterSettings nodeSettings;
   private JComboBox<IndigoType> cmbInputType;
   
   private ColumnSelectionComboxBox cmbColumn; // set in constructor
   
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   
   private DataTableSpec inputTableSpec; // set in constructor and at loading settings
   
   public IndigoSplitterNodeDialog(IndigoSplitterSettings settings, DialogComponents dialogComponents, IndigoType[] types) {
      
      super();
      
      this.nodeSettings = settings;
      cmbInputType = new JComboBox<>(types);
      
      // set indigo column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      cmbColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      // bound component and corresponding setting
      registerDialogComponents();
      
      // create dialog panel and place components on it
      IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target column", cmbColumn);

      // assign listeners
      cmbInputType.addItemListener(inputTypeChangeListener);
      
      // load extra dilaog components if any
      dialogComponents.loadDialogComponents(dialogPanel, nodeSettings);
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      // add dialog panel as a tab
      addTab("Standard settings", dialogPanel.getPanel());
   }
   
   public IndigoSplitterNodeDialog(IndigoSplitterSettings settings, IndigoType[] types) {
      this(settings, null, types);
   }
   
   public IndigoSplitterNodeDialog(IndigoType[] types) {
      this(new IndigoSplitterSettings(), null, types);
   }

   private void registerDialogComponents() {

      nodeSettings.registerDialogComponent(cmbColumn, IndigoSplitterSettings.INPUT_PORT, nodeSettings.colName);
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      
   }

   
   /** listens to inputType item's state and updates indigoColumn combobox in case of changes */
   
   private final ItemListener inputTypeChangeListener = new ItemListener() {
      
      @Override
      public void itemStateChanged(ItemEvent e) {
         
         String prevType = nodeSettings.inputType.getStringValue();
         String newType = cmbInputType.getSelectedItem().toString();
         // should update if only input type value has being changed
         boolean typeChanged = !newType.equals(prevType);
         
         if (inputTableSpec != null && typeChanged) {
 
            IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
            Class<? extends DataValue>[] valueClasses = 
                  type.getClassesConvertableToIndigoDataClass();
            DataValueColumnFilter columnFilter = new DataValueColumnFilter(valueClasses);
 
            // update columns to pick with new column filter
            try {
               cmbColumn.update(inputTableSpec, null, false, columnFilter);
            } catch (NotConfigurableException ex) {
               ex.printStackTrace();
            }
         }
         
      }
   };
   
   @Override
   protected void loadSettingsFrom(NodeSettingsRO settings, 
         DataTableSpec[] specs) throws NotConfigurableException {
      try {
         // load settings
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         // set input spec
         inputTableSpec = specs[IndigoSplitterSettings.INPUT_PORT];
         
         // fire events
         inputTypeChangeListener.itemStateChanged(null);
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   };
   
   @Override
   protected void saveSettingsTo(NodeSettingsWO settings)
         throws InvalidSettingsException {

      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
      
   }

}
