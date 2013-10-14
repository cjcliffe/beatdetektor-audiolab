/*
    This file is part of CubicVR.

    CubicVR is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    CubicVR is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CubicVR; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    
    
    Source (c) 2003 by Charles J. Cliffe unless otherwise specified
    To contact me, e-mail or MSN to cjcliffe@hotmail.com or by ICQ#7188044
*/

#pragma once

/*	
	bitmap.h -- bitmap and bitmapArray class

	The bitmap class is used to manage 2D bitmaps for fonts, sprites, etc

	TODO: change this to STL, perhaps drop this class for a better alternative
	TODO: doxygen commenting
  */



/* single color bitmaps */
class Bitmap 
{
private:
public:

	int width, height;
	unsigned char *bitmapdata;
	
	void set(int width_in, int height_in, unsigned char *bitmapdata_in);
	static Bitmap create(int width, int height, unsigned char *bitmapdata_in);
	void draw(int xpos, int ypos);
};


class BitmapArray
{
private:
public:

	int indexsize;
	int currentindex;
	Bitmap *bitmapdata;

	BitmapArray(int indexsize_in);
	BitmapArray(void);

	void add(Bitmap bitmap_in);
	void addTo(Bitmap bitmap_in, int index);
	

	void removelast(void);
	void removeFromn(int index);

	void destroy(void);

	void draw(int index, int xpos, int ypos);
};
