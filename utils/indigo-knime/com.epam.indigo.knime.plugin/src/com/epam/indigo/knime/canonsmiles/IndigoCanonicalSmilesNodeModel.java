package com.epam.indigo.knime.canonsmiles;

import java.io.File;
import java.io.IOException;

import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;
import org.knime.core.data.RowKey;
import org.knime.core.data.StringValue;
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
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.cell.IndigoDataValue;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoCanonicalSmilesNodeModel extends IndigoNodeModel {

   private final IndigoCanonicalSmilesNodeSettings nodeSettings = 
         new IndigoCanonicalSmilesNodeSettings();
   
   public IndigoCanonicalSmilesNodeModel() {
      super(1, 1);
   }

   @Override
   protected BufferedDataTable[] execute(BufferedDataTable[] inData,
         ExecutionContext exec) throws Exception {

      BufferedDataTable bufferedDataTable = inData[IndigoCanonicalSmilesNodeSettings.INPUT_PORT];
      
      DataTableSpec inSpec = bufferedDataTable.getDataTableSpec();
      DataTableSpec spec = getDataTableSpec(inSpec);
      
      // output container
      BufferedDataContainer outputContainer = exec.createDataContainer(spec);
      
      // target column index
      int colIdx = spec.findColumnIndex(nodeSettings.colName.getStringValue());

      if (colIdx == -1)
         throw new Exception("column not found");

      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // create converter to get column with both original and indigo representation
      IndigoConverter converter = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);

      int rowNumber = 1;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      for (DataRow inputRow : bufferedDataTable) {
         
         RowKey key = inputRow.getKey();
         DataCell[] cells = new DataCell[inputRow.getNumCells() + 1];
         
         int idx;
         
         for (idx = 0; idx < inputRow.getNumCells(); idx++)
            cells[idx] = inputRow.getCell(idx);
         
         // by default result appended cell is missing one
         cells[idx] = DataType.getMissingCell();
         
         if (!cells[colIdx].isMissing()) {
            try {
               IndigoPlugin.lock();
               
               // extract indigo representation
               DataCell newCell = converter.convert(inputRow.getCell(colIdx));
               
               // extract indigo object
               IndigoObject io;
               
               // extract indigo representation
               if (newCell.getType().isCompatible(StringValue.class)) {
                  String repr = ((StringValue)newCell).getStringValue();
                  if (IndigoType.MOLECULE.equals(indigoType))
                     io = indigo.loadMolecule(repr);
                  else
                     io = indigo.loadReaction(repr);
               } else {
                  DataValue adapter = ((AdapterCell) newCell).getAdapter(indigoType.getIndigoDataValueClass());
                  io = ((IndigoDataValue) adapter).getIndigoObject();
               }
               
               
               // try to extract canonical smiles from indigo object
               cells[idx] = StringCellFactory.create(io.canonicalSmiles());
               
            }catch (IndigoException e) {
               appendWarningMessage("Error while translating to Indigo or extracting canonical smiles "
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

      DataTableSpec inSpec = inSpecs[IndigoCanonicalSmilesNodeSettings.INPUT_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, indigoType);
      
      if (nodeSettings.newColName.getStringValue() == null || 
    		  "".equals(nodeSettings.newColName.getStringValue()))
         nodeSettings.newColName.setStringValue(nodeSettings.colName.getStringValue() 
               + " (canonical SMILES)");
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return new DataTableSpec[] { getDataTableSpec(inSpec) };
   }
   
   private DataTableSpec getDataTableSpec(DataTableSpec inSpec) throws InvalidSettingsException {
      
      if (nodeSettings.newColName.getStringValue() == null || nodeSettings.newColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No new column name specified");
      
      // create output column specs
      DataColumnSpec specs[] = new DataColumnSpec[inSpec.getNumColumns() + 1];

      // copy input specs
      int i;
      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);
      
      // type for column with canonical smiles
      DataType outputType = StringCell.TYPE;
      
      specs[i] = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), 
            outputType).createSpec();
      
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
      IndigoCanonicalSmilesNodeSettings s = new IndigoCanonicalSmilesNodeSettings();
      s.loadSettingsFrom(settings);
      
      if (s.colName.getStringValue() == null || s.colName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No column name given");
      
      
      if ((s.newColName.getStringValue() == null) || (s.newColName.getStringValue().length() < 1))
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

}
