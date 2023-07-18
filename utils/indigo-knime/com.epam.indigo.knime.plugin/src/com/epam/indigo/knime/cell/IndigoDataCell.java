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

import java.nio.ByteBuffer;

import org.knime.core.data.DataCell;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;

@SuppressWarnings("serial")
public abstract class IndigoDataCell extends DataCell implements IndigoDataValue {
   protected ByteBuffer byteBuffer;
//   protected IndigoObject _object;
   
   @Override
   abstract public IndigoObject getIndigoObject() throws IndigoException;
   
   protected IndigoDataCell(byte[] buf) {
      byteBuffer = ByteBuffer.wrap(buf);
   }
   
   protected IndigoDataCell() {
   }

   protected byte[] getBuffer() {
      if(byteBuffer == null)
         throw new RuntimeException("internal error: buffer is not initialized correctly");
      
      return byteBuffer.array();
   }
}
//@SuppressWarnings("serial")
//public abstract class IndigoDataCell extends DataCell implements IndigoDataValue {
//
//   protected IndigoObject _object;
//
//   @Override
//   public IndigoObject getIndigoObject() {
//   	return _object;
//   }
//
//   public IndigoDataCell(IndigoObject obj) {
//      _object = obj;
//   }
//}
