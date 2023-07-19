package com.epam.indigo.knime.standardizer;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeLogger;
import org.knime.core.node.NodeView;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.common.transformer.IndigoTransformerBase;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog.DialogComponents;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoStandardizerNodeFactory extends 
                  NodeFactory<IndigoTransformerNodeModel> {

   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoStandardizerNodeFactory.class);
   
   @Override
   public IndigoTransformerNodeModel createNodeModel() {

      final IndigoStandardizerNodeSettings nodeSettings = new IndigoStandardizerNodeSettings();
         
      return new IndigoStandardizerNodeModel("standardizer", nodeSettings, new IndigoTransformerBase() {
         
         @Override
         public void transformWithRow(IndigoObject io, boolean reaction,
               DataRow row) {
            // Options to apply
            List<String> options = new ArrayList<String>(Arrays.asList(nodeSettings.pickedOptions.getStringArrayValue()));
            List<String> optionsRequiredCoord = IndigoStandardizerUtils.getOptionsByScope(3);
            
            // find out options and optionsRequiredCoord intesection
            optionsRequiredCoord.retainAll(options);
            
            // Ignore options with scope = 3 if atoms coordinates are not defined
            if ((optionsRequiredCoord.size() > 0) && (!isAtomsCoordinatesDefined(io))) {
               options.removeAll(optionsRequiredCoord);
               LOGGER.warn(row.getKey() + ": The following options are ignored as they require coordinates to be defined: " 
                                          + String.join(", ", optionsRequiredCoord));
            }
            
            // Do standardizing of the molecule with the final list of options
            doStandardizingWithOptions(io, options);
            
         }
         
         // Returns true if at least one of coordinates differs from zero
         private boolean isAtomsCoordinatesDefined(IndigoObject mol) {
            // run through atoms
            for (IndigoObject atom: mol.iterateAtoms()) {
               float[] coord = atom.xyz();
               // run through coordinates: x, y, z
               for (float axis: coord) {
                  if (Math.abs(axis) > 0) return true;
               }
            }
            return false;
         }
     
         // set selected options to true, standardize and set them back 
         private void doStandardizingWithOptions(IndigoObject mol, List<String> options) {
            setStandardizeOptions(options, true);
            mol.standardize();
            setStandardizeOptions(options, false);
         }
         
         // set all these options to value
         private void setStandardizeOptions(List<String> options, boolean value) {
            Indigo indigo = IndigoPlugin.getIndigo();
            for (String option : options) {
               indigo.setOption(option, value);
            }
         }
         
         @Override
         public void initialize(DataTableSpec inSpec) {
         }
      });
   }

   @Override
   protected int getNrNodeViews() {
      return 0;
   }

   @Override
   public NodeView<IndigoTransformerNodeModel> createNodeView(int viewIndex,
         IndigoTransformerNodeModel nodeModel) {
      return null;
   }

   @Override
   protected boolean hasDialog() {
      return true;
   }

   @Override
   protected NodeDialogPane createNodeDialogPane() {

      final IndigoStandardizerNodeSettings settings = new IndigoStandardizerNodeSettings();
      
      DialogComponents dialogComponents = new IndigoStandardizerDialogComponents();
      
      return new IndigoTransformerNodeDialog("standardizer", settings, dialogComponents, 
            new IndigoType[] {IndigoType.MOLECULE, IndigoType.QUERY_MOLECULE});
   }

}
