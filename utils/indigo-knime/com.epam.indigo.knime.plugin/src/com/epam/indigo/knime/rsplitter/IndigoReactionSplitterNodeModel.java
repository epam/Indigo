package com.epam.indigo.knime.rsplitter;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import org.knime.chem.types.CMLAdapterCell;
import org.knime.chem.types.CMLValue;
import org.knime.chem.types.MolAdapterCell;
import org.knime.chem.types.RxnValue;
import org.knime.chem.types.SmartsAdapterCell;
import org.knime.chem.types.SmartsCellFactory;
import org.knime.chem.types.SmartsValue;
import org.knime.chem.types.SmilesAdapterCell;
import org.knime.chem.types.SmilesCellFactory;
import org.knime.chem.types.SmilesValue;
import org.knime.core.data.AdapterValue;
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
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.cell.IndigoCellFactory;
import com.epam.indigo.knime.common.IndigoNodeModel;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.converter.IndigoConverterFactory;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

/**
 * This is the model implementation of IndigoReactionSplitter.
 * 
 * 
 */
public class IndigoReactionSplitterNodeModel extends IndigoNodeModel {
   
   abstract public class ReactionRule {
      int colIdx = -1;
      abstract List<IndigoObject> extractSelf(IndigoObject reaction);
      abstract String extractSelf(String reaction);
   }
   /*
    * Reaction types building
    */
   public class ReactantReactionRule extends ReactionRule {
      List<IndigoObject> extractSelf(IndigoObject reaction) {
         LinkedList<IndigoObject> result = new LinkedList<IndigoObject>();
         for(IndigoObject mol : reaction.iterateReactants())
            result.add(mol.clone());
         return result;
      }
      String extractSelf(String reaction) {
         String result = null;
         String[] splittedReaction = reaction.split(">");
         if(splittedReaction.length > 0 && splittedReaction[0] != null && !splittedReaction[0].isEmpty())
            result = splittedReaction[0];
         return result;
      }
   }
   public class ProductReactionRule extends ReactionRule {
      List<IndigoObject> extractSelf(IndigoObject reaction) {
         LinkedList<IndigoObject> result = new LinkedList<IndigoObject>();
         for(IndigoObject mol : reaction.iterateProducts())
            result.add(mol.clone());
         return result;
      }
      
      String extractSelf(String reaction) {
         String result = null;
         String[] splittedReaction = reaction.split(">");
         if(splittedReaction.length > 2 && splittedReaction[2] != null && !splittedReaction[2].isEmpty())
            result = splittedReaction[2];
         return result;
      }
   }
   public class CatalystReactionRule extends ReactionRule {
      List<IndigoObject> extractSelf(IndigoObject reaction) {
         LinkedList<IndigoObject> result = new LinkedList<IndigoObject>();
         for(IndigoObject mol : reaction.iterateCatalysts())
            result.add(mol.clone());
         return result;
      }
      
      String extractSelf(String reaction) {
         String result = null;
         String[] splittedReaction = reaction.split(">");
         if(splittedReaction.length > 1 && splittedReaction[1] != null && !splittedReaction[1].isEmpty())
            result = splittedReaction[1];
         return result;
      }
   }
   
   private final IndigoReactionSplitterSettings nodeSettings = new IndigoReactionSplitterSettings();

