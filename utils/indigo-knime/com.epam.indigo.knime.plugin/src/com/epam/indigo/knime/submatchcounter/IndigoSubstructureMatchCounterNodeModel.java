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

package com.epam.indigo.knime.submatchcounter;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.knime.core.data.*;
import org.knime.core.node.*;
import org.knime.core.data.def.*;

import com.epam.indigo.*;
import com.epam.indigo.knime.cell.IndigoCellFactory;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;
import com.epam.indigo.knime.submatchcounter.IndigoSubstructureMatchCounterSettings.Uniqueness;

public class IndigoSubstructureMatchCounterNodeModel extends IndigoNodeModel
{
   
   public static final int TARGET_PORT = 0;
   public static final int QUERY_PORT = 1;
   
   IndigoSubstructureMatchCounterSettings nodeSettings = new IndigoSubstructureMatchCounterSettings();

   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoSubstructureMatchCounterNodeModel.class);
   
   protected IndigoSubstructureMatchCounterNodeModel()
   {
      super(2, 1);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      final BufferedDataTable targetsTable = inData[TARGET_PORT];
      final BufferedDataTable queriesTable = inData[QUERY_PORT];
      
      final DataTableSpec targetItemsSpec = targetsTable.getDataTableSpec();
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      int colIdx = targetItemsSpec.findColumnIndex(nodeSettings.targetColName.getStringValue());
      
      // create converter to extract new column type
      IndigoConverter converter = IndigoConverterFactory.
            createConverter(targetItemsSpec, colIdx, indigoType);
      
      final DataTableSpec outSpec = getOutDataTableSpec(targetsTable.getDataTableSpec(), 
               queriesTable, converter, colIdx);

      BufferedDataContainer outputContainer = exec.createDataContainer(outSpec);
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      List<IndigoObject> queries = loadQueries(queriesTable);
      
      int rowNumber = 1;

      for (DataRow inputRow : targetsTable) {
         RowKey key = inputRow.getKey();

         DataCell[] cells = 
            getResultRow(outSpec, colIdx, queries, inputRow, converter);

         outputContainer.addRowToTable(new DefaultRow(key, cells));
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) targetsTable.size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }
      
      // turn options off
      indigo.setOption("ignore-stereochemistry-errors", false);
      indigo.setOption("treat-x-as-pseudoatom", false);
      
      handleWarningMessages();

      outputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable() };
   }

   private List<IndigoObject> loadQueries(BufferedDataTable queriesTableData)
         throws Exception {
      DataTableSpec queryItemsSpec = queriesTableData.getDataTableSpec();
      
      int queryColIdx = queryItemsSpec.findColumnIndex(nodeSettings.queryColName.getStringValue());
      boolean reaction = IndigoType.REACTION.toString().equals(nodeSettings.inputType.getStringValue());
      
      IndigoType queryType = reaction ? IndigoType.QUERY_REACTION : IndigoType.QUERY_MOLECULE;      
      IndigoConverter converter = IndigoConverterFactory.createConverter(queryItemsSpec, queryColIdx, queryType,
            nodeSettings.treatStringAsSMARTS.getBooleanValue());
      
      List<IndigoObject> queries = new ArrayList<IndigoObject>();
      
      
      
      if (queriesTableData.size() == 0)
         LOGGER.warn("There are no query molecules in the table");

      boolean warningPrinted = false;
      for (DataRow row : queriesTableData) {
         
         IndigoObject io = getIndigoQueryStructureOrNull(row.getCell(queryColIdx), converter);
         
         queries.add(io);
         
         if (io == null && !warningPrinted) {
            LOGGER.warn("query table contains missing cells");
            warningPrinted = true;
         }
      }
      return queries;
   }
   
   private IndigoObject getIndigoQueryStructureOrNull(DataCell cell, IndigoConverter converter) 
         throws Exception {
      
      if (cell.isMissing())
         return null;
      
      IndigoObject io = null;
      
      try {
         IndigoPlugin.lock();
         DataCell newCell = converter.convert(cell);    

         // extract indigo representation
         IndigoType queryType = IndigoType.QUERY_MOLECULE;
         io = extractIndigoObject(newCell, queryType, nodeSettings.treatStringAsSMARTS.getBooleanValue());
         
      } catch(IndigoException e) {
         LOGGER.warn("error while loading query structure: " + e.getMessage());
      } finally {
         IndigoPlugin.unlock();
      }
      
      return io;
   }
   
   private DataCell[] getResultRow(DataTableSpec outSpec, int colIdx,
         List<IndigoObject> queries, DataRow inputRow, IndigoConverter converter) throws Exception
   {
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      DataCell[] cells;
      int cIdx;
      
      cells = new DataCell[outSpec.getNumColumns()];
      
      for (cIdx = 0; cIdx < inputRow.getNumCells(); cIdx++)
         cells[cIdx] = inputRow.getCell(cIdx);
      
      DataCell targetCell = inputRow.getCell(colIdx);
      /*
       *  Mark all columns as missing
       */
      if (nodeSettings.appendColumn.getBooleanValue()) {
         cells[cIdx] = DataType.getMissingCell();
         cIdx++;
      }
      
      for (int j = 0; j < queries.size(); j++) {
         cells[cIdx] = DataType.getMissingCell();
         cIdx++;
      }
      cIdx = inputRow.getNumCells();
      
      if (targetCell.isMissing()) {
         return cells;
      }
      
      try {
         IndigoPlugin.lock();
         
         IndigoObject target = null;
         String errMes = null;
         DataCell newCell = null;
         IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
         
         try {
            
            // extract indigo representation
            newCell = converter.convert(cells[colIdx]);
            target = extractIndigoObject(newCell, indigoType);
            
            errMes = target.checkBadValence();
         } catch (IndigoException e) {
            errMes = e.getMessage();
         } 
         
         if(errMes != null && !"".equals(errMes)) {
            appendWarningMessage("Error while processing target structure with RowId = '" + inputRow.getKey() + "': " +  errMes);
            
            return cells;         }
         
         IndigoObject highlighted = target;
         
         indigo.setOption("embedding-uniqueness", Uniqueness.values()[nodeSettings.uniqueness
               .getIntValue()].name().toLowerCase());
         
         IndigoObject matcher = indigo.substructureMatcher(highlighted);
         
         for (IndigoObject q : queries) {
            if (q == null)
               continue;
            try {
               int matchCount = matcher.countMatches(q);

               cells[cIdx++] = new IntCell(matchCount);

               if (nodeSettings.highlight.getBooleanValue()) {
                  for (IndigoObject match : matcher.iterateMatches(q)) {
                     highlightStructures(match, q);
                  }
               }

            } catch (IndigoException e) {
               appendWarningMessage("Error while calling substructure counter: target key = '"
                     + inputRow.getKey() + "'");
            }
         }
         
         // add highlighted or replaced by highlighted 
         if (nodeSettings.highlight.getBooleanValue()) {
            DataType type = newCell.getType();
            DataCell cell = IndigoCellFactory.createCell(highlighted, null, type, indigoType);
            if (nodeSettings.appendColumn.getBooleanValue()) 
               cells[cells.length-1] = cell;
            else
               cells[colIdx] = cell;
         }
         
      } finally {
         IndigoPlugin.unlock();
      }
      return cells;
   }

   private void highlightStructures(IndigoObject match, IndigoObject query) {
      
      IndigoType stype = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      switch (stype) {
      case MOLECULE:
         highlightMolecule(match, query);
         break;
      case REACTION:
         for(IndigoObject mol : query.iterateMolecules()) 
            highlightMolecule(match, mol);
         break;
      default:
         // it's an impossible case
         break;
      }
   }
   
   private void highlightMolecule(IndigoObject match, IndigoObject query) {
      for (IndigoObject atom : query.iterateAtoms()) {
         IndigoObject mapped = match.mapAtom(atom);
         if (mapped != null)
            mapped.highlight();
      }
      for (IndigoObject bond : query.iterateBonds()) {
         IndigoObject mapped = match.mapBond(bond);
         if (mapped != null)
            mapped.highlight();
      }
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void reset ()
   {
   }

   protected DataTableSpec getOutDataTableSpec (DataTableSpec inSpec, final BufferedDataTable queriesTable, 
         IndigoConverter converter, int colIdx) throws InvalidSettingsException
   {
      int queryRowsCount = (int) queriesTable.size();
      if (nodeSettings.newColName.getStringValue() == null || nodeSettings.newColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("New column name must be specified");
      
      DataColumnSpec[] specs;

      int additionalRows = queryRowsCount;
      if (nodeSettings.appendColumn.getBooleanValue())
         additionalRows += 1;
      
      specs = new DataColumnSpec[inSpec.getNumColumns() + additionalRows];

      int i;

      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);

      if (nodeSettings.useNewColumnName.getBooleanValue())
      {
         for (int j = 0; j < queryRowsCount; j++)
         {
            String suffix = "";
            if (queryRowsCount > 1)
               suffix = String.format(" %d", j + 1);
            String value = nodeSettings.newColName.getStringValue() + suffix;
            
            specs[i] = new DataColumnSpecCreator(value, IntCell.TYPE).createSpec();
            i++;
         }
      }
      else
      {
         int queryColCounterNameIdx = 
               queriesTable.getSpec().findColumnIndex(nodeSettings.queryCounterColName.getStringValue());
         
         for (DataRow row : queriesTable) {
            specs[i] = 
                  new DataColumnSpecCreator(row.getCell(queryColCounterNameIdx).toString(), IntCell.TYPE).createSpec();
            i++;
         }
      }

      // Add molecule column that highlighted by each query   
      if (nodeSettings.appendColumn.getBooleanValue()) {
         specs[i] = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), 
               converter.getConvertedType()).createSpec();
         i++;
      }
      
      return new DataTableSpec(specs);
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected DataTableSpec[] configure (final DataTableSpec[] inSpecs)
         throws InvalidSettingsException
   {

      boolean reaction = IndigoType.REACTION.toString().equals(nodeSettings.inputType.getStringValue());
      
      IndigoType targetType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoType queryType = reaction ? IndigoType.QUERY_REACTION : IndigoType.QUERY_MOLECULE;
      
      searchIndigoCompatibleColumn(inSpecs[TARGET_PORT], nodeSettings.targetColName, targetType);
      searchIndigoCompatibleColumn(inSpecs[QUERY_PORT], nodeSettings.queryColName, queryType);
      searchIndigoColumn(inSpecs[QUERY_PORT], nodeSettings.queryCounterColName, StringValue.class);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      // Return null table because table specification depends on the 
      // number of the rows in the query table
      return new DataTableSpec[] {null};
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
      IndigoSubstructureMatchCounterSettings s = new IndigoSubstructureMatchCounterSettings();
      s.loadSettingsFrom(settings);
      if (s.targetColName.getStringValue() == null || s.targetColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
      if (s.queryColName.getStringValue() == null || s.queryColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("query column name must be specified");
      if (s.newColName.getStringValue() == null || s.newColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("new counter column name must be specified");
      if (s.appendColumn.getBooleanValue())
         if (s.appendColumnName.getStringValue() == null || s.appendColumnName.getStringValue().length() < 1)
            throw new InvalidSettingsException("new highlighted column name must be specified");
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
