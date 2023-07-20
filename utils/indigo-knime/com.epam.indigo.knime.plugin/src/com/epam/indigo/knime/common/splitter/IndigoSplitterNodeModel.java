package com.epam.indigo.knime.common.splitter;

import java.io.File;
import java.io.IOException;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
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

public class IndigoSplitterNodeModel extends IndigoNodeModel {

   private static final NodeLogger LOGGER = NodeLogger.getLogger(IndigoSplitterNodeModel.class);
   
   private IndigoSplitterSettings nodeSettings;
   private IndigoSplitter splitter;
   
   public IndigoSplitterNodeModel(IndigoSplitterSettings settings, IndigoSplitter splitter) {
      super(1, 3);
      this.nodeSettings = settings;
      this.splitter = splitter;
   }

   @Override
   protected void loadInternals(File nodeInternDir, ExecutionMonitor exec)
         throws IOException, CanceledExecutionException {
      // TODO Auto-generated method stub
      
   }

   @Override
   protected void saveInternals(File nodeInternDir, ExecutionMonitor exec)
         throws IOException, CanceledExecutionException {
      // TODO Auto-generated method stub
      
   }

   @Override
   protected void saveSettingsTo(NodeSettingsWO settings) {
      nodeSettings.saveSettingsTo(settings);      
   }

   @Override
   protected void validateSettings(NodeSettingsRO settings)
         throws InvalidSettingsException {
      IndigoSplitterSettings s = new IndigoSplitterSettings();
      s.loadSettingsFrom(settings);
      if (s.colName.getStringValue() == null || s.colName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No column name given");      
   }

   @Override
   protected void loadValidatedSettingsFrom(NodeSettingsRO settings)
         throws InvalidSettingsException {
      nodeSettings.loadSettingsFrom(settings);      
   }

   @Override
   protected void reset() {
   }

   @Override
   protected DataTableSpec[] configure(DataTableSpec[] inSpecs)
         throws InvalidSettingsException {
      
      DataTableSpec inSpec = inSpecs[IndigoSplitterSettings.INPUT_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // find first appropriate column and set its name to colName setting
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, indigoType);

      if(nodeSettings.warningMessage != null) setWarningMessage(nodeSettings.warningMessage);
      
      // the last output is equal to input as it's supposed to put there all the rows with wrong data cells
      return new DataTableSpec[]{inSpec, inSpec, inSpec};
   }
   
   @Override
   protected BufferedDataTable[] execute(BufferedDataTable[] inData,
         ExecutionContext exec) throws Exception {

      BufferedDataTable inDataSpec = inData[IndigoSplitterSettings.INPUT_PORT];
      DataTableSpec inputTableSpec = inDataSpec.getDataTableSpec();
      int colIdx = inputTableSpec.findColumnIndex(nodeSettings.colName.getStringValue());
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // create cell's converter to get indigo representation
      IndigoConverter converter = IndigoConverterFactory.createConverter(inputTableSpec, colIdx, indigoType);
      
      BufferedDataContainer matchOutputCon = exec.createDataContainer(inputTableSpec); // matching rows
      BufferedDataContainer unmatchOutputCon = exec.createDataContainer(inputTableSpec); // not matching rows
      BufferedDataContainer invalidOutputCon = exec.createDataContainer(inputTableSpec); // wrong data
      
      int rowNumber = 1;
      long tableSize = inDataSpec.size();
      BufferedDataContainer[] cons = new BufferedDataContainer[] {matchOutputCon, unmatchOutputCon};
      
      Indigo indigo = IndigoPlugin.getIndigo();
      for (DataRow inputRow : inDataSpec) {
         
         
         try {
            
            IndigoPlugin.lock();

            // take into account settins to process input
            indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
            indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());

            
            // add indigo representation into cell
            DataCell cell = inputRow.getCell(colIdx);
            
            if (cell.isMissing()) 
               throw new IndigoException(cell, "cell is missing");

            // extract indigo representation
            DataCell newCell = converter.convert(cell);
            IndigoObject io = extractIndigoObject(newCell, indigoType);
            
            // append row to an appropriate container
            splitter.distribute(cons, inputRow, io);
            
         } catch (IndigoException ex) {
            // add warning message
            LOGGER.warn("Could not load target structure with RowId '"
                  + inputRow.getKey() + "': " + ex.getMessage());
            // put the row into container for bad input
            invalidOutputCon.addRowToTable(inputRow);
         } finally {
            
            // turn options off
            indigo.setOption("ignore-stereochemistry-errors", false);
            indigo.setOption("treat-x-as-pseudoatom", false);
            
            IndigoPlugin.unlock();
         }
         
         exec.checkCanceled();
         exec.setProgress(
               rowNumber / (double) tableSize,
               "Processing row " + rowNumber);
         rowNumber++;
         
      }

      // handle warning messages
      handleWarningMessages();
      // close containers
      matchOutputCon.close();
      unmatchOutputCon.close();
      invalidOutputCon.close();
      
      return new BufferedDataTable[] {matchOutputCon.getTable(), 
            unmatchOutputCon.getTable(), invalidOutputCon.getTable()};
   }
   
}
