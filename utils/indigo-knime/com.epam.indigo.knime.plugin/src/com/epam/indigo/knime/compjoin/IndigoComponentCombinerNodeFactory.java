package com.epam.indigo.knime.compjoin;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

/**
 * <code>NodeFactory</code> for the "IndigoComponentCombiner" Node.
 * 
 *
 * @author 
 */
public class IndigoComponentCombinerNodeFactory 
        extends NodeFactory<IndigoComponentCombinerNodeModel> {

    /**
     * {@inheritDoc}
     */
    @Override
    public IndigoComponentCombinerNodeModel createNodeModel() {
        return new IndigoComponentCombinerNodeModel();
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
    public NodeView<IndigoComponentCombinerNodeModel> createNodeView(final int viewIndex,
            final IndigoComponentCombinerNodeModel nodeModel) {
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
        return new IndigoComponentCombinerNodeDialog(new IndigoType[] {IndigoType.MOLECULE, IndigoType.QUERY_MOLECULE});
    }

}

