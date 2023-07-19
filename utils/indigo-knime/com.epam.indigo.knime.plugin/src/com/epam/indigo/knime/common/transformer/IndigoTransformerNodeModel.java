package com.epam.indigo.knime.common.transformer;

import java.io.File;
import java.io.IOException;

import org.knime.base.data.replace.ReplacedColumnsDataRow;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.container.ColumnRearranger;
import org.knime.core.data.container.SingleCellFactory;
import org.knime.core.node.BufferedDataTable;
import org.knime.core.node.CanceledExecutionException;
import org.knime.core.node.ExecutionContext;
import org.knime.core.node.ExecutionMonitor;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.cell.IndigoCellFactory;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoTransformerNodeModel extends IndigoNodeModel {

   private final IndigoTransformerSettings nodeSettings;
   IndigoTransformerBase transformer;
   String message;
   
   public IndigoTransformerNodeModel(String message, IndigoTransformerSettings settings, IndigoTransformerBase transformer) {
      super(1, 1);
      this.message = message;
      this.transformer = transformer;
      this.nodeSettings = settings;
   }
   
   public IndigoTransformerNodeModel(String message, IndigoTransformerBase transformer) {
      this(message, new IndigoTransformerSettings(), transformer);
   }
   
   @Override
   protected BufferedDataTable[] execute(BufferedDataTable[] inData,
         ExecutionContext exec) throws Exception {
      
      BufferedDataTable bufferedDataTable = inData[IndigoTransformerSettings.INPUT_PORT];
      
      ColumnRearranger crea = createRearranger(bufferedDataTable.getDataTableSpec());

      BufferedDataTable rearrangeTable = exec.createColumnRearrangeTable(
            bufferedDataTable, crea, exec);
      
      handleWarningMessages();
      
      return new BufferedDataTable[] { rearrangeTable };
   }
   
   /**
    * Method creates ColumnRearranger in according to upcoming operations under data table
    * @param inSpec Input DataTableSpec
    * @return ColumnRearranger with appended or replaced new column
    */
   private ColumnRearranger createRearranger(DataTableSpec inSpec) {
      
      // create default rearranger
      ColumnRearranger crea = new ColumnRearranger(inSpec);
      
      boolean append = nodeSettings.appendColumn.getBooleanValue();
      int colIdx = inSpec.findColumnIndex(nodeSettings.colName.getStringValue());
      
      // create column spec
      String name;
      if (append)
         name = DataTableSpec.getUniqueColumnName(inSpec, 
                  nodeSettings.newColName.getStringValue());
      else 
         name = nodeSettings.colName.getStringValue();
            
      DataType colType = inSpec.getColumnSpec(colIdx).getType();
      
      DataColumnSpec cs = new DataColumnSpecCreator(name, colType).createSpec();
      
      // update rearranger with new column spec using factory
      TransformerCellFactory fac = new TransformerCellFactory(inSpec, cs, nodeSettings, colIdx);
      
      if (append)
         crea.append(fac);
      else
         crea.replace(fac, colIdx);
      
      return crea;
   }

   @Override
   protected DataTableSpec[] configure(DataTableSpec[] inSpecs)
         throws InvalidSettingsException {

      DataTableSpec inSpec = inSpecs[IndigoTransformerSettings.INPUT_PORT];
      IndigoType type = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // find first appropriate column and set its name to colName setting
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, type);
      
      if (nodeSettings.newColName.getStringValue() == null ||
    		  "".equals(nodeSettings.newColName.getStringValue()))
         nodeSettings.newColName.setStringValue(nodeSettings.colName.getStringValue() + "(" + message + ")");
      
      if(nodeSettings.warningMessage != null) setWarningMessage(nodeSettings.warningMessage);
      
      return new DataTableSpec[] { createRearranger(inSpec).createSpec() };
   }
   
   
   @Override
   protected void loadInternals(File nodeInternDir, ExecutionMonitor exec)
         throws IOException, CanceledExecutionException {
   }

   @Override
   protected void saveInternals(File nodeInternDir, ExecutionMonitor exec)
         throws IOException, CanceledExecutionException {
   }

   @Override
   protected void saveSettingsTo(NodeSettingsWO settings) {
      nodeSettings.saveSettingsTo(settings);
   }

   @Override
   protected void validateSettings(NodeSettingsRO settings)
         throws InvalidSettingsException {
      IndigoTransformerSettings s = new IndigoTransformerSettings();
      s.loadSettingsFrom(settings);
      if (s.colName.getStringValue() == null || s.colName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No column name given");
      
      if (s.appendColumn.getBooleanValue() && ((s.newColName.getStringValue() == null) || 
            (s.newColName.getStringValue().length() < 1)))
         throw new InvalidSettingsException("No name for new column given");
   }

   @Override
   protected void loadValidatedSettingsFrom(NodeSettingsRO settings)
         throws InvalidSettingsException {
      nodeSettings.loadSettingsFrom(settings);
   }

   @Override
   protected void reset() {
   }

   
   /**
    * Cell factory for coolumn rearranger
    */
   class TransformerCellFactory extends SingleCellFactory {
      
      private int colIdx;
      private IndigoTransformerSettings settings;
      private DataTableSpec inSpec;
      
      public TransformerCellFactory(final DataTableSpec inSpec, DataColumnSpec newColSpec, IndigoTransformerSettings settings, int columnIndex) {
         super(newColSpec);
         this.inSpec = inSpec;
         this.settings = settings;
         this.colIdx = columnIndex;
         
         transformer.initialize(inSpec);
      }

      @Override
      public DataCell getCell(DataRow row) {
         DataCell inputCell = row.getCell(colIdx);
         if (inputCell.isMissing())
            return inputCell;
         else
         {
            // indigo type set in dialog settings
            IndigoType indigoType = IndigoType.findByString(settings.inputType.getStringValue());
            // cell's type
            DataType cellType = inputCell.getType();

            IndigoObject io;
            Indigo indigo = IndigoPlugin.getIndigo();

            try {
               IndigoPlugin.lock();

               // take into account settins to process input
               indigo.setOption("ignore-stereochemistry-errors", settings.ignoreStereochemistryErrors.getBooleanValue());
               indigo.setOption("treat-x-as-pseudoatom", settings.treatXAsPseudoatom.getBooleanValue());
               
               // create cell's converter 
               IndigoConverter converter = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);
               
               // create cell containing both original and indigo representations
               DataCell newCell = converter.convert(inputCell);
              
               // replace old cell representation in the row by the new one
               ReplacedColumnsDataRow newRow = new ReplacedColumnsDataRow(row, newCell, colIdx);
               
               // extract indigo representation
               io = extractIndigoObject(newCell, indigoType);
               
               boolean reaction = IndigoType.REACTION.equals(indigoType) || 
                     IndigoType.QUERY_REACTION.equals(indigoType);
               
               // perform desired transformations
               transformer.transformWithRow(io, reaction, newRow);
               
               // create and return adapter cell with with both original (but changed) and indigo representations
               // source ('normal') representation must be provided for successor vendor nodes in workflow
               DataCell resultCell = DataType.getMissingCell();
               
               resultCell = IndigoCellFactory.createCell(io, null, cellType, indigoType);
               
               return resultCell;
               
            } catch (IndigoException ie) {
               appendWarningMessage("Could not process structure with RowId='" + 
                     row.getKey() + "': " + ie.getMessage());
               return DataType.getMissingCell();
            } finally {
               
               // turn options off
               indigo.setOption("ignore-stereochemistry-errors", false);
               indigo.setOption("treat-x-as-pseudoatom", false);
               
               IndigoPlugin.unlock();
            }
         }
      }
   }
}
