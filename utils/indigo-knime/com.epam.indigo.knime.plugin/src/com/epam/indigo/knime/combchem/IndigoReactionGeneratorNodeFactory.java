package com.epam.indigo.knime.combchem;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

/**
 * <code>NodeFactory</code> for the "IndigoMoleculeTransform" Node.
 * 
 * 
 * @author
 */
public class IndigoReactionGeneratorNodeFactory extends
      NodeFactory<IndigoReactionGeneratorNodeModel> {

   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoReactionGeneratorNodeModel createNodeModel() {
      return new IndigoReactionGeneratorNodeModel();
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
   public NodeView<IndigoReactionGeneratorNodeModel> createNodeView(
         final int viewIndex, final IndigoReactionGeneratorNodeModel nodeModel) {
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
      return new IndigoReactionGeneratorNodeDialog(new IndigoType[] {IndigoType.QUERY_REACTION}, 
            new IndigoType[] {IndigoType.MOLECULE});
   }

}
