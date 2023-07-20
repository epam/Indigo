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

package com.epam.indigo.knime.valence;

import org.knime.core.node.*;

import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoValenceCheckerNodeFactory extends
      NodeFactory<IndigoValenceCheckerNodeModel>
{

   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoValenceCheckerNodeModel createNodeModel ()
   {
      return new IndigoValenceCheckerNodeModel();
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public int getNrNodeViews ()
   {
      return 0;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeView<IndigoValenceCheckerNodeModel> createNodeView (
         final int viewIndex, final IndigoValenceCheckerNodeModel nodeModel)
   {
      return null;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public boolean hasDialog ()
   {
      return true;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeDialogPane createNodeDialogPane ()
   {
      return new IndigoValenceCheckerNodeDialog(new IndigoType[] {IndigoType.MOLECULE, IndigoType.REACTION});
   }

}
