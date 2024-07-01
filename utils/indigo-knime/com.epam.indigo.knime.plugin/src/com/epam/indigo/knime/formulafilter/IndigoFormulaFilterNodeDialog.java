package com.epam.indigo.knime.formulafilter;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataValue;
import org.knime.core.data.StringValue;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.NotConfigurableException;
import org.knime.core.node.util.ColumnSelectionComboxBox;
import org.knime.core.node.util.DataValueColumnFilter;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.types.IndigoType;

@SuppressWarnings("unchecked")
public class IndigoFormulaFilterNodeDialog extends NodeDialogPane {

   private final IndigoFormulaFilterNodeSettings nodeSettings;
   
   private final ColumnSelectionComboxBox cmbTargetColumn;
   private final ColumnSelectionComboxBox cmbQueryColumn = new ColumnSelectionComboxBox(
         (Border) null, StringValue.class);
   private final JTextField txtNewColName = new JTextField(20);
   private final JCheckBox chbAppendQueryKeyColumn = new JCheckBox("Append queries row ID column");
   private final JTextField txtQueryKeyColumnName = new JTextField(20);
   
   private final JCheckBox chbAppendQueryMatchCountKeyColumn = new JCheckBox("Append match count column");
   private final JTextField txtQueryMatchCountKeyColumn = new JTextField(20);
   
   private final JRadioButton rbSatisfyAllExceptSelected = new JRadioButton("All comparisons");
   private final JRadioButton rbSatisfyAnyAtLeastSelected = new JRadioButton("At least ");
   
   JLabel lblRelation = new JLabel("Relation:");
   private final JRadioButton rbLessThanOrEqualComparison = new JRadioButton("<=");
   private final JRadioButton rbEqualComparison = new JRadioButton("=");
   private final JRadioButton rbMoreThanOrEqualComparison = new JRadioButton("=>");
   private final JSpinner spnSatisfyAnyAtLeast = new JSpinner(new SpinnerNumberModel(1, 0, Integer.MAX_VALUE, 1));
   
   private final JComboBox<IndigoType> cmbInputType;
   private final JCheckBox chbTreatXAsPseudoatom = new JCheckBox("Treat X as pseudoatom");
   private final JCheckBox chbIgnoreStereochemistryErrors = new JCheckBox("Ignore stereochemistry errors");
   
   private DataTableSpec inputTableSpec;
   
