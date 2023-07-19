package com.epam.indigo.knime.rbuilder;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import org.knime.chem.types.SmartsAdapterCell;
import org.knime.chem.types.SmartsCellFactory;
import org.knime.chem.types.SmartsValue;
import org.knime.chem.types.SmilesCellFactory;
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
 * This is the model implementation of IndigoReactionBuilder.
 * 
 */
public class IndigoReactionBuilderNodeModel extends IndigoNodeModel {
   
   
   class ReactionSmilesBuilder {
      public String reactant;
      public String product;
      public String catalyst;
      
      public String toString() {
         StringBuilder smilesBuilder = new StringBuilder();
         if(reactant != null)
            smilesBuilder.append(reactant);
         smilesBuilder.append(">");
         if(catalyst != null)
            smilesBuilder.append(catalyst);
         smilesBuilder.append(">");
         if(product != null)
            smilesBuilder.append(product);
         return smilesBuilder.toString();
      }
   }
   
   abstract public class ReactionRule {
      int colIdx = -1;
      IndigoConverter conv = null;
      abstract void addSelf(IndigoObject resultMol, IndigoObject resultReaction);
      abstract void addSelf(String resultMol, ReactionSmilesBuilder smilesBuilder);
   }
   /*
    * Reaction types building
    */
   public class ReactantReactionRule extends ReactionRule {
      void addSelf(IndigoObject resultMol, IndigoObject resultReaction) {
         resultReaction.addReactant(resultMol);
      }
      void addSelf(String resultMol, ReactionSmilesBuilder smilesBuilder) {
         smilesBuilder.reactant = resultMol;
      }
   }
   public class ProductReactionRule extends ReactionRule {
      void addSelf(IndigoObject resultMol, IndigoObject resultReaction) {
         resultReaction.addProduct(resultMol);
      }
      void addSelf(String resultMol, ReactionSmilesBuilder smilesBuilder) {
         smilesBuilder.product = resultMol;
      }
   }
   public class CatalystReactionRule extends ReactionRule {
      void addSelf(IndigoObject resultMol, IndigoObject resultReaction) {
         resultReaction.addCatalyst(resultMol);
      }
      void addSelf(String resultMol, ReactionSmilesBuilder smilesBuilder) {
         smilesBuilder.catalyst = resultMol;
      }
   }

   private final IndigoReactionBuilderSettings nodeSettings = new IndigoReactionBuilderSettings();

