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

package com.epam.indigo.knime.valence;

import java.io.File;
import java.io.IOException;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.RowKey;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.data.def.StringCell;
import org.knime.core.node.BufferedDataContainer;
import org.knime.core.node.BufferedDataTable;
import org.knime.core.node.CanceledExecutionException;
import org.knime.core.node.ExecutionContext;
import org.knime.core.node.ExecutionMonitor;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeLogger;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoValenceCheckerNodeModel extends IndigoNodeModel
{
   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoValenceCheckerNodeModel.class);
   
   private final IndigoValenceCheckerSettings nodeSettings = new IndigoValenceCheckerSettings();

   /**
    * Constructor for the node model.
    */
   protected IndigoValenceCheckerNodeModel()
   {
      super(1, 3);
   }

   protected DataTableSpec[] getDataTableSpecs (DataTableSpec inputTableSpec)
         throws InvalidSettingsException
   {
      int colIdx = inputTableSpec.findColumnIndex(nodeSettings.colName.getStringValue());
      if (colIdx == -1)
         throw new InvalidSettingsException("column not found");
      
      /*
       * Create invalid output spec
       */
      DataColumnSpec invalidOutputColumnSpec = new DataColumnSpecCreator(
            nodeSettings.colName.getStringValue(), StringCell.TYPE).createSpec();
      DataColumnSpec[] invalidOutputColumnSpecs = new DataColumnSpec[inputTableSpec
            .getNumColumns()];

      for (int i = 0; i < inputTableSpec.getNumColumns(); i++)
      {
         DataColumnSpec columnSpec = inputTableSpec.getColumnSpec(i);

         if (i == colIdx)
            invalidOutputColumnSpecs[i] = invalidOutputColumnSpec;
         else
            invalidOutputColumnSpecs[i] = columnSpec;
      }

      return new DataTableSpec[] { inputTableSpec, new DataTableSpec(invalidOutputColumnSpecs), inputTableSpec };
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {

      BufferedDataTable bufferedDataTable = inData[IndigoValenceCheckerSettings.INPUT_PORT];
      
      DataTableSpec inputTableSpec = bufferedDataTable.getDataTableSpec();
      DataTableSpec[] outputSpecs = getDataTableSpecs(inputTableSpec);

      BufferedDataContainer validOutputContainer = exec
            .createDataContainer(outputSpecs[0]);
      BufferedDataContainer invalidInputDataContainer = exec
            .createDataContainer(outputSpecs[0]);
      BufferedDataContainer invalidOutputContainer = exec
            .createDataContainer(outputSpecs[1]);

      int colIdx = inputTableSpec.findColumnIndex(nodeSettings.colName.getStringValue());

      if (colIdx == -1)
         throw new Exception("column not found");
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // create cell's converter 
      IndigoConverter converter = IndigoConverterFactory.createConverter(inputTableSpec, colIdx, indigoType);

      int rowNumber = 1;
      Indigo indigo = IndigoPlugin.getIndigo();

      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      for (DataRow inputRow : bufferedDataTable)
      {
         RowKey key = inputRow.getKey();
         DataCell[] cells;
         boolean invalid = false;

         cells = new DataCell[inputRow.getNumCells()];

         DataCell newCell = DataType.getMissingCell();
         DataCell inputCell = inputRow.getCell(colIdx);
         
         String str = null;
         if (!inputCell.isMissing())
            try {
               IndigoPlugin.lock();
               
               newCell = converter.convert(inputCell);
               
               // extract indigo representation
               IndigoObject io = extractIndigoObject(newCell, indigoType);
               str = io.checkBadValence();
            } catch (IndigoException e) {
               LOGGER.warn("Could not indigo representation at row with key: " + inputRow.getKey() + 
                     "; error message: " + e.getMessage());
               invalid = false;
            } finally {
               IndigoPlugin.unlock();
            }
         else
            invalid = true;
         
         if (str != null && !"".equals(str))
         {
            for (int i = 0; i < inputRow.getNumCells(); i++)
            {
               if (i == colIdx)
                  cells[i] = new StringCell(str);
               else
                  cells[i] = inputRow.getCell(i);
            }
            invalidOutputContainer.addRowToTable(new DefaultRow(key, cells));
         }
         else
         {
            if(invalid)
               invalidInputDataContainer.addRowToTable(inputRow);
            else
               validOutputContainer.addRowToTable(inputRow);
         }
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) bufferedDataTable.size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }

      validOutputContainer.close();
      invalidOutputContainer.close();
      invalidInputDataContainer.close();
      return new BufferedDataTable[] { validOutputContainer.getTable(),
            invalidOutputContainer.getTable(), invalidInputDataContainer.getTable() };
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
      DataTableSpec inSpec = inSpecs[IndigoValenceCheckerSettings.INPUT_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, indigoType);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }

      return getDataTableSpecs(inSpecs[0]);
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
