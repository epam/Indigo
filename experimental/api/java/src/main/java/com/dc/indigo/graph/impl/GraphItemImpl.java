package com.dc.indigo.graph.impl;

import java.util.Map;

import com.dc.indigo.core.Coord3d;
import com.dc.indigo.core.impl.Coord3dImpl;
import com.dc.indigo.core.impl.ItemImpl;
import com.dc.indigo.core.impl.NativeMapImpl;
import com.dc.indigo.graph.Graph;
import com.dc.indigo.graph.GraphItem;

public class GraphItemImpl extends ItemImpl implements GraphItem {

	protected GraphItemImpl(int thisID) {
		super(thisID);
		// TODO Auto-generated constructor stub
	}

	public Graph getGraph() {
		return new GraphImpl(_getGraph(ID));
	}

	public native String getLabel();

	public native void setLabel(String v);

	public Map<String, String> getStyle() {
		return new NativeMapImpl<>(_getStyle(ID));
	}

	public native boolean getSelected();

	public native void setSelected(boolean v);

	public native boolean getHighlighted();

	public native void setHighlighted(boolean v);

	public Coord3d getCoord3d(){
		return new Coord3dImpl(_getCoord3d(ID));
	}

	private native int _getGraph(int thisID);

	private native _getStyle(int thisID);

	private native _getCoord3d(int thisID);

}
