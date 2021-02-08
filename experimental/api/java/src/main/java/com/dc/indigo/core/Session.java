package com.dc.indigo.core;


public interface Session {

	public com.dc.indigo.graph.Graph getGraph(int thisID);
	public com.dc.indigo.core.Coord3d getCoord3d(int thisID);
	public NativeMap<String, String> getNativeMap(int _getStyle);
}
