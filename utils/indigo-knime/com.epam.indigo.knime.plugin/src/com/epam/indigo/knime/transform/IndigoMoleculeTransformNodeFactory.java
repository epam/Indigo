package com.epam.indigo.knime.transform;

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
public class IndigoMoleculeTransformNodeFactory extends
      NodeFactory<IndigoMoleculeTransformNodeModel> {

   /**
    * {@inheritDoc}
    */
   @Override
   public IndigoMoleculeTransformNodeModel createNodeModel() {
      return new IndigoMoleculeTransformNodeModel();
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
   public NodeView<IndigoMoleculeTransformNodeModel> createNodeView(
         final int viewIndex, final IndigoMoleculeTransformNodeModel nodeModel) {
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
      return new IndigoMoleculeTransformNodeDialog(new IndigoType[] {IndigoType.MOLECULE},
            new IndigoType[] {IndigoType.QUERY_REACTION});
   }

}
