package com.dc.indigo.graph.impl;


public class GraphItemGeneratedImpl extends com.dc.indigo.core.impl.ItemImpl implements com.dc.indigo.graph.GraphItem {
	
	protected GraphItemGeneratedImpl(int thisID) {
		super(thisID);
	}
 
	public com.dc.indigo.graph.Graph getGraph() {
		return	session.getItem(com.dc.indigo.graph.Graph.class, _getGraph(thisID));
	}
    
	private native int _getGraph(int thisID); //
    
	public void setHighlighted(boolean v) {
			_setHighlighted(thisID, v);
	}
    
	private native void _setHighlighted(int thisID, boolean v); //
    
	public java.util.Map<java.lang.String, java.lang.String> getStyle() {
		return	_getStyle(thisID);
	}
    
	private native java.util.Map<java.lang.String, java.lang.String> _getStyle(int thisID); //
    
	public void setSelected(boolean v) {
			_setSelected(thisID, v);
	}
    
	private native void _setSelected(int thisID, boolean v); //
    
	public boolean getHighlighted() {
		return	_getHighlighted(thisID);
	}
    
	private native boolean _getHighlighted(int thisID); //
    
	public boolean getSelected() {
		return	_getSelected(thisID);
	}
    
	private native boolean _getSelected(int thisID); //
    
	public com.dc.indigo.core.Coord3d getCoord3d() {
		return	session.getItem(com.dc.indigo.core.Coord3d.class, _getCoord3d(thisID));
	}
    
	private native int _getCoord3d(int thisID); //
    
	public void setLabel(java.lang.String v) {
			_setLabel(thisID, v);
	}
    
	private native void _setLabel(int thisID, java.lang.String v); //
    
	public java.lang.String getLabel() {
		return	_getLabel(thisID);
	}
    
	private native java.lang.String _getLabel(int thisID); //
    
}



