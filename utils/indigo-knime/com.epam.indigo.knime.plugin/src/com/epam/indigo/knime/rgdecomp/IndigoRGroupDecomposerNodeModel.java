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

package com.epam.indigo.knime.rgdecomp;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import org.knime.chem.types.MolAdapterCell;
import org.knime.chem.types.MolCellFactory;
import org.knime.core.data.*;
import org.knime.core.data.container.CloseableRowIterator;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.node.*;

import com.epam.indigo.*;
import com.epam.indigo.knime.cell.*;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoRGroupDecomposerNodeModel extends IndigoNodeModel
{
   IndigoRGroupDecomposerSettings nodeSettings = new IndigoRGroupDecomposerSettings();
   
   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoRGroupDecomposerNodeModel.class);
   
   /**
    * Constructor for the node model.
    */
   protected IndigoRGroupDecomposerNodeModel()
   {
      super(2, 2);
   }

   protected DataTableSpec calcDataTableSpec (DataTableSpec inSpec, int rsites) 
         throws Exception
   {
      DataColumnSpec[] specs = new DataColumnSpec[inSpec.getNumColumns() + rsites + 1];

      int colIdx = inSpec.findColumnIndex(nodeSettings.molColumn.getStringValue());
      if (colIdx == -1)
         throw new Exception("column not found");
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoConverter rgConv = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);
      DataType type = rgConv.getConvertedType();
      
      int i;

      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);

      specs[inSpec.getNumColumns()] = 
            new DataColumnSpecCreator(nodeSettings.newScafColName.getStringValue(), type).createSpec();
      
      for (i = 1; i <= rsites; i++)
         specs[inSpec.getNumColumns() + i] =
            new DataColumnSpecCreator(nodeSettings.newColPrefix.getStringValue() + i, type).createSpec();

      return new DataTableSpec(specs);
   }
   
   protected DataTableSpec calcDataTableScaffoldSpec (DataTableSpec scafSpec)
   {
      return new DataTableSpec(new DataColumnSpecCreator("Scaffold", MolAdapterCell.RAW_TYPE).createSpec());
   }
   
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      BufferedDataTable molDataTable = inData[IndigoRGroupDecomposerSettings.MOL_PORT];
      BufferedDataTable scafDataTable = inData[IndigoRGroupDecomposerSettings.SCAF_PORT];
      
      int molColIdx = molDataTable.getDataTableSpec().findColumnIndex(nodeSettings.molColumn.getStringValue());
      if (molColIdx == -1)
         throw new Exception("column not found");

      int scafColIdx = scafDataTable.getDataTableSpec().findColumnIndex(nodeSettings.scaffoldColumn.getStringValue());
      if (scafColIdx == -1)
         throw new Exception("scaffold column not found");
      
      BufferedDataContainer rgOutputContainer = null;
      BufferedDataContainer scafOutputContainer = 
            exec.createDataContainer(calcDataTableScaffoldSpec(scafDataTable.getDataTableSpec()));
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settings to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      try {
         IndigoObject deco = null;
         IndigoObject query = null;
         /*
          * Create scaffold query
          */
         CloseableRowIterator scafIter = scafDataTable.iterator();
         if (!scafIter.hasNext())
            throw new RuntimeException("no query molecule found in the data source");
         
         
         IndigoType rgType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
         IndigoType queryType = IndigoType.MOLECULE.equals(rgType) ? IndigoType.QUERY_MOLECULE : 
            IndigoType.QUERY_REACTION;
         
         IndigoConverter queryConv = IndigoConverterFactory.
               createConverter(scafDataTable.getDataTableSpec(), scafColIdx, queryType, 
                     nodeSettings.treatStringAsSMARTS.getBooleanValue());
         IndigoConverter rgConv = IndigoConverterFactory.
               createConverter(molDataTable.getDataTableSpec(), molColIdx, rgType);
         
         DataRow queryRow = scafIter.next();
         
         if(queryRow.getCell(scafColIdx).isMissing())
            throw new RuntimeException("query molecule cell is missing");
         
         DataCell newQueryCell = DataType.getMissingCell();
         
         try {
            
            /*
             * Query adapter cell with original and indigo representation
             */
            newQueryCell = queryConv.convert(queryRow.getCell(scafColIdx));    

            /*
             * Extract indigo representation
             */
            query = extractIndigoObject(newQueryCell, queryType, nodeSettings.treatStringAsSMARTS.getBooleanValue());
            
         } catch (IndigoException e) {
            throw new RuntimeException("can not load query molecule: " + e.getMessage());
         }
         
         if (scafIter.hasNext())
            LOGGER.warn("second data source contains more than one row; ignoring all others");
         
         if(query == null)
            throw new RuntimeException("no query molecule found in the data source");
         /*
          * Create decomposer
          */
         try {
            IndigoPlugin.lock();
            
            deco = indigo.createDecomposer(query);
         
         } catch (IndigoException e) {
            deco = null;
            throw new RuntimeException("Error while create decomposer: " + e.getMessage());
         } finally {
            IndigoPlugin.unlock();
         }
         
         /*
          * Iterate input molecules table
          */
         HashMap<RowKey, ArrayList<DataCell> > rgCellsMap = new HashMap<RowKey, ArrayList<DataCell> >();
         
         for (DataRow inputRow : molDataTable) {
            ArrayList<DataCell> rgCells = new ArrayList<DataCell>();
            rgCellsMap.put(inputRow.getKey(), rgCells);

            DataCell cell = inputRow.getCell(molColIdx);
            
            if (cell.isMissing()) {
               LOGGER.warn("Molecule could not be converted to Indigo: ignoring");
               continue;
            }
               
            int numCells = inputRow.getNumCells();
            /*
             * Fulfill initial cells
             */
            for (int i = 0; i < numCells; i++)
               rgCells.add(inputRow.getCell(i));
            
            try {
               IndigoPlugin.lock();
               
               /*
                * Derived indigo representation from cell
                */
               DataCell newMolCell = rgConv.convert(cell);
               IndigoObject inputMol = extractIndigoObject(newMolCell, rgType);
               /*
                * Create decomposition
                */
               IndigoObject decoItem  = deco.decomposeMolecule(inputMol);
               /*
                * Append scaffold and all R-sites
                */
               IndigoObject decoMol = decoItem.decomposedMoleculeWithRGroups();
               /*
                * Iterate all Rgroups
                */
               for (IndigoObject rg : decoMol.iterateRGroups()) {
                  IndigoObject fragIter = rg.iterateRGroupFragments();
                  if (fragIter.hasNext()) {
                     IndigoObject frag = fragIter.next();
                     while(numCells + rg.index() >= rgCells.size())
                        rgCells.add(DataType.getMissingCell());
                     /*
                      * Create cell containing both original and indigo representation
                      */
                     DataCell fragCell = IndigoCellFactory.createCell(frag, null, newMolCell.getType(), rgType);
                     rgCells.set(numCells + rg.index(), fragCell);
                     /*
                      * Remove fragment from molecule
                      */
                     frag.remove();
                  }
               }
               /*
                * Add scaffold cell containing both original and indigo representation
                */
               DataCell decoCell = IndigoCellFactory.createCell(decoMol, null, newMolCell.getType(), rgType);
               rgCells.set(numCells, decoCell);
               
            } catch (IndigoException e) {
               appendWarningMessage("can not decompose target molecule RowId = '" + inputRow.getKey() + "': " + e.getMessage());
               continue;
            } finally {
               IndigoPlugin.unlock();
            }
         }
         
         if(deco != null) {
            /*
             * Append scaffold
             */
            DataCell[] scafCells = new DataCell[1];
            String molfile = deco.decomposedMoleculeScaffold().molfile();
            scafCells[0] = queryType.
                  createAdapterContainingIndigoRepr(MolCellFactory.createAdapterCell(molfile), true, false);
            scafOutputContainer.addRowToTable(new DefaultRow("Row1", scafCells));
            
            int rsites = deco.decomposedMoleculeScaffold().countRSites();
            /*
             * Append all rgroups
             */
            DataTableSpec rgSpec = calcDataTableSpec(molDataTable.getDataTableSpec(), rsites);
            rgOutputContainer = exec.createDataContainer(rgSpec);
            
            for (DataRow inputRow : molDataTable) {
               int numCells = inputRow.getNumCells()+ rsites + 1;
               RowKey inputKey = inputRow.getKey();
               ArrayList<DataCell> rgCells = rgCellsMap.get(inputKey);
               
               DataCell[] outputCells = new DataCell[numCells];
               
               for (int i = 0; i < numCells; i++) {
                  if(i < rgCells.size())
                     outputCells[i] = rgCells.get(i);
                  else
                     outputCells[i] = DataType.getMissingCell();
               }
               rgOutputContainer.addRowToTable(new DefaultRow(inputKey, outputCells));
            }
            
         }
         
      } catch (Exception e) {
         appendWarningMessage("decomposition error: " + e.getMessage());
      }
      
      if(rgOutputContainer == null) 
         rgOutputContainer = exec.createDataContainer(calcDataTableSpec(molDataTable.getDataTableSpec(), 0));
      
      handleWarningMessages();
      rgOutputContainer.close();
      scafOutputContainer.close();
      
      return new BufferedDataTable[] { rgOutputContainer.getTable(), scafOutputContainer.getTable() };
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
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      searchIndigoCompatibleColumn(inSpecs[IndigoRGroupDecomposerSettings.MOL_PORT], nodeSettings.molColumn, indigoType);
      
      IndigoType queryType = IndigoType.MOLECULE.equals(indigoType) ? IndigoType.QUERY_MOLECULE : IndigoType.QUERY_REACTION;
      searchIndigoCompatibleColumn(inSpecs[IndigoRGroupDecomposerSettings.SCAF_PORT], nodeSettings.scaffoldColumn, queryType);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return new DataTableSpec[2];
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
      IndigoRGroupDecomposerSettings s = new IndigoRGroupDecomposerSettings();
      s.loadSettingsFrom(settings);
      
      if (s.molColumn.getStringValue() == null || s.molColumn.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
      if (s.scaffoldColumn.getStringValue() == null || s.scaffoldColumn.getStringValue().length() < 1)
         throw new InvalidSettingsException("query column name must be specified");
      if (s.newColPrefix.getStringValue() == null || s.newColPrefix.getStringValue().length() < 1)
         throw new InvalidSettingsException("prefix must be specified");
      if (s.newScafColName.getStringValue() == null || s.newScafColName.getStringValue().length() < 1)
    	  throw new InvalidSettingsException("scaffold column name must be specified");
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
