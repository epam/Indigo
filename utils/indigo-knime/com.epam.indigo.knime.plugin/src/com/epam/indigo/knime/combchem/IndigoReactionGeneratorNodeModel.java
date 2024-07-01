package com.epam.indigo.knime.combchem;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataRow;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.node.BufferedDataContainer;
import org.knime.core.node.BufferedDataTable;
import org.knime.core.node.CanceledExecutionException;
import org.knime.core.node.ExecutionContext;
import org.knime.core.node.ExecutionMonitor;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.port.PortType;
import org.knime.core.node.port.PortTypeRegistry;

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
public class IndigoReactionGeneratorNodeModel extends IndigoNodeModel {

   private final IndigoReactionGeneratorSettings nodeSettings = new IndigoReactionGeneratorSettings();
   
   /**
    * Constructor for the node model.
    */
   protected IndigoReactionGeneratorNodeModel() {
      // Second molecule input is optional
      super(new PortType[] {
               BufferedDataTable.TYPE, BufferedDataTable.TYPE,
               PortTypeRegistry.getInstance().getPortType(BufferedDataTable.class, true)},
            new PortType[] { BufferedDataTable.TYPE });
   }

   List<IndigoObject> readIndigoObjects (BufferedDataTable table, String columnName, IndigoType indigoType,
         boolean stringAsSMARTS)
   {
      DataTableSpec spec = table.getSpec();
      int colIdx = spec.findColumnIndex(columnName);
      if(colIdx == -1)
         throw new RuntimeException("column '" + columnName + "' can not be found");
      
      IndigoConverter conv = IndigoConverterFactory.createConverter(table.getDataTableSpec(), colIdx, 
            indigoType, stringAsSMARTS);
      
      LinkedList<IndigoObject> objects = new LinkedList<IndigoObject>();

      for (DataRow inputRow : table) {
         DataCell dataCell = inputRow.getCell(colIdx);
         if (dataCell.isMissing()) {
            appendWarningMessage("Missing cell has been skipped for RowId '" + inputRow.getKey() + "'");
            continue;
         }
         try {
            IndigoPlugin.lock();
            
            // convert data cell into adapter if required and extract indigo object
            DataCell newCell = conv.convert(dataCell);
            IndigoObject obj = extractIndigoObject(newCell, indigoType, stringAsSMARTS);
            // add extracted indigo object to the list
            objects.add(obj);
            
         } catch (IndigoException e) {
            appendWarningMessage("Error while loading structure with RowId '" + inputRow.getKey() + "': " + e.getMessage());
         } finally {
            IndigoPlugin.unlock();
         }
      }
      
      return objects;
   }
   
   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute(final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception {
   
      BufferedDataTable reactionTable = inData[IndigoReactionGeneratorSettings.REACTION_PORT];
      BufferedDataTable moleculeTable1 = inData[IndigoReactionGeneratorSettings.MOL_PORT1];
      BufferedDataTable moleculeTable2 = inData[IndigoReactionGeneratorSettings.MOL_PORT2];

      DataTableSpec outputSpec = getDataTableSpec();
      BufferedDataContainer outputContainer = exec.createDataContainer(outputSpec);

      Indigo indigo = IndigoPlugin.getIndigo(); 
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());

      IndigoType reacIndigoType = IndigoType.findByString(nodeSettings.reactionType.getStringValue());
      IndigoType molIndigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      List<IndigoObject> reactions = readIndigoObjects(reactionTable, nodeSettings.reactionColumn.getStringValue(), 
            reacIndigoType, nodeSettings.treatStringAsSMARTS.getBooleanValue());
    
      if (reactions.size() != 1)
         throw new RuntimeException("Only one reaction is expected, but " + reactions.size() + " reactions was provided.");
    
      List<List<IndigoObject>> mols = new ArrayList<List<IndigoObject>>(); 
      mols.add(readIndigoObjects(moleculeTable1, nodeSettings.molColumn1.getStringValue(), molIndigoType, false));
      if (moleculeTable2 != null)
         mols.add(readIndigoObjects(moleculeTable2, nodeSettings.molColumn2.getStringValue(), molIndigoType, false));

      IndigoObject reaction = reactions.get(0);
      IndigoObject output;
      int outputCount;
      try {
         IndigoPlugin.lock();
         indigo.setOption("rpe-max-products-count", nodeSettings.productsCountLimit.getIntValue());
         
         /*
         indigo.setOption("rpe-max-depth", max_depth);
         int max_pr_count = Integer.parseInt(max_products_text_field.getText());
         indigo.setOption("rpe-max-products-count", max_pr_count);
         indigo.setOption("rpe-multistep-reactions", is_multistep_reactions_check.getState());
         if (is_one_tube_check.getState())
            indigo.setOption("rpe-mode", "one-tube");
         else
            indigo.setOption("rpe-mode", "grid");
         indigo.setOption("rpe-self-reaction", is_self_react_check.getState());
         */
         IndigoObject monomers_table = indigo.createArray();
         if (reaction.countReactants() != mols.size())
            throw new RuntimeException("Number of reactans is not equal to the number of inputs.");
         for (int i = 0; i < reaction.countReactants(); i++)
         {
            IndigoObject array = indigo.createArray();
            if (i < mols.size()) {
               for (IndigoObject m: mols.get(i))
                  array.arrayAdd(m);
            }
            
            monomers_table.arrayAdd(array);
         }
         
         output = indigo.reactionProductEnumerate(reaction, monomers_table);
         outputCount = output.count();
      } finally {
         IndigoPlugin.unlock();
      }
  
      for (int rowNumber = 0; rowNumber < outputCount; rowNumber++) {
         DataCell resultCell = null;
         try {
            IndigoPlugin.lock();
            Format outputFormat = Format.valueOf(nodeSettings.outputType.getStringValue());
            resultCell = IndigoCellFactory.createCell(output.at(rowNumber), null, 
                  getOutputTypeByFormat(outputFormat), IndigoType.REACTION);
         } finally {
            IndigoPlugin.unlock();
         }
         DataCell[] outputCells = new DataCell[outputSpec.getNumColumns()];
         outputCells[0] = resultCell; 

         outputContainer.addRowToTable(new DefaultRow(String.format("Reaction%d", rowNumber),
               outputCells));
         
         exec.checkCanceled();
         exec.setProgress(rowNumber / (double)outputCount,
               "Adding row " + rowNumber);
      }
      
      handleWarningMessages();
      outputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable() };
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
      
