package com.dc.indigo.graph;

import java.util.Map;

import com.dc.indigo.core.Coord3d;
import com.dc.indigo.core.Item;

public interface GraphItem extends Item {

	Graph getGraph();

	String getLabel();

	void setLabel(String v);

	Map<String, String> getStyle();

	boolean getSelected();

	void setSelected(boolean v);

	boolean getHighlighted();

	void setHighlighted(boolean v);

	Coord3d getCoord3d();
}
