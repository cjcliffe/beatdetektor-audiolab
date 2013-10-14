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
    To contact me, e-mail or MSN to cj@cubicproductions.com or by ICQ#7188044
    http://www.cubicproductions.com
*/

#pragma once

/*

	font.h -- Font class

	The font class is for providing a quick way to use an array of bitmaps.

	TODO: convert this to use STL, perhaps drop this class for a better alternative

  */

#include "Bitmap.h"
#include "bit_font.h"

#include <string>

class Font
{
private:
public:

	int width, height, draw_width, draw_height;
	BitmapArray fontlist;

	Font(int width_in, int height_in, unsigned char *font_in);
	Font();

	void string(int xpos, int ypos, const char *str_in);
	void stringShadow(int xpos, int ypos, const char *str_in);
	void stringSpecialShadow(int xpos, int ypos, const char *str_in);
//	void string(int xpos, int ypos, const std::string &str_in);
//	void stringShadow(int xpos, int ypos, const std::string &str_in);
//	void stringSpecialShadow(int xpos, int ypos, const std::string &str_in);

	int hPixelToChar(int pixelh);
	int wPixelToChar(int pixelw);
	
	int getHeight();
	int getWidth();
};

