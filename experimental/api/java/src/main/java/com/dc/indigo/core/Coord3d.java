package com.dc.indigo.core;

import com.dc.builder.annotations.NativeClass;

@NativeClass
public interface Coord3d extends Item {
	float getX();

	void setX(float v);

	float getY();

	void setY(float v);

	float getZ();

	void setZ(float v);

	float getSizeX();

	void setSizeX(float v);

	float getSizeY();

	void setSizeY(float v);

	float getSizeZ();

	void setSizeZ(float v);

	float getRotX();

	void setRotX(float v);

	float getRotY();

	void setRotY(float v);

	float getRotZ();

	void setRotZ(float v);
}
