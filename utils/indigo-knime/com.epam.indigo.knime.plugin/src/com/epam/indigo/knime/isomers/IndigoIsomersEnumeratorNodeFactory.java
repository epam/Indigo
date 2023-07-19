package com.epam.indigo.knime.isomers;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

/**
 * <code>NodeFactory</code> for the "IndigoIsomersEnumerator" Node.
 * 
 *
 * @author 
 */
public class IndigoIsomersEnumeratorNodeFactory 
        extends NodeFactory<IndigoIsomersEnumeratorNodeModel> {

    /**
     * {@inheritDoc}
     */
    @Override
    public IndigoIsomersEnumeratorNodeModel createNodeModel() {
        return new IndigoIsomersEnumeratorNodeModel();
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
    public NodeView<IndigoIsomersEnumeratorNodeModel> createNodeView(final int viewIndex,
            final IndigoIsomersEnumeratorNodeModel nodeModel) {
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
        return new IndigoIsomersEnumeratorNodeDialog(new IndigoType[] {IndigoType.MOLECULE});
    }

}

