package com.epam.indigo.knime.common;

import java.util.LinkedList;

import org.knime.chem.types.CMLAdapterCell;
import org.knime.chem.types.InchiAdapterCell;
import org.knime.chem.types.MolAdapterCell;
import org.knime.chem.types.RxnAdapterCell;
import org.knime.chem.types.SdfAdapterCell;
import org.knime.chem.types.SmartsAdapterCell;
import org.knime.chem.types.SmilesAdapterCell;
import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;
import org.knime.core.data.StringValue;
import org.knime.core.data.def.StringCell;
import org.knime.core.data.def.StringCell.StringCellFactory;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeModel;
import org.knime.core.node.defaultnodesettings.SettingsModelString;
import org.knime.core.node.port.PortType;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.cell.IndigoDataValue;
import com.epam.indigo.knime.common.converter.IndigoConverter;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public abstract class IndigoNodeModel extends NodeModel
{
   protected IndigoNodeModel (int nrInDataPorts, int nrOutDataPorts)
   {
      super(nrInDataPorts, nrOutDataPorts);
   }

   protected IndigoNodeModel (final PortType[] inPortTypes,
         final PortType[] outPortTypes)
   {
      super(inPortTypes, outPortTypes);
   }
   
   
   /**
    * This method searches for a fit column to apply desired transformation. Takes the first appropriate one. 
    * Firstly looking for an adaptable to indigo types. If colNameModel is not null, then it will just check the column for validity.
    * @param spec Input tabel spec
    * @param colNameModel Setting model for column
    * @param type Indigo type (molecule, reaction, molecule or reaction query)
    * @return Column name set to column name setting
    * @throws InvalidSettingsException Thrown in case than there is no an appropriate column
    */
   protected String searchIndigoCompatibleColumn(DataTableSpec spec, SettingsModelString colNameModel, IndigoType type) 
         throws InvalidSettingsException {
      
      Class<? extends DataValue> indigoValueClass = type.getIndigoDataValueClass();
      Class<? extends DataValue>[] valueClasses =  type.getClassesConvertableToIndigoDataClass();
      String colName = colNameModel.getStringValue();
      
      // first search for a column adaptable to indigo value class
      if (colName == null) {
         for (DataColumnSpec cs : spec) {
            if (cs.getType().isAdaptable(indigoValueClass)) {
               colName = cs.getName();
               setWarningMessage("Column autoselect: selected column '" + colName + "' as " + indigoValueClass.getName());
               colNameModel.setStringValue(colName);
               break;
            }
         }
      }
      
      // check if there is a column containing an appropriate adapter cells
      if (colName == null) {
         //check if a column type is adaptable to one of value classes
         for (DataColumnSpec cs : spec) {
            if (cs.getType().isAdaptableToAny(valueClasses)) {
               colName = cs.getName();
               setWarningMessage("Column autoselect: selected column '" + colName);
               colNameModel.setStringValue(colName);
               break;
            }
         }
      } 
      
      // if colName still equals null then consider case if a column type is compatible to corresponding value class
      if (colName == null) {
         outer_loop:
         for (DataColumnSpec cs : spec) {
            for (Class<? extends DataValue> cls : valueClasses) {
               if (cs.getType().isCompatible(cls)) {
                  colName = cs.getName();
                  setWarningMessage("Column autoselect: selected column '" + colName + "' as " + cls.getName());
                  colNameModel.setStringValue(colName);
                  break outer_loop;
               }
            }
         }
         if (colName == null)
            throw new InvalidSettingsException("No compatible value class found in the input table");
      } else {
         // make sure colName is valid
         if (!spec.containsName(colName))
            throw new InvalidSettingsException("Column '" + colName + "' does not exist in input table");
         
         // make sure column type (of column with name colName) is compatible to any of approprite
         DataColumnSpec cs = spec.getColumnSpec(colName);
         if (!compatibleToAny(cs, valueClasses)) {
            throw new InvalidSettingsException("Column '" + colName + "' is not compatible to " + type.toString());
         }
      }
      return colName;
   }
   
   /** Check if column's (columnSpec) type is compatible to any of value classes (valueClasses) */
   private boolean compatibleToAny(DataColumnSpec columnSpec, Class<? extends DataValue>[] valueClasses) {
      DataType type = columnSpec.getType();
      for (Class<? extends DataValue> cls : valueClasses) {
         if (type.isCompatible(cls)) {
            return true;
         }
      }
      return false;
   }

   protected void searchIndigoColumn (DataTableSpec spec, SettingsModelString colName, Class<? extends DataValue> cls)
   throws InvalidSettingsException {
      colName.setStringValue(searchIndigoColumn(spec, colName.getStringValue(), cls));
   }
   
   
   protected String searchIndigoColumn (DataTableSpec spec, String colName, Class<? extends DataValue> cls)
   throws InvalidSettingsException
   {
      if (colName == null)
      {
         for (DataColumnSpec cs : spec)
         {
            if (cs.getType().isCompatible(cls))
            {
               if (colName != null)
               {
                  setWarningMessage("Selected default column '" + colName + "' as " + cls.getName());
                  break;
               }
               else
                  colName = cs.getName();
            }
         }
         if (colName == null)
            throw new InvalidSettingsException("No " + cls.getName() + " found in the input table");
      }
      else
      {
         if (!spec.containsName(colName))
            throw new InvalidSettingsException("Column '" + colName + "' does not exist in input table");
         if (!spec.getColumnSpec(colName).getType().isCompatible(cls))
            throw new InvalidSettingsException("Column '" + colName + "' is not a " + cls.getName());
      }
      return colName;
   }

   
   /**
    * Takes input {@link DataTableSpec}, converts a column and returns new spec with the converted column. The converted
    * column is added to the end.
    * @param inSpec {@link DataTableSpec}
    * @param converter {@link IndigoConverter}
    * @param colIdx Column's index
    * @return {@link DataTableSpec} with converted column
    */
   protected DataTableSpec getOutputTableSpec(DataTableSpec inSpec, IndigoConverter converter, int colIdx) {

      if (inSpec == null) {
         throw new RuntimeException("Input data table spec is null");
      }
      
      DataType newColType = converter.getConvertedType();
      
      // get array of types for new data table spec
      DataType[] types = new DataType[inSpec.getNumColumns()];
      for (int i = 0; i < types.length; i++) {
         types[i] = inSpec.getColumnSpec(i).getType();
      }
      types[colIdx] = newColType;

      // create output table spec using old names and column specs but the column with added indigo repr
      DataTableSpec outSpec = new DataTableSpec(inSpec.getColumnNames(), types);
      
      return outSpec;
   }
   
   public enum Format {
      Smiles, Mol, SMARTS, Sdf, CML, InChi, Rxn, String;
   }
   
   
   public DataType getOutputTypeByFormat(Format format) {
      
      DataType type = null;
      
      switch (format) {
      case Smiles:
         type = SmilesAdapterCell.RAW_TYPE;
         break;
      case CML:
         type = CMLAdapterCell.RAW_TYPE;
         break;
      case InChi:
         type = InchiAdapterCell.RAW_TYPE;
         break;
      case Mol:
         type = MolAdapterCell.RAW_TYPE;
         break;
      case Rxn:
         type = RxnAdapterCell.RAW_TYPE;
         break;
      case SMARTS:
         type = SmartsAdapterCell.RAW_TYPE;
         break;
      case Sdf:
         type = SdfAdapterCell.RAW_TYPE;
         break;
      case String:
         type = StringCell.TYPE;
         break;
      default:
         // an impossible case
         break;
      }

      return type;
   }
   
   protected IndigoObject extractIndigoObject(DataCell cell, IndigoType indigoType, boolean treatStringAsSMARTS) 
         throws IndigoException {
      
      IndigoObject io = null;
      Indigo indigo = IndigoPlugin.getIndigo();
      
      if (StringCellFactory.TYPE.equals(cell.getType())) {
         
         String repr = ((StringValue)cell).getStringValue();
         
         switch (indigoType) {
         case MOLECULE:
            io = indigo.loadMolecule(repr);
            break;
         case QUERY_MOLECULE:
            io = treatStringAsSMARTS ? indigo.loadSmarts(repr) : indigo.loadQueryMolecule(repr);
            io.aromatize();
            break;
         case QUERY_REACTION:
            io = treatStringAsSMARTS ? indigo.loadSmarts(repr) : indigo.loadQueryReaction(repr);
            io.aromatize();
            break;
         case REACTION:
            io = indigo.loadReaction(repr);
            break;
         }
         
      } else {
         DataValue adapter = ((AdapterCell) cell).getAdapter(indigoType.getIndigoDataValueClass());
         io = ((IndigoDataValue) adapter).getIndigoObject();
      }
      
      return io;
   }
   
   protected IndigoObject extractIndigoObject(DataCell cell, IndigoType indigoType) 
         throws IndigoException {
      return extractIndigoObject(cell, indigoType, false);
   }
   
   /*
    * Prepare warning message list
    */
   protected final LinkedList<String> warningMessages = new LinkedList<String>();
   public final int MAX_WARNING_SIZE = 10;
   protected void appendWarningMessage(String mes) {
      /*
       * Add only first N messages
       */
      if(warningMessages.size() < MAX_WARNING_SIZE)
         warningMessages.addLast(mes);
      else
         warningMessages.addLast(null);
   }
   
   protected void handleWarningMessages() {
      /*
       * Create warning message
       */
      if(!warningMessages.isEmpty()) {
         StringBuilder warnMessage = new StringBuilder();
         int size = 0;
         for(String mes : warningMessages) {
            if(size < MAX_WARNING_SIZE) {
               if(size > 0)
                  warnMessage.append('\n');
               warnMessage.append(mes);
            } else {
               warnMessage.append('\n');
               warnMessage.append(warningMessages.size() - MAX_WARNING_SIZE);
               warnMessage.append(" other warning messages ...");
               break;
            }
            ++size;
         }
         setWarningMessage(warnMessage.toString());
      }
      // clean warning set
      warningMessages.clear();
      
   }
}
