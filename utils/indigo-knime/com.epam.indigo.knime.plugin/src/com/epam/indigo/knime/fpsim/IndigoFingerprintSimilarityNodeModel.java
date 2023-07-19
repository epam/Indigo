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

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import org.knime.core.data.*;
import org.knime.core.data.container.CloseableRowIterator;
import org.knime.core.data.def.*;
import org.knime.core.data.vector.bitvector.*;
import org.knime.core.node.*;

import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.fpsim.IndigoFingerprintSimilaritySettings.Aggregation;
import com.epam.indigo.knime.fpsim.IndigoFingerprintSimilaritySettings.Metric;

public class IndigoFingerprintSimilarityNodeModel extends IndigoNodeModel
{
   IndigoFingerprintSimilaritySettings nodeSettings = new IndigoFingerprintSimilaritySettings();
   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoFingerprintSimilarityNodeModel.class);

   /**
    * Constructor for the node model.
    */
   protected IndigoFingerprintSimilarityNodeModel()
   {
      super(2, 1);
   }

   protected DataTableSpec getDataTableSpec (DataTableSpec inSpec) throws InvalidSettingsException
   {
      DataColumnSpec[] specs = new DataColumnSpec[inSpec.getNumColumns() + 1];

      if (nodeSettings.newColName.getStringValue() == null || nodeSettings.newColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No new column name specified");
      
      int i;

      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);

      specs[i] = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(),
            DoubleCell.TYPE).createSpec();

      return new DataTableSpec(specs);
   }

   protected float calcSimilarity (BitVectorValue bitvector, BitVectorValue template) throws Exception
   {
      if (bitvector.length() != template.length())
         throw new Exception("fingerprint's length does not match the template");
      
      long a = template.cardinality();
      long b = bitvector.cardinality();
      long c = 0;
      
      long ia = template.nextSetBit(0), ib = bitvector.nextSetBit(0);
      
      while (ia != -1 && ib != -1)
      {
         if (ia < ib)
            ia = template.nextSetBit(ia + 1);
         else if (ia > ib)
            ib = bitvector.nextSetBit(ib + 1);
         else
         {
            c++;
            ia = template.nextSetBit(ia + 1);
            ib = bitvector.nextSetBit(ib + 1);
         }
      }
      
      if (c == 0)
         return 0;
      else if (Metric.Tanimoto.toString().equals(nodeSettings.metric.getStringValue()))
         return (float)c / (a + b - c);
      else if (Metric.EuclidSub.toString().equals(nodeSettings.metric.getStringValue()))
         return (float)c / a;
      else
         return (float)c / (float)(nodeSettings.tverskyAlpha.getFloatValue() * (a - c) +
                            nodeSettings.tverskyBeta.getFloatValue()  * (b - c) + c); 
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      DataTableSpec spec = getDataTableSpec(inData[0].getDataTableSpec());
      DataTableSpec spec2 = getDataTableSpec(inData[1].getDataTableSpec());

      BufferedDataContainer outputContainer = exec.createDataContainer(spec);

      int colIdx = spec.findColumnIndex(nodeSettings.targetColumn.getStringValue());
      if (colIdx == -1)
         throw new Exception("column not found");

      int colIdx2 = spec2.findColumnIndex(nodeSettings.queryColumn.getStringValue());
      if (colIdx2 == -1)
         throw new Exception("second column not found");

      ArrayList<BitVectorValue> templates = new ArrayList<BitVectorValue>(); 
      
      boolean missingPrinted = false; 
      
      {
         CloseableRowIterator it = inData[1].iterator();
         
         if (!it.hasNext())
            throw new Exception("no template fingerprint found in the data source");

         while (it.hasNext())
         {
            DataRow row = it.next();
            DataCell cell = row.getCell(colIdx2);
            if (cell.isMissing())
            {
               if (!missingPrinted)
                  LOGGER.warn("Missing values were skipped");
               missingPrinted = true;
               continue;
            }
            templates.add((BitVectorValue)cell);
         }
      }
      
      CloseableRowIterator it = inData[0].iterator();
      int rowNumber = 1;

      while (it.hasNext())
      {
         DataRow inputRow = it.next();
         RowKey key = inputRow.getKey();
         DataCell[] cells = new DataCell[inputRow.getNumCells() + 1];
         DataCell cell = inputRow.getCell(colIdx);
         if (cell.isMissing())
         {
            if (!missingPrinted)
               LOGGER.warn("Missing values were skipped");
            missingPrinted = true;
            continue;
         }
         
         BitVectorValue bitvector = (BitVectorValue)cell;

         float result = 0;
         int count = 0;
         
         if (Aggregation.Minimum.toString().equals(nodeSettings.aggregation.getStringValue()))
            result = 1000000;
         
         for (BitVectorValue template : templates)
         {
            float sim = calcSimilarity(bitvector, template);

            if (Aggregation.Minimum.toString().equals(nodeSettings.aggregation.getStringValue())) {
               if (sim < result)
                  result = sim;
            }
            if (Aggregation.Maximum.toString().equals(nodeSettings.aggregation.getStringValue())) {
               if (sim > result)
                  result = sim;
            }
            if (Aggregation.Average.toString().equals(nodeSettings.aggregation.getStringValue())) {
               result += sim;
            }
            count++;
         }

         if (Aggregation.Average.toString().equals(nodeSettings.aggregation.getStringValue()))
            result /= count;
         
         int i;
         
         for (i = 0; i < inputRow.getNumCells(); i++)
            cells[i] = inputRow.getCell(i);
         cells[i++] = new DoubleCell(result);

         outputContainer.addRowToTable(new DefaultRow(key, cells));
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) inData[0].size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }

      outputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable() };
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void reset ()
   {
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected DataTableSpec[] configure (final DataTableSpec[] inSpecs)
         throws InvalidSettingsException
   {
      nodeSettings.targetColumn.setStringValue(searchIndigoColumn(inSpecs[0], nodeSettings.targetColumn.getStringValue(), BitVectorValue.class));
      nodeSettings.queryColumn.setStringValue(searchIndigoColumn(inSpecs[1], nodeSettings.queryColumn.getStringValue(), BitVectorValue.class));
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      return new DataTableSpec[] { getDataTableSpec(inSpecs[0]) };
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveSettingsTo (final NodeSettingsWO settings)
   {
      nodeSettings.saveSettingsTo(settings);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadValidatedSettingsFrom (final NodeSettingsRO settings)
         throws InvalidSettingsException
   {
      nodeSettings.loadSettingsFrom(settings);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void validateSettings (final NodeSettingsRO settings)
         throws InvalidSettingsException
   {
      IndigoFingerprintSimilaritySettings s = new IndigoFingerprintSimilaritySettings();
      s.loadSettingsFrom(settings);
      if (s.targetColumn.getStringValue() == null || s.targetColumn.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
      if (s.queryColumn.getStringValue() == null || s.queryColumn.getStringValue().length() < 1)
         throw new InvalidSettingsException("template column name must be specified");
      if (s.newColName.getStringValue() == null || s.newColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("new column name must be specified");
      
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadInternals (final File internDir,
         final ExecutionMonitor exec) throws IOException,
         CanceledExecutionException
   {
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveInternals (final File internDir,
         final ExecutionMonitor exec) throws IOException,
         CanceledExecutionException
   {
   }
}
