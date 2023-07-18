package com.epam.indigo.knime.fremover;

import java.awt.BorderLayout;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.JCheckBox;

import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;
import org.knime.core.node.defaultnodesettings.SettingsModelStringArray;

import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;
import com.epam.indigo.knime.fremover.IndigoFeatureRemoverUtils.Remover;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerBase;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog.DialogComponents;

public class IndigoFeatureRemoverNodeFactory extends 
                        NodeFactory<IndigoTransformerNodeModel>{

   public class IndigoFeatureRemoverNodeSettings extends IndigoTransformerSettings {
      

      public final SettingsModelStringArray selectedFeatures = 
            new SettingsModelStringArray("selectedFeatures", null);
      
      public IndigoFeatureRemoverNodeSettings() {
        
         // register default general settings
         super();
         // register the node specific settings
         addSettingsParameter(selectedFeatures);
         
      }
   }
   
   
   @Override
   public IndigoTransformerNodeModel createNodeModel() {
      final IndigoFeatureRemoverNodeSettings settings = new IndigoFeatureRemoverNodeSettings();
      
      return new IndigoTransformerNodeModel("feature remover", settings, 
               new IndigoTransformerBase() {
                  
                  @Override
                  public void transformWithRow(IndigoObject io, boolean reaction, DataRow row) {
                     String[] features = settings.selectedFeatures.getStringArrayValue();
                     if (features != null) {
                        for (String s : features) {
                           Remover fRem = IndigoFeatureRemoverUtils.removers.get(s);
                           if(reaction)
                              for(IndigoObject mol : io.iterateMolecules())
                                 fRem.removeFeature(mol);
                           else
                              fRem.removeFeature(io);
                        }                        
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

      final IndigoFeatureRemoverNodeSettings rsettings = new IndigoFeatureRemoverNodeSettings();
      
      DialogComponents dialogComponents = new DialogComponents() {

         private final Map<String, JCheckBox> features = new HashMap<String, JCheckBox>();
         
         @Override
         public void loadDialogComponents(IndigoDialogPanel dialogPanel,
               IndigoTransformerSettings settings) {
            
            dialogPanel.addItemsPanel("Remove the following features:");
            
            // put components (two on every line)
            for (int i = 0; i < 5; i++) {
              
               String key1 = IndigoFeatureRemoverUtils.names.get(i);
               String key2 = IndigoFeatureRemoverUtils.names.get(i+5);
               
               JCheckBox cb1 = new JCheckBox(key1);
               JCheckBox cb2 = new JCheckBox(key2);
               
               features.put(key1, cb1);
               features.put(key2, cb2);
               dialogPanel.addItem(cb1, cb2, BorderLayout.WEST, BorderLayout.WEST);
               
            }
            
         }
         
         @Override
         public void saveToSettings(IndigoTransformerSettings settings) {
            
            List<String> selected = new ArrayList<String>();
            
            for (String s : features.keySet())
               if (features.get(s).isSelected())
                  selected.add(s);
            
            ((IndigoFeatureRemoverNodeSettings) settings).selectedFeatures.
                           setStringArrayValue((String[])selected.toArray(new String[]{}));
         }
         
         
         @Override
         public void loadFromSettings(IndigoTransformerSettings settings) {
            
            for (JCheckBox cb : features.values())
               cb.setSelected(false);

            String[] selected = ((IndigoFeatureRemoverNodeSettings) settings).
                           selectedFeatures.getStringArrayValue();
            
            if(selected != null)
               for (String s : selected)
                  features.get(s).setSelected(true);
         }
         
      };
      
      return new IndigoTransformerNodeDialog("feature remover", rsettings, dialogComponents);
   }

}
