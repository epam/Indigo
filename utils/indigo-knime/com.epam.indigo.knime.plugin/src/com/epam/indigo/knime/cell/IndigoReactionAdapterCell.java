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
public class IndigoReactionAdapterCell extends AdapterCell implements IndigoReactionValue {

   private ByteBuffer byteBuffer;
   
   /** Constructor to be used in deserializer only */
   @SuppressWarnings("unchecked")
   private IndigoReactionAdapterCell(byte[] buffer) throws IOException {
      super(new IndigoReactionCell(buffer));
      setByteBuffer(ByteBuffer.wrap(buffer));
   }

   public static final DataType RAW_TYPE = DataType.getType(IndigoReactionAdapterCell.class);
   
   public static class Serializer extends AdapterCellSerializer<IndigoReactionAdapterCell> {

      @Override
      public void serialize(IndigoReactionAdapterCell cell,
            DataCellDataOutput output) throws IOException {
         byte[] buf = cell.getByteBuffer().array();
         output.writeInt(buf.length);
         output.write(buf);
      }
      
      @Override
      public IndigoReactionAdapterCell deserialize(DataCellDataInput input)
            throws IOException {
         int bufferLength = input.readInt();
         byte[] buffer = new byte[bufferLength];
         input.readFully(buffer);
         return new IndigoReactionAdapterCell(buffer);
      }
      
   }   
   
   @SuppressWarnings("unchecked")
   public IndigoReactionAdapterCell(final IndigoReactionCell reactionCell) {
      super(reactionCell);
      setByteBuffer(ByteBuffer.wrap(reactionCell.getBuffer()));
   }
   
   @Override
   public IndigoObject getIndigoObject() throws IndigoException {
      return ((IndigoReactionValue)lookupFromAdapterMap(IndigoReactionValue.class))
            .getIndigoObject();
   }

   /** Put cells are equal if exact matching is not null */
   @Override
   protected boolean equalsDataCell(DataCell dc) {
      IndigoReactionAdapterCell other = (IndigoReactionAdapterCell)dc;
      return IndigoDataValue.indigoObjectsMatch(getIndigoObject(), 
            other.getIndigoObject());
   }

   /** Put values are equal if exact matching is not null */
   @Override
   protected boolean equalContent(DataValue otherValue) {
      IndigoReactionValue other = (IndigoReactionValue) otherValue;
      return IndigoDataValue.indigoObjectsMatch(getIndigoObject(), 
            other.getIndigoObject());
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
   public String toString() {
      /** Uses toString() method of the IndigoReactionCell */
      return ((IndigoReactionCell)lookupFromAdapterMap(IndigoReactionCell.class)).toString();
   }

}
