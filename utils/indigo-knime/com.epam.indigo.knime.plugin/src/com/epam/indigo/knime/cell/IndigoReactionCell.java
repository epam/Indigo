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
import java.nio.ByteBuffer;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataCellDataInput;
import org.knime.core.data.DataCellDataOutput;
import org.knime.core.data.DataCellSerializer;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.plugin.IndigoPlugin;

@SuppressWarnings("serial")
public class IndigoReactionCell extends IndigoDataCell implements IndigoReactionValue
{
   public static class Serializer implements DataCellSerializer<IndigoReactionCell> {
      /**
       * {@inheritDoc}
       */
      public void serialize(final IndigoReactionCell cell, final DataCellDataOutput out) throws IOException {
         byte[] buf = cell.getBuffer();
         out.writeInt(buf.length);
         out.write(buf);
      }

      /**
       * {@inheritDoc}
       */
      public IndigoReactionCell deserialize(final DataCellDataInput input) throws IOException {
         int buf_len = input.readInt();
         byte[] buf = new byte[buf_len];
         input.readFully(buf);
         return new IndigoReactionCell(buf);
      }
   }

   public static final DataType TYPE = DataType.getType(IndigoReactionCell.class);

   public IndigoReactionCell (IndigoObject obj)
   {
      super();

      // Try to serialize to check unexpected configurations: extraordinary charge or etc.
      try
      {
         IndigoPlugin.lock();
         byteBuffer = ByteBuffer.wrap(obj.serialize());
      }
      finally
      {
         IndigoPlugin.unlock();
      }

   }

   public IndigoReactionCell(byte[] buf) {
      super(buf);
   }

   @Override
   public String toString ()
   {
      try
      {
         IndigoPlugin.lock();
         IndigoObject object = getIndigoObject();
         
         // Return a SMILES string if it can be calculated
         try
         {
            return object.smiles();
         }
         catch (IndigoException e)
         {
            // If SMILES is not an option, return the unique Indigo's object ID
            return "<Indigo object #" + object.self + ">";
         }
      }
      catch (IndigoException e)
      {
         return null;
      } finally {
         IndigoPlugin.unlock();
      }
   }

   /** Put cells are equal if exact matching is not null */
   @Override
   protected boolean equalsDataCell (DataCell dc)
   {
      IndigoDataCell other = (IndigoDataCell)dc;
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
   public int hashCode ()
   {
      return 0;
   }

   @Override
   public IndigoObject getIndigoObject() throws IndigoException{
      byte[] buf = getBuffer();
      IndigoObject res;
      try {
         IndigoPlugin.lock();
         res = IndigoPlugin.getIndigo().unserialize(buf);
      } finally {
         IndigoPlugin.unlock();
      }
      return res;
   }
   

}
