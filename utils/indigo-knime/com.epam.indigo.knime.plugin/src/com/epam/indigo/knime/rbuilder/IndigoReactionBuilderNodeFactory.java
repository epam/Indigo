package com.epam.indigo.knime.rbuilder;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

/**
 * <code>NodeFactory</code> for the "IndigoReactionBuilder" Node.
 * 
 * 
 * @author
 */
public class IndigoReactionBuilderNodeFactory extends
      NodeFactory<IndigoReactionBuilderNodeModel> {

   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoReactionBuilderNodeModel createNodeModel() {
      return new IndigoReactionBuilderNodeModel();
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public int getNrNodeViews() {
      return 0;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeView<IndigoReactionBuilderNodeModel> createNodeView(
         final int viewIndex, final IndigoReactionBuilderNodeModel nodeModel) {
      return null;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public boolean hasDialog() {
      return true;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public NodeDialogPane createNodeDialogPane() {
      return new IndigoReactionBuilderNodeDialog(new IndigoType[] {IndigoType.MOLECULE, IndigoType.QUERY_MOLECULE});
   }

}
