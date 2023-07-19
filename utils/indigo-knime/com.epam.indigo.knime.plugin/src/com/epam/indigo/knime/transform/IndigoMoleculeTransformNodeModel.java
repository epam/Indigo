package com.epam.indigo.knime.transform;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.def.DefaultRow;
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
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

/**
 * This is the model implementation of IndigoMoleculeTransform.
 * 
 */
public class IndigoMoleculeTransformNodeModel extends IndigoNodeModel {

   private final IndigoMoleculeTransformSettings nodeSettings = new IndigoMoleculeTransformSettings();
   
   // the logger instance
   private static final NodeLogger LOGGER = NodeLogger
         .getLogger(IndigoMoleculeTransformNodeModel.class);

   /**
    * Constructor for the node model.
    */
   protected IndigoMoleculeTransformNodeModel() {
      super(2, 1);
   }

   class Transformation	{
      IndigoObject reaction;
      String rowid; 
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute(final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception {
     
      BufferedDataTable reactionTable = inData[IndigoMoleculeTransformSettings.REACTION_PORT];
      BufferedDataTable moleculeTable = inData[IndigoMoleculeTransformSettings.MOL_PORT];
      
      DataTableSpec moleculeSpec = moleculeTable.getDataTableSpec();
      DataTableSpec reactionSpec = reactionTable.getDataTableSpec();
      
      /*
       * Search columns
       */
      int moleculeColIdx = moleculeSpec.findColumnIndex(nodeSettings.molColumn.getStringValue());
      if(moleculeColIdx == -1)
         throw new RuntimeException("column '" + nodeSettings.molColumn.getStringValue() + "' can not be found");
      
      int reactionColIdx = reactionSpec.findColumnIndex(nodeSettings.reactionColumn.getStringValue());
      if(reactionColIdx == -1)
         throw new RuntimeException("column '" + nodeSettings.reactionColumn.getStringValue() + "' can not be found");

      IndigoType moleculeType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoType reactionType = IndigoType.findByString(nodeSettings.rectionType.getStringValue());
      
      IndigoConverter moleculeConv = IndigoConverterFactory.createConverter(moleculeSpec, moleculeColIdx, moleculeType);
      IndigoConverter reactionConv = IndigoConverterFactory.createConverter(reactionSpec, reactionColIdx, reactionType,
            nodeSettings.treatStringAsSMARTS.getBooleanValue());
      
      DataTableSpec outputSpec = getDataTableSpec(moleculeSpec, moleculeConv);
      
      BufferedDataContainer outputContainer = exec.createDataContainer(outputSpec);
      
      boolean missingWarning = false;
      /*
       * Create reactions list
       */
      LinkedList<Transformation> reactionsList = new LinkedList<Transformation>();
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      for(DataRow inputRow : reactionTable) {
         
         DataCell dataCell = inputRow.getCell(reactionColIdx);
         
         if(dataCell.isMissing()) {
            if(!missingWarning)
               LOGGER.warn("table with reactions contains missing cells: skipping");
            missingWarning = true;
            continue;
         }
         try {
            IndigoPlugin.lock();

            Transformation transformation = new Transformation();
            transformation.rowid = inputRow.getKey().toString();
            
            DataCell newCell = reactionConv.convert(dataCell);    
            transformation.reaction = extractIndigoObject(newCell, reactionType, 
                  nodeSettings.treatStringAsSMARTS.getBooleanValue());
            
            reactionsList.add(transformation);
            
         } catch (IndigoException e) {
            appendWarningMessage("Error while loading reaction with RowId '" + inputRow.getKey() + "': " + e.getMessage());
         } finally {
            IndigoPlugin.unlock();
         }
      }
      /*
       * Iterate and transform molecules
       */
      missingWarning = false;
      /*
       * Define transform column
       */
      int transformColIdx = -1;
      if(nodeSettings.appendColumn.getBooleanValue()) 
         transformColIdx = outputSpec.getNumColumns() - 1;
      else
         transformColIdx = moleculeColIdx;
      
      int rowNumber = 1;
      for (DataRow inputRow : moleculeTable) {
         DataCell dataCell = inputRow.getCell(moleculeColIdx);
         DataCell transformCell = null;
         
         if (dataCell.isMissing())
            transformCell = dataCell;
         
         if(reactionsList.isEmpty()) {
            if(!missingWarning)
               LOGGER.warn("reactions list is empty: no transformations were applied");
            transformCell = dataCell;
            missingWarning = true;
         }
         /*
          * Transform given molecule
          */
         if(transformCell == null) {
            String transformRowId = null;
            try {
               IndigoPlugin.lock();
               
               DataCell newCell = moleculeConv.convert(dataCell);
               
               // extract indigo representation
               IndigoObject indigoMolecule = extractIndigoObject(newCell, moleculeType);
               /*
                * Apply all the reactions from the list
                */
               for(Transformation transformation : reactionsList) {
                  transformRowId = transformation.rowid; 
                  indigo.transform(transformation.reaction, indigoMolecule);
               }
               /*
                * Create result cell
                */
               transformCell = IndigoCellFactory.createCell(indigoMolecule, null, moleculeConv.getConvertedType(), moleculeType);
               
            } catch (IndigoException e) {
               String message = "Warning while applying a transformation: " + e.getMessage() + " for molecule with rowId: " + inputRow.getKey().toString();
               if (transformRowId != null)
                  message += ". Transformation rowId: " + transformRowId;
               appendWarningMessage(message);
               transformCell = null;
            } finally {
               IndigoPlugin.unlock();
            }
         }
         /*
          * Create result row cells
          */
         if(transformCell == null)
            transformCell = DataType.getMissingCell();
         
         DataCell[] outputCells = new DataCell[outputSpec.getNumColumns()];
         for (int cIdx = 0; cIdx < outputSpec.getNumColumns(); cIdx++) {
            if(cIdx == transformColIdx)
               outputCells[cIdx] = transformCell;
            else
               outputCells[cIdx] = inputRow.getCell(cIdx);
         }
         /*
          * Add result row
          */
         outputContainer.addRowToTable(new DefaultRow(inputRow.getKey(),
               outputCells));
         
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) moleculeTable.size(),
               "Adding row " + rowNumber);
         rowNumber++;
      }
      
      
      handleWarningMessages();
      outputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable() };
   }

   private DataTableSpec getDataTableSpec(DataTableSpec inSpec, IndigoConverter conv) {
      DataColumnSpec[] specs;
      
      if(nodeSettings.appendColumn.getBooleanValue())
         specs = new DataColumnSpec[inSpec.getNumColumns() + 1];
      else
         specs = new DataColumnSpec[inSpec.getNumColumns()];
      
      int colIdx;

      for (colIdx = 0; colIdx < inSpec.getNumColumns(); colIdx++)
         specs[colIdx] = inSpec.getColumnSpec(colIdx);
      
      if(nodeSettings.appendColumn.getBooleanValue())
         specs[colIdx] = new DataColumnSpecCreator(
               DataTableSpec.getUniqueColumnName(inSpec, nodeSettings.newColName.getStringValue()), 
               conv.getConvertedType()).createSpec();
      else{
         String name = nodeSettings.molColumn.getStringValue();
         int idx = inSpec.findColumnIndex(name);
         specs[idx] = new DataColumnSpecCreator(name, conv.getConvertedType()).createSpec();
      }
      
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
      
      DataTableSpec mspec = inSpecs[IndigoMoleculeTransformSettings.MOL_PORT];
      DataTableSpec rspec = inSpecs[IndigoMoleculeTransformSettings.REACTION_PORT];
      
      IndigoType mType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoType rType = IndigoType.findByString(nodeSettings.rectionType.getStringValue());
      
      searchIndigoCompatibleColumn(mspec, nodeSettings.molColumn, mType);
      searchIndigoCompatibleColumn(rspec, nodeSettings.reactionColumn, rType);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      int colIdx = mspec.findColumnIndex(nodeSettings.molColumn.getStringValue());
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      IndigoConverter conv = IndigoConverterFactory.createConverter(mspec, colIdx, indigoType);
      
      return new DataTableSpec[] { getDataTableSpec(mspec, conv) };
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
      IndigoMoleculeTransformSettings s = new IndigoMoleculeTransformSettings();
      s.loadSettingsFrom(settings);
      
      if (s.appendColumn.getBooleanValue() && ((s.newColName.getStringValue() == null) || (s.newColName.getStringValue().length() < 1)))
         throw new InvalidSettingsException("No name for new column given");
      
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
