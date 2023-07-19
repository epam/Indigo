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
import com.epam.indigo.IndigoInchi;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.plugin.IndigoPlugin;

import org.knime.core.data.DataType;

public class IndigoMolCellFactory implements FromSimpleString, FromComplexString, FromReader{

   private static final NodeLogger LOGGER =
         NodeLogger.getLogger(IndigoMolCellFactory.class);
   
   /**
    * @param input any of String, SDF, Smiles, CML, Mol or InChI molecule representation
    * @return Indigo object in case of valid input, null - otherwise;
    * @exception NullPointerException if input string is empty
    */
   private IndigoObject getIndigoObjectFromString(String input) {
      
      if (input == null) {
         throw new NullPointerException("mol value must not be null");
      }
      
      IndigoObject io;
      
      try {
         IndigoPlugin.lock();
         
         if (input.startsWith("InChI=") || input.startsWith("AuxInfo="))
         {
            // case of InChi format
            IndigoInchi inchi = IndigoPlugin.getIndigoInchi();
            io = inchi.loadMolecule(input);
         } else {
            // case of other possible formats
            Indigo indigo = IndigoPlugin.getIndigo();
            io = indigo.loadMolecule(input);
         }
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
   public static final DataType TYPE = IndigoMolCell.TYPE;
   
   @Override
   public DataType getDataType() {
      return TYPE;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public DataCell createCell(String input) {
      IndigoObject io = getIndigoObjectFromString(input);
      return createAdapterCell(io);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public DataCell createCell(Reader input) throws ParseException, IOException {
      IndigoObject io = getIndigoObjectFromString(IOUtils.toString(input));
      return createAdapterCell(io);
   }

   /**
    * Creates indigo mol adapter cell
    * @param io indigo object (Molecule)
    * @return IndigoMolAdapterCell instance if io != null. MissingCell - otherwise.
    */
   public static DataCell createAdapterCell(final IndigoObject io) {
      DataCell cell = (io == null) ? DataType.getMissingCell() : 
         new IndigoMolAdapterCell(new IndigoMolCell(io));
      return cell;
   }

}
