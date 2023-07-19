package com.epam.indigo.knime.common.transformer;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

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

public class IndigoTransformerNodeDialog extends NodeDialogPane {
   
   
   public static interface DialogComponents {
      public void loadDialogComponents(IndigoDialogPanel dialogPanel, IndigoTransformerSettings settings);
      
      // two methods to store and restore data to settings from dialog components and vice versa
      public default void saveToSettings(IndigoTransformerSettings settings) {};
      public default void loadFromSettings(IndigoTransformerSettings settings) {};
   }
   
   private final IndigoTransformerSettings nodeSettings;
   
   // point how to treat input data:by default as molecule or reaction (but queries)
   // not final field as some nodes require another set of possible inputs
   private JComboBox<IndigoType> cmbInputType;
   
   private final ColumnSelectionComboxBox cmbColumn; // set in constructor
   private final JCheckBox chbAppendColumn = new JCheckBox("Append column");
   private final JTextField txtNewColName = new JTextField(20);
   
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   
   /** short node's description */
   private final String description; // set in constructor
   
   private DataTableSpec inputTableSpec; // set in constructor and at loading settings
   
   /** Store dialog components if exist */
   private DialogComponents dialogComponents = null; // null by default
   
   
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
            Class<? extends DataValue>[] valueClasses = type.getClassesConvertableToIndigoDataClass();
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
   
   /** Listens to checkbox state's changing */
   private ChangeListener checkboxListener = new ChangeListener() {
      public void stateChanged (final ChangeEvent e) {
         if (chbAppendColumn.isSelected()) {
            txtNewColName.setEnabled(true);
            if (txtNewColName.getText().isEmpty()) {
               
               String col = cmbColumn.getSelectedColumn();
               
               // take into account that column with the name may alredy exist
               // remove spaces and any postfix in parentheses: '(#i)', '(dearom)' 
               col = col.replaceAll("\\(.*\\)", "").trim();

               String end = " (" + description + ")";
               txtNewColName.setText(col + end);
            }
         } else {
            txtNewColName.setEnabled(false);
         }
      }
   };
   
   public IndigoTransformerNodeDialog(String description, IndigoTransformerSettings settings, 
         DialogComponents dialogComponents, IndigoType... types) {
      
      super();
      this.nodeSettings = settings;
      // for specific node
      this.description = description;
      
      // By default available types are molecule and reaction
      if (types.length == 0)
         cmbInputType = new JComboBox<>(new IndigoType[] {IndigoType.MOLECULE, 
               IndigoType.REACTION});
      else 
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
      dialogPanel.addItem(chbAppendColumn, txtNewColName);

      // assign listeners
      cmbInputType.addItemListener(inputTypeChangeListener);
      chbAppendColumn.addChangeListener(checkboxListener);
      
      // load extra dilaog components if any
      if(dialogComponents != null){
         this.dialogComponents = dialogComponents;
         dialogComponents.loadDialogComponents(dialogPanel, nodeSettings);
      }
      
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      // add dialog panel as a tab
      addTab("Standard settings", dialogPanel.getPanel());
   }
   
   public IndigoTransformerNodeDialog(String description, IndigoTransformerSettings settings) {
      this(description, settings, null);
   }

   public IndigoTransformerNodeDialog(String description) {
      this(description, new IndigoTransformerSettings(), null);
   }

   
   public void setInputType(JComboBox<IndigoType> cb) {
      cmbInputType = cb;
   }
   
   private void registerDialogComponents() {
	   
      nodeSettings.registerDialogComponent(cmbColumn, IndigoTransformerSettings.INPUT_PORT, nodeSettings.colName);
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbAppendColumn, nodeSettings.appendColumn);
      nodeSettings.registerDialogComponent(txtNewColName, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      
   }


   @Override
   protected void loadSettingsFrom(NodeSettingsRO settings,
         DataTableSpec[] specs) throws NotConfigurableException {
      try {
         // load settings
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         // set input spec
         inputTableSpec = specs[IndigoTransformerSettings.INPUT_PORT];
         
         // fire events
         checkboxListener.stateChanged(null);
         inputTypeChangeListener.itemStateChanged(null);
         
         // set dialog components if exist
         if (dialogComponents != null) 
            dialogComponents.loadFromSettings(nodeSettings);
            
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }
   
   @Override
   protected void saveSettingsTo(NodeSettingsWO settings)
         throws InvalidSettingsException {

      // update sattings for dialog components if exist before saving to NodeSettingsWO
      if (dialogComponents != null) 
         dialogComponents.saveToSettings(nodeSettings);
      
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
      
   }

}
