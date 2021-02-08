package com.dc.indigo.graph.impl;


public class GraphItemImpl extends com.dc.indigo.core.ItemImpl implements com.dc.indigo.core.Item { 
	public void setLabel(java.lang.String v)) {
void
		return _setLabel(thisID, , v));
	}

	private native void _setLabel(int thisID, , java.lang.String v));;

	public java.lang.String getLabel()) {
java.lang.String
		return _getLabel(thisID, );
	}

	private native java.lang.String _getLabel(int thisID, );;

	public void setHighlighted(boolean v)) {
void
		return _setHighlighted(thisID, , v));
	}

	private native void _setHighlighted(int thisID, , boolean v));;

	public boolean getSelected()) {
boolean
		return _getSelected(thisID, );
	}

	private native boolean _getSelected(int thisID, );;

	public com.dc.indigo.graph.Graph getGraph()) {
		return session.getItem(com.dc.indigo.graph.Graph.class, _getGraph(thisID, )); 
			}

	private native int _getGraph(int thisID, );;

	public boolean getHighlighted()) {
boolean
		return _getHighlighted(thisID, );
	}

	private native boolean _getHighlighted(int thisID, );;

	public void setSelected(boolean v)) {
void
		return _setSelected(thisID, , v));
	}

	private native void _setSelected(int thisID, , boolean v));;

	public com.dc.indigo.core.Coord3d getCoord3d()) {
		return session.getItem(com.dc.indigo.core.Coord3d.class, _getCoord3d(thisID, )); 
			}

	private native int _getCoord3d(int thisID, );;

	public java.util.Map getStyle()) {
java.util.Map
		return _getStyle(thisID, );
	}

	private native java.util.Map _getStyle(int thisID, );;

}



