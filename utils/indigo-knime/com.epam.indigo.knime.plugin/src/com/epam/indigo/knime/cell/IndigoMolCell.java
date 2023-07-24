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

import com.epam.indigo.*;
import com.epam.indigo.knime.plugin.IndigoPlugin;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.knime.core.data.*;

@SuppressWarnings("serial")
public class IndigoMolCell extends IndigoDataCell implements IndigoMolValue
{

   public static class Serializer implements DataCellSerializer<IndigoMolCell> {
      /**
       * {@inheritDoc}
       */
      public void serialize(final IndigoMolCell cell, final DataCellDataOutput out) throws IOException {
         byte[] buf = cell.getBuffer();
         out.writeInt(buf.length);
         out.write(buf);
      }

      /**
       * {@inheritDoc}
       */
      public IndigoMolCell deserialize(final DataCellDataInput input) throws IOException {
         int buf_len = input.readInt();
         byte[] buf = new byte[buf_len];
         input.readFully(buf);
         return new IndigoMolCell(buf);
      }
   }

   public static final DataType TYPE = DataType.getType(IndigoMolCell.class);

   public IndigoMolCell (IndigoObject obj)
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

   
   public IndigoMolCell(byte[] buf) {
      super(buf);
   }

   @Override
   public String toString ()
   {
      try
      {
         IndigoPlugin.lock();
         IndigoObject obj = getIndigoObject();
         
         // Return a SMILES string if it can be calculated
         try
         {
            return obj.smiles();
         }
         catch (IndigoException e)
         {
            // If SMILES is not an option, return the unique Indigo's object ID
            return "<Indigo object #" + obj.self + ">";
         }
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
      IndigoMolValue otherVal = (IndigoMolValue) otherValue;
      
      return IndigoDataValue.indigoObjectsMatch(getIndigoObject(), 
            otherVal.getIndigoObject());
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
