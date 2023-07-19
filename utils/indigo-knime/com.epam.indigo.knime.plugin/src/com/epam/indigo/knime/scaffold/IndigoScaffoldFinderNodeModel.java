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

package com.epam.indigo.knime.scaffold;


import java.io.File;
import java.io.IOException;

import org.knime.chem.types.MolCellFactory;
import org.knime.core.data.*;
import org.knime.core.data.def.*;
import org.knime.core.node.*;

import com.epam.indigo.*;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoScaffoldFinderNodeModel extends IndigoNodeModel
{
   IndigoScaffoldFinderSettings nodeSettings = new IndigoScaffoldFinderSettings();
   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoScaffoldFinderNodeModel.class);

   /**
    * Constructor for the node model.
    */
   protected IndigoScaffoldFinderNodeModel()
   {
      super(1, 1);
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      DataTableSpec inSpec = inData[IndigoScaffoldFinderSettings.INPUT_PORT].getDataTableSpec();
      
      DataColumnSpec spec = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), MolCellFactory.TYPE).createSpec();
      
      DataTableSpec outSpec = new DataTableSpec(spec);
      
      BufferedDataContainer outputContainer = exec.createDataContainer(outSpec);

      int colIdx = inSpec.findColumnIndex(nodeSettings.colName.getStringValue());

      if (colIdx == -1)
         throw new Exception("column not found");
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // create cell's converter 
      IndigoConverter converter = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);

      IndigoObject scaffolds = null;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      try
      {
         IndigoPlugin.lock();

         // take into account settins to process input
         indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
         indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
         
         IndigoObject arr = indigo.createArray();
         
         for (DataRow inputRow : inData[IndigoScaffoldFinderSettings.INPUT_PORT])
         {
            DataCell cell = inputRow.getCell(colIdx);
            
            if(cell.isMissing()) {
               LOGGER.warn("Molecule table contains missing cells: skipping the row with RowId = '" + inputRow.getKey() + "'");
               continue;
            }
               
            DataCell converted = converter.convert(cell);
            
            String str = null;
            IndigoObject molObj = null;
            try {
               
               // extract indigo representation
               molObj = extractIndigoObject(converted, indigoType);
               str = molObj.checkBadValence();
            } catch (IndigoException e) {
               str = e.getMessage();
            }
            if (str != null && !"".equals(str)) {
               LOGGER.warn("Molecule table contains incorrect molecules: skipping the row with RowId = '" + inputRow.getKey() + "': " + str);
            } else if(molObj != null) {
               arr.arrayAdd(molObj);
            }
         }
         IndigoObject extracted = null;
         
         if (nodeSettings.tryExactMethod.getBooleanValue())
         {
            try
            {
               extracted = indigo.extractCommonScaffold(arr, "exact " + nodeSettings.maxIterExact.getIntValue());
            }
            catch (IndigoException e)
            {
               LOGGER.warn("exact method has reached iteration limit: trying to search approximate");
            }
         }
         
         if (extracted == null)
            extracted = indigo.extractCommonScaffold(arr, "approx " + nodeSettings.maxIterApprox.getIntValue());
         
         scaffolds = extracted.allScaffolds();
      }
      catch (IndigoException e)
      {
         LOGGER.error("internal error while launching extract scaffold: " + e.getMessage());
      }
      finally
      {
         
         // turn options off
         indigo.setOption("ignore-stereochemistry-errors", false);
         indigo.setOption("treat-x-as-pseudoatom", false);
         
         IndigoPlugin.unlock();
      }
      if (scaffolds != null) {
         int i = 1;
         {
            for (IndigoObject scaf : scaffolds.iterateArray()) {
               DataCell[] cells = new DataCell[1];
               String molfile = scaf.molfile();
               
               // create mol adapter cell
               cells[0] = IndigoType.QUERY_MOLECULE.
                     createAdapterContainingIndigoRepr(MolCellFactory.createAdapterCell(molfile), true, false);
               outputContainer.addRowToTable(new DefaultRow("Row" + i++, cells));
            }
         }
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
      
      DataTableSpec inSpec = inSpecs[IndigoScaffoldFinderSettings.INPUT_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      if (nodeSettings.newColName.getStringValue() == null || nodeSettings.newColName.getStringValue().length() < 1)
         nodeSettings.newColName.setStringValue("Scaffold");
      
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, indigoType);
     
      DataColumnSpec spec = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), MolCellFactory.TYPE).createSpec();
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) setWarningMessage(nodeSettings.warningMessage);
      
      return new DataTableSpec[] { new DataTableSpec(spec) };
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
      IndigoScaffoldFinderSettings s = new IndigoScaffoldFinderSettings();
      s.loadSettingsFrom(settings);
      if (s.colName.getStringValue() == null || s.colName.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
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
