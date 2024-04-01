package com.epam.indigo.knime.rautomapper;

import java.io.File;
import java.io.IOException;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
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
import com.epam.indigo.knime.cell.IndigoCellFactory;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

/**
 * This is the model implementation of ReactionAutomapper.
 * 
 * 
 * @author
 */
public class IndigoReactionAutomapperNodeModel extends IndigoNodeModel {

   private final IndigoReactionAutomapperSettings nodeSettings = new IndigoReactionAutomapperSettings();

   public static final int INPUT_PORT = 0;

   /**
    * Constructor for the node model.
    */
   protected IndigoReactionAutomapperNodeModel() {
      super(1, 2);
      // change default value
      nodeSettings.inputType.setStringValue(IndigoType.REACTION.toString());
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute(final BufferedDataTable[] inData, final ExecutionContext exec) throws Exception {
      
      BufferedDataTable bufferedDataTable = inData[INPUT_PORT];
      
      DataTableSpec inputTableSpec = bufferedDataTable.getDataTableSpec();
      
      DataTableSpec[] outputSpecs = getDataTableSpecs(inputTableSpec);

      BufferedDataContainer validOutputContainer = exec.createDataContainer(outputSpecs[0]);
      BufferedDataContainer invalidOutputContainer = exec.createDataContainer(outputSpecs[1]);

      int colIdx = inputTableSpec.findColumnIndex(nodeSettings.reactionColumn.getColumnName());
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoConverter conv = IndigoConverterFactory.createConverter(inputTableSpec, colIdx, indigoType, 
            nodeSettings.treatStringAsSMARTS.getBooleanValue());

      int newColIdx = nodeSettings.appendColumn.getBooleanValue() ? inputTableSpec.getNumColumns() : colIdx;
      
      String aamParameters = nodeSettings.getAAMParameters();
      Format outputFormat = Format.valueOf(nodeSettings.outputType.getStringValue());
      
      int rowNumber = 1;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      for (DataRow inputRow : bufferedDataTable) {
         DataCell[] cells = new DataCell[inputRow.getNumCells() + (nodeSettings.appendColumn.getBooleanValue() ? 1 : 0)];

         DataCell dataCell = inputRow.getCell(colIdx);
         DataCell newCell = null;
         String message = null;

         try {
            IndigoPlugin.lock();

            if(dataCell.isMissing())
               newCell = DataType.getMissingCell();
            else {
               
               DataCell tempCell = conv.convert(dataCell);    
               IndigoObject reaction = extractIndigoObject(tempCell, indigoType, nodeSettings.treatStringAsSMARTS.getBooleanValue());
               
               /*
                * Setup timeout
                */
               if(nodeSettings.useAamTimeout.getBooleanValue())
                  indigo.setOption("aam-timeout", nodeSettings.aamTimeout.getIntValue());
               else
                  indigo.setOption("aam-timeout", 0);
               /*
                * Perform AAM
                */
               reaction.automap(aamParameters);
               /*
                * Highlight reacting centers by correcting them
                */
               if(nodeSettings.highlightReactingCenters.getBooleanValue())
                  reaction.correctReactingCenters();
               /*
                * Create data cell
                */

               if (Format.String.equals(outputFormat)) {
                  newCell = StringCellFactory.create(reaction.rxnfile());
               } else {
                  DataType type = getOutputTypeByFormat(outputFormat);
                  newCell = IndigoCellFactory.createCell(reaction, null, type, indigoType);
               }
                  
            }
               
         } catch (IndigoException e) {
            message = "Could not calculate AAM for RowId '" + inputRow.getKey() + "': " + e.getMessage();
            appendWarningMessage(message);
         } finally {
            IndigoPlugin.unlock();
         }

         if (newCell != null) {
            for (int i = 0; i < inputRow.getNumCells(); i++) {
               cells[i] = (!nodeSettings.appendColumn.getBooleanValue() && i == newColIdx) ? newCell : inputRow.getCell(i);
            }
            if (nodeSettings.appendColumn.getBooleanValue()) {
               cells[newColIdx] = newCell;
            }
            validOutputContainer.addRowToTable(new DefaultRow(inputRow.getKey(), cells));
         } else {
            for (int i = 0; i < inputRow.getNumCells(); i++) {
               cells[i] = (!nodeSettings.appendColumn.getBooleanValue() && i == newColIdx) ? new StringCell(message) : inputRow.getCell(i);
            }
            if (nodeSettings.appendColumn.getBooleanValue()) {
               cells[newColIdx] = new StringCell(message);
            }
            invalidOutputContainer.addRowToTable(new DefaultRow(inputRow.getKey(), cells));
         }
         
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) bufferedDataTable.size(), "Adding row " + rowNumber);

         rowNumber++;
      }

