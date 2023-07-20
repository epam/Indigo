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

package com.epam.indigo.knime.submatcher;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.knime.core.data.*;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.data.def.IntCell;
import org.knime.core.data.def.StringCell;
import org.knime.core.node.*;

import com.epam.indigo.*;
import com.epam.indigo.knime.cell.IndigoCellFactory;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;
import com.epam.indigo.knime.submatcher.IndigoSubstructureMatcherSettings.MoleculeMode;
import com.epam.indigo.knime.submatcher.IndigoSubstructureMatcherSettings.ReactionMode;

public class IndigoSubstructureMatcherNodeModel extends IndigoNodeModel
{
   public static final int TARGET_PORT = 0;
   public static final int QUERY_PORT = 1;
   
   IndigoSubstructureMatcherSettings nodeSettings = new IndigoSubstructureMatcherSettings();

   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoSubstructureMatcherNodeModel.class);
   
   /**
    * Constructor for the node model.
    */
   protected IndigoSubstructureMatcherNodeModel()
   {
      super(2, 2);
   }
   
   protected DataTableSpec getDataTableSpec (DataTableSpec inSpec, IndigoConverter converter, int colIdx) 
         throws InvalidSettingsException {
      
      if (nodeSettings.appendColumn.getBooleanValue())
         if (nodeSettings.newColName.getStringValue().length() < 1)
            throw new InvalidSettingsException("New column name must be specified");
      
      DataColumnSpec[] specs;
      
      boolean appendCol = nodeSettings.appendColumn.getBooleanValue();
      boolean appendKeyCol = nodeSettings.appendQueryKeyColumn.getBooleanValue();
      boolean appendMatchCountCol = nodeSettings.appendQueryMatchCountKeyColumn.getBooleanValue();
      
      int columnsCount = inSpec.getNumColumns();
      
      if (appendCol) columnsCount++;
      if (appendKeyCol) columnsCount++;
      if (appendMatchCountCol) columnsCount++;
      
      specs = new DataColumnSpec[columnsCount];
      
      int i;
      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);
      
      if (appendCol) 
         specs[i++] = 
               new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), converter.getConvertedType()).createSpec();
      if (appendKeyCol)
         specs[i++] = new DataColumnSpecCreator(nodeSettings.queryKeyColumn.getStringValue(), StringCell.TYPE).createSpec();
      if (appendMatchCountCol)
         specs[i++] = new DataColumnSpecCreator(nodeSettings.queryMatchCountKeyColumn.getStringValue(), IntCell.TYPE).createSpec();
      
      return new DataTableSpec(specs);
   }
   
   private List<QueryWithData> loadQueries(BufferedDataTable queriesTableData)
         throws Exception {
      DataTableSpec queryItemsSpec = queriesTableData.getDataTableSpec();

      int queryColIdx = queryItemsSpec.findColumnIndex(nodeSettings.queryColName.getStringValue());
      
      List<QueryWithData> queries = new ArrayList<QueryWithData>();
      
      if (queriesTableData.size() == 0)
         LOGGER.warn("There are no query molecules in the table");

      int index = 0;
      boolean warningPrinted = false;
      boolean reaction = IndigoType.REACTION.toString().equals(nodeSettings.inputType.getStringValue());
      IndigoType queryType = reaction ? IndigoType.QUERY_REACTION : IndigoType.QUERY_MOLECULE;
      IndigoConverter converter = IndigoConverterFactory.createConverter(queryItemsSpec, queryColIdx, queryType,
            nodeSettings.treatStringAsSMARTS.getBooleanValue());
      
      for(DataRow row : queriesTableData) {
         
         queries.add(new QueryWithData(row, queryColIdx, reaction, converter));
         
         if (queries.get(index).query == null && !warningPrinted) {
            LOGGER.warn("query table contains missing or invalid cells");
            warningPrinted = true;
         }
         index++;
      }
      return queries;
   }

   class AlignTargetQueryData
   {
      final IndigoObject query;
      boolean first = true;
      int natomsAlign = 0;
      int[] atoms = null;
      float[] xyz = null;
      
      public AlignTargetQueryData(IndigoObject mol) {
         query = mol;
      }

      public void align (IndigoObject target, IndigoObject match)
      {
         int i = 0;
         
         if (!target.hasCoord())
            target.layout();

         if (first)
         {
            for (IndigoObject atom : query.iterateAtoms())
            {
               IndigoObject mapped = match.mapAtom(atom);
               if (mapped != null && (mapped.isPseudoatom() || mapped.atomicNumber() != 1))
                  natomsAlign++;
            }
            if (natomsAlign > 1)
            {
               atoms = new int[natomsAlign];
               xyz = new float[natomsAlign * 3];
               
               for (IndigoObject atom : query.iterateAtoms())
               {
                  IndigoObject mapped = match.mapAtom(atom);
                  
                  IndigoObject atomForAlign;
                  if (nodeSettings.alignByQuery.getBooleanValue())
                     atomForAlign = atom;
                  else 
                     atomForAlign = mapped;
                     
                  if (mapped != null && (mapped.isPseudoatom() || mapped.atomicNumber() != 1))
                  {
                     atoms[i] = mapped.index();
                     System.arraycopy(atomForAlign.xyz(), 0, xyz, i++ * 3, 3);
                  }
               }
               if (nodeSettings.alignByQuery.getBooleanValue())
                  target.alignAtoms(atoms, xyz);
            }
            first = false;
         }
         else
         {
            if (atoms != null)
            {
               for (IndigoObject atom : query.iterateAtoms())
               {
                  IndigoObject mapped = match.mapAtom(atom);
                  if (mapped != null && (mapped.isPseudoatom() || mapped.atomicNumber() != 1))
                     atoms[i++] = mapped.index();
               }

               target.alignAtoms(atoms, xyz);
            }
         }
      }
   }
   
   class QueryWithData
   {
      IndigoObject query;
      String rowKey;
      final ArrayList<AlignTargetQueryData> alignData = new ArrayList<AlignTargetQueryData>();
      private final boolean reaction; 
      
      public QueryWithData(DataRow row, int colIdx, boolean reaction, IndigoConverter converter) 
            throws Exception {
         this.reaction = reaction;
         this.query = getIndigoQueryStructureOrNull(row.getCell(colIdx), converter);
         this.rowKey = row.getKey().toString();
         /*
          * Prepare align data
          */
         if (query != null) {
            if (this.reaction) {
               for(IndigoObject mol : query.iterateMolecules())
                  alignData.add(new AlignTargetQueryData(mol));
            } else {
               alignData.add(new AlignTargetQueryData(query));
            }
         }
      }
      
      public void align (IndigoObject target, IndigoObject match) {
         for (AlignTargetQueryData qdata : alignData) {
            if (this.reaction)
               target = match.mapMolecule(qdata.query);
            qdata.align(target, match);
         }
      }
      
      private IndigoObject getIndigoQueryStructureOrNull(DataCell cell, IndigoConverter converter) 
            throws Exception {
         
         IndigoObject io = null;
         
         try {
            IndigoPlugin.lock();
            DataCell newCell = converter.convert(cell);    

            // extract indigo representation
            IndigoType queryType = this.reaction ? IndigoType.QUERY_REACTION : IndigoType.QUERY_MOLECULE;
            io = extractIndigoObject(newCell, queryType, nodeSettings.treatStringAsSMARTS.getBooleanValue());
            
         } catch(IndigoException e) {
            LOGGER.warn("error while loading query structure: " + e.getMessage());
         } finally {
            IndigoPlugin.unlock();
         }
         
         return io;
      }
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      DataTableSpec targetSpec = inData[TARGET_PORT].getDataTableSpec();
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      int colIdx = targetSpec.findColumnIndex(nodeSettings.targetColName.getStringValue());
      
      // create converter to extract new column type
      IndigoConverter converter = IndigoConverterFactory.
            createConverter(targetSpec, colIdx, indigoType);
      
      DataTableSpec outSpec = getDataTableSpec(targetSpec, converter, colIdx);
      
      BufferedDataContainer matchedOutputContainer = exec.createDataContainer(outSpec);
      
      BufferedDataContainer unmatchedOutputContainer = exec.createDataContainer(outSpec);

      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      List<QueryWithData> queries = loadQueries(inData[QUERY_PORT]);
      
      int rowNumber = 1;
      boolean warningPrinted = false;
      
      for (DataRow inputRow : inData[TARGET_PORT]) {
         DataCell inputCell = inputRow.getCell(colIdx);
         
         boolean hasMatch = true;
         int matchCount = 0;
         
         if(inputCell.isMissing()) {
            if(!warningPrinted)
               LOGGER.warn("target table contains missing cells");
            warningPrinted = true;
            hasMatch = false;
         }
         
         IndigoObject target = null;
         StringBuilder queriesRowKey = new StringBuilder();
         DataCell newCell = null;
         
         if (hasMatch) {
            try {
               IndigoPlugin.lock();
               
               // convert original cell
               newCell = converter.convert(inputCell);
               
               // extract indigo representation
               target = extractIndigoObject(newCell, indigoType);
               
            } catch (IndigoException e) {
               appendWarningMessage("Could not load target structure with RowId '" + inputRow.getKey() + "': " + e.getMessage());
               hasMatch = false;
            } finally {
               
               IndigoPlugin.unlock();
            }
         }

         /*
          * Count matches
          */
         if (hasMatch) {
            
            matchCount = getMatchCount(target, queries, inputRow.getKey().getString(), queriesRowKey);

            /*
             *  Check matchCount
             */
            if (nodeSettings.matchAnyAtLeastSelected.getBooleanValue())
               hasMatch = (matchCount >= nodeSettings.matchAnyAtLeast.getIntValue());
            if (nodeSettings.matchAllSelected.getBooleanValue())
               hasMatch = (matchCount == queries.size());
         }
         
         /*
          * Create output
          */
         int columnsCount = inputRow.getNumCells();
         
         if (nodeSettings.appendColumn.getBooleanValue())
            columnsCount++;
         if (nodeSettings.appendQueryKeyColumn.getBooleanValue())
            columnsCount++;
         if (nodeSettings.appendQueryMatchCountKeyColumn.getBooleanValue())
            columnsCount++;
         
         DataCell[] cells = new DataCell[columnsCount];
         int i;
         
         for (i = 0; i < inputRow.getNumCells(); i++) {
            cells[i] = inputRow.getCell(i);
         }
         
         if (nodeSettings.appendColumn.getBooleanValue()){
            
             // Create adapter cell with indigo representation inside
            DataCell cell = DataType.getMissingCell();

            try {
               DataType type = newCell.getType();
               cell = IndigoCellFactory.createCell(target, null, type, indigoType);
            } catch (IndigoException e) {
               LOGGER.warn("Could not create cell from Indigo object at RowId '" + inputRow.getKey() + "': " + e.getMessage());
            }
            
            cells[i++] = cell;
         }
         if (nodeSettings.appendQueryKeyColumn.getBooleanValue())
            cells[i++] = new StringCell(queriesRowKey.toString());
         if (nodeSettings.appendQueryMatchCountKeyColumn.getBooleanValue())
            cells[i++] = new IntCell(matchCount);
         
         DataRow newRow = new DefaultRow(inputRow.getKey(), cells);
         
         if (hasMatch)
            matchedOutputContainer.addRowToTable(newRow);
         else
            unmatchedOutputContainer.addRowToTable(newRow);
            
               
         
         if (target != null)
            target.dispose();
         
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) inData[TARGET_PORT].size(),
               "Processing row " + rowNumber);
         rowNumber++;
      }
      
      // turn options off
      indigo.setOption("ignore-stereochemistry-errors", false);
      indigo.setOption("treat-x-as-pseudoatom", false);
      
      handleWarningMessages();
      matchedOutputContainer.close();
      unmatchedOutputContainer.close();
      return new BufferedDataTable[] { matchedOutputContainer.getTable(),
            unmatchedOutputContainer.getTable()};
   }

   private int getMatchCount(IndigoObject target, List<QueryWithData> queries, String inputRowKey, StringBuilder queriesRowKey) {
      int matchCount = 0;
      
      for (QueryWithData query : queries) {
         if (query.query == null)
            continue;

         try {
            IndigoPlugin.lock();
            if (matchTarget(query, target)) {
               matchCount++;
               if (queriesRowKey.length() > 0)
                  queriesRowKey.append(", ");
               
               queriesRowKey.append(query.rowKey);
            }
         } catch (IndigoException e) {
            LOGGER.warn("indigo error while matching: target key='" + inputRowKey + "' query key='" + query.rowKey + "': " + e.getMessage());
         } finally {
            IndigoPlugin.unlock();
         }
      }
      return matchCount;
   }

   private boolean matchTarget(QueryWithData queryData, IndigoObject target) {
      
      IndigoType stype = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      switch (stype) {
      case MOLECULE:
         return matchMoleculeTarget(queryData, target);
      case REACTION:
         return matchReactionTarget(queryData, target);
      default:
         // it's an impossible case
         break;
      }
      return false;
   }

   private boolean matchReactionTarget(QueryWithData queryData, IndigoObject target) {

      IndigoObject match = null;
      String mode = "";
      
      IndigoObject query = queryData.query;
      Indigo indigo = IndigoPlugin.getIndigo();
      
      if(ReactionMode.DaylightAAM.toString().equals(nodeSettings.mode.getStringValue()))
         mode = "Daylight-AAM";
      
      match = indigo.substructureMatcher(target, mode).match(query);
      
      if (match != null && nodeSettings.exact.getBooleanValue()) {
         // test that the target does not have unmapped heavy atoms
         int nmappedHeavy = 0;
         int targetHeavy = 0;
         
         for (IndigoObject mol : query.iterateMolecules()) {
            for (IndigoObject atom : mol.iterateAtoms()) {
               IndigoObject mapped = match.mapAtom(atom);
               if (mapped != null)
                  if (mapped.isRSite() || mapped.isPseudoatom() || mapped.atomicNumber() > 1)
                     nmappedHeavy++;
            }
         }

         for (IndigoObject mol : target.iterateMolecules()) 
            targetHeavy += mol.countHeavyAtoms();
         
         if (nmappedHeavy < targetHeavy)
            match = null;
      }
      if (match != null) {
         if (nodeSettings.highlight.getBooleanValue()) {
            for (IndigoObject mol : query.iterateMolecules()) {
               for (IndigoObject atom : mol.iterateAtoms()) {
                  IndigoObject mapped = match.mapAtom(atom);
                  if (mapped != null)
                     mapped.highlight();
               }
               for (IndigoObject bond : mol.iterateBonds()) {
                  IndigoObject mapped = match.mapBond(bond);
                  if (mapped != null)
                     mapped.highlight();
               }
            }
         }
         if (nodeSettings.align.getBooleanValue())
            queryData.align(target, match);
      }
      
      
      return (match != null);
   }

   private boolean matchMoleculeTarget(QueryWithData queryData, IndigoObject target)
   {
      Indigo indigo = IndigoPlugin.getIndigo();
      IndigoObject query = queryData.query;
      
      String mode = "";
      
      IndigoObject match = null, matcher = null;
      
      if (!nodeSettings.exact.getBooleanValue() || target.countHeavyAtoms() <= query.countAtoms())
      {
         if (MoleculeMode.Resonance.toString().equals(nodeSettings.mode.getStringValue()))
            mode = "RES";
         else if (MoleculeMode.Tautomer.toString().equals(nodeSettings.mode.getStringValue()))
         {
            mode = "TAU R* R-C";
 
            indigo.clearTautomerRules();
            indigo.setTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te");
            indigo.setTautomerRule(2, "0C", "N,O,P,S");
            indigo.setTautomerRule(3, "1C", "N,O");
         }
      
         matcher = indigo.substructureMatcher(target, mode); 
         match = matcher.match(query);
      }
      
      if (match != null && nodeSettings.exact.getBooleanValue())
      {
         // test that the target does not have unmapped heavy atoms
         int nmappedHeavy = 0;
         
         for (IndigoObject atom : query.iterateAtoms())
         {
            IndigoObject mapped = match.mapAtom(atom);
            if (mapped != null)
               if (mapped.isRSite() || mapped.isPseudoatom() || mapped.atomicNumber() > 1)
                  nmappedHeavy++;
         }
         
         if (nmappedHeavy < target.countHeavyAtoms())
            match = null;
      }
      
      if (match != null)
      {
         if (nodeSettings.highlight.getBooleanValue())
         {
            for (IndigoObject atom : query.iterateAtoms())
            {
               IndigoObject mapped = match.mapAtom(atom);
               if (mapped != null)
                  mapped.highlight();
            }
            for (IndigoObject bond : query.iterateBonds())
            {
               IndigoObject mapped = match.mapBond(bond);
               if (mapped != null)
                  mapped.highlight();
            }
         }
         
         if (nodeSettings.align.getBooleanValue())
            queryData.align(target, match);
      }
      if (match != null)
         match.dispose();
      if (matcher != null)
         matcher.dispose();
      return (match != null);
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
      DataTableSpec targetSpec = inSpecs[TARGET_PORT];
      DataTableSpec querySpec = inSpecs[QUERY_PORT];
      
      boolean reaction = IndigoType.REACTION.toString().equals(nodeSettings.inputType.getStringValue());
      
      IndigoType targetType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoType queryType = reaction ? IndigoType.QUERY_REACTION : IndigoType.QUERY_MOLECULE;
      
      searchIndigoCompatibleColumn(targetSpec, nodeSettings.targetColName, targetType);
      searchIndigoCompatibleColumn(querySpec, nodeSettings.queryColName, queryType);
      
      int colIdx = targetSpec.findColumnIndex(nodeSettings.targetColName.getStringValue());
      
      IndigoConverter converter = IndigoConverterFactory.createConverter(targetSpec, colIdx, targetType);
      
      DataTableSpec outSpec = getDataTableSpec(targetSpec, converter, colIdx);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return new DataTableSpec[] { outSpec, outSpec };
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
      IndigoSubstructureMatcherSettings s = new IndigoSubstructureMatcherSettings();
      s.loadSettingsFrom(settings);

      if (s.targetColName.getStringValue() == null || s.targetColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
      if (s.queryColName.getStringValue() == null || s.queryColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("query column name must be specified");
      if (s.appendColumn.getBooleanValue() && (s.newColName.getStringValue() == null || s.newColName.getStringValue().length() < 1))
         throw new InvalidSettingsException("new column name must be specified");
      if (s.appendQueryKeyColumn.getBooleanValue() && (s.queryKeyColumn.getStringValue() == null || s.queryKeyColumn.getStringValue().length() < 1))
         throw new InvalidSettingsException("query key column name must be specified");
      if (s.appendQueryMatchCountKeyColumn.getBooleanValue() && (s.queryMatchCountKeyColumn.getStringValue() == null || s.queryMatchCountKeyColumn.getStringValue().length() < 1))
         throw new InvalidSettingsException("query match count column name must be specified");
      if (!s.matchAllSelected.getBooleanValue() && !s.matchAnyAtLeastSelected.getBooleanValue())
         throw new InvalidSettingsException("At least one match option should be selected: match any or match all");
      if (s.appendColumn.getBooleanValue() && !s.highlight.getBooleanValue() && !s.align.getBooleanValue())
         throw new InvalidSettingsException("without highlighting or alignment, appending new column makes no sense");
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
