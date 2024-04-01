/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems, Inc.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 ***************************************************************************/

package com.epam.indigo.knime.cell;

import java.io.IOException;

import org.knime.core.data.*;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.plugin.IndigoPlugin;

@SuppressWarnings("serial")
public class IndigoQueryMolCell extends IndigoDataCell implements IndigoQueryMolValue
{
   public static class Serializer implements DataCellSerializer<IndigoQueryMolCell>
   {
      public void serialize(final IndigoQueryMolCell cell, final DataCellDataOutput out) throws IOException {
         byte[] buf = cell.getBuffer();
         if (cell.isSmarts())
            out.writeChar('S');
         else
            out.writeChar('Q');
         
         out.writeInt(buf.length);
         out.write(buf);
      }

      /**
       * {@inheritDoc}
       */
      public IndigoQueryMolCell deserialize(final DataCellDataInput input) throws IOException {
         boolean smarts;
         char c = input.readChar();
         if (c == 'S')
            smarts = true;
         else if (c == 'Q')
            smarts = false;
         else
            throw new IOException("cannot deserialize: bad 1-st symbol");
         int buf_len = input.readInt();
         byte[] buf = new byte[buf_len];
         input.readFully(buf);
         
         return new IndigoQueryMolCell(buf, smarts);
      }
      
   }

   private boolean smarts;
   
   public static final DataType TYPE = DataType.getType(IndigoQueryMolCell.class);

   public String getSource ()
   {
      return new String(byteBuffer.array());
   }
   
   public boolean isSmarts ()
   {
      return smarts;
   }
   
   private IndigoQueryMolCell(byte[] buf, boolean smarts) {
      super(buf);
      this.smarts = smarts;
   }

   public static IndigoQueryMolCell fromString(String str) {
      IndigoQueryMolCell res = new IndigoQueryMolCell(str.getBytes(), false);
      /*
       * Check correctness 
       */
      res.getIndigoObject();
      return res;
   }
   
   public static IndigoQueryMolCell fromSmarts(String str) {
      IndigoQueryMolCell res = new IndigoQueryMolCell(str.getBytes(), true);
      /*
       * Check correctness 
       */
      res.getIndigoObject();
      return res;
   }

   @Override
   public String toString ()
   {
      try
      {
         IndigoPlugin.lock();
         IndigoObject obj = getIndigoObject();
         
         // Return the name if it is present
         if (obj.name() != null && obj.name().length() > 0)
            return obj.name();
         
         // Otherwise, return the unique Indigo's object ID
         return "<Indigo object #" + obj.self + ">";
      }
      catch (IndigoException e)
      {
         return null;
      }
      finally
      {
         IndigoPlugin.unlock();
      }
   }

   @Override
   protected boolean equalsDataCell (DataCell dc)
   {
      return false;
   }

   @Override
   public int hashCode ()
   {
      return 0;
   }

   @Override
   public IndigoObject getIndigoObject() throws IndigoException {
      Indigo indigo = IndigoPlugin.getIndigo();
      IndigoObject res = null;
      byte[] buf = getBuffer();
      try {
         IndigoPlugin.lock();
         indigo.setOption("ignore-stereochemistry-errors", true);
         if(smarts)
            res = indigo.loadSmarts(buf);
         else
            res = indigo.loadQueryMolecule(buf);
         res.aromatize();
      } finally {
         IndigoPlugin.unlock();
      }
      return res;
   }

}
