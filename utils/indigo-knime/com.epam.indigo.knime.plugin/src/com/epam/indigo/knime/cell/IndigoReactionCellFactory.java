package com.epam.indigo.knime.cell;

import java.io.IOException;
import java.io.Reader;
import java.text.ParseException;

import org.apache.commons.io.IOUtils;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataCellFactory.*;
import org.knime.core.node.NodeLogger;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.plugin.IndigoPlugin;

import org.knime.core.data.DataType;

public class IndigoReactionCellFactory implements FromSimpleString, FromComplexString, FromReader {

   private static final NodeLogger LOGGER =
         NodeLogger.getLogger(IndigoReactionCellFactory.class);
   
   /**
    * @param input any of String, RXN, Smiles or CML reaction representation
    * @return Indigo object in case of valid input, null - otherwise;
    * @exception NullPointerException if input string is empty
    */
   private IndigoObject getIndigoObjectFromString(String input) {
      
      if (input == null) {
         throw new NullPointerException("reaction value must not be null");
      }

      IndigoObject io;
      
      try {
         IndigoPlugin.lock();
         
            Indigo indigo = IndigoPlugin.getIndigo();
            io = indigo.loadMolecule(input);
            
      } catch (IndigoException e) {
         LOGGER.warn("cannot create indigo object using this input: " + input, e);
         // this method is used by cell creating method, null will be translated into missing cell
         io = null;
      } finally {
         IndigoPlugin.unlock();
      }
      
      return io;
   }
   
   /** Type for indigo mol cell */
   public static final DataType TYPE = IndigoReactionCell.TYPE;
   
   @Override
   public DataType getDataType() {
      return TYPE;
   }

   @Override
   public DataCell createCell(Reader input) throws ParseException, IOException {
      IndigoObject io = getIndigoObjectFromString(IOUtils.toString(input));
      return createAdapterCell(io);
   }

   @Override
   public DataCell createCell(String input) {
      IndigoObject io = getIndigoObjectFromString(input);
      return createAdapterCell(io);
   }

   /**
    * Creates indigo reaction adapter cell
    * @param io indigo object (Molecule)
    * @return IndigoReactionAdapterCell instance if io != null. MissingCell - otherwise.
    */
   public static DataCell createAdapterCell(final IndigoObject io) {
      DataCell cell = (io == null) ? DataType.getMissingCell() : 
         new IndigoReactionAdapterCell(new IndigoReactionCell(io));
      return cell;
   }
   
}
