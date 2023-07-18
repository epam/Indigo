package com.epam.indigo.knime.inchi;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

public class IndigoInchiNodeFactory extends NodeFactory<IndigoInchiNodeModel> {

   @Override
   public IndigoInchiNodeModel createNodeModel() {
      return new IndigoInchiNodeModel();
   }

   @Override
   protected int getNrNodeViews() {
      return 0;
   }

   @Override
   public NodeView<IndigoInchiNodeModel> createNodeView(int viewIndex,
         IndigoInchiNodeModel nodeModel) {
      return null;
   }

   @Override
   protected boolean hasDialog() {
      return true;
   }

   @Override
   protected NodeDialogPane createNodeDialogPane() {
      return new IndigoInchiNodeDialog(new IndigoType[] {IndigoType.MOLECULE});
   }

}
