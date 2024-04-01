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

package com.epam.indigo.knime.molprop;

import java.io.IOException;

import org.knime.core.data.*;
import org.knime.core.data.def.*;
import org.knime.core.node.*;

import com.epam.indigo.*;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

import java.io.*;

public class IndigoMoleculePropertiesNodeModel extends IndigoNodeModel
{
   
   public static final int INPUT_PORT = 0;
   
   private final IndigoMoleculePropertiesSettings nodeSettings = new IndigoMoleculePropertiesSettings();

   /**
    * Constructor for the node model.
    */
   protected IndigoMoleculePropertiesNodeModel()
   {
      super(1, 2);
   }

   protected DataTableSpec getDataTableSpec (DataTableSpec inSpec) throws InvalidSettingsException
   {
      if (nodeSettings.selectedProps.getStringArrayValue() == null)
         throw new InvalidSettingsException("properties not selected");
      
      DataColumnSpec[] specs = new DataColumnSpec[inSpec.getNumColumns()
            + nodeSettings.selectedProps.getStringArrayValue().length];

      int i;

      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);

      for (String key : nodeSettings.selectedProps.getStringArrayValue())
         specs[i++] = IndigoMoleculePropertiesUtils.colSpecs.get(key);

      return new DataTableSpec(specs);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      final BufferedDataTable bufferedDataTable = inData[INPUT_PORT];
      
      DataTableSpec spec = getDataTableSpec(bufferedDataTable.getDataTableSpec());

      BufferedDataContainer outputContainer = exec.createDataContainer(spec);
      BufferedDataContainer invalidInputContainer = 
            exec.createDataContainer(bufferedDataTable.getDataTableSpec());

      int colIdx = spec.findColumnIndex(nodeSettings.colName.getStringValue());
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoConverter converter = 
            IndigoConverterFactory.createConverter(bufferedDataTable.getDataTableSpec(), colIdx, indigoType);

      if (colIdx == -1)
         throw new Exception("column not found");

      int rowNumber = 1;
      boolean warningPrinted = false;

      Indigo indigo = IndigoPlugin.getIndigo();
      
      for (DataRow inputRow : bufferedDataTable)
      {
         boolean invalid = false;
         RowKey key = inputRow.getKey();
         DataCell[] cells = new DataCell[inputRow.getNumCells()
               + nodeSettings.selectedProps.getStringArrayValue().length];
         int i;

         for (i = 0; i < inputRow.getNumCells(); i++)
            cells[i] = inputRow.getCell(i);
         
         for (i = inputRow.getNumCells(); i < cells.length; i++)
            cells[i] = DataType.getMissingCell();
         
         i = inputRow.getNumCells();
         //get all the pointed atoms
         String[] atoms = nodeSettings.userSpecifiedAtoms.getStringValue().split(",");
         // trim the elements
         for (int j = 0; j < atoms.length; j++) {
            atoms[j] = atoms[j].trim();
         }
         if (!inputRow.getCell(colIdx).isMissing()) {
            try {
               IndigoPlugin.lock();
               
               // take into account settins to process input
               indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
               indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
               
               // replace cell by adapter one containing indigo representation
               DataCell newCell = converter.convert(inputRow.getCell(colIdx));
               
               IndigoObject io = extractIndigoObject(newCell, indigoType);
               
               for (String prop : nodeSettings.selectedProps.getStringArrayValue()) {
                  DataCell cell = null;
                  try {
                     cell = IndigoMoleculePropertiesUtils.calculators.get(prop).calculate(io, atoms);
                  } catch (IndigoException ex) {
                     cell = DataType.getMissingCell();
                     LOGGER.warn("Cannot calculate " + prop + " for row " + inputRow.getKey() + ": " + ex.getMessage(), ex);
                  }
                  cells[i++] = cell;
               }
            }catch (IndigoException e) {
               appendWarningMessage("error while calculating structure properties for RowId '" + inputRow.getKey() + "': " + e.getMessage());
               invalid = true;
            } finally {
               // turn options off
               indigo.setOption("ignore-stereochemistry-errors", false);
               indigo.setOption("treat-x-as-pseudoatom", false);
               
               IndigoPlugin.unlock();
            }
         } else {
            if(!warningPrinted)
               LOGGER.warn("Input table contains missing cells");
            warningPrinted = true;
            invalid = true;
         }
         
         if (!invalid)
            outputContainer.addRowToTable(new DefaultRow(key, cells));
         else
            invalidInputContainer.addRowToTable(inputRow);
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) bufferedDataTable.size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }

      handleWarningMessages();
      outputContainer.close();
      invalidInputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable(), invalidInputContainer.getTable() };
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
      
      DataTableSpec inSpec = inSpecs[INPUT_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, indigoType);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      return new DataTableSpec[] { getDataTableSpec(inSpecs[0]), inSpec};
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
      IndigoMoleculePropertiesSettings s = new IndigoMoleculePropertiesSettings();
      s.loadSettingsFrom(settings);
      
      if (s.colName.getStringValue() == null || s.colName.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
      if (s.searchEditField.getStringValue() != null && s.searchEditField.getStringValue().length() > 0) {
         throw new InvalidSettingsException("clean search fields before saving settings");
      }
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

   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoMoleculePropertiesNodeModel.class);
}