   /**
    * Constructor for the node model.
    */
   protected IndigoReactionSplitterNodeModel() {
      super(1, 1);
      
      // change default value
      nodeSettings.inputType.setStringValue(IndigoType.REACTION.toString());
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute(final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception {
     
      BufferedDataTable inputTable = inData[IndigoReactionSplitterSettings.INPUT_PORT];
      
      DataTableSpec inputSpec = inputTable.getDataTableSpec();
      /*
       * Create output spec
       */
      
      int colIdx = inputSpec.findColumnIndex(nodeSettings.reactionColumn.getStringValue());
      
      if (colIdx == -1)
         throw new Exception("column '" + nodeSettings.reactionColumn.getStringValue() + "' not found");
      
      DataTableSpec[] outputSpecs = getDataTableSpec(inputSpec);
      BufferedDataContainer outputContainer = exec.createDataContainer(outputSpecs[0]);
      
      /*
       * Define the rules
       */
      HashMap<SettingsModelBoolean, ReactionRule> reactionRules = 
            new HashMap<SettingsModelBoolean, ReactionRule>();
      
      reactionRules.put(nodeSettings.extractReactants, new ReactantReactionRule());
      reactionRules.put(nodeSettings.extractProducts, new ProductReactionRule());
      reactionRules.put(nodeSettings.extractCatalysts, new CatalystReactionRule());
      
      final HashMap<SettingsModelBoolean, SettingsModelString> columnMap = nodeSettings
      .getSettingsColumnMap();
      
      /*
       * Create converter for target column
       */
      boolean stringAsSMARTS = nodeSettings.treatStringAsSMARTS.getBooleanValue();
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      IndigoConverter conv = IndigoConverterFactory.createConverter(inputSpec, colIdx, 
            indigoType, stringAsSMARTS);
      
      DataType targetType = inputSpec.getColumnSpec(colIdx).getType();
      DataType resultType = defineColumnsType(targetType);
      boolean isSmarts = targetType.isCompatible(SmartsValue.class);
      boolean isSmiles = targetType.isCompatible(SmilesValue.class);
      /*
       * Iterate through all the table
       */
      int rowNumber = 1;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());
      
      for (DataRow inputRow : inputTable) {
         
         /*
          * Prepare cells
          */
         DataCell[] outputCells = new DataCell[outputSpecs[0].getNumColumns()];
         Arrays.fill(outputCells, DataType.getMissingCell());
         
         int c_idx;
         for (c_idx = 0; c_idx < inputRow.getNumCells(); c_idx++)
            outputCells[c_idx] = inputRow.getCell(c_idx);
         
         DataCell dataCell = inputRow.getCell(colIdx);
         /*
          * Check for missing cell
          */
         if (dataCell.isMissing()) {
            for (SettingsModelBoolean keyMap : columnMap.keySet()) {
               if (keyMap.getBooleanValue()) {
                  /*
                   * Add missing cells
                   */
                  outputCells[c_idx] = DataType.getMissingCell();
                  c_idx++;
               }
            }
         } else {

            try {
               IndigoPlugin.lock();

               // Convert cell
               DataCell newCell = conv.convert(dataCell); 
               IndigoObject inputReaction = extractIndigoObject(newCell, indigoType, stringAsSMARTS);
               
               for (SettingsModelBoolean keyMap : columnMap.keySet()) {
                  if (keyMap.getBooleanValue()) {
                     
                     // For molecules and query molecules
                     IndigoObject resultMolecule = null;
                     // For smiles and smarts and string
                     String resultStr = null;

                     ReactionRule reactionRule = reactionRules.get(keyMap);
                     
                     /*
                      * Create objects
                      */
                     if(targetType.isCompatible(RxnValue.class) ||
                           targetType.isCompatible(CMLValue.class)) {

                        List<IndigoObject> molecules = reactionRule.extractSelf(inputReaction);
                        /*
                         * Merge all components
                         */
                        for(IndigoObject mol : molecules) {
                           if(resultMolecule == null)
                              resultMolecule = mol;
                           else
                              resultMolecule.merge(mol);
                        }
                     } else {
                        /*
                         * Get source for smile or smarts
                         */
                        String src;
                        if (isSmarts) 
                           src = ((AdapterValue)newCell).getAdapter(SmartsValue.class).getSmartsValue();
                        else if (isSmiles)
                           src = ((AdapterValue)newCell).getAdapter(SmilesValue.class).getSmilesValue();
                        else
                           src = ((StringCell)dataCell).getStringValue();
                        
                        resultStr = reactionRule.extractSelf(src);
                     }
                     
                     /*
                      * Create output cells
                      */
                     IndigoType molIndigoType = IndigoType.REACTION.equals(indigoType) ? IndigoType.MOLECULE :
                        IndigoType.QUERY_MOLECULE;
                     
                     if (resultMolecule != null) 
                        outputCells[c_idx] = IndigoCellFactory.createCell(resultMolecule, null, resultType, 
                              molIndigoType);
                     else if (resultStr != null) {
                        DataCell cell = null;
                        if (isSmarts)
                           cell = SmartsCellFactory.createAdapterCell(resultStr);
                        else if (isSmiles)
                           cell = SmilesCellFactory.createAdapterCell(resultStr);
                        else
                           if (stringAsSMARTS){
                              cell = StringCellFactory.create(indigo.loadSmarts(resultStr).molfile());
                           } else
                              cell = StringCellFactory.create(indigo.loadMolecule(resultStr).molfile());
                              
                        if (StringCell.TYPE.equals(resultType))
                           outputCells[c_idx] = cell;
                        else
                           outputCells[c_idx] = molIndigoType.createAdapterContainingIndigoRepr(cell, true, stringAsSMARTS);
                     } else
                        outputCells[c_idx] = DataType.getMissingCell();
                     
                     /*
                      * Increment column index
                      */
                     c_idx++;
                  }
               }
            } catch (Exception e) {
               appendWarningMessage("Could not split reaction for RowId '" + inputRow.getKey() + "': " + e.getMessage());
            } finally {
               IndigoPlugin.unlock();
            }
         }
         /*
          * Add result row
          */
         outputContainer.addRowToTable(new DefaultRow(inputRow.getKey(),  outputCells));

         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) inputTable.size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }
      
      handleWarningMessages();
      outputContainer.close();
      return new BufferedDataTable[] { outputContainer.getTable() };
   }

