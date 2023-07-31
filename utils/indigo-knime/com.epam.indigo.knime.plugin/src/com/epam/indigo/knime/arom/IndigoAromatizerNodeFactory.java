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

package com.epam.indigo.knime.arom;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformer;

public class IndigoAromatizerNodeFactory extends
      NodeFactory<IndigoTransformerNodeModel>
{

   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoTransformerNodeModel createNodeModel ()
   {
      return new IndigoTransformerNodeModel("aromatize molecule",
            new IndigoTransformer()
            {
               @Override
               public void transform (IndigoObject io, boolean reaction)
               {
                  io.aromatize();
               }
            });
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
   public NodeView<IndigoTransformerNodeModel> createNodeView (final int viewIndex,
         final IndigoTransformerNodeModel nodeModel)
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
      return new IndigoTransformerNodeDialog("aromatized");
   }
}