   /**
    * Constructor for the node model.
    */
   protected IndigoReactionBuilderNodeModel() {
      super(1, 2);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected BufferedDataTable[] execute(final BufferedDataTable[] inData,
         final ExecutionContext exec) throws Exception {
      
      BufferedDataTable inputTable = inData[IndigoReactionBuilderSettings.INPUT_PORT];
      DataTableSpec inputSpec = inputTable.getDataTableSpec();
      /*
       * Create output spec
       */
      DataTableSpec outputSpec = getOutputColumnSpecs(inputSpec);
      BufferedDataContainer outputContainer = exec.createDataContainer(outputSpec);
      BufferedDataContainer invalidDataContainer = exec.createDataContainer(inputSpec);
      
      // define indigo input type
      IndigoType indigoType  = IndigoType.findByString(nodeSettings.inputType.getStringValue());      
      // how to treat input string
      boolean treatStringAsSMARTS = nodeSettings.treatStringAsSMARTS.getBooleanValue();
      
      /*
       * Define the rules
       */
      HashMap<SettingsModelBoolean, ReactionRule> reactionRules = new HashMap<SettingsModelBoolean, ReactionRule>();
      reactionRules.put(nodeSettings.addReactants, new ReactantReactionRule());
      reactionRules.put(nodeSettings.addProducts, new ProductReactionRule());
      reactionRules.put(nodeSettings.addCatalysts, new CatalystReactionRule());
      
      HashMap<SettingsModelBoolean, SettingsModelString> columnMap = nodeSettings.getSettingsColumnMap();
      /*
       * Search the columns
       */
      for(SettingsModelBoolean keyMap : columnMap.keySet()) {
         if(keyMap.getBooleanValue()) {
            ReactionRule reactionRule = reactionRules.get(keyMap);
            String keyValue = columnMap.get(keyMap).getStringValue();
            /*
             * Search column index
             */
            reactionRule.colIdx = inputSpec.findColumnIndex(keyValue);
            if (reactionRule.colIdx == -1)
               throw new RuntimeException("column '" + keyValue + "' not found");
            /*
             * Set converter
             */
            
            boolean smarts = StringCellFactory.TYPE.equals(inputSpec.getColumnSpec(reactionRule.colIdx).getType());
            reactionRule.conv = IndigoConverterFactory.createConverter(inputSpec, 
                  reactionRule.colIdx, indigoType, smarts);
            
         }
      }
      
      // result column type
      Format outputFormat = Format.valueOf(nodeSettings.outputType.getStringValue());
      DataType resultType = getOutputTypeByFormat(outputFormat);

      
      IndigoType reacIndigoType = IndigoType.MOLECULE.equals(indigoType) ? IndigoType.REACTION :
         IndigoType.QUERY_REACTION;
      
      Indigo indigo = IndigoPlugin.getIndigo();
      // take into account settins to process input
      indigo.setOption("ignore-stereochemistry-errors", nodeSettings.ignoreStereochemistryErrors.getBooleanValue());
      indigo.setOption("treat-x-as-pseudoatom", nodeSettings.treatXAsPseudoatom.getBooleanValue());

      // Iterate through all the table
      int rowNumber = 1;
      
      boolean smarts = SmartsAdapterCell.RAW_TYPE.equals(resultType);
      boolean reaction = IndigoType.REACTION.equals(reacIndigoType);
      
      for (DataRow inputRow : inputTable) {
         
         // Prepare cells
         int size = inputRow.getNumCells() + 1;
         DataCell[] outputCells = new DataCell[size];
         boolean invalidRow = false;

         // variables to store result reaction
         IndigoObject resultReaction = null;
         ReactionSmilesBuilder smilesBuilder = null;
         
         // init them
         if (smarts) 
            smilesBuilder = new ReactionSmilesBuilder();
         else {
            if (reaction)
               resultReaction = indigo.createReaction();
            else
               resultReaction = indigo.createQueryReaction();
         }
         
         try {
            IndigoPlugin.lock();

            int idx;
            for (idx = 0; idx < inputRow.getNumCells(); idx++)
               outputCells[idx] = inputRow.getCell(idx);
            
            // iterate rules
            for (SettingsModelBoolean keyMap : reactionRules.keySet()) {
               if (keyMap.getBooleanValue()) {
                  ReactionRule reactionRule = reactionRules.get(keyMap);

                  // Prepare data cell
                  DataCell dataCell = reactionRule.conv.convert(outputCells[reactionRule.colIdx]);
                  
                  if (dataCell.isMissing())
                     continue;
                  
                  // Append molecule from cell
                  String src;
                  if (smarts) {
                     src = ((AdapterValue)dataCell).getAdapter(SmartsValue.class).getSmartsValue();
                     reactionRule.addSelf(src, smilesBuilder);
                  } else {
                     IndigoObject inputMol = extractIndigoObject(dataCell, indigoType, treatStringAsSMARTS);
                     // Iterate components
                     for (IndigoObject comp : inputMol.iterateComponents())
                        reactionRule.addSelf(comp.clone(), resultReaction);
                  }
               }
            }
         } catch (Exception e) {
            appendWarningMessage("Could not calculate result reaction for RowId '" + inputRow.getKey()
            + "': " + e.getMessage());
            invalidRow = true;
         } finally {
            IndigoPlugin.unlock();
         }
         
         // create output cell
         if(!invalidRow) {
            try {
               IndigoPlugin.lock();
               
               if (resultReaction != null) {
                  if (Format.String.equals(outputFormat))
                     outputCells[size-1] = StringCellFactory.create(resultReaction.rxnfile());
                  else
                     outputCells[size-1] = IndigoCellFactory.createCell(resultReaction, null, 
                           resultType , reacIndigoType);
               } else {
                  DataCell cell = null;

                  if (smarts) 
                     cell = SmartsCellFactory.createAdapterCell(smilesBuilder.toString());
                  else 
                     cell = SmilesCellFactory.createAdapterCell(smilesBuilder.toString());
                  
                  outputCells[size-1] = reacIndigoType.
                        createAdapterContainingIndigoRepr(cell, true, treatStringAsSMARTS);
               }
            } catch (IndigoException e) {
               appendWarningMessage("can not create result cell: " + e.getMessage());
               invalidRow = true;
            } finally {
               IndigoPlugin.unlock();
            }
         }
         
         // add result row
         if (!invalidRow)
            outputContainer.addRowToTable(new DefaultRow(inputRow.getKey(), outputCells));
         else
            invalidDataContainer.addRowToTable(inputRow);

         exec.checkCanceled();
         exec.setProgress(rowNumber / (double) inputTable.size(),
               "Adding row " + rowNumber);

         rowNumber++;
      }

      handleWarningMessages();
      outputContainer.close();
      invalidDataContainer.close();
      
      return new BufferedDataTable[] { outputContainer.getTable(), invalidDataContainer.getTable()};
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
      
      DataTableSpec dataTableSpec = inSpecs[IndigoReactionBuilderSettings.INPUT_PORT];
      
      IndigoType type = IndigoType.findByString(nodeSettings.inputType.getStringValue());
      
      HashMap<SettingsModelBoolean, SettingsModelString> columnMap = 
            nodeSettings.getSettingsColumnMap();
      
      for (SettingsModelBoolean keyMap : columnMap.keySet()) {
         searchIndigoCompatibleColumn(dataTableSpec, columnMap.get(keyMap), type);
      }
      
      DataTableSpec spec = getOutputColumnSpecs(dataTableSpec);
      
      if(nodeSettings.warningMessage != null) {
         setWarningMessage(nodeSettings.warningMessage);
      }

      return new DataTableSpec[] { spec,  dataTableSpec };
   }
   
   
   private DataTableSpec getOutputColumnSpecs(DataTableSpec inSpec) 
         throws InvalidSettingsException {
      
      int colCount = inSpec.getNumColumns();

      // create output column specs
      DataColumnSpec[] specs = new DataColumnSpec[colCount + 1];

      int colIdx;
      
      // copy specs
      for (colIdx = 0; colIdx < colCount; colIdx++) {
         specs[colIdx] = inSpec.getColumnSpec(colIdx);
      }
      
      // define result column spec
      // define name
      String name = nodeSettings.newColName.getStringValue();
      // define type
      Format outputFormat = Format.valueOf(nodeSettings.outputType.getStringValue());
      DataType newColType = getOutputTypeByFormat(outputFormat);
      
      IndigoType indigoType = IndigoType.findByString(nodeSettings.inputType.getStringValue());

      // gather target column types
      HashMap<SettingsModelBoolean, SettingsModelString> columnMap = 
            nodeSettings.getSettingsColumnMap();
      ArrayList<DataType> colTypes = new ArrayList<>();
      
      for (SettingsModelBoolean keyMap : columnMap.keySet()) {
         if (keyMap.getBooleanValue())
            colTypes.add(inSpec.getColumnSpec(columnMap.get(keyMap).getStringValue()).getType());
      }
      
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
      
      specs[colIdx] = new DataColumnSpecCreator(name, newColType).createSpec();
      
      return new DataTableSpec(specs);
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
      
      IndigoReactionBuilderSettings s = new IndigoReactionBuilderSettings();
      s.loadSettingsFrom(settings);
      
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
