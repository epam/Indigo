package com.epam.indigo.knime.compjoin;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.knime.chem.types.SmartsCell;
import org.knime.chem.types.SmartsCellFactory;
import org.knime.chem.types.SmartsValue;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.data.def.StringCell;
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
import com.epam.indigo.knime.cell.IndigoCellFactory;
import com.epam.indigo.knime.cell.IndigoMolValue;
import com.epam.indigo.knime.cell.IndigoQueryMolValue;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

/**
 * This is the model implementation of IndigoComponentCombiner.
 * 
 * 
 */
public class IndigoComponentCombinerNodeModel extends IndigoNodeModel {

   
   private final IndigoComponentCombinerSettings nodeSettings = new IndigoComponentCombinerSettings();
   // the logger instance
   private static final NodeLogger LOGGER = NodeLogger
         .getLogger(IndigoComponentCombinerNodeModel.class);

   /**
    * Constructor for the node model.
    */
   protected IndigoComponentCombinerNodeModel() {
      super(1, 2);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute(final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception {
      
      BufferedDataTable inputTable = inData[IndigoComponentCombinerSettings.INPUT_PORT];
      DataTableSpec inputSpec = inputTable.getDataTableSpec();
      /*
       * Create output spec
       */
      DataTableSpec outputSpec = getDataTableSpec(inputSpec);
      
      // for valid data
      BufferedDataContainer outputContainer = exec.createDataContainer(outputSpec);
      // for invalid data
      BufferedDataContainer invalidOutputContainer = exec.createDataContainer(inputSpec);
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // define new column spec
      Format outputFormat = Format.valueOf(nodeSettings.outputType.getStringValue());
      DataType newColType = getOutputTypeByFormat(outputFormat);

      /*
       * Search column indexes
       */
      ArrayList<Integer> inclIdxLst = new ArrayList<>();
      for (String colName : nodeSettings.colNames.getIncludeList()) {
         int colIdx = inputSpec.findColumnIndex(colName);

         if (colIdx == -1)
            throw new RuntimeException("column '" + colName + "' not found");
         
         inclIdxLst.add(colIdx);
      }
      
      /*
       * Create indigo converters for every of the columns
       */
      HashMap<Integer, IndigoConverter> converters = new HashMap<>();
      for (Integer idx : inclIdxLst) {
         IndigoConverter conv = IndigoConverterFactory.createConverter(inputSpec, idx, indigoType, 
               nodeSettings.treatStringAsSMARTS.getBooleanValue());
         converters.put(idx, conv);
      }
      
      /*
       * Iterate through all the table
       */
      int rowNumber = 1;
      
      for (DataRow inputRow : inputTable) {
         
         DataCell[] outputCells= new DataCell[inputRow.getNumCells() + 1];
         outputCells[outputCells.length-1] = DataType.getMissingCell(); // default value; will be replaced if ok
         Indigo indigo = IndigoPlugin.getIndigo();
         
         try {
            
            IndigoPlugin.lock();
            
            // take into account settins to process input
            indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
            indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
         
            int colIdx;
            
            // copy all cells from the row
            for (colIdx = 0; colIdx < inputRow.getNumCells(); colIdx++)
               outputCells[colIdx] = inputRow.getCell(colIdx);
         
            /*
             * Add merged cell
             */
            
            boolean isSmarts = Format.SMARTS.equals(outputFormat);
            
            if (isSmarts) {
               // string builder to gather joined data
               StringBuilder src = new StringBuilder();
               for (Integer idx : inclIdxLst) {
                  String s;
                  if (outputCells[idx].getType().isCompatible(SmartsValue.class)) {
                     s = ((SmartsCell)outputCells[idx]).getSmartsValue();
                  } else {
                     s = ((StringCell)outputCells[idx]).getStringValue();
                  }
                     
                  if (src.length() == 0) {
                     src.append(s);
                  } else {
                     src.append(".");
                     src.append(s);
                  }
               }
               
               outputCells[colIdx] = indigoType.createAdapterContainingIndigoRepr(SmartsCellFactory.
                     createAdapterCell(src.toString()), true, nodeSettings.treatStringAsSMARTS.getBooleanValue());
               
            } else {
               
               IndigoObject io = null;
               
               // merge all the cells
               for (Integer idx : inclIdxLst) {
                  
                  // extract indigo representation
                  DataCell newCell = converters.get(idx).convert(outputCells[idx]);
                  if (io == null) 
                     io = extractIndigoObject(newCell, indigoType);
                  else
                     io.merge(extractIndigoObject(newCell, indigoType));
               }

               outputCells[colIdx] = IndigoCellFactory.createCell(io, null, newColType, indigoType);
               
            }

            outputContainer.addRowToTable(new DefaultRow(inputRow.getKey(), outputCells));
            
         } catch (IndigoException e) {
            LOGGER.warn("could not convert to indigo format one of cell in row: " + inputRow.getKey() + ": " + e.getMessage());
            invalidOutputContainer.addRowToTable(inputRow);
         } finally {
            IndigoPlugin.unlock();
         }
         
         
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) inputTable.size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }
      
      handleWarningMessages();
      outputContainer.close();
      invalidOutputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable(), invalidOutputContainer.getTable() };
   }
   
   
   protected DataTableSpec getDataTableSpec (DataTableSpec inSpec) throws InvalidSettingsException {
      
      DataColumnSpec[] specs = new DataColumnSpec[inSpec.getNumColumns() + 1];
      
      List<String> included = nodeSettings.colNames.getIncludeList();
      
      // gather included column types
      ArrayList<DataType> colTypes = new ArrayList<>();
      for(String name : included) 
         colTypes.add(inSpec.getColumnSpec(name).getType());
      
      int colIdx;

      for (colIdx = 0; colIdx < inSpec.getNumColumns(); colIdx++) {
         specs[colIdx] = inSpec.getColumnSpec(colIdx);
      }

      // define new column spec
      Format outputFormat = Format.valueOf(nodeSettings.outputType.getStringValue());
      DataType newColType = getOutputTypeByFormat(outputFormat);

      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      // check settings validity
      if (IndigoType.MOLECULE.equals(indigoType)) {
         for (DataType dataType : colTypes) {
            if (dataType.isCompatible(IndigoQueryMolValue.class)) {
               throw new InvalidSettingsException("Some of included columns has query type!");
            }
         }
      }
      
      if (IndigoType.QUERY_MOLECULE.equals(indigoType)) {
         
         for (DataType dataType : colTypes) {
            if (dataType.isCompatible(IndigoMolValue.class))
               throw new InvalidSettingsException("Some of included columns contains non-query indigo adapter!");
         }
         
         if (Format.SMARTS.equals(outputFormat)) {
            for (DataType dataType : colTypes) {
               boolean string = StringCell.TYPE.equals(dataType);
               boolean smarts = dataType.isCompatible(SmartsValue.class);
               if (!string && !smarts) {
                  throw new InvalidSettingsException("To have SMARTS format as output format all the "
                        + "included columns must have String or SMARTS type.");
               }
            }
         }
         
      }
      
      // set spec for the last new column
      specs[colIdx] = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), newColType).createSpec();

      return new DataTableSpec(specs);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void reset() {
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected DataTableSpec[] configure(final DataTableSpec[] inSpecs)
         throws InvalidSettingsException {
      
      DataTableSpec dataTableSpec = inSpecs[IndigoComponentCombinerSettings.INPUT_PORT];
      
      List<String> included = nodeSettings.colNames.getIncludeList();
      List<String> excluded = nodeSettings.colNames.getExcludeList();
      
      if (included.isEmpty() && excluded.isEmpty()) {
         
         ArrayList<String> excludeList = new ArrayList<String>();
         ArrayList<String> includeList = new ArrayList<String>();
         
         for (int i = 0; i < dataTableSpec.getNumColumns(); i++) {
            DataColumnSpec colSpec = dataTableSpec.getColumnSpec(i);
            if (nodeSettings.columnFilter.includeColumn(colSpec)) 
               includeList.add(colSpec.getName());
            else
               excludeList.add(colSpec.getName());
         }
         
         nodeSettings.colNames.setIncludeList(includeList);
         nodeSettings.colNames.setExcludeList(excludeList);
      }
      
      return new DataTableSpec[] { getDataTableSpec(dataTableSpec), dataTableSpec };
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveSettingsTo(final NodeSettingsWO settings) {
      nodeSettings.saveSettingsTo(settings);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadValidatedSettingsFrom(final NodeSettingsRO settings)
         throws InvalidSettingsException {
      nodeSettings.loadSettingsFrom(settings);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void validateSettings(final NodeSettingsRO settings)
         throws InvalidSettingsException {
      IndigoComponentCombinerSettings s = new IndigoComponentCombinerSettings();
      s.loadSettingsFrom(settings);
      
      if(s.colNames.getIncludeList().isEmpty())
         throw new InvalidSettingsException("component column list is empty");
      
      if(s.newColName.getStringValue() == null || s.newColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("result column name can not be empty");
      
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadInternals(final File internDir,
         final ExecutionMonitor exec) throws IOException,
         CanceledExecutionException {
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveInternals(final File internDir,
         final ExecutionMonitor exec) throws IOException,
         CanceledExecutionException {
   }

}
