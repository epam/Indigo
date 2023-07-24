package com.epam.indigo.knime.rsplitter;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

/**
 * <code>NodeFactory</code> for the "IndigoReactionSplitter" Node.
 * 
 * 
 * @author
 */
public class IndigoReactionSplitterNodeFactory extends
      NodeFactory<IndigoReactionSplitterNodeModel> {

   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoReactionSplitterNodeModel createNodeModel() {
      return new IndigoReactionSplitterNodeModel();
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
   public NodeView<IndigoReactionSplitterNodeModel> createNodeView(
         final int viewIndex, final IndigoReactionSplitterNodeModel nodeModel) {
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
      return new IndigoReactionSplitterNodeDialog(new IndigoType[] {IndigoType.REACTION, 
            IndigoType.QUERY_REACTION});
   }

}
