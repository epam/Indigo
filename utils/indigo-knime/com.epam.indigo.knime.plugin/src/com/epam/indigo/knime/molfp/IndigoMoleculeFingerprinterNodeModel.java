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

package com.epam.indigo.knime.molfp;

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
import org.knime.core.data.vector.bitvector.DenseBitVectorCell;
import org.knime.core.data.vector.bitvector.DenseBitVectorCellFactory;
import org.knime.core.data.vector.bitvector.SparseBitVectorCell;
import org.knime.core.data.vector.bitvector.SparseBitVectorCellFactory;
import org.knime.core.node.*;

import com.epam.indigo.*;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public class IndigoMoleculeFingerprinterNodeModel extends IndigoNodeModel
{

   private final IndigoMoleculeFingerprinterSettings nodeSettings = new IndigoMoleculeFingerprinterSettings();
   
   /**
    * Constructor for the node model.
    */
   protected IndigoMoleculeFingerprinterNodeModel ()
   {
      super(1, 1);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute (final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception
   {
      BufferedDataTable bufferedDataTable = inData[IndigoMoleculeFingerprinterSettings.INPUT_PORT];
      
      DataTableSpec inSpec = bufferedDataTable.getDataTableSpec();
      DataTableSpec spec = getDataTableSpec(inSpec);

      BufferedDataContainer outputContainer = exec.createDataContainer(spec);

      int colIdx = spec.findColumnIndex(nodeSettings.colName.getStringValue());
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      IndigoConverter converter = IndigoConverterFactory.createConverter(inSpec, colIdx, indigoType);

      if (colIdx == -1)
         throw new Exception("column not found");

      int rowNumber = 1;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      String fpType = getFingerprintType();
      
      // firstly apply settings to Indigo object
      try {
    	  IndigoPlugin.lock();
    	  
          // take into account settins to process input
          indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
          indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());

          // apply fingerprint options
          indigo.setOption("fp-sim-qwords", nodeSettings.fpSimQWords.getIntValue());
          indigo.setOption("fp-ord-qwords", nodeSettings.fpOrdQWords.getIntValue());
          indigo.setOption("fp-tau-qwords", nodeSettings.fpTauQWords.getIntValue());
          indigo.setOption("fp-any-qwords", nodeSettings.fpAnyQWords.getIntValue());
          indigo.setOption("fp-ext-enabled", nodeSettings.includeEXTPart.getBooleanValue());
          
      } catch (IndigoException e) {
    	  appendWarningMessage("Error while setting Indigo: " + e.getMessage());
      } finally {
    	  IndigoPlugin.unlock();
      }
      
      for (DataRow inputRow : bufferedDataTable) {
         
         RowKey key = inputRow.getKey();
         DataCell[] cells = new DataCell[inputRow.getNumCells() + 1];
         
         int cellIdx;
         String fp = null;
         
         for (cellIdx = 0; cellIdx < inputRow.getNumCells(); cellIdx++)
            cells[cellIdx] = inputRow.getCell(cellIdx);

         if (!inputRow.getCell(colIdx).isMissing()) {
            try {
               IndigoPlugin.lock();
               
               // extract indigo representation
               DataCell newCell = converter.convert(inputRow.getCell(colIdx));
               IndigoObject io = extractIndigoObject(newCell, indigoType);

               io.aromatize();
               fp = io.fingerprint(fpType).toString();
               
            }catch (IndigoException e) {
               appendWarningMessage("Error while calculating structure fingerprint for RowId = '" + inputRow.getKey()+ "': " + e.getMessage());
            } finally {
               IndigoPlugin.unlock();
            }
         }
         
         if(fp != null) {
            if (nodeSettings.denseFormat.getBooleanValue())
               cells[cellIdx] = new DenseBitVectorCellFactory(fp).createDataCell();
            else
               cells[cellIdx] = new SparseBitVectorCellFactory(fp).createDataCell();
         }
         else
            cells[cellIdx] = DataType.getMissingCell();
         
         outputContainer.addRowToTable(new DefaultRow(key, cells));

         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) bufferedDataTable.size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }
      
      handleWarningMessages();
      outputContainer.close();

      return new BufferedDataTable[] { outputContainer.getTable() };
   }

   private String getFingerprintType() {
	   
	   String type = "sim";
	   
	   if (nodeSettings.similarityFp.getBooleanValue())
		   type = "sim";
	   else if (nodeSettings.substructureFp.getBooleanValue())
		   type = "sub";
	   else if (nodeSettings.substructureResonanceFp.getBooleanValue())
		   type = "sub-res";
	   else if (nodeSettings.substructureTautomerFp.getBooleanValue())
		   type = "sub-tau";
	   else if (nodeSettings.fullFp.getBooleanValue())
		   type = "full";
	   
	   return type;
}

/**
    * {@inheritDoc}
    */
   @Override
   protected void reset ()
   {
   }

   protected DataTableSpec getDataTableSpec (DataTableSpec inSpec) throws InvalidSettingsException {
      
      DataColumnSpec[] specs = new DataColumnSpec[inSpec.getNumColumns() + 1];

      if (nodeSettings.newColName.getStringValue() == null || nodeSettings.newColName.getStringValue().length() < 1)
         throw new InvalidSettingsException("No new column name specified");
      
      int i;
      for (i = 0; i < inSpec.getNumColumns(); i++)
         specs[i] = inSpec.getColumnSpec(i);

      DataType outputType;
      if (nodeSettings.denseFormat.getBooleanValue())
    	  outputType = DenseBitVectorCell.TYPE;
      else
    	  outputType = SparseBitVectorCell.TYPE;
      specs[i] = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), outputType).createSpec(); 
         
      return new DataTableSpec(specs);
   }
   
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected DataTableSpec[] configure (final DataTableSpec[] inSpecs)
         throws InvalidSettingsException
   {
      DataTableSpec inSpec = inSpecs[IndigoMoleculeFingerprinterSettings.INPUT_PORT];
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      searchIndigoCompatibleColumn(inSpec, nodeSettings.colName, indigoType);

      if (nodeSettings.newColName.getStringValue() == null)
         nodeSettings.newColName.setStringValue(nodeSettings.colName.getStringValue() + " (fingerprint)");
      
      /*
       * Set loading parameters warning message
       */
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return new DataTableSpec[] { getDataTableSpec(inSpec) };
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
      IndigoMoleculeFingerprinterSettings s = new IndigoMoleculeFingerprinterSettings();
      s.loadSettingsFrom(settings);
      
      if (s.fpSimQWords.getIntValue() < 0 || s.fpOrdQWords.getIntValue() < 0 
    		  || s.fpTauQWords.getIntValue() < 0 || s.fpAnyQWords.getIntValue() < 0)
         throw new InvalidSettingsException("fingerprint size must be a positive integer");
      if (s.colName.getStringValue() == null || "".equals(s.colName.getStringValue()))
         throw new InvalidSettingsException("No column name given");
      if (s.newColName.getStringValue() == null || "".equals(s.newColName.getStringValue()))
         throw new InvalidSettingsException("No name for new column given");
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
