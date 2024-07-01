/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems, Inc.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 ***************************************************************************/

package com.epam.indigo.knime.bondreplacer;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog.DialogComponents;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;
import com.epam.indigo.knime.common.transformer.IndigoTransformer;

public class IndigoBondReplacerNodeFactory extends
      NodeFactory<IndigoTransformerNodeModel>
{
   public enum BondOrder {
      Single, Double, Triple, Aromatic
   }
   
   public class IndigoBondReplacerNodeSettings extends IndigoTransformerSettings
   {
      public final SettingsModelString bondOrder = new SettingsModelString("bondOrder", BondOrder.Single.toString());
      public final SettingsModelBoolean replaceHighlighted = new SettingsModelBoolean("replaceHighlighted", false);

      public IndigoBondReplacerNodeSettings() {
         super();
         addSettingsParameter(bondOrder);
         addSettingsParameter(replaceHighlighted);
      }
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoTransformerNodeModel createNodeModel ()
   {
      final IndigoBondReplacerNodeSettings settings = new IndigoBondReplacerNodeSettings();
      return new IndigoTransformerNodeModel("replace bonds", settings,
            new IndigoTransformer()
            {
               @Override
               public void transform (IndigoObject io, boolean reaction)
               {
                  String bondOrderString = settings.bondOrder.getStringValue();
                  int newOrder;
                  if (BondOrder.Single.toString().equals(bondOrderString))
                     newOrder = 1;
                  else if (BondOrder.Double.toString().equals(bondOrderString))
                     newOrder = 2;
                  else if (BondOrder.Triple.toString().equals(bondOrderString))
                     newOrder = 3;
                  else if (BondOrder.Aromatic.toString().equals(bondOrderString))
                     newOrder = 4;
                  else
                     throw new RuntimeException("Unknown bond order type: " + bondOrderString);
                  
                  if(reaction) {
                     for (IndigoObject mol : io.iterateMolecules()) 
                        replaceBonds(settings.replaceHighlighted.getBooleanValue(), mol, newOrder);
                  } else {
                     replaceBonds(settings.replaceHighlighted.getBooleanValue(), io, newOrder);
                  }
               }

               private void replaceBonds(boolean replaceHighlighted,
                     IndigoObject io, int newOrder) {
                  for (IndigoObject bond : io.iterateBonds()) {
                     if (replaceHighlighted && !bond.isHighlighted())
                        continue;
                     bond.setBondOrder(newOrder);
                  }
               }
            });
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public int getNrNodeViews ()
   {
      return 0;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeView<IndigoTransformerNodeModel> createNodeView (final int viewIndex,
         final IndigoTransformerNodeModel nodeModel)
   {
      return null;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public boolean hasDialog ()
   {
      return true;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeDialogPane createNodeDialogPane ()
   {
      final IndigoBondReplacerNodeSettings rsettings = new IndigoBondReplacerNodeSettings();
      
      DialogComponents dialogComponents = new DialogComponents() {
         private final JComboBox<Object> cmbBondOrder = new JComboBox<Object>(new Object[] {
               BondOrder.Single, BondOrder.Double, BondOrder.Triple,
               BondOrder.Aromatic });
         private final JCheckBox chbReplaceHighlighted = new JCheckBox(
               "Replace only highlighted bonds");
         
         @Override
         public void loadDialogComponents(IndigoDialogPanel dialogPanel, IndigoTransformerSettings settings) {
            settings.registerDialogComponent(cmbBondOrder, rsettings.bondOrder);
            settings.registerDialogComponent(chbReplaceHighlighted,
                  rsettings.replaceHighlighted);

            // Initialize
            dialogPanel.addItemsPanel("Replacer settings");
            dialogPanel.addItem("Bond order", cmbBondOrder);
            dialogPanel.addItem(chbReplaceHighlighted);
         }
         
      };
      return new IndigoTransformerNodeDialog("bonds replaced", rsettings, dialogComponents);
   }

}