   private final ChangeListener changeListener = new ChangeListener() {
      public void stateChanged (ChangeEvent e)
      {

         txtQueryKeyColumnName.setEnabled(chbAppendQueryKeyColumn.isSelected());
         txtQueryMatchCountKeyColumn.setEnabled(chbAppendQueryMatchCountKeyColumn.isSelected());
         
         spnSatisfyAnyAtLeast.setEnabled(rbSatisfyAnyAtLeastSelected.isSelected());
         
         updateNullableEdit(txtQueryKeyColumnName, cmbTargetColumn.getSelectedColumn() + " (query row ID)");
         updateNullableEdit(txtQueryMatchCountKeyColumn, cmbTargetColumn.getSelectedColumn() + " (queries matched)");
         updateNullableEdit(txtNewColName, cmbTargetColumn.getSelectedColumn() + " (matched)");
      }
   };
   
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
               cmbTargetColumn.update(inputTableSpec, null, false, columnFilter);
            } catch (NotConfigurableException ex) {
               ex.printStackTrace();
            }
         }
         
      }
   };
   
   public IndigoFormulaFilterNodeDialog(IndigoFormulaFilterNodeSettings settings, 
         IndigoType[] types) {
      
      super();
   
      nodeSettings = settings;
      cmbInputType = new JComboBox<>(types);
      
      
      // set indigo column combobox taking into account selected type
      IndigoType type = (IndigoType) cmbInputType.getSelectedItem();
      cmbTargetColumn = new ColumnSelectionComboxBox((Border) null, 
            type.getClassesConvertableToIndigoDataClass());
      
      // bound component and corresponding setting
      registerDialogComponents();
      
     IndigoDialogPanel dialogPanel = new IndigoDialogPanel();
      
      dialogPanel.addItemsPanel("Column Settings");
      dialogPanel.addItem("Target column", cmbTargetColumn);
      dialogPanel.addItem("Query column", cmbQueryColumn);
      dialogPanel.addItemsPanel("Filter Settings");
      
      dialogPanel.addItem(lblRelation);
      ButtonGroup comparisonOptions = new ButtonGroup();
      comparisonOptions.add(rbLessThanOrEqualComparison);
      comparisonOptions.add(rbEqualComparison);
      comparisonOptions.add(rbMoreThanOrEqualComparison);
      
      dialogPanel.addItem(rbLessThanOrEqualComparison);
      dialogPanel.addItem(rbEqualComparison);
      dialogPanel.addItem(rbMoreThanOrEqualComparison);
      
      ((JSpinner.DefaultEditor)spnSatisfyAnyAtLeast.getEditor()).getTextField().setColumns(4);

      JPanel p2 = new JPanel(new GridBagLayout());
      GridBagConstraints c2 = new GridBagConstraints();
      c2.anchor = GridBagConstraints.WEST;
      c2.insets = new Insets(2, 2, 2, 2);
      c2.gridy = 0;
      c2.gridx = 0;
      
      JLabel satisfyLabel = new JLabel("Satisfy");
      IndigoDialogPanel.setDefaultComponentFont(satisfyLabel);
      p2.add(satisfyLabel, c2);
      c2.gridx++;
      p2.add(rbSatisfyAnyAtLeastSelected, c2);
      c2.gridx++;
      p2.add(spnSatisfyAnyAtLeast, c2);
      c2.gridx++;
      JLabel comparisonsLabel = new JLabel(" comparisons");
      IndigoDialogPanel.setDefaultComponentFont(comparisonsLabel);
      p2.add(comparisonsLabel, c2);
     
      c2.gridy++;
      c2.gridx = 1;
      c2.gridwidth = 3;
      p2.add(rbSatisfyAllExceptSelected, c2);
      
      ButtonGroup bg = new ButtonGroup();
      bg.add(rbSatisfyAllExceptSelected);
      bg.add(rbSatisfyAnyAtLeastSelected);
      
      
      dialogPanel.addItem(p2, new JPanel());
      
      dialogPanel.addItemsPanel("Column Key Settings");
      dialogPanel.addItem(chbAppendQueryKeyColumn, txtQueryKeyColumnName);
      dialogPanel.addItem(chbAppendQueryMatchCountKeyColumn, txtQueryMatchCountKeyColumn);
    
      // load treating settings
      dialogPanel.addItemsPanel("Treating Settings");
      dialogPanel.addItem("Input type", cmbInputType);
      dialogPanel.addItem(chbTreatXAsPseudoatom);
      dialogPanel.addItem(chbIgnoreStereochemistryErrors);
      
      /*
       * Add all change listeners
       */
      chbAppendQueryKeyColumn.addChangeListener(changeListener);
      chbAppendQueryMatchCountKeyColumn.addChangeListener(changeListener);
      
      rbLessThanOrEqualComparison.addChangeListener(changeListener);
      rbEqualComparison.addChangeListener(changeListener);
      rbMoreThanOrEqualComparison.addChangeListener(changeListener);
      
      rbSatisfyAllExceptSelected.addChangeListener(changeListener);
      rbSatisfyAnyAtLeastSelected.addChangeListener(changeListener);
      
      cmbInputType.addItemListener(inputTypeChangeListener);
      
      
      
      addTab("Standard settings", dialogPanel.getPanel());
   }
   
   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbTargetColumn, IndigoFormulaFilterNodeModel.TARGET_PORT, nodeSettings.targetColName);
      nodeSettings.registerDialogComponent(cmbQueryColumn, IndigoFormulaFilterNodeModel.QUERY_PORT, nodeSettings.queryColName);
      
      nodeSettings.registerDialogComponent(chbAppendQueryKeyColumn, nodeSettings.appendQueryKeyColumn);

      nodeSettings.registerDialogComponent(chbAppendQueryKeyColumn, nodeSettings.appendQueryKeyColumn);
      nodeSettings.registerDialogComponent(txtQueryKeyColumnName, nodeSettings.queryKeyColumn);

      nodeSettings.registerDialogComponent(chbAppendQueryMatchCountKeyColumn, nodeSettings.appendQueryMatchCountKeyColumn);
      nodeSettings.registerDialogComponent(txtQueryMatchCountKeyColumn, nodeSettings.queryMatchCountKeyColumn);

      nodeSettings.registerDialogComponent(rbLessThanOrEqualComparison, nodeSettings.lessThanOrEqualComparison);
      nodeSettings.registerDialogComponent(rbEqualComparison, nodeSettings.equalComparison);
      nodeSettings.registerDialogComponent(rbMoreThanOrEqualComparison, nodeSettings.greaterThanOrEqualComparison);
      
      nodeSettings.registerDialogComponent(rbSatisfyAllExceptSelected, nodeSettings.matchAllSelected);
      nodeSettings.registerDialogComponent(rbSatisfyAnyAtLeastSelected, nodeSettings.matchAnyAtLeastSelected);
      nodeSettings.registerDialogComponent(spnSatisfyAnyAtLeast, nodeSettings.matchAnyAtLeast);
      
      nodeSettings.registerDialogComponent(cmbInputType, nodeSettings.inputType);
      nodeSettings.registerDialogComponent(chbTreatXAsPseudoatom, nodeSettings.treatXAsPseudoatom);
      nodeSettings.registerDialogComponent(chbIgnoreStereochemistryErrors, nodeSettings.ignoreStereochemistryErrors);
      
   }
   
   private void updateNullableEdit (JTextField field, String defValue)
   {
      if (field.isEnabled() && field.getText().length() < 1)
         field.setText(defValue);
      if (!field.isEnabled() && field.getText().length() > 0 && field.getText().equals(defValue))
         field.setText("");
   }
   
   @Override
   protected void loadSettingsFrom(NodeSettingsRO settings,
         DataTableSpec[] specs) throws NotConfigurableException {

      try {
         // load settings
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         // set input spec
         inputTableSpec = specs[IndigoFormulaFilterNodeModel.TARGET_PORT];
         
         // fire events
         inputTypeChangeListener.itemStateChanged(null);
         changeListener.stateChanged(null);
         
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
