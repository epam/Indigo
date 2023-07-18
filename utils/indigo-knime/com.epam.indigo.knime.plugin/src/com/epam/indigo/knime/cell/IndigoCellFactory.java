package com.epam.indigo.knime.cell;

import org.knime.chem.types.CMLCellFactory;
import org.knime.chem.types.CMLValue;
import org.knime.chem.types.InchiCellFactory;
import org.knime.chem.types.InchiValue;
import org.knime.chem.types.MolCellFactory;
import org.knime.chem.types.MolValue;
import org.knime.chem.types.RxnCellFactory;
import org.knime.chem.types.RxnValue;
import org.knime.chem.types.SdfCellFactory;
import org.knime.chem.types.SdfValue;
import org.knime.chem.types.SmartsCellFactory;
import org.knime.chem.types.SmartsValue;
import org.knime.chem.types.SmilesCellFactory;
import org.knime.chem.types.SmilesValue;
import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataType;
import org.knime.core.data.StringValue;
import org.knime.core.data.def.StringCell;
import org.knime.core.data.def.StringCell.StringCellFactory;

import com.epam.indigo.IndigoInchi;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.types.IndigoType;
import com.epam.indigo.knime.plugin.IndigoPlugin;

/**
 * Well, this class is needed to create adapter cells containing Indigo representation in case
 * when it matters to save original type it came from. Used when original cell's value was changed.
 *
 */
public class IndigoCellFactory {

   /**
    * This method creates new adapter cell from an indigo object or string smarts representation 
    * taking into account original type of cell. In case of input original type is string, this 
    * method returns string cell .
    * @param io {@link IndigoObject} of a chemical structure
    * @param smarts Chemical structure written in SMARTS format. NOTE: can be null
    * @param originType {@link DataType} of original cell the {@link IndigoObject} was created from
    * @param indigoType {@link IndigoType} to create
    * @param treatStringAsSMARTS if false then string is treated as Smiles
    * @return New adapter cell with original and indigo representation inside.
    */
   @SuppressWarnings("unchecked")
   public static DataCell createCell(IndigoObject io, String smarts, DataType originType, 
         IndigoType indigoType, boolean treatStringAsSMARTS) {
      
      boolean isSmarts = (smarts != null);

      DataCell cell = null;
      String repr = "";

      // create 'normal' adapter cell from the Indigo object
      if (originType.isCompatible(MolValue.class)) {
         repr = io.molfile();
         cell = MolCellFactory.createAdapterCell(repr);
      } else if (originType.isCompatible(SdfValue.class)) {
         repr = io.molfile() + "\n$$$$\n";
         cell = SdfCellFactory.createAdapterCell(repr);
      } else if (originType.isCompatible(CMLValue.class)) {
         repr = io.cml();
         cell = CMLCellFactory.createAdapterCell(repr);
      } else if (originType.isCompatible(SmilesValue.class)) {
         repr = io.smiles();
         cell = SmilesCellFactory.createAdapterCell(repr);
      } else if (originType.isCompatible(InchiValue.class)) {
         IndigoInchi inchi = IndigoPlugin.getIndigoInchi();
         repr = inchi.getInchi(io);
         cell = InchiCellFactory.createAdapterCell(repr);
      } else if (originType.isCompatible(RxnValue.class)) {
         repr = io.rxnfile();
         cell = RxnCellFactory.createAdapterCell(repr);
      }  else if (originType.isCompatible(SmartsValue.class)) {
         assert(smarts != null); // if it's smarts, then smarts mustn't be null
         repr = smarts;
         cell = SmartsCellFactory.createAdapterCell(repr);
      } else if (originType.isCompatible(StringValue.class) && isSmarts && treatStringAsSMARTS) {
         repr = smarts; 
         isSmarts = true;            
         cell = SmartsCellFactory.createAdapterCell(repr);
      } else if (originType.isCompatible(StringValue.class)){
         if (IndigoType.MOLECULE.equals(indigoType) || IndigoType.QUERY_MOLECULE.equals(indigoType))
            repr = io.molfile();
         else
            repr = io.rxnfile();
         cell = StringCellFactory.create(repr);
      }
      
      
      // add indigo adapter cell if cell does not have string type
      if (cell != null && !cell.getType().equals(StringCell.TYPE))
         if (IndigoType.MOLECULE.equals(indigoType)) {
            cell = ((AdapterCell) cell).cloneAndAddAdapter(IndigoMolCellFactory.createAdapterCell(io), IndigoMolValue.class);
         }
         else if (IndigoType.REACTION.equals(indigoType)) {
            cell = ((AdapterCell) cell).cloneAndAddAdapter(IndigoReactionCellFactory.createAdapterCell(io), IndigoReactionValue.class);
         }
         else if (IndigoType.QUERY_MOLECULE.equals(indigoType)) {
            cell = ((AdapterCell) cell).cloneAndAddAdapter(IndigoQueryMolCellFactory.createAdapterCell(repr, isSmarts), IndigoQueryMolValue.class);
         }
         else if (IndigoType.QUERY_REACTION.equals(indigoType)) {
            cell = ((AdapterCell) cell).cloneAndAddAdapter(IndigoQueryReactionCellFactory.createAdapterCell(repr, isSmarts), IndigoQueryReactionValue.class);
         }
            
      
      return cell;
   }
   
   public static DataCell createCell(IndigoObject io, String smarts, DataType originType, 
         IndigoType indigoType) {
      return createCell(io, smarts, originType, indigoType, false);
   }
   
}