      handleWarningMessages();
      validOutputContainer.close();
      invalidOutputContainer.close();
      return new BufferedDataTable[] { validOutputContainer.getTable(), invalidOutputContainer.getTable() };
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void reset() {
   }

   protected DataTableSpec[] getDataTableSpecs(DataTableSpec inputTableSpec) 
         throws InvalidSettingsException {
      
      boolean appendColumn = nodeSettings.appendColumn.getBooleanValue();
      
      if (nodeSettings.reactionColumn.getStringValue() == null || 
            nodeSettings.reactionColumn.getStringValue().length() < 1)
         throw new InvalidSettingsException("Column name not specified");

      if (appendColumn)
         if (nodeSettings.newColName.getStringValue() == null || 
         nodeSettings.newColName.getStringValue().length() < 1)
            throw new InvalidSettingsException("No new column name specified");

      int colIdx = inputTableSpec.findColumnIndex(nodeSettings.reactionColumn.getStringValue());
      if (colIdx == -1)
         throw new InvalidSettingsException("column not found");
      
      String newColName = nodeSettings.newColName.getStringValue();
      
      int newColIdx = inputTableSpec.getNumColumns();
      if (!appendColumn) {
         newColName = nodeSettings.reactionColumn.getStringValue();
         newColIdx = colIdx;
      }

      DataType outputType = getOutputTypeByFormat(Format.
            valueOf(nodeSettings.outputType.getStringValue()));

      DataColumnSpec validOutputColumnSpec = 
            new DataColumnSpecCreator(newColName, outputType).createSpec();
      DataColumnSpec invalidOutputColumnSpec = 
            new DataColumnSpecCreator(newColName, StringCell.TYPE).createSpec();

      DataColumnSpec[] validOutputColumnSpecs, invalidOutputColumnSpecs;

      int numColumns = appendColumn ? inputTableSpec.getNumColumns() + 1 : 
         inputTableSpec.getNumColumns();
      
      validOutputColumnSpecs = new DataColumnSpec[numColumns];
      invalidOutputColumnSpecs = new DataColumnSpec[numColumns];

      // copy specs
      for (int i = 0; i < inputTableSpec.getNumColumns(); i++) {
         DataColumnSpec columnSpec = inputTableSpec.getColumnSpec(i);
            validOutputColumnSpecs[i] = columnSpec;
            invalidOutputColumnSpecs[i] = columnSpec;
      }
      // replace spec by new ones
      validOutputColumnSpecs[newColIdx] = validOutputColumnSpec;
      invalidOutputColumnSpecs[newColIdx] = invalidOutputColumnSpec;

      return new DataTableSpec[] { new DataTableSpec(validOutputColumnSpecs), 
            new DataTableSpec(invalidOutputColumnSpecs) };
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected DataTableSpec[] configure(final DataTableSpec[] inSpecs) throws InvalidSettingsException {
      
      IndigoType type = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      searchIndigoCompatibleColumn(inSpecs[INPUT_PORT], nodeSettings.reactionColumn, type);
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return getDataTableSpecs(inSpecs[INPUT_PORT]);
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
   protected void loadValidatedSettingsFrom(final NodeSettingsRO settings) throws InvalidSettingsException {
      nodeSettings.loadSettingsFrom(settings);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void validateSettings(final NodeSettingsRO settings) throws InvalidSettingsException {
      IndigoReactionAutomapperSettings s = new IndigoReactionAutomapperSettings();
      s.loadSettingsFrom(settings);
      
      if (s.appendColumn.getBooleanValue() && (s.newColName.getStringValue() == null || s.newColName.getStringValue().length() < 1))
         throw new InvalidSettingsException("new column name must be specified");
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadInternals(final File internDir, final ExecutionMonitor exec) throws IOException, CanceledExecutionException {
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveInternals(final File internDir, final ExecutionMonitor exec) throws IOException, CanceledExecutionException {
   }

}
