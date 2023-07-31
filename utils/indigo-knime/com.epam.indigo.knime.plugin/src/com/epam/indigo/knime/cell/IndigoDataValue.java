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

import org.knime.core.data.DataValue;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.plugin.IndigoPlugin;

public interface IndigoDataValue extends DataValue {

	public abstract IndigoObject getIndigoObject() throws IndigoException;
	
   /** Compare two indigo objects by mean of exact matching checking */
   public static boolean indigoObjectsMatch(IndigoObject io1, IndigoObject io2) {
      
      try {
         IndigoPlugin.lock();
         
         IndigoObject match = IndigoPlugin.getIndigo().exactMatch(io1, io2);
         
         if (match != null)
            return true;
      }
      catch (IndigoException e)
      {
         // ignore the exception; default to the false result
      }
      finally
      {
         IndigoPlugin.unlock();
      }
      
      return false;
      
   }

}