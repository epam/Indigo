package com.epam.indigo.knime.weightfilter;

import java.text.DecimalFormat;

import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JRadioButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.text.DefaultFormatterFactory;
import javax.swing.text.NumberFormatter;

import org.knime.core.data.DataRow;
import org.knime.core.node.BufferedDataContainer;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.splitter.IndigoSplitter;
import com.epam.indigo.knime.common.splitter.IndigoSplitterNodeDialog;
import com.epam.indigo.knime.common.splitter.IndigoSplitterNodeModel;
import com.epam.indigo.knime.common.splitter.IndigoSplitterSettings;
import com.epam.indigo.knime.common.splitter.IndigoSplitterNodeDialog.DialogComponents;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoWeightSplitterNodeFactory extends NodeFactory<IndigoSplitterNodeModel> {

   @Override
   public IndigoSplitterNodeModel createNodeModel() {
      
      final IndigoWeightSplitterNodeSettings nodeSettings = 
            new IndigoWeightSplitterNodeSettings();
      
      
      return new IndigoWeightSplitterNodeModel(nodeSettings, new IndigoSplitter() {
         
         @Override
         public void distribute(BufferedDataContainer[] cons, DataRow row,
               IndigoObject io) {

            BufferedDataContainer matchOutput = cons[0]; // matching rows
            BufferedDataContainer unmatchOutput = cons[1]; // not matching rows
            
            float weight = 0;
            
            if (nodeSettings.molecularWeight.getBooleanValue()) {
               weight = io.molecularWeight();
            } else if (nodeSettings.mostAbundantMass.getBooleanValue()) {
               weight = io.mostAbundantMass();
            } else if (nodeSettings.monoisotopicMass.getBooleanValue()) {
               weight = io.monoisotopicMass();
            }
            
               
            float error = 1e-7f; // let's put 1e-7 order as molecule's weight has that accuracy; 
            
            boolean equalMin = Math.abs(weight - (float)nodeSettings.minMolWeight.getDoubleValue()) <= error;
            boolean equalMax = Math.abs(weight - (float)nodeSettings.maxMolWeight.getDoubleValue()) <= error;
            
            boolean match = !((nodeSettings.minMolWeightActive.getBooleanValue() && 
                                                (!equalMin && weight < nodeSettings.minMolWeight.getDoubleValue()))
                             || (nodeSettings.maxMolWeightActive.getBooleanValue() && 
                                                (!equalMax && weight > nodeSettings.maxMolWeight.getDoubleValue())));
            
            // distribute the row
            if (match)
               matchOutput.addRowToTable(row);
            else
               unmatchOutput.addRowToTable(row);
            
         }
      });
   }

   @Override
   protected int getNrNodeViews() {
      return 0;
   }

   @Override
   public NodeView<IndigoSplitterNodeModel> createNodeView(int viewIndex,
         IndigoSplitterNodeModel nodeModel) {
      return null;
   }

   @Override
   protected boolean hasDialog() {
      return true;
   }

   @Override
   protected NodeDialogPane createNodeDialogPane() {

      final IndigoWeightSplitterNodeSettings nodeSettings = 
            new IndigoWeightSplitterNodeSettings();
      
      DialogComponents dialogComponents = new DialogComponents() {
         
         JLabel lblBounds = new JLabel("Bounds:");
         private final JFormattedTextField txtMaxMolWeight = new JFormattedTextField();
         private final JFormattedTextField txtMinMolWeight = new JFormattedTextField();
         
         JLabel lblCriterion = new JLabel("Criterion:");
         private final JRadioButton rbMolecularWeight = new JRadioButton("Molecular weight");
         private final JRadioButton rbMostAbundantMass = new JRadioButton("Most abundant mass");
         private final JRadioButton rbMonoisotopicMass = new JRadioButton("Monoisotopic mass");
         
         private final JCheckBox chbMinMolWeightActive = new JCheckBox(
               "Lower bound for weight/mass");
         private final JCheckBox chbMaxMolWeightActive = new JCheckBox(
               "Upper bound for weight/mass");
         
         private final ChangeListener changeListener = new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
               
               txtMinMolWeight.setEnabled(chbMinMolWeightActive.isSelected());
               txtMaxMolWeight.setEnabled(chbMaxMolWeightActive.isSelected());
               
            }
         };
         
         @Override
         public void loadDialogComponents(IndigoDialogPanel dialogPanel,
               IndigoSplitterSettings settings) {

            registerDialogComponents(settings);
            
            dialogPanel.addItemsPanel("Filter Settings");

            dialogPanel.addItem(lblCriterion);

            ButtonGroup comparisonOptions = new ButtonGroup();
            comparisonOptions.add(rbMolecularWeight);
            comparisonOptions.add(rbMostAbundantMass);
            comparisonOptions.add(rbMonoisotopicMass);
            
            dialogPanel.addItem(rbMolecularWeight);
            dialogPanel.addItem(rbMostAbundantMass);
            dialogPanel.addItem(rbMonoisotopicMass);
            
            dialogPanel.addItem(lblBounds);
            
            // set format with the following pattern
            String pattern = "####.#########";
            DecimalFormat format = new DecimalFormat(pattern);
            NumberFormatter formatter = new NumberFormatter(format);
            DefaultFormatterFactory factory = new DefaultFormatterFactory(formatter); 
            txtMinMolWeight.setColumns(10);
            txtMinMolWeight.setFormatterFactory(factory);
            txtMaxMolWeight.setColumns(10);
            txtMaxMolWeight.setFormatterFactory(factory);
            
            dialogPanel.addItem(chbMinMolWeightActive, txtMinMolWeight);
            dialogPanel.addItem(chbMaxMolWeightActive, txtMaxMolWeight);

            chbMinMolWeightActive.addChangeListener(changeListener);
            chbMaxMolWeightActive.addChangeListener(changeListener);
            
            changeListener.stateChanged(null);
            
         }
         
         private void registerDialogComponents(IndigoSplitterSettings settings) {
            
            settings.registerDialogComponent(rbMolecularWeight, nodeSettings.molecularWeight);
            settings.registerDialogComponent(rbMostAbundantMass, nodeSettings.mostAbundantMass);
            settings.registerDialogComponent(rbMonoisotopicMass, nodeSettings.monoisotopicMass);
            
            settings.registerDialogComponent(txtMinMolWeight, nodeSettings.minMolWeight);
            settings.registerDialogComponent(txtMaxMolWeight, nodeSettings.maxMolWeight);

            settings.registerDialogComponent(chbMinMolWeightActive,
                  nodeSettings.minMolWeightActive);
            settings.registerDialogComponent(chbMaxMolWeightActive,
                  nodeSettings.maxMolWeightActive);
            
         }
         
      };
      
      
      return new IndigoSplitterNodeDialog(nodeSettings, dialogComponents, new IndigoType[] {IndigoType.MOLECULE});
   }

}