   private DataTableSpec[] getDataTableSpec(DataTableSpec inSpec) 
         throws InvalidSettingsException {
      
      /*
       * Create Indigo converter for input column
       */
      int colIdx = inSpec.findColumnIndex(nodeSettings.reactionColumn.getStringValue());
      if (colIdx == -1)
         throw new InvalidSettingsException("column not found");
      
      
      /*
       * Define type for new molecule columns
       */
      DataType resultType = defineColumnsType(inSpec.getColumnSpec(colIdx).getType());
      
      final HashMap<SettingsModelBoolean, SettingsModelString> columnMap = nodeSettings
            .getSettingsColumnMap();
      
      /*
       * Prepare specs
       */

      int colCount = inSpec.getNumColumns();
      for (SettingsModelBoolean keyMap : columnMap.keySet())
         if (keyMap.getBooleanValue())
            colCount++;
      
      DataColumnSpec[] specs = new DataColumnSpec[colCount];

      int idx;

      for (idx = 0; idx < inSpec.getNumColumns(); idx++)
         specs[idx] = inSpec.getColumnSpec(idx);
      
      /*
       * Append new specs
       */
      for(SettingsModelBoolean keyMap : columnMap.keySet()) {
         if(keyMap.getBooleanValue()) {
            SettingsModelString keyValue = columnMap.get(keyMap);
            specs[idx] = new DataColumnSpecCreator(keyValue.getStringValue(), resultType).
                  createSpec();
            idx++;
         }
      }
      
      return new DataTableSpec[] {new DataTableSpec(specs)};
   }

   private DataType defineColumnsType(DataType colType) {
      DataType resultType = null;
      
      if (colType.isCompatible(RxnValue.class)) 
         resultType = MolAdapterCell.RAW_TYPE;
      else if (colType.isCompatible(CMLValue.class))
         resultType = CMLAdapterCell.RAW_TYPE;
      else if (colType.isCompatible(SmilesValue.class))
         resultType = SmilesAdapterCell.RAW_TYPE;
      else if (colType.isCompatible(SmartsValue.class))
         resultType = SmartsAdapterCell.RAW_TYPE;
      else 
         resultType = colType;

      return resultType;
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
      
      IndigoType type = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      searchIndigoCompatibleColumn(inSpecs[IndigoReactionSplitterSettings.INPUT_PORT], 
            nodeSettings.reactionColumn, type);
      
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }
      
      return getDataTableSpec(inSpecs[IndigoReactionSplitterSettings.INPUT_PORT]);
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
      
      IndigoReactionSplitterSettings s = new IndigoReactionSplitterSettings();
      s.loadSettingsFrom(settings);
      
      boolean extractionExists = false;
      
      final HashMap<SettingsModelBoolean, SettingsModelString> columnMap = s.getSettingsColumnMap();
      
      for(SettingsModelBoolean keyMap : columnMap.keySet()) 
         extractionExists |= keyMap.getBooleanValue();
      
      if(!extractionExists)
         throw new InvalidSettingsException("output extraction columns were not defined");
      
      for(SettingsModelBoolean keyMap : columnMap.keySet()) {
         SettingsModelString keyValue = columnMap.get(keyMap);
         if(keyMap.getBooleanValue() && (keyValue.getStringValue() == null || keyValue.getStringValue().length() < 1))
            throw new InvalidSettingsException("'" + keyValue.getKey() + "' column name can not be empty");
      }
      
      
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
