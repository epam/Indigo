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

package com.epam.indigo.knime.fpsim;

import java.awt.*;
import java.awt.event.*;
import java.text.*;

import javax.swing.*;
import javax.swing.border.Border;

import org.knime.core.data.*;
import org.knime.core.data.vector.bitvector.BitVectorValue;
import org.knime.core.node.*;
import org.knime.core.node.util.*;

import com.epam.indigo.knime.fpsim.IndigoFingerprintSimilaritySettings.Aggregation;
import com.epam.indigo.knime.fpsim.IndigoFingerprintSimilaritySettings.Metric;

public class IndigoFingerprintSimilarityNodeDialog extends NodeDialogPane
{
   @SuppressWarnings("unchecked")
   private final ColumnSelectionComboxBox cmbTargetColumn = new ColumnSelectionComboxBox(
         (Border) null, BitVectorValue.class);
   @SuppressWarnings("unchecked")
   private final ColumnSelectionComboxBox cmbQueryColumn = new ColumnSelectionComboxBox(
         (Border) null, BitVectorValue.class);
   
   private final JTextField txtNewColumn = new JTextField(20);
   private final JComboBox<Object> cmbMetrics = new JComboBox<Object>(new Object[] {
         Metric.Tanimoto.toString(), Metric.EuclidSub.toString(), Metric.Tversky.toString() });
   
   private final JComboBox<Object> cmbAggregation = new JComboBox<Object>(new Object[] {
         Aggregation.Average.toString(), Aggregation.Minimum.toString(), Aggregation.Maximum.toString()});
   
   JLabel lblalpha = new JLabel("alpha:");
   JLabel lblBeta = new JLabel("beta:");
   JFormattedTextField txtAlpha = new JFormattedTextField(
         NumberFormat.getNumberInstance());
   JFormattedTextField txtBeta = new JFormattedTextField(
         NumberFormat.getNumberInstance());
   JPanel metricsPanel = new JPanel();

   private final IndigoFingerprintSimilaritySettings nodeSettings = new IndigoFingerprintSimilaritySettings();
   private ActionListener metricsListener = new ActionListener()
   {
      @Override
      public void actionPerformed (ActionEvent arg0)
      {
         if (Metric.Tversky.toString().equals(cmbMetrics.getSelectedItem()))
         {
            txtAlpha.setVisible(true);
            txtBeta.setVisible(true);
            lblalpha.setVisible(true);
            lblBeta.setVisible(true);
         }
         else
         {
            txtAlpha.setVisible(false);
            txtBeta.setVisible(false);
            lblalpha.setVisible(false);
            lblBeta.setVisible(false);
         }
      }
   };

   protected IndigoFingerprintSimilarityNodeDialog()
   {
      super();
      
      registerDialogComponents();
      
      JPanel p = new JPanel(new GridBagLayout());

      GridBagConstraints c = new GridBagConstraints();

      c.anchor = GridBagConstraints.WEST;
      c.insets = new Insets(2, 2, 2, 2);

      c.gridy = 0;
      c.gridx = 0;
      p.add(new JLabel("Column with fingerprints"), c);
      c.gridx = 1;
      p.add(cmbTargetColumn, c);

      c.gridy++;
      c.gridx = 0;
      p.add(new JLabel("Column with reference fingerprint(s)"), c);
      c.gridx = 1;
      p.add(cmbQueryColumn, c);
     
      c.gridy++;
      c.gridx = 0;
      p.add(new JLabel("New column"), c);
      c.gridx = 1;
      p.add(txtNewColumn, c);

      txtAlpha.setColumns(3);
      txtBeta.setColumns(3);

      c.gridy++;
      c.gridx = 0;
      p.add(new JLabel("Metric"), c);
      c.gridx = 1;
      p.add(cmbMetrics, c);
      c.gridy++;
      c.gridx = 0;
      p.add(new JLabel(), c);
      c.gridx = 1;
      metricsPanel.add(lblalpha);
      metricsPanel.add(txtAlpha);
      metricsPanel.add(lblBeta);
      metricsPanel.add(txtBeta);
      p.add(metricsPanel, c);
      
      c.gridy++;
      c.gridx = 0;
      p.add(new JLabel("Aggregation type"), c);
      c.gridx = 1;
      p.add(cmbAggregation, c);

      addTab("Standard settings", p);

      cmbMetrics.addActionListener(metricsListener );
   }

   private void registerDialogComponents() {
      nodeSettings.registerDialogComponent(cmbTargetColumn, IndigoFingerprintSimilaritySettings.TARGET_PORT, nodeSettings.targetColumn);
      nodeSettings.registerDialogComponent(cmbQueryColumn, IndigoFingerprintSimilaritySettings.QUERY_PORT, nodeSettings.queryColumn);
      nodeSettings.registerDialogComponent(txtNewColumn, nodeSettings.newColName);
      nodeSettings.registerDialogComponent(txtAlpha, nodeSettings.tverskyAlpha);
      nodeSettings.registerDialogComponent(txtBeta, nodeSettings.tverskyBeta);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadSettingsFrom (final NodeSettingsRO settings,
         final DataTableSpec[] specs) throws NotConfigurableException
   {
      try {
         nodeSettings.loadSettingsFrom(settings);
         nodeSettings.loadDialogSettings(specs);
         
         cmbMetrics.setSelectedItem(Metric.valueOf(nodeSettings.metric.getStringValue()));
         cmbAggregation.setSelectedItem(Aggregation.valueOf(nodeSettings.aggregation.getStringValue()));
         
         metricsListener.actionPerformed(null);
         
      } catch (InvalidSettingsException e) {
         throw new NotConfigurableException(e.getMessage());
      }
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveSettingsTo (final NodeSettingsWO settings)
         throws InvalidSettingsException
   {
      nodeSettings.metric.setStringValue(cmbMetrics.getSelectedItem().toString());
      nodeSettings.aggregation.setStringValue(cmbAggregation.getSelectedItem().toString());
      
      nodeSettings.saveDialogSettings();
      nodeSettings.saveSettingsTo(settings);
   }
}
