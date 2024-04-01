package com.epam.indigo.knime.io.rxnreader;

import org.knime.core.node.NodeDialogPane;
import org.knime.core.node.NodeFactory;
import org.knime.core.node.NodeView;

public class RXNReaderNodeFactory extends NodeFactory<RXNReaderNodeModel> {

	@Override
	public RXNReaderNodeModel createNodeModel() {
		return new RXNReaderNodeModel();
	}

	@Override
	protected int getNrNodeViews() {
		return 0;
	}

	@Override
	public NodeView<RXNReaderNodeModel> createNodeView(int viewIndex,
			RXNReaderNodeModel nodeModel) {
		return null;
	}

	@Override
	protected boolean hasDialog() {
		return true;
	}

	@Override
	protected NodeDialogPane createNodeDialogPane() {
		return new RXNReaderNodeDialog();
	}

}
