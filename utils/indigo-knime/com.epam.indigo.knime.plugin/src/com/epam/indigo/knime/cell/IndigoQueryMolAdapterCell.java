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
public class IndigoQueryMolAdapterCell extends AdapterCell implements IndigoQueryMolValue {

   private ByteBuffer byteBuffer;
   private boolean smarts;
   
   /** Constructor to be used in deserializer only */
   @SuppressWarnings("unchecked")
   private IndigoQueryMolAdapterCell(byte[] buffer, boolean smarts) throws IOException {
      super((smarts ? IndigoQueryMolCell.fromSmarts(new String(buffer)) : 
         IndigoQueryMolCell.fromString(new String(buffer))));
      setByteBuffer(ByteBuffer.wrap(buffer));
      setSmarts(smarts);
   }
   
   public static class Serializer extends AdapterCellSerializer<IndigoQueryMolAdapterCell> {

      @Override
      public void serialize(IndigoQueryMolAdapterCell cell,
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
      public IndigoQueryMolAdapterCell deserialize(DataCellDataInput input)
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
         return new IndigoQueryMolAdapterCell(buf, smarts);
      }
      
   }
   
   public static final DataType RAW_TYPE = DataType.getType(IndigoQueryMolAdapterCell.class);
   
   @SuppressWarnings("unchecked")
   public IndigoQueryMolAdapterCell(final IndigoQueryMolCell queryMolCell) {
      super(queryMolCell);
      setByteBuffer(ByteBuffer.wrap(queryMolCell.getBuffer()));
      setSmarts(queryMolCell.isSmarts());
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

   @Override
   public IndigoObject getIndigoObject() throws IndigoException {
      return ((IndigoQueryMolValue)lookupFromAdapterMap(IndigoQueryMolValue.class)).getIndigoObject();
   }

   @Override
   public String toString() {
      return ((IndigoQueryMolValue)lookupFromAdapterMap(IndigoQueryMolValue.class)).toString();
   }
   
   public boolean isSmarts() {
      return smarts;
   }

   public void setSmarts(boolean smarts) {
      this.smarts = smarts;
   }

}
