package com.dc.indigo.core;

/**
 * 
 * @author Dmitrii_Chernov
 * 
 * 	X, Y, Z  - center (diagonals intersection point) of circumscribed parallepiped
 *  sizeX, sizeY, sizeZ  - dimensions of circumscribed parallepiped
 *  rotX, rotY, rotZ  - rotation of circumscribed parallepiped
 */
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
