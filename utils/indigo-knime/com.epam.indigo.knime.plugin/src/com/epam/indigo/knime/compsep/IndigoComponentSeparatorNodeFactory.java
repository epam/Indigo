package com.epam.indigo.knime.compsep;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoComponentSeparatorNodeFactory extends
      NodeFactory<IndigoComponentSeparatorNodeModel>
{

   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoComponentSeparatorNodeModel createNodeModel()
   {
      return new IndigoComponentSeparatorNodeModel();
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public int getNrNodeViews()
   {
      return 0;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeView<IndigoComponentSeparatorNodeModel> createNodeView(
         final int viewIndex, final IndigoComponentSeparatorNodeModel nodeModel)
   {
      return null;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public boolean hasDialog()
   {
      return true;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeDialogPane createNodeDialogPane()
   {
      return new IndigoComponentSeparatorNodeDialog(new IndigoType[] {IndigoType.MOLECULE});
   }
}
