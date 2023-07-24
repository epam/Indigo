package com.epam.indigo.knime.layout;


import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeModel;
import com.epam.indigo.knime.common.transformer.IndigoTransformer;

/**
 * <code>NodeFactory</code> for the "IndigoLayout2D" Node.
 * 
 *
 * @author 
 */
public class IndigoLayout2DNodeFactory 
        extends NodeFactory<IndigoTransformerNodeModel> {

    /**
     * {@inheritDoc}
     */
    @Override
    public IndigoTransformerNodeModel createNodeModel() {
       return new IndigoTransformerNodeModel("2d coordinates", new IndigoTransformer() {
               @Override
               public void transform(IndigoObject io, boolean reaction) {
                  io.layout();
               }
            });
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
    public NodeView<IndigoTransformerNodeModel> createNodeView(final int viewIndex,
            final IndigoTransformerNodeModel nodeModel) {
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
        return new IndigoTransformerNodeDialog("with coordinates");
    }

}

