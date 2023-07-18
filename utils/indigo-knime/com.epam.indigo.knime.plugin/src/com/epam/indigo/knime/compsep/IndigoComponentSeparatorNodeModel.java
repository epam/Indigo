package com.epam.indigo.knime.compsep;


import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import org.knime.core.data.*;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.node.*;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.cell.*;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoComponentSeparatorNodeModel extends IndigoNodeModel
{
   
   // the logger instance
   private static final NodeLogger LOGGER = NodeLogger
         .getLogger(IndigoComponentSeparatorNodeModel.class);

   protected IndigoComponentSeparatorNodeModel()
   {
      super(1, 1);
   }
   
   private final IndigoComponentSeparatorSettings nodeSettings = new IndigoComponentSeparatorSettings();

   protected DataTableSpec calcDataTableSpec (DataTableSpec inSpec, IndigoConverter conv, int ncolumns)
   {
      DataColumnSpec[] specs = new DataColumnSpec[inSpec.getNumColumns() + ncolumns];

      int i;

      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);

      for (i = 1; i <= ncolumns; i++)
         specs[inSpec.getNumColumns() + i - 1] =
            new DataColumnSpecCreator(nodeSettings.newColPrefix.getStringValue() + i, conv.getConvertedType()).createSpec();

      return new DataTableSpec(specs);
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute(final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      BufferedDataTable bufferedDataTable = inData[IndigoComponentSeparatorSettings.INPUT_PORT];
      
      DataTableSpec inSpec = bufferedDataTable.getDataTableSpec();
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      
      int colIdx = inSpec.findColumnIndex(nodeSettings.colName.getStringValue());
      if (colIdx == -1)
         throw new Exception("column not found");

      // create cell's converter 
      IndigoConverter conv = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);
      
      //  let's find out max row length for the future output table
      int maxcomp = 0;

      Indigo indigo = IndigoPlugin.getIndigo();
      // take into account settings to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      // fill tempContainer in with rows in which replace target cells by cells containing original repr as well as indigo repr
      for (DataRow inputRow : bufferedDataTable)
      {
         DataCell cell = inputRow.getCell(colIdx);
         
         if (!cell.isMissing()) {
            
            try {
               IndigoPlugin.lock();
            
               // get indigo representation
               DataCell newCell = conv.convert(cell);
               IndigoObject io = extractIndigoObject(newCell, indigoType);

               // learn indigo object's components
               int ncomp = io.countComponents();
               
               // compare with previous max 
               if (maxcomp < ncomp)
                  maxcomp = ncomp;
               
            } 
            catch (IndigoException e) {
               LOGGER.warn("could not convert input value into Indigo in RowId = '" + inputRow.getKey() + "': " + e.getMessage());
            }
            finally {
               IndigoPlugin.unlock();
            }
         }
      }
      /*
       * Check for limit
       */
      if(nodeSettings.limitComponentNumber.getBooleanValue()) {
         if(maxcomp > nodeSettings.componentNumber.getIntValue())
            maxcomp = nodeSettings.componentNumber.getIntValue();
      }
      
      DataTableSpec spec = calcDataTableSpec(inSpec, conv, maxcomp);
      BufferedDataContainer outputContainer = exec.createDataContainer(spec);
      
      // fill output container with original rows + target's components
      for (DataRow inputRow : bufferedDataTable)
      {
         RowKey key = inputRow.getKey();
         DataCell[] cells = new DataCell[inputRow.getNumCells() + maxcomp];
         int i;

         for (i = 0; i < inputRow.getNumCells(); i++)
            cells[i] = inputRow.getCell(i);

         for (i = 0; i < maxcomp; i++)
            cells[inputRow.getNumCells() + i] =  DataType.getMissingCell();
         
         DataCell cell = inputRow.getCell(colIdx);
         
         if (!cell.isMissing())
         {
            try
            {
               IndigoPlugin.lock();

               // get indigo representation
               DataCell newCell = conv.convert(cell);
               // extract indigo adapter
               IndigoObject target = extractIndigoObject(newCell, indigoType);
               
               ArrayList<IndigoObject> collection = new ArrayList<IndigoObject>();
               
               for (IndigoObject comp : target.iterateComponents())
                  collection.add(comp.clone());
               
               Collections.sort(collection, new Comparator<IndigoObject>()
               {
                  @Override
                  public int compare(IndigoObject io1, IndigoObject io2)
                  {
                      return io2.countAtoms() - io1.countAtoms();
                  }
               });
               int collectionSize = collection.size();
               
               /*
                * Check for limit
                */
               if(nodeSettings.limitComponentNumber.getBooleanValue()) {
                  if(collectionSize > nodeSettings.componentNumber.getIntValue())
                     collectionSize = nodeSettings.componentNumber.getIntValue();
               }
               /*
                * Add cells
                */
               for (i = 0; i < collectionSize; i++)
                  cells[inputRow.getNumCells() + i] = IndigoCellFactory.createCell(collection.get(i), 
                        null, conv.getConvertedType(), indigoType);
            } catch (IndigoException e) {
               LOGGER.warn("Could not separate the molecule with RowId = '" + inputRow.getKey() + "': " + e.getMessage());
            }
            finally
            {
               IndigoPlugin.unlock();
            }
         }
         
         outputContainer.addRowToTable(new DefaultRow(key, cells));
      }
      
      handleWarningMessages();
      outputContainer.close();
      
      return new BufferedDataTable[] { outputContainer.getTable() };
   }


   /**
    * {@inheritDoc}
    */
   @Override
   protected void reset()
   {
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected DataTableSpec[] configure(final DataTableSpec[] inSpecs)
         throws InvalidSettingsException
   {
      searchIndigoCompatibleColumn(inSpecs[0], nodeSettings.colName, 
            IndigoType.findByString(nodeSettings.inputType.getStringValue()));      
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      return new DataTableSpec[1];
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveSettingsTo(final NodeSettingsWO settings)
   {
      nodeSettings.saveSettingsTo(settings);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadValidatedSettingsFrom(final NodeSettingsRO settings)
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
      IndigoComponentSeparatorSettings s = new IndigoComponentSeparatorSettings();
      s.loadSettingsFrom(settings);

      if (s.colName.getStringValue() == null || s.colName.getStringValue().length() < 1)
         throw new InvalidSettingsException("column name must be specified");
      if (s.newColPrefix.getStringValue() == null || s.newColPrefix.getStringValue().length() < 1)
         throw new InvalidSettingsException("prefix must be specified");
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void loadInternals(final File internDir,
         final ExecutionMonitor exec) throws IOException,
         CanceledExecutionException
   {
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void saveInternals(final File internDir,
         final ExecutionMonitor exec) throws IOException,
         CanceledExecutionException
   {
   }
}
