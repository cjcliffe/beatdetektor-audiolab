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

/* 2D Bitmap class for Font class*/
#include "Bitmap.h"
#include <string>
#include <GLUT/glut.h>


/* Bitmap class */
void Bitmap::set(int width_in, int height_in, unsigned char *bitmapdata_in)
{
	width = width_in;
	height = height_in;
	bitmapdata = bitmapdata_in;

}


/* width and height must be divisible by 8 */
Bitmap Bitmap::create(int width, int height, unsigned char *bitmapdata_in)
{
	Bitmap t_bitmap;
	int i, j, k;
	unsigned long q;

	//	char bit_list[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	
	unsigned long *bitmapdata_t;


	t_bitmap.width = width;
	t_bitmap.height = height;

	t_bitmap.bitmapdata = (unsigned char *)std::malloc(sizeof(unsigned long)*height);
	bitmapdata_t = (unsigned long *)std::malloc(sizeof(unsigned long)*height);
	

	memset(bitmapdata_t,0,sizeof(unsigned long)*height);

	for (j = 0; j < height; j++)
		for (i = 0; i < (width/8); i++)
		{
			q = 1;
			for (k = 0; k < width; k++)
			{
				if (*(bitmapdata_in+(j*width+width-k))) 
					*(bitmapdata_t+((height-1)-j)) += q;
				q *= 2;
			}
		}
	

		memcpy(t_bitmap.bitmapdata,bitmapdata_t,sizeof(unsigned long)*height);
		
		return t_bitmap;
}


void Bitmap::draw(int xpos, int ypos)
{
#ifndef OPENGL_ES
#ifndef ARCH_PSP
	glPixelTransferi(GL_UNPACK_ALIGNMENT, 4);
	glRasterPos2i(xpos, ypos);
	glBitmap(width, height, 0, 0, 0, 0, bitmapdata);
#else
#warning PSP Architecture -- need pixel transfer / texture load equivalent
#endif
#endif
#ifdef OPENGL_ES
#warning no pixel transfer for drawing bitmaps
#endif
}




/* BitmapArray class */

BitmapArray::BitmapArray(int indexsize_in)
{

	indexsize = indexsize_in;
	bitmapdata = (Bitmap *)std::malloc(sizeof(Bitmap)*indexsize);
	currentindex = 0;

}


BitmapArray::BitmapArray(void)
{
	BitmapArray(255);
}

void BitmapArray::add(Bitmap bitmap_in)
{
	if (currentindex < indexsize)
	{
		bitmapdata[currentindex] = bitmap_in;
		currentindex++;
	}
	

}



void BitmapArray::addTo(Bitmap bitmap_in, int index)
{

	if ((index < indexsize) && (index >= 0))
	{
		bitmapdata[currentindex] = bitmap_in;
		currentindex++;
	}

}



void BitmapArray::removelast(void)
{
	if(currentindex > 0)
	{
		std::free(bitmapdata[currentindex].bitmapdata);
		currentindex--;
	}
}



void BitmapArray::removeFromn(int index)
{
	if((index >= 0) && (index < indexsize))
	{
		std::free(bitmapdata[currentindex].bitmapdata);
		currentindex--;
	}
}


void BitmapArray::destroy(void)
{
	std::free(bitmapdata);
	indexsize = 0;

}


void BitmapArray::draw(int index, int xpos, int ypos)
{

	bitmapdata[index].draw(xpos,ypos);
}
