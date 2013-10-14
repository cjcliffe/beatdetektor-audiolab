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

#include "Font.h"
#include <GLUT/glut.h>

Font::Font(int width_in, int height_in, unsigned char *font_in)
{
	int i;
	
	fontlist = BitmapArray(255);
		
	draw_width = width = width_in;
	draw_height = height = height_in;

	for (i = 0; i < 255; i++)
	{
		fontlist.add(Bitmap::create(width_in, height_in, font_in+i*width*height));
	}
}


Font::Font()
{
	Font(8,8, (unsigned char *)bit_font);
}

void Font::string(int xpos, int ypos, const char *str_in)
{
	int len = strlen(str_in);

	for (int i = 0; i < len; i++)
	{
		fontlist.draw(str_in[i], xpos + (i * draw_width), ypos);
	}
}


//void Font::string(int xpos, int ypos, const std::string &str_in)
//{
//	string(xpos,ypos,str_in.c_str());
//}

void Font::stringSpecialShadow(int xpos, int ypos, const char *str_in)
{
	glColor3f(0.5f,0.5f,0.5f);

	for(int y = -2; y <= 2; y++)
		for(int x = -2; x <= 2; x++)
		{
			Font::string(xpos+x, ypos+y, str_in);
		}

	glColor3f(1.0f,1.0f,1.0f);
	Font::string(xpos, ypos, str_in);
}

//void Font::stringSpecialShadow(int xpos, int ypos, const std::string &str_in)
//{
//	Font::stringSpecialShadow(xpos,ypos,str_in.c_str());	
//}

void Font::stringShadow(int xpos, int ypos, const char *str_in)
{
	glColor3f(0.0f,0.0f,0.0f);
	Font::string(xpos+2, ypos-2, str_in);
	glColor3f(1.0f,1.0f,1.0f);
	Font::string(xpos, ypos, str_in);
}


//void Font::stringShadow(int xpos, int ypos, const std::string &str_in)
//{
//	Font::stringShadow(xpos,ypos,str_in.c_str());	
//}


int Font::getHeight()
{
	return draw_height;
}


int Font::getWidth()
{
	return draw_width;
}


int Font::hPixelToChar(int pixelh)
{
	return pixelh/height;
}


int Font::wPixelToChar(int pixelw)
{
	return pixelw/width;
}
