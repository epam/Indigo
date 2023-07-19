package com.epam.indigo.knime.areplacer;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeLogger;
import org.knime.core.node.NodeView;
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog.DialogComponents;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;
import com.epam.indigo.knime.common.transformer.IndigoTransformerBase;

public class IndigoAtomReplacerNodeFactory extends 
                        NodeFactory<IndigoTransformerNodeModel>{

   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoAtomReplacerNodeFactory.class);
   
   public class IndigoAtomReplacerNodeSettings extends IndigoTransformerSettings {
      
      public final SettingsModelString newAtomLabel = new SettingsModelString("newAtomLabel", "*");
      public final SettingsModelBoolean replaceHighlighted = new SettingsModelBoolean("replaceHighlighted", false);
      public final SettingsModelBoolean replaceSpecificAtom = new SettingsModelBoolean("replaceSpecificAtom", false);
      public final SettingsModelBoolean replaceAttachmentPoints = new SettingsModelBoolean("replaceAttachmentPoints", false);
      public final SettingsModelString specificAtom = new SettingsModelString("specificAtom", null);
      
      public IndigoAtomReplacerNodeSettings() {
         
         // register default general settings
         super();
         
         // register the node specific settings
         addSettingsParameter(newAtomLabel);
         addSettingsParameter(replaceHighlighted);
         addSettingsParameter(replaceSpecificAtom);
         addSettingsParameter(specificAtom);
         addSettingsParameter(replaceAttachmentPoints);
      }
      
   }
   
   @Override
   public IndigoTransformerNodeModel createNodeModel() {

      final IndigoAtomReplacerNodeSettings settings = new IndigoAtomReplacerNodeSettings();

      return new IndigoTransformerNodeModel("atom replacer", settings,
            new IndigoTransformerBase() {

         @Override
         public void initialize(DataTableSpec inSpec) {
         }

         @Override
         public void transformWithRow(IndigoObject io, boolean reaction,
               DataRow row) {
            
            // Split atoms to replace into a set
            Set<String> atomToReplace = new HashSet<String>();
            
            // fill atomToReplace set
            if (settings.replaceSpecificAtom.getBooleanValue())
            {
               String[] atoms = settings.specificAtom.getStringValue().split("\\s*,\\s*");
               atomToReplace.addAll(Arrays.asList(atoms));
            }
            
            boolean replaceAllAtoms = !settings.replaceHighlighted.getBooleanValue() && 
                  !settings.replaceSpecificAtom.getBooleanValue();
   
            if (!settings.replaceAttachmentPoints.getBooleanValue() || !replaceAllAtoms)
            {
               if(reaction)
                  for(IndigoObject mol : io.iterateMolecules())
                     replaceSpecificAtoms(atomToReplace, mol);
               else
                  replaceSpecificAtoms(atomToReplace, io);
            }
            
            if (settings.replaceAttachmentPoints.getBooleanValue())
            {
               if(reaction)
                  for(IndigoObject mol : io.iterateMolecules())
                     replaceAttachmentPoints(mol);
               else
                  replaceAttachmentPoints(io);
                  
            }
         
      }
      
      private void replaceAttachmentPoints(IndigoObject molStructure) {
         List<Integer> atomsWithAttach = new ArrayList<Integer>();
         int maxOrder = molStructure.countAttachmentPoints();
         
         for (int order = 1; order <= maxOrder; order++)
            for (IndigoObject atomWithAttachment: molStructure.iterateAttachmentPoints(order))
               atomsWithAttach.add(atomWithAttachment.index());
         
         molStructure.clearAttachmentPoints();
         List<Integer> newAtoms = new ArrayList<Integer>();
         for (int idx : atomsWithAttach)
         {
            IndigoObject atom = molStructure.getAtom(idx);
            IndigoObject newAtom;
            if (settings.newAtomLabel.getStringValue().matches("R\\d*"))
               newAtom = molStructure.addRSite(settings.newAtomLabel.getStringValue());
            else
               newAtom = molStructure.addAtom(settings.newAtomLabel.getStringValue());
            atom.addBond(newAtom, 1);
            newAtoms.add(newAtom.index());
         }
         
         // Layout added atoms if coordinates are present
         try
         {
            if (molStructure.hasCoord())
               molStructure.getSubmolecule(newAtoms).layout();
         }
         catch (IndigoException ex)
         {
            LOGGER.warn("Layout exception: " + ex.getMessage());
         }
      }
      
      private void replaceSpecificAtoms(Set<String> atomToReplace, IndigoObject molStructure) {
            List<Integer> atoms = new ArrayList<Integer>();
            
            for (IndigoObject atom : molStructure.iterateAtoms()) {
               if (settings.replaceHighlighted.getBooleanValue() && !atom.isHighlighted())
                  continue;
               if (settings.replaceSpecificAtom.getBooleanValue() && !atomToReplace.contains(atom.symbol()))
                  continue;
               atoms.add(atom.index());
            }

            for (int idx : atoms) {
               IndigoObject atom = molStructure.getAtom(idx);
               if (settings.newAtomLabel.getStringValue().matches("R\\d*"))
                  atom.setRSite(settings.newAtomLabel.getStringValue());
               else
                  atom.resetAtom(settings.newAtomLabel.getStringValue());
            }
         }
   
      });
   }

   @Override
   protected int getNrNodeViews() {
      return 0;
   }

   @Override
   public NodeView<IndigoTransformerNodeModel> createNodeView(
         int viewIndex, IndigoTransformerNodeModel nodeModel) {
      return null;
   }

   @Override
   protected boolean hasDialog() {
      return true;
   }

   @Override
   protected NodeDialogPane createNodeDialogPane() {
      
      final IndigoAtomReplacerNodeSettings rsettings = new IndigoAtomReplacerNodeSettings();
      
      DialogComponents dialogComponents = new DialogComponents() {
         
         private final JTextField txtNewAtomLabelComp = new JTextField(4);
         private final JCheckBox chbReplaceHighlighted = new JCheckBox("Replace only highlighted atoms");
         private final JCheckBox chbReplaceSpecificAtoms = new JCheckBox("Replace specific atoms");
         private final JComboBox<String> cmbSpecificAtomGroup = new JComboBox<String>();
         private final JTextField txtSpecificAtoms = new JTextField(20);
         private final Map<String, String> specificAtomGroupsMap = new HashMap<String, String>();
         private final String customTitle = "Custom ...";
         private final JCheckBox cmbReplaceAttachmentPoints = new JCheckBox("Replace attachment points");
         
         // events listeners
         private final ChangeListener changeListener = new ChangeListener() {
            public void stateChanged (ChangeEvent e)
            {
               // (dis)enable combobox and textbox
               txtSpecificAtoms.setEnabled(chbReplaceSpecificAtoms.isSelected());
               cmbSpecificAtomGroup.setEnabled(chbReplaceSpecificAtoms.isSelected());
               
               // if textfield is not filled yet
               if (txtSpecificAtoms.getText().isEmpty() && cmbSpecificAtomGroup.isEnabled() &&
                     !customTitle.equals(cmbSpecificAtomGroup.getSelectedItem().toString())) {
                  String selected = cmbSpecificAtomGroup.getSelectedItem().toString();
                  txtSpecificAtoms.setText(specificAtomGroupsMap.get(selected));
               }
            }
         };
         
         // action to perform when new atom group is selected or text is changed
         private void updateAtomsListByEvent (Object source)
         {
            // Compare values from drop-down listbox and the text field
            Object selected = cmbSpecificAtomGroup.getSelectedObjects()[0];

            String groupElements = specificAtomGroupsMap.get(selected);
            String elementsFromField = txtSpecificAtoms.getText();
            
            if (groupElements != null && groupElements.equals(elementsFromField))
               return;
            
            if (source == txtSpecificAtoms) {
               // Try to find such value from the predefined set
               if (specificAtomGroupsMap.containsValue(elementsFromField)) {
                  for (String key: specificAtomGroupsMap.keySet()) {
                     if (specificAtomGroupsMap.get(key).equals(elementsFromField)) {
                        cmbSpecificAtomGroup.setSelectedItem(key);
                        return;
                     }
                  }
               } else {
                  cmbSpecificAtomGroup.setSelectedItem(customTitle);
               }
            } else if (groupElements != null) {
               // Just put the value from the combobox
               if (txtSpecificAtoms.getText() != groupElements)
                  txtSpecificAtoms.setText(groupElements);
            } 
         }
         
         private final ActionListener actionListener = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
               updateAtomsListByEvent(arg0.getSource());
            }
         };
         
         private final DocumentListener documentListener = new DocumentListener() {

            @Override
            public void changedUpdate(DocumentEvent arg0) {
               updateAtomsListByEvent(txtSpecificAtoms);
            }

            @Override
            public void insertUpdate(DocumentEvent arg0) {
               updateAtomsListByEvent(txtSpecificAtoms);
            }

            @Override
            public void removeUpdate(DocumentEvent arg0) {
               updateAtomsListByEvent(txtSpecificAtoms);
            }
         };
         
         private void registerDialogComponents(IndigoTransformerSettings settings) {
            
            settings.registerDialogComponent(txtNewAtomLabelComp, rsettings.newAtomLabel);
            settings.registerDialogComponent(chbReplaceHighlighted, rsettings.replaceHighlighted);
            settings.registerDialogComponent(chbReplaceSpecificAtoms, rsettings.replaceSpecificAtom);
            settings.registerDialogComponent(txtSpecificAtoms, rsettings.specificAtom);
            settings.registerDialogComponent(cmbReplaceAttachmentPoints, rsettings.replaceAttachmentPoints);
            
         }

         private void loadAtomGroups ()
         {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            try {
               ArrayList<String> atomGroups = new ArrayList<String>();
               
               DocumentBuilder db = dbf.newDocumentBuilder();
               Document doc = db.parse(getClass().getResourceAsStream("IndigoAtomReplacerConfig.xml"));
               Element docElement = doc.getDocumentElement();
               NodeList groups = docElement.getElementsByTagName("Group");
               int groupsCount = groups.getLength();
               for (int i = 0; i < groupsCount; i++) {
                  Element group = (Element)groups.item(i);
                  String name = group.getAttribute("name");
                  String elementsValue = "";
                  NodeList elements = group.getElementsByTagName("Elements");
                  if (elements.getLength() > 0) {
                     elementsValue = elements.item(0).getFirstChild().getNodeValue();
                  }
                  NodeList includes = group.getElementsByTagName("Include");
                  int includesCount = includes.getLength(); 
                  for (int j = 0; j < includesCount; j++) {
                     String groupName = includes.item(j).getFirstChild().getNodeValue();
                     if (!"".equals(elementsValue))
                        elementsValue += ", ";
                     elementsValue += specificAtomGroupsMap.get(groupName);  
                  }
                  specificAtomGroupsMap.put(name, elementsValue);
                  atomGroups.add(name);
               }
               Collections.sort(atomGroups);
               for (String group : atomGroups)
                  cmbSpecificAtomGroup.addItem(group);
            } catch (ParserConfigurationException e) {
               LOGGER.error(e);
               e.printStackTrace();
            } catch (SAXException e) {
               LOGGER.error(e);
               e.printStackTrace();
            } catch (IOException e) {
               LOGGER.error(e);
               e.printStackTrace();
            }
            
            cmbSpecificAtomGroup.addItem(customTitle);
         }
         
         @Override
         public void loadDialogComponents(IndigoDialogPanel dialogPanel,
               IndigoTransformerSettings settings) {

            registerDialogComponents(settings);
            
            // put items on the dialog panel
            dialogPanel.addItemsPanel("Atom Settings");
            dialogPanel.addItem("New atom label:", txtNewAtomLabelComp);
            dialogPanel.addItem(chbReplaceHighlighted);
            dialogPanel.addItem(chbReplaceSpecificAtoms);
            dialogPanel.addItem(cmbSpecificAtomGroup, txtSpecificAtoms);
            dialogPanel.addItem(cmbReplaceAttachmentPoints);
            
            // read config and load data from there
            loadAtomGroups();
            
            // assign listeners
            chbReplaceSpecificAtoms.addChangeListener(changeListener);
            txtSpecificAtoms.getDocument().addDocumentListener(documentListener);
            cmbSpecificAtomGroup.addActionListener(actionListener);
            
            changeListener.stateChanged(null);
         }
      };
      
      return new IndigoTransformerNodeDialog("atom replacer", rsettings, dialogComponents);
   }

}
