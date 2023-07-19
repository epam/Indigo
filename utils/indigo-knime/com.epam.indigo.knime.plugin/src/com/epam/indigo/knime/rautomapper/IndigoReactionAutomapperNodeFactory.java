package com.epam.indigo.knime.rautomapper;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

import com.epam.indigo.knime.common.types.IndigoType;

/**
 * <code>NodeFactory</code> for the "ReactionAutomapper" Node.
 * 
 *
 * @author 
 */
public class IndigoReactionAutomapperNodeFactory 
        extends NodeFactory<IndigoReactionAutomapperNodeModel> {

    /**
     * {@inheritDoc}
     */
    @Override
    public IndigoReactionAutomapperNodeModel createNodeModel() {
        return new IndigoReactionAutomapperNodeModel();
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
    public NodeView<IndigoReactionAutomapperNodeModel> createNodeView(final int viewIndex,
            final IndigoReactionAutomapperNodeModel nodeModel) {
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
        return new IndigoReactionAutomapperNodeDialog(new IndigoType[] {IndigoType.REACTION, 
              IndigoType.QUERY_REACTION});
    }

}

