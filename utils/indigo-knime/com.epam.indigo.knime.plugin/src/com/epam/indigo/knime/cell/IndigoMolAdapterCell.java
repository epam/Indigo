package com.epam.indigo.knime.cell;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataCellDataInput;
import org.knime.core.data.DataCellDataOutput;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;

@SuppressWarnings("serial")
public class IndigoMolAdapterCell extends AdapterCell implements IndigoMolValue {

   private ByteBuffer byteBuffer;
   
   /** Constructor to be used in deserializer only */
   @SuppressWarnings("unchecked")
   private IndigoMolAdapterCell(byte[] buffer) throws IOException {
      super(new IndigoMolCell(buffer));
      setByteBuffer(ByteBuffer.wrap(buffer));
   }

   public static final DataType RAW_TYPE = DataType.getType(IndigoMolAdapterCell.class);
   
   public static class Serializer extends AdapterCellSerializer<IndigoMolAdapterCell> {

      @Override
      public void serialize(IndigoMolAdapterCell cell,
            DataCellDataOutput output) throws IOException {
         byte[] buf = cell.getByteBuffer().array();
         output.writeInt(buf.length);
         output.write(buf);
      }
      
      @Override
      public IndigoMolAdapterCell deserialize(DataCellDataInput input)
            throws IOException {
         int bufferLength = input.readInt();
         byte[] buffer = new byte[bufferLength];
         input.readFully(buffer);
         return new IndigoMolAdapterCell(buffer);
      }
      
   }
   
   @SuppressWarnings("unchecked")
   public IndigoMolAdapterCell(final IndigoMolCell molCell) {
      super(molCell);
      setByteBuffer(ByteBuffer.wrap(molCell.getBuffer()));
   }

   
   /** Put cells are equal if exact matching is not null */
   @Override
   protected boolean equalsDataCell(DataCell dc) {
         IndigoMolAdapterCell other = (IndigoMolAdapterCell)dc;

         return IndigoDataValue.indigoObjectsMatch(getIndigoObject(), 
               other.getIndigoObject());
   }
   
   /** Put values are equal if exact matching is not null */
   @Override
   protected boolean equalContent(DataValue otherValue) {
      IndigoMolValue otherVal = (IndigoMolValue) otherValue;

      return IndigoDataValue.indigoObjectsMatch(getIndigoObject(), 
            otherVal.getIndigoObject());
   }

   @Override
   public int hashCode() {
      /** 
       * Such primitive implementation as IndigoObject can be Molecule, Query Molecule, Array, Match, Map
       * and so on. As a result there is nothing stable to rely on for implementation. 
       * */
      return 0;
   } 
   
   public ByteBuffer getByteBuffer() {
      return byteBuffer;
   }

   public void setByteBuffer(ByteBuffer byteBuffer) {
      this.byteBuffer = byteBuffer;
   }

   @Override
   public IndigoObject getIndigoObject() throws IndigoException {
      return ((IndigoMolValue)lookupFromAdapterMap(IndigoMolValue.class))
            .getIndigoObject();
   }

   @Override
   public String toString() {
      /** Uses toString() method of the IndigoMolCell */
      return ((IndigoMolValue)lookupFromAdapterMap(IndigoMolValue.class)).toString();
   }
   
}