      DataTableSpec rspec = inSpecs[IndigoReactionGeneratorSettings.REACTION_PORT];
      DataTableSpec mspec1 = inSpecs[IndigoReactionGeneratorSettings.MOL_PORT1];
      DataTableSpec mspec2 = inSpecs[IndigoReactionGeneratorSettings.MOL_PORT2];
      
      IndigoType reactionType = IndigoType.findByString(nodeSettings.reactionType.getStringValue());
      IndigoType moleculeType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      searchIndigoCompatibleColumn(rspec, nodeSettings.reactionColumn, reactionType);
      searchIndigoCompatibleColumn(mspec1, nodeSettings.molColumn1, moleculeType);
      if (mspec2 != null) {
         searchIndigoCompatibleColumn(mspec2, nodeSettings.molColumn2, moleculeType);
      } else {
         nodeSettings.molColumn2.setStringValue(null);
      }

      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      
      
      return new DataTableSpec[] { getDataTableSpec() };
   }

   private DataTableSpec getDataTableSpec() {

      Format outputFormat = Format.valueOf(nodeSettings.outputType.getStringValue());
      
      DataColumnSpec spec = new DataColumnSpecCreator(nodeSettings.newColName.getStringValue(), 
            getOutputTypeByFormat(outputFormat)).createSpec();
      
      return new DataTableSpec(spec);
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
      IndigoReactionGeneratorSettings s = new IndigoReactionGeneratorSettings();
      s.loadSettingsFrom(settings);
      
      if ((s.newColName.getStringValue() == null) || (s.newColName.getStringValue().length() < 1))
         throw new InvalidSettingsException("No name for new column given");
      if (s.productsCountLimit.getIntValue() < 0)
         throw new InvalidSettingsException("Products count limit cannot be negative");
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
