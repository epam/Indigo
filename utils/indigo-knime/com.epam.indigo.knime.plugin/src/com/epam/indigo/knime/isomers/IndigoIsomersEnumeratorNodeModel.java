package com.epam.indigo.knime.isomers;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.RowKey;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.data.def.StringCell;
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
 * This is the model implementation of IndigoIsomersEnumerator.
 * 
 * 
 */
public class IndigoIsomersEnumeratorNodeModel extends IndigoNodeModel {

   private final IndigoIsomersEnumeratorSettings nodeSettings = new IndigoIsomersEnumeratorSettings();

   /**
    * Constructor for the node model.
    */
   protected IndigoIsomersEnumeratorNodeModel() {
      super(1, 1);
   }
   
   private List<DataCell> enumerateIsomers (IndigoObject io, DataType originType, IndigoType indigoType) throws Exception
   {
      ArrayList<DataCell> results = new ArrayList<>();
      
      ArrayList<IndigoObject> stereoObjects = new ArrayList<IndigoObject>();
      
      // Collect stereo bonds
      if (nodeSettings.cisTransIsomers.getBooleanValue())
      {
         for (IndigoObject bond : io.iterateBonds())
         {
            int stereo = bond.bondStereo(); 
            if (stereo == Indigo.CIS || stereo == Indigo.TRANS)
               stereoObjects.add(bond);
         }
      }
      if (nodeSettings.tetrahedralIsomers.getBooleanValue())
      {
         for (IndigoObject atom : io.iterateAtoms())
         {
            int stereo = atom.stereocenterType(); 
            if (stereo == Indigo.ABS || stereo == Indigo.OR || stereo == Indigo.AND)
               stereoObjects.add(atom);
         }
      }
      
      if (stereoObjects.size() > 16)
         throw new Exception("Number of stereoobjects is too big");
      
      GrayCodes grayCodes = new GrayCodes(stereoObjects.size());
      Set<String> canonicalSmiles = new HashSet<String>();
      while (true)
      {
         String cano = io.canonicalSmiles();
         if (canonicalSmiles.add(cano)) {
            DataCell cell = IndigoCellFactory.createCell(io, null, originType, indigoType);
            results.add(cell);
         }

         grayCodes.next();
         if (grayCodes.isDone()) 
            break;

         int index = grayCodes.getBitChangeIndex();
         stereoObjects.get(index).invertStereo();
      }
      
      return results;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      BufferedDataTable bufferedDataTable = inData[IndigoIsomersEnumeratorSettings.INPUT_PORT];
      
      DataTableSpec spec = getDataTableSpec(bufferedDataTable.getDataTableSpec());

      BufferedDataContainer outputContainer = exec.createDataContainer(spec);
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());

      int colIdx = spec.findColumnIndex(nodeSettings.colName.getStringValue());
      int destColIdx = colIdx;
      
      if (nodeSettings.appendColumn.getBooleanValue())
         destColIdx = spec.findColumnIndex(nodeSettings.newColName.getStringValue());
      
      int destRowisColIdx = -1;
      if (nodeSettings.appendRowidColumn.getBooleanValue())
         destRowisColIdx = spec.findColumnIndex(nodeSettings.newRowidColName.getStringValue());

      if (colIdx == -1 || destColIdx == -1)
         throw new Exception("column not found");

      int rowNumber = 1;

      // create cell's converter 
      IndigoConverter conv = IndigoConverterFactory.createConverter(bufferedDataTable.getDataTableSpec(), 
            colIdx, indigoType);
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      for (DataRow inputRow : bufferedDataTable)
      {
         RowKey srcKey = inputRow.getKey();
         DataCell[] cells = new DataCell[spec.getNumColumns()];
         
         int cellIdx;
         
         DataCell inputCell = inputRow.getCell(colIdx);
         
         if (!inputCell.isMissing())
            try {
               int isomerIndex = 1;
               
               IndigoPlugin.lock();
               
               // create cell containing both original and indigo representations
               DataCell newCell = conv.convert(inputCell);
               // extract indigo representation
               IndigoObject io = extractIndigoObject(newCell, indigoType);
               
               for (cellIdx = 0; cellIdx < inputRow.getNumCells(); cellIdx++)
                  if (cellIdx != destColIdx)
                     cells[cellIdx] = inputRow.getCell(cellIdx);
               
               for (DataCell iso : enumerateIsomers(io, conv.getConvertedType(), indigoType))
               {
                  cells[destColIdx] = iso;
                  
                  if (destRowisColIdx != -1)
                     cells[destRowisColIdx] = new StringCell(srcKey.getString());
                  
                  RowKey destKey = new RowKey(String.format("%s_%d", srcKey.getString(), isomerIndex));
                  outputContainer.addRowToTable(new DefaultRow(destKey, cells));
                  exec.checkCanceled();
                  exec.setProgress(rowNumber / (double) bufferedDataTable.size(),
                        "Adding row " + rowNumber);
   
                  rowNumber++;
                  isomerIndex++;
               }
            }catch (IndigoException e) {
               appendWarningMessage("Error while calculating structure fingerprint for RowId = '" + inputRow.getKey()+ "': " + e.getMessage());
            } finally {
               IndigoPlugin.unlock();
            }
      }
      
      handleWarningMessages();

      outputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable() };
   }
   
   protected DataTableSpec getDataTableSpec (DataTableSpec inSpec) throws InvalidSettingsException
   {
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      int colIdx = inSpec.findColumnIndex(nodeSettings.colName.getStringValue());
      IndigoConverter conv = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);
      
      ArrayList<DataColumnSpec> specs = new ArrayList<DataColumnSpec>();

      for (int i = 0; i < inSpec.getNumColumns(); i++)
         specs.add(inSpec.getColumnSpec(i));
      
      
      DataColumnSpec colSpec;
      

      // Output column
      if (nodeSettings.appendColumn.getBooleanValue()) {
         colSpec = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), 
               conv.getConvertedType()).createSpec();
         specs.add(colSpec); 
      } else {
         colSpec = new DataColumnSpecCreator(specs.get(colIdx).getName(), 
               conv.getConvertedType()).createSpec();
         specs.set(colIdx, colSpec);
      }
      
      if (nodeSettings.appendRowidColumn.getBooleanValue())
         specs.add(new DataColumnSpecCreator(
               nodeSettings.newRowidColName.getStringValue(), StringCell.TYPE).createSpec()); 
         
      DataColumnSpec[] specsArray = specs.toArray(new DataColumnSpec[0]); 
      return new DataTableSpec(specsArray);
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
      DataTableSpec spec = inSpecs[0];

      searchIndigoCompatibleColumn(spec, nodeSettings.colName, 
            IndigoType.findByString(nodeSettings.inputType.getStringValue()));
      /*
       * Set loading parameters warning message
       */
      if (nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }

      return new DataTableSpec[] { getDataTableSpec(spec) };
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
      IndigoIsomersEnumeratorSettings s = new IndigoIsomersEnumeratorSettings();
      s.loadSettingsFrom(settings);

      if (s.appendColumn.getBooleanValue()
            && ((s.newColName.getStringValue() == null) || (s.newColName
                  .getStringValue().length() < 1)))
         throw new InvalidSettingsException("No name for new column given");
      if (s.appendRowidColumn.getBooleanValue()
            && ((s.newRowidColName.getStringValue() == null) || (s.newColName
                  .getStringValue().length() < 1)))
         throw new InvalidSettingsException(
               "No name for new rowid column given");
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
