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

package com.epam.indigo.knime.formulafilter;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.knime.core.data.*;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.data.def.IntCell;
import org.knime.core.data.def.StringCell;
import org.knime.core.node.*;

import com.epam.indigo.*;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoFormulaFilterNodeModel extends IndigoNodeModel
{
   public static final int TARGET_PORT = 0;
   public static final int QUERY_PORT = 1;
   
   IndigoFormulaFilterNodeSettings nodeSettings = new IndigoFormulaFilterNodeSettings();

   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoFormulaFilterNodeModel.class);
   
   /**
    * Constructor for the node model.
    */
   protected IndigoFormulaFilterNodeModel()
   {
      super(2, 3);
   }
   
   protected DataTableSpec getDataTableSpec (DataTableSpec inputTableSpec) 
         throws InvalidSettingsException
   {
      DataColumnSpec[] specs;
      
      boolean appendKeyCol = nodeSettings.appendQueryKeyColumn.getBooleanValue();
      boolean appendMatchCountCol = nodeSettings.appendQueryMatchCountKeyColumn.getBooleanValue();
      
      int columnsCount = inputTableSpec.getNumColumns();
      if (appendKeyCol) columnsCount++;
      if (appendMatchCountCol) columnsCount++;
      
      specs = new DataColumnSpec[columnsCount];

      int i;
      
      for (i = 0; i < inputTableSpec.getNumColumns(); i++)
         specs[i] = inputTableSpec.getColumnSpec(i);
      
      if (appendKeyCol)
         specs[i++] = new DataColumnSpecCreator(nodeSettings.queryKeyColumn.getStringValue(), StringCell.TYPE).createSpec();
      if (appendMatchCountCol)
         specs[i++] = new DataColumnSpecCreator(nodeSettings.queryMatchCountKeyColumn.getStringValue(), IntCell.TYPE).createSpec();
      
      return new DataTableSpec(specs);
   }
   

   private Map<String, Integer> grossFormulaToMap(String grossFormula) {
      HashMap<String, Integer> map = new HashMap<String, Integer>();
      
      String[] atomNumPairs = grossFormula.replaceAll("\\s", "").split("(?<=((\\d)|(\\p{L})))(?=\\p{Lu})");
      for (String atomNumPair : atomNumPairs) {
         String[] atomAndNum = atomNumPair.split("(?<=\\p{L})(?=\\d)");
         // take into account that both of the presentation (X and X1) in gross formula are equal
         map.put(atomAndNum[0], (atomAndNum.length > 1 ? Integer.parseInt(atomAndNum[1]) : 1));
      }
      return map;
   }
   
   private List<MolMap> loadQueries(BufferedDataTable queriesTableData)
         throws InvalidSettingsException
   {
      DataTableSpec queryItemsSpec = queriesTableData.getDataTableSpec();

      int queryColIdx = queryItemsSpec.
            findColumnIndex(nodeSettings.queryColName.getStringValue());
      
      List<MolMap> molMaps = new ArrayList<MolMap>();
      
      if (queriesTableData.size() == 0)
         LOGGER.warn("There are no query molecules in the table");

      int index = 0;
      boolean warningPrinted = false;
      
      for(DataRow row : queriesTableData) {
         
         molMaps.add(new MolMap(row, queryColIdx));
         
         if (molMaps.get(index).map == null && !warningPrinted) {
            LOGGER.warn("query table contains missing or invalid cells");
            warningPrinted = true;
         }
         index++;
      }
      return molMaps;
   }
   
   class MolMap
   {
      Map<String, Integer> map;
      String rowKey;
      
      public MolMap(DataRow row, int colIdx) {
         map = grossFormulaToMap(row.getCell(colIdx).toString());
         rowKey = row.getKey().toString();
      }
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      DataTableSpec inSpec = inData[TARGET_PORT].getDataTableSpec();
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      int colIdx = inSpec.findColumnIndex(nodeSettings.targetColName.getStringValue());
      
      // create converter to extract new column type
      IndigoConverter converter = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);
      
      DataTableSpec outSpec = getDataTableSpec(inSpec);
      
      BufferedDataContainer matchedOutputContainer = exec
            .createDataContainer(outSpec);
      
      BufferedDataContainer unmutchedOutputContainer = exec
            .createDataContainer(outSpec);

      BufferedDataContainer invalidInputContainer = exec
            .createDataContainer(inSpec);

      List<MolMap> queries = loadQueries(inData[QUERY_PORT]);
      
      int rowNumber = 1;
      boolean warningPrinted = false;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      for (DataRow inputRow : inData[TARGET_PORT]) {
         DataCell inputCell = inputRow.getCell(colIdx);
         
         boolean hasMatch = true;
         boolean invalid = false;
         int matchCount = 0;
         
         if(inputCell.isMissing()) {
            if(!warningPrinted)
               LOGGER.warn("target table contains missing cells");
            warningPrinted = true;
            invalid = true;
         }
         
         IndigoObject io = null;
         StringBuilder queriesRowKey = new StringBuilder();
         if (hasMatch && !invalid) {
            try {
               
               IndigoPlugin.lock();

               // take into account settins to process input
               indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
               indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
               
               DataCell newCell = converter.convert(inputCell);
               
               // extract indigo representation
               io = extractIndigoObject(newCell, indigoType);
               
            } catch (IndigoException e) {
               appendWarningMessage("Could not load target structure with RowId '" + inputRow.getKey() + "': " + e.getMessage());
               invalid = true;
            } finally {
               // turn options off
               indigo.setOption("ignore-stereochemistry-errors", false);
               indigo.setOption("treat-x-as-pseudoatom", false);
               
               IndigoPlugin.unlock();
            }
         }
         
         //Count matches
         if (hasMatch && !invalid) {
            matchCount = getMatchCount(io, queries, inputRow.getKey().getString(), queriesRowKey);
             //  Check matchCount
            if (nodeSettings.matchAnyAtLeastSelected.getBooleanValue())
               hasMatch = (matchCount >= nodeSettings.matchAnyAtLeast.getIntValue());
            if (nodeSettings.matchAllSelected.getBooleanValue())
               hasMatch = (matchCount == queries.size());
         }
         
         /*
          * Create output
          */
         if (!invalid) {
            int columnsCount = inputRow.getNumCells();
            if (nodeSettings.appendQueryKeyColumn.getBooleanValue())
               columnsCount++;
            if (nodeSettings.appendQueryMatchCountKeyColumn.getBooleanValue())
               columnsCount++;
            
            DataCell[] cells = new DataCell[columnsCount];
            int i;

            for (i = 0; i < inputRow.getNumCells(); i++) {
               cells[i] = inputRow.getCell(i);
            }
            if (nodeSettings.appendQueryKeyColumn.getBooleanValue())
               cells[i++] = new StringCell(queriesRowKey.toString());
            if (nodeSettings.appendQueryMatchCountKeyColumn.getBooleanValue())
               cells[i++] = new IntCell(matchCount);
            
            DataRow row = new DefaultRow(inputRow.getKey(), cells);
            
            if (hasMatch)
               matchedOutputContainer.addRowToTable(row);
            else
               unmutchedOutputContainer.addRowToTable(row);
         } else {
            invalidInputContainer.addRowToTable(inputRow);
         }
         if (io != null)
            io.dispose();
         
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) inData[TARGET_PORT].size(),
               "Processing row " + rowNumber);
         rowNumber++;
      }
      
      handleWarningMessages();
      matchedOutputContainer.close();
      unmutchedOutputContainer.close();
      invalidInputContainer.close();
      return new BufferedDataTable[] { matchedOutputContainer.getTable(),
            unmutchedOutputContainer.getTable(), invalidInputContainer.getTable() };
   }
   
   private int getMatchCount(IndigoObject target, List<MolMap> molMaps, String inputRowKey, StringBuilder queriesRowKey) {
      int matchCount = 0;
      
      Integer[] desiredRelations = {0, 0};
      
      if (nodeSettings.lessThanOrEqualComparison.getBooleanValue()) {
         desiredRelations[1] = -1;
      } else if (nodeSettings.greaterThanOrEqualComparison.getBooleanValue()) {
         desiredRelations[1] = 1;
      }
      
      for (MolMap molMap : molMaps) {
         if (molMap == null)
            continue;

         try {
            IndigoPlugin.lock();
            if (Arrays.asList(desiredRelations).contains(matchTarget(molMap, target))) {
               matchCount++;
               if (queriesRowKey.length() > 0)
                  queriesRowKey.append(", ");
               
               queriesRowKey.append(molMap.rowKey);
            }
         } catch (IndigoException e) {
            LOGGER.warn("indigo error while matching: target key='" + inputRowKey + "' query key='" + molMap.rowKey + "': " + e.getMessage());
         } finally {
            IndigoPlugin.unlock();
         }
      }
      return matchCount;
   }

 
   private Integer matchTarget(MolMap queryMap, IndigoObject target) {
      
      class ComparableMap {
         
         Map<String, Integer> map;
         
         public ComparableMap(Map<String, Integer> map) {
            this.map = map;
         }

         public Map<String, Integer> getMap() {
            return map;
         }
         
         boolean less(ComparableMap cm) {
            boolean res = true;
            for(String key: map.keySet()) {
               if (map.get(key) > cm.getMap().get(key)) res = false;
            }
            return res;
         }
         
         boolean more(ComparableMap cm) {
            boolean res = true;
            for(String key: map.keySet()) {
               if (map.get(key) < cm.getMap().get(key)) res = false;
            }
            return res;
         }
         
         boolean match(ComparableMap cm) {
            return map.equals(cm.getMap());
         }
         
      }
      
      ComparableMap targetCompareMap = new ComparableMap(grossFormulaToMap(target.grossFormula()));
      ComparableMap queryCompareMap = new ComparableMap(queryMap.map);
      
      Set<String> targetKeys = targetCompareMap.getMap().keySet(), queryKeys = queryMap.map.keySet();

      boolean targetContainsQuery = targetKeys.containsAll(queryKeys);
      boolean queryContainsTarget = queryKeys.containsAll(targetKeys);
      
      int result = Integer.MAX_VALUE;
      
      if (targetContainsQuery && queryContainsTarget) {
         if (targetCompareMap.match(queryCompareMap)) {
            result = 0;
         } else if (targetCompareMap.less(queryCompareMap)) {
            result = -1;
         } else if (targetCompareMap.more(queryCompareMap)) {
            result = 1;
         }
      } else if (targetContainsQuery) {
         if (queryCompareMap.less(targetCompareMap)) {
            result = 1;
         }
      } else if (queryContainsTarget) {
         if (targetCompareMap.less(queryCompareMap)) {
            result = -1;
         }
      }
      
      return result;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void reset ()
   {
      nodeSettings.queryColName.setStringValue(null);
      nodeSettings.targetColName.setStringValue(null);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected DataTableSpec[] configure (final DataTableSpec[] inSpecs)
         throws InvalidSettingsException
   {
      DataTableSpec inSpec = inSpecs[TARGET_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      searchIndigoCompatibleColumn(inSpec, nodeSettings.targetColName, indigoType);

      searchIndigoColumn(inSpecs[QUERY_PORT], nodeSettings.queryColName, StringValue.class);
      
      DataTableSpec outSpec = getDataTableSpec(inSpec);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return new DataTableSpec[] { outSpec, outSpec, inSpec };
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
      IndigoFormulaFilterNodeSettings s = new IndigoFormulaFilterNodeSettings();
      s.loadSettingsFrom(settings);

      if (s.targetColName.getStringValue() == null || s.targetColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
      if (s.appendQueryKeyColumn.getBooleanValue() && (s.queryKeyColumn.getStringValue() == null || s.queryKeyColumn.getStringValue().length() < 1))
         throw new InvalidSettingsException("query key column name must be specified");
      if (s.appendQueryMatchCountKeyColumn.getBooleanValue() && (s.queryMatchCountKeyColumn.getStringValue() == null || s.queryMatchCountKeyColumn.getStringValue().length() < 1))
         throw new InvalidSettingsException("query match count column name must be specified");
      if (!s.matchAllSelected.getBooleanValue() && !s.matchAnyAtLeastSelected.getBooleanValue())
         throw new InvalidSettingsException("At least one match option should be selected: match any or match all");
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
