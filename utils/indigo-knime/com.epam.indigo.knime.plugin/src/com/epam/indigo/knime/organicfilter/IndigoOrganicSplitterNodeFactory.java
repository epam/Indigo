package com.epam.indigo.knime.organicfilter;

import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;

import javax.swing.JCheckBox;

import org.knime.core.data.DataRow;
import org.knime.core.node.BufferedDataContainer;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.splitter.IndigoSplitter;
import com.epam.indigo.knime.common.splitter.IndigoSplitterNodeDialog;
import com.epam.indigo.knime.common.splitter.IndigoSplitterNodeDialog.DialogComponents;
import com.epam.indigo.knime.common.splitter.IndigoSplitterNodeModel;
import com.epam.indigo.knime.common.splitter.IndigoSplitterSettings;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoOrganicSplitterNodeFactory extends NodeFactory<IndigoSplitterNodeModel>{

   public class IndigoOrganicSplitterNodeSettings extends IndigoSplitterSettings {
      
      public final SettingsModelBoolean notIncludingAlkaliMetalSalts = 
            new SettingsModelBoolean("notIncludingAlkaliMetalSalts", true);
      public final SettingsModelBoolean notContainingDBlockElements = 
            new SettingsModelBoolean("notContainingDBlockElements", true);
      
      public IndigoOrganicSplitterNodeSettings() {

         super();
         
         addSettingsParameter(notIncludingAlkaliMetalSalts);
         addSettingsParameter(notContainingDBlockElements);
         
      }
      
   }
   
   @Override
   public IndigoSplitterNodeModel createNodeModel() {

      final IndigoOrganicSplitterNodeSettings nodeSettings = 
            new IndigoOrganicSplitterNodeSettings();
      
      return new IndigoSplitterNodeModel(nodeSettings, new IndigoSplitter() {
         
         @Override
         public void distribute(BufferedDataContainer[] cons, DataRow row,
               IndigoObject io) {
            
            BufferedDataContainer matchOutput = cons[0]; // matching rows
            BufferedDataContainer unmatchOutput = cons[1]; // not matching rows
            
            boolean organic = isOrganic(io, nodeSettings.notIncludingAlkaliMetalSalts.getBooleanValue(), 
                  nodeSettings.notContainingDBlockElements.getBooleanValue());
            
            // distribute the row
            if (organic)
               matchOutput.addRowToTable(row);
            else
               unmatchOutput.addRowToTable(row);
            
         }
         
         private boolean isOrganic(IndigoObject molecule, boolean notIncludingAlkaliMetalSalts, boolean notContainingDBlockElements) {

            boolean organic = true;
            String molSMILES = molecule.canonicalSmiles();

            // convert molecule into map<atom, countAtom>
            HashMap<String, Integer> atomMap = new HashMap<String, Integer>();
            String[] atomNumPairs = molecule.grossFormula().replaceAll("\\s", "").split("(?<=((\\d)|(\\p{L})))(?=\\p{Lu})");
            for (String atomNumPair : atomNumPairs) {
               String[] atomAndNum = atomNumPair.split("(?<=\\p{L})(?=\\d)");
               atomMap.put(atomAndNum[0], (atomAndNum.length > 1 ? Integer.parseInt(atomAndNum[1]) : 1));
            }
            
            // check if molecule contains both carbon and hydrogen
            organic = atomMap.get("C") != null && atomMap.get("H") != null;

            // check if molecule contains an atom from d-block elements
            if (organic && notContainingDBlockElements){
               // d-block elements
               String[] elements = {"Sc", "Ti", "V",  "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
                                    "Y",  "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd",
                                    "Lu", "Hf", "Ta", "W",  "Re", "Os", "Ir", "Pt", "Au", "Hg",
                                    "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn"};
               Set<String> dblock = new HashSet<String>(Arrays.<String>asList(elements));

               // check for an intersection
//               organic = Collections.disjoint(dblock, atomMap.keySet());
               
               organic = (atomMap.keySet().stream().filter(s -> dblock.contains(s)).collect(Collectors.toList()).size() == 0);
               
            }
            
            // check if molecule is an organic alkali metal salt
            if (organic && notIncludingAlkaliMetalSalts) {
               if (!molSMILES.isEmpty() && molSMILES.contains(".")) {
                  // alkali metals 
                  String[] elements= {"Li", "Na", "K", "Rb", "Cs", "Fr"};
                  Set<String> alkaliMetals = new HashSet<String>(Arrays.<String>asList(elements));
                  
                  // check if molecule contains alkali metal
                  String cation = "";
                  for (String metal : alkaliMetals) {
                     if (atomMap.keySet().contains(metal)) {
                        cation = metal;
                        break;
                     }  
                  }
                  
                  // check charge of contained ions to ensure it's a salt 
                  if (!cation.isEmpty()) {
                     int cationCharge = 0, anionCharge = 0;
                     for(IndigoObject atom: molecule.iterateAtoms()) {
                        if (atom.symbol().equals(cation)) {
                           cationCharge += atom.charge();
                        } else {
                           anionCharge += atom.charge();
                        }
                     }
                     if (cationCharge > 0 && anionCharge < 0) {
                        organic = false;
                     }
                  }
               }
            }
            
            // check if molecule is among exclusions
            if (organic && !molSMILES.isEmpty()) {
               String carbonicAcidSMILES = "OC(O)=O";
               String carbonateSMILES = "C[O-]C([O-])=O";
               String cyanideSMILES = "[C-]#N";
               Set<String> carbonOxid = new HashSet<String>(Arrays.asList(new String[]{"C", "O"})); 
               
               organic = !(molSMILES.equals(carbonicAcidSMILES) 
                     || molSMILES.contains(carbonateSMILES)
                     || molSMILES.contains(cyanideSMILES)
                     || (atomMap.keySet().size() == 2 && carbonOxid.containsAll(atomMap.keySet())));
            }

            return organic;
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
      
      final IndigoOrganicSplitterNodeSettings nodeSettings = 
            new IndigoOrganicSplitterNodeSettings();
      
      DialogComponents dialogComponents = new DialogComponents() {
         
         private final JCheckBox chbNotIncludingAlkaliMetalSalts = new JCheckBox(
               "Interpret alkali metal salts as inorganic molecules");
         private final JCheckBox chbNotContainingDBlockElements = new JCheckBox(
               "Interpret molecules containing d-block elements as inorganic molecules");

         @Override
         public void loadDialogComponents(IndigoDialogPanel dialogPanel,
               IndigoSplitterSettings settings) {

            settings.registerDialogComponent(chbNotContainingDBlockElements, nodeSettings.notContainingDBlockElements);
            settings.registerDialogComponent(chbNotIncludingAlkaliMetalSalts, nodeSettings.notIncludingAlkaliMetalSalts);
            
            dialogPanel.addItemsPanel("Splitter settings");
            dialogPanel.addItem(chbNotIncludingAlkaliMetalSalts);
            dialogPanel.addItem(chbNotContainingDBlockElements);
            
         }
      };
      
      return new IndigoSplitterNodeDialog(nodeSettings, dialogComponents, new IndigoType[] {IndigoType.MOLECULE});
   }

}
