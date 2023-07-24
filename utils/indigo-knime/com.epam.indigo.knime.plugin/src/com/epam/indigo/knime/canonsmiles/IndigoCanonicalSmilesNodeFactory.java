package com.epam.indigo.knime.canonsmiles;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoCanonicalSmilesNodeFactory extends NodeFactory<IndigoCanonicalSmilesNodeModel> {

   @Override
   public IndigoCanonicalSmilesNodeModel createNodeModel() {
      return new IndigoCanonicalSmilesNodeModel();
   }

   @Override
   protected int getNrNodeViews() {
      return 0;
   }

   @Override
   public NodeView<IndigoCanonicalSmilesNodeModel> createNodeView(int viewIndex,
         IndigoCanonicalSmilesNodeModel nodeModel) {
      return null;
   }

   @Override
   protected boolean hasDialog() {
      return true;
   }

   @Override
   protected NodeDialogPane createNodeDialogPane() {
      return new IndigoCanonicalSmilesNodeDialog(new IndigoType[] {IndigoType.MOLECULE, IndigoType.REACTION});
   }

}
