package com.epam.indigo.knime.murcko;

import java.util.ArrayList;

import javax.swing.JCheckBox;

import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerBase;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog.DialogComponents;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;
import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoMurckoScaffoldNodeFactory extends
                     NodeFactory<IndigoTransformerNodeModel> {

   public class IndigoMurckoScaffoldNodeSettings extends IndigoTransformerSettings {
      
      public final SettingsModelBoolean removeTerminalRings3 = new SettingsModelBoolean("removeTerminalRings3", false);
      public final SettingsModelBoolean removeTerminalRings4 = new SettingsModelBoolean("removeTerminalRings4", false);
      
      public IndigoMurckoScaffoldNodeSettings() {
         super();
         addSettingsParameter(removeTerminalRings3);
         addSettingsParameter(removeTerminalRings4);
      }
      
   }
   
   @Override
   public IndigoTransformerNodeModel createNodeModel() {

      final IndigoMurckoScaffoldNodeSettings settings = new IndigoMurckoScaffoldNodeSettings();
      
      return new IndigoTransformerNodeModel("murcko scaffold", settings, 
            new IndigoTransformerBase() {
               
               protected void calculateMurckoScaffold (IndigoObject mol)
               {
                  int natoms;
               
                  do // have to to the mess in a loop because of tricky CC(C)C case and such
                  {
                     natoms = mol.countAtoms();
                     ArrayList<IndigoObject> hangingAtoms = new ArrayList<IndigoObject>();
                     
                     for (IndigoObject atom : mol.iterateAtoms())
                        if (atom.degree() <= 1)
                           hangingAtoms.add(atom);
                     
                     while (hangingAtoms.size() > 0)
                     {
                        ArrayList<IndigoObject> toRemove = new ArrayList<IndigoObject>();
                        ArrayList<IndigoObject> hangingNext = new ArrayList<IndigoObject>();
                        
                        for (IndigoObject atom : hangingAtoms)
                        {
                           if (atom.degree() == 0)
                              toRemove.add(atom);
                           else
                           {
                              IndigoObject nei = atom.iterateNeighbors().next();
                              
                              if (nei.degree() <= 2 || nei.bond().bondOrder() == 1)
                                 toRemove.add(atom);
                              else
                              {
                                 if (!hangingNext.contains(atom))
                                    hangingNext.add(atom);
                              }
                           }
                        }
               
                        for (IndigoObject atom : toRemove)
                        {
                           if (atom.degree() > 0)
                           {
                              IndigoObject nei = atom.iterateNeighbors().next();
                              
                              if (nei.degree() == 2)
                              {
                                 boolean found = false;
                                 
                                 for (IndigoObject a : toRemove)
                                    if (a.index() == nei.index())
                                    {
                                       found = true;
                                       break;
                                    }
                                 
                                 if (!found)
                                 {
                                    for (IndigoObject a : hangingNext)
                                       if (a.index() == nei.index())
                                       {
                                          found = true;
                                          break;
                                       }
                                    if (!found)
                                       hangingNext.add(nei);
                                 }
                              }
                           }
                        }
                        
                        if (toRemove.isEmpty())
                           break;
                        
                        for (IndigoObject atom : toRemove)
                           atom.remove();
                        
                        hangingAtoms = hangingNext;
                     }
                  } while (natoms > mol.countAtoms());
               }
               
               protected boolean removeTerminalRing (IndigoObject mol)
               {
                  if (!settings.removeTerminalRings3.getBooleanValue() && !settings.removeTerminalRings4.getBooleanValue())
                     return false;
                  
                  for (IndigoObject ring : mol.iterateSSSR())
                  {
                     int nsubst = 0;
                     boolean ok = true;
                     
                     if (settings.removeTerminalRings3.getBooleanValue() && ring.countAtoms() == 3)
                        ;
                     else if (settings.removeTerminalRings4.getBooleanValue() && ring.countAtoms() == 4)
                        ;
                     else
                        continue;
                     
                     for (IndigoObject atom : ring.iterateAtoms())
                     {
                        if (atom.degree() == 3)
                           nsubst++;
                        else if (atom.degree() > 3)
                        {
                           ok = false;
                           break;
                        }
                     }
                          
                     if (ok && nsubst <= 1)
                     {
                        for (IndigoObject atom : ring.iterateAtoms())
                           atom.remove();
                        return true;
                     }
                  }
                  return false;
               }
               
               protected boolean removeO2Group (IndigoObject mol)
               {
                  IndigoObject query = mol.getIndigo().loadQueryMolecule("[*D1]=[*]=[*D1]");
                  IndigoObject match = mol.getIndigo().substructureMatcher(mol).match(query);
                  
                  if (match != null && match.mapAtom(query.getAtom(1)).degree() < 4)
                  {
                     match.mapAtom(query.getAtom(0)).remove();
                     match.mapAtom(query.getAtom(2)).remove();
                     return true;
                  }
                  return false;
               }
               
               protected boolean removeOfromNO (IndigoObject mol)
               {
                  IndigoObject query = mol.getIndigo().loadQueryMolecule("[OD1]=[N+0]");
                  IndigoObject match = mol.getIndigo().substructureMatcher(mol).match(query);
                  
                  if (match != null)
                  {
                     match.mapAtom(query.getAtom(0)).remove();
                     return true;
                  }
                  
                  query = mol.getIndigo().loadQueryMolecule("[O-D1]-[N+]");
                  match = mol.getIndigo().substructureMatcher(mol).match(query);
                  
                  if (match != null)
                  {
                     int h = match.mapAtom(query.getAtom(1)).countImplicitHydrogens();
                     match.mapAtom(query.getAtom(0)).remove();
                     match.mapAtom(query.getAtom(1)).resetCharge();
                     match.mapAtom(query.getAtom(1)).setImplicitHCount(h);
                     return true;
                  }
                  
                  return false;
               }
               
               @Override
               public void transformWithRow(IndigoObject io, boolean reaction,
                     DataRow row) {

                  while (true)
                  {
                     if (removeOfromNO(io))
                        continue;
                     
                     calculateMurckoScaffold(io);
                     
                     if (removeTerminalRing(io))
                        continue;
                     
                     if (removeO2Group(io))
                        continue;
                     
                     break;
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
      
      final IndigoMurckoScaffoldNodeSettings nodeSettings = new IndigoMurckoScaffoldNodeSettings();
      
      DialogComponents dialogComponents = new DialogComponents() {
         
         private final JCheckBox chbRemoveTerminalRings3 = new JCheckBox("Remove terminal 3-rings");
         private final JCheckBox chbRemoveTerminalRings4 = new JCheckBox("Remove terminal 4-rings");
         
         @Override
         public void loadDialogComponents(IndigoDialogPanel dialogPanel,
               IndigoTransformerSettings settings) {

            // bound components from dialog and correspondent settings
            nodeSettings.registerDialogComponent(chbRemoveTerminalRings3, nodeSettings.removeTerminalRings3);
            nodeSettings.registerDialogComponent(chbRemoveTerminalRings4, nodeSettings.removeTerminalRings4);
            
            dialogPanel.addItemsPanel("Murcko Scaffold Settings");
            dialogPanel.addItem(chbRemoveTerminalRings3);
            dialogPanel.addItem(chbRemoveTerminalRings4);
            
         }
      };
      
      return new IndigoTransformerNodeDialog("murcko scaffold", nodeSettings, dialogComponents, IndigoType.MOLECULE);
   }

}
