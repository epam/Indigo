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

package com.epam.indigo.knime.highlighter;

import java.util.ArrayList;
import java.util.List;

import javax.swing.JCheckBox;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.StringValue;
import org.knime.core.data.collection.CollectionDataValue;
import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelColumnName;
import org.knime.core.node.util.ColumnSelectionComboxBox;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog.DialogComponents;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.common.transformer.IndigoTransformerBase;

public class IndigoHighlighterNodeFactory extends
      NodeFactory<IndigoTransformerNodeModel>
{
   public class IndigoHighlighterNodeSettings extends IndigoTransformerSettings
   {
      public final SettingsModelBoolean clearHighlighting = new SettingsModelBoolean("clearHighlighting", false);
      public final SettingsModelColumnName atomsColumn = new SettingsModelColumnName("atomsColumn", null);
      public final SettingsModelColumnName bondsColumn = new SettingsModelColumnName("bondsColumn", null);
      public final SettingsModelBoolean highlightAtoms = new SettingsModelBoolean("highlightAtoms", false);
      public final SettingsModelBoolean highlightBonds = new SettingsModelBoolean("highlightBonds", false);

      public IndigoHighlighterNodeSettings() {
         super();
         addSettingsParameter(clearHighlighting);
         addSettingsParameter(highlightAtoms);
         addSettingsParameter(highlightBonds);
         addSettingsParameter(atomsColumn);
         addSettingsParameter(bondsColumn);
      }
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoTransformerNodeModel createNodeModel ()
   {
      final IndigoHighlighterNodeSettings settings = new IndigoHighlighterNodeSettings();
      return new IndigoTransformerNodeModel("highlight", settings,
            new IndigoTransformerBase() {
         
               int atomsColIndex = -1, bondsColIndex = -1;
         
               private List<Integer> getIndices (final DataRow row, final int index)
               {
                  DataCell cell = row.getCell(index);
                  ArrayList<Integer> indices = new ArrayList<Integer>();
                  if (cell instanceof CollectionDataValue)
                  {
                     CollectionDataValue coll = (CollectionDataValue)cell;
                     for (DataCell subcell : coll)
                        if (!subcell.isMissing())
                           indices.add(Integer.parseInt(subcell.toString()));
                  }
                  else
                  {
                     String str = row.getCell(index).toString();
                     if (row.getCell(index).isMissing())
                        return indices;
                     String[] results = str.split("[ ,;]");
                     for (String s : results) {
                        if (s.length() > 0)
                           indices.add(Integer.parseInt(s));
                     }
                  }
                  return indices;
               }
               
               @Override
               public void transformWithRow(IndigoObject io, boolean reaction, DataRow row) {
                  if (settings.clearHighlighting.getBooleanValue())
                  {
                     for (IndigoObject a : io.iterateAtoms())
                        a.unhighlight();
                     for (IndigoObject b : io.iterateBonds())
                        b.unhighlight();
                  }

                  if (settings.highlightAtoms.getBooleanValue())
                     for (Integer idx : getIndices(row, atomsColIndex))
                        io.getAtom(idx - 1).highlight();
                  if (settings.highlightBonds.getBooleanValue())
                     for (Integer idx : getIndices(row, bondsColIndex))
                        io.getBond(idx - 1).highlight();
               }
               
               @Override
               public void initialize(DataTableSpec inSpec) {
                  if (settings.highlightAtoms.getBooleanValue())
                     atomsColIndex = inSpec.findColumnIndex(settings.atomsColumn.getColumnName());
                  if (settings.highlightBonds.getBooleanValue())
                     bondsColIndex = inSpec.findColumnIndex(settings.bondsColumn.getColumnName());
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
      final IndigoHighlighterNodeSettings rsettings = new IndigoHighlighterNodeSettings();
      
      DialogComponents dialogComponents = new DialogComponents() {
         @SuppressWarnings("unchecked")
         private final ColumnSelectionComboxBox cmbAtomIndicesColumn = 
               new ColumnSelectionComboxBox((Border)null, new Class[] { StringValue.class, CollectionDataValue.class }); 
         
         @SuppressWarnings("unchecked")
         private final ColumnSelectionComboxBox cmbBondIndicesColumn = 
               new ColumnSelectionComboxBox((Border)null, new Class[] { StringValue.class, CollectionDataValue.class }); 
         
         private final JCheckBox chbHighlightAtoms = new JCheckBox("Highlight atoms");
         private final JCheckBox chbHighlightBonds = new JCheckBox("Highlight bonds");
         private final JCheckBox chbClearHighlighting = new JCheckBox("Clear highlighting");
         
         @Override
         public void loadDialogComponents(IndigoDialogPanel dialogPanel, IndigoTransformerSettings settings) {
            settings.registerDialogComponent(chbClearHighlighting, rsettings.clearHighlighting);
            settings.registerDialogComponent(chbHighlightAtoms, rsettings.highlightAtoms);
            settings.registerDialogComponent(chbHighlightBonds, rsettings.highlightBonds);
            settings.registerDialogComponent(cmbAtomIndicesColumn, IndigoHighlighterNodeSettings.INPUT_PORT, rsettings.atomsColumn);
            settings.registerDialogComponent(cmbBondIndicesColumn, IndigoHighlighterNodeSettings.INPUT_PORT, rsettings.bondsColumn);

            // Initialize
            dialogPanel.addItemsPanel("Highlighter settings");
            dialogPanel.addItem(chbClearHighlighting);
            dialogPanel.addItem(chbHighlightAtoms);
            dialogPanel.addItem("Atom indices column", cmbAtomIndicesColumn);
            dialogPanel.addItem(chbHighlightBonds);
            dialogPanel.addItem("Bond indices column", cmbBondIndicesColumn);
            
            ChangeListener changeListener = new ChangeListener() {
               public void stateChanged(final ChangeEvent e) {
                  cmbAtomIndicesColumn.setEnabled(chbHighlightAtoms.isSelected());
                  cmbBondIndicesColumn.setEnabled(chbHighlightBonds.isSelected());
               }
            };
            
            chbHighlightAtoms.addChangeListener(changeListener);
            chbHighlightBonds.addChangeListener(changeListener);
            
            changeListener.stateChanged(null);
         }
         
      };
      return new IndigoTransformerNodeDialog("highlighted", rsettings, dialogComponents, new IndigoType[] {IndigoType.MOLECULE});
   }

}
