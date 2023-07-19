package com.epam.indigo.knime.inchi;

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
import org.knime.core.data.def.StringCell.StringCellFactory;
import org.knime.core.node.BufferedDataContainer;
import org.knime.core.node.BufferedDataTable;
import org.knime.core.node.CanceledExecutionException;
import org.knime.core.node.ExecutionContext;
import org.knime.core.node.ExecutionMonitor;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoInchi;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoInchiNodeModel extends IndigoNodeModel {

   private final IndigoInchiNodeSettings nodeSettings = 
         new IndigoInchiNodeSettings();
   
   public IndigoInchiNodeModel() {
      super(1, 1);
   }

   @Override
   protected BufferedDataTable[] execute(BufferedDataTable[] inData,
         ExecutionContext exec) throws Exception {

      BufferedDataTable bufferedDataTable = inData[IndigoInchiNodeSettings.INPUT_PORT];
      
      DataTableSpec inSpec = bufferedDataTable.getDataTableSpec();
      DataTableSpec spec = getDataTableSpec(inSpec);
      
      // output container
      BufferedDataContainer outputContainer = exec.createDataContainer(spec);
      
      // target column index
      int colIdx = spec.findColumnIndex(nodeSettings.colName.getStringValue());

      if (colIdx == -1)
         throw new Exception("column not found");

      boolean appendInchiKey = nodeSettings.appendInchiKeyColumn.getBooleanValue();
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // create converter to get column with both original and indigo representation
      IndigoConverter converter = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);

      int rowNumber = 1;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      IndigoInchi inchi = new IndigoInchi(indigo);
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      for (DataRow inputRow : bufferedDataTable) {
         
         RowKey key = inputRow.getKey();
         DataCell[] cells = new DataCell[spec.getNumColumns()];
         
         int idx;
         
         for (idx = 0; idx < inputRow.getNumCells(); idx++)
            cells[idx] = inputRow.getCell(idx);
         
         // by default result appended cells are missing one
         cells[idx] = DataType.getMissingCell();
         if (appendInchiKey) {
            cells[idx + 1] = DataType.getMissingCell();
         }
         
         if (!cells[colIdx].isMissing()) {
            try {
               IndigoPlugin.lock();
               
               // extract indigo representation
               DataCell newCell = converter.convert(inputRow.getCell(colIdx));
               IndigoObject io = extractIndigoObject(newCell, indigoType);
               
               // try to extract InChi and InChiKey values from indigo object
               String inchiString = inchi.getInchi(io);
               
               cells[idx] = StringCellFactory.create(inchiString);
               if (appendInchiKey) {
                  cells[idx + 1] = StringCellFactory.create(inchi.getInchiKey(inchiString));
               }
               
            }catch (IndigoException e) {
               appendWarningMessage("Error while translating to Indigo or extracting InChi//InChhiKey value "
                     + "for RowId = '" + inputRow.getKey()+ "': " + e.getMessage());
            } finally {
               IndigoPlugin.unlock();
            }
         } else {
            appendWarningMessage("Target cell at row with key " + inputRow.getKey() + " is missing");
         }
         
         // add new row
         outputContainer.addRowToTable(new DefaultRow(key, cells));
         
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) bufferedDataTable.size(),
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
   
   @Override
   protected DataTableSpec[] configure(DataTableSpec[] inSpecs)
         throws InvalidSettingsException {

      DataTableSpec inSpec = inSpecs[IndigoInchiNodeSettings.INPUT_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, indigoType);
      
      // set inchi column name
      if (nodeSettings.inchiColName.getStringValue() == null || 
            "".equals(nodeSettings.inchiColName.getStringValue())) {
         String inchiCol = DataTableSpec.getUniqueColumnName(inSpec, nodeSettings.colName.getStringValue() 
               + " (InChi)");
         nodeSettings.inchiColName.setStringValue(inchiCol);
      }

      // set inchi key column name
      if (nodeSettings.inchiKeyColName.getStringValue() == null || 
            "".equals(nodeSettings.inchiKeyColName.getStringValue())) {
         String inchiKeyCol = DataTableSpec.getUniqueColumnName(inSpec, nodeSettings.colName.getStringValue() 
               + " (InChiKey)");
         nodeSettings.inchiKeyColName.setStringValue(inchiKeyCol);
      }
      
      // set loading parameters warning message
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return new DataTableSpec[] { getDataTableSpec(inSpec) };
   }
   
   private DataTableSpec getDataTableSpec(DataTableSpec inSpec) throws InvalidSettingsException {
      
      if (nodeSettings.inchiColName.getStringValue() == null || nodeSettings.inchiColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No InChi column name specified");

      if (nodeSettings.inchiKeyColName.getStringValue() == null || nodeSettings.inchiKeyColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No InChiKey column name specified");
      
      // create output column specs
      int inchiCol = 1;
      int inchiKeyCol = nodeSettings.appendInchiKeyColumn.getBooleanValue() ? 1 : 0;
      DataColumnSpec specs[] = new DataColumnSpec[inSpec.getNumColumns() + inchiCol + inchiKeyCol];

      // copy input specs
      int i;
      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);
      
      // types for inchi columns
      DataType inchiType = StringCell.TYPE;
      specs[i] = new DataColumnSpecCreator(nodeSettings.inchiColName.getStringValue(), 
            inchiType).createSpec();
      
      // add one more column spec for inchi key
      if (inchiKeyCol == 1) {
         i++;

         DataType inchiKeyType = StringCell.TYPE;
         specs[i] = new DataColumnSpecCreator(nodeSettings.inchiKeyColName.getStringValue(), 
               inchiKeyType).createSpec();
         
      }
      
      return new DataTableSpec(specs);
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
      IndigoInchiNodeSettings s = new IndigoInchiNodeSettings();
      s.loadSettingsFrom(settings);
      
      if (s.colName.getStringValue() == null || s.colName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No column name given");
      
      
      if ((s.inchiColName.getStringValue() == null) || (s.inchiColName.getStringValue().length() < 1))
         throw new InvalidSettingsException("No name for inchi column given");

      if (s.appendInchiKeyColumn.getBooleanValue()) {
         if ((s.inchiKeyColName.getStringValue() == null) || (s.inchiKeyColName.getStringValue().length() < 1))
            throw new InvalidSettingsException("No name for inchi key column given");
      }
   }

   @Override
   protected void loadValidatedSettingsFrom(NodeSettingsRO settings)
         throws InvalidSettingsException {
      nodeSettings.loadSettingsFrom(settings);      
   }

   @Override
   protected void reset() {
   }

}
