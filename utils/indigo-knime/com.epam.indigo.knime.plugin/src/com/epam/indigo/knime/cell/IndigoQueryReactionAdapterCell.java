package com.epam.indigo.knime.cell;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataCellDataInput;
import org.knime.core.data.DataCellDataOutput;
import org.knime.core.data.DataType;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;

@SuppressWarnings("serial")
public class IndigoQueryReactionAdapterCell extends AdapterCell implements IndigoQueryReactionValue {

   private ByteBuffer byteBuffer;
   private boolean smarts;
   
   /** Constructor to be used in deserializer only */
   @SuppressWarnings("unchecked")
   private IndigoQueryReactionAdapterCell(byte[] buffer, boolean smarts) throws IOException {
      super((smarts ? IndigoQueryReactionCell.fromSmarts(new String(buffer)) : 
         IndigoQueryReactionCell.fromString(new String(buffer))));
      setByteBuffer(ByteBuffer.wrap(buffer));
      setSmarts(smarts);
   }
   
   public static final DataType RAW_TYPE = DataType.getType(IndigoQueryReactionAdapterCell.class);
   
   public static class Serializer extends AdapterCellSerializer<IndigoQueryReactionAdapterCell> {

      @Override
      public void serialize(IndigoQueryReactionAdapterCell cell,
            DataCellDataOutput output) throws IOException {
         byte[] buf = cell.getByteBuffer().array();
         if (cell.isSmarts())
            output.writeChar('S');
         else
            output.writeChar('Q');
         output.writeInt(buf.length);
         output.write(buf);
      }
      
      @Override
      public IndigoQueryReactionAdapterCell deserialize(DataCellDataInput input)
            throws IOException {
         boolean smarts;
         char c = input.readChar();
         if (c == 'S')
            smarts = true;
         else if (c == 'Q')
            smarts = false;
         else
            throw new IOException("Cannot deserialize: bad 1-st symbol");
         int buf_len = input.readInt();
         byte[] buf = new byte[buf_len];
         input.readFully(buf);
         return new IndigoQueryReactionAdapterCell(buf, smarts);
      }
      
   }
   
   @SuppressWarnings("unchecked")
   public IndigoQueryReactionAdapterCell(final IndigoQueryReactionCell queryReactionCell) {
      super(queryReactionCell);
      setByteBuffer(ByteBuffer.wrap(queryReactionCell.getBuffer()));
      setSmarts(queryReactionCell.isSmarts());
   }

   @Override
   public IndigoObject getIndigoObject() throws IndigoException {
      return ((IndigoQueryReactionValue)lookupFromAdapterMap(IndigoQueryReactionValue.class)).getIndigoObject();
   }
   
   @Override
   public String toString() {
      return ((IndigoQueryReactionValue)lookupFromAdapterMap(IndigoQueryReactionValue.class)).toString();
   }

   @Override
   protected boolean equalsDataCell(DataCell dc) {
      return false;
   }

   @Override
   public int hashCode() {
      return 0;
   }

   public ByteBuffer getByteBuffer() {
      return byteBuffer;
   }

   public void setByteBuffer(ByteBuffer byteBuffer) {
      this.byteBuffer = byteBuffer;
   }

   public boolean isSmarts() {
      return smarts;
   }

   public void setSmarts(boolean smarts) {
      this.smarts = smarts;
   }

}
