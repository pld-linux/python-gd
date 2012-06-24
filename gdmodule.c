/***********************************************************
Copyright 1995 Richard Jones, Bureau of Meteorology Australia.
richard.jones@bom.gov.au

This module is a python wrapper for the GD library (version 1.1.1).
From the GD docs:
"COPYRIGHT 1994 BY THE QUEST CENTER AT COLD SPRING HARBOR LABS. 
Permission granted for unlimited use, provided that 
the Quest Center at Cold Spring Harbor Labs is given
credit for the library in the user-visible documentation of 
your software. If you modify gd, we ask that you share the
modifications with us so they can be added to the
distribution. See gd.html for details.
"

version 0.22

******************************************************************/

#include <Python.h>
#include <gd.h>
#include <gdfonts.h>
#include <gdfontl.h>
#include <gdfontmb.h>
#include <gdfontt.h>
#include <gdfontg.h>
#include <string.h>
#include <errno.h>

static PyObject *ErrorObject;
extern int errno;

/* DOCSTRING */
static char *gdModuleDocString = "GD module is an interface to the GD library written by Thomas Bouttel.\n\
\'It allows your code to quickly draw images complete with lines, arcs,\n\
text, multiple colors, cut and paste from other images, and flood fills,\n\
and write out the result as a .GIF file. This is particularly useful in\n\
World Wide Web applications, where .GIF is the format used for inline images.\'\n\
It has been extended in some ways from the original GD library.";

/*
** Declarations for objects of type image
*/

typedef struct i_o {
	PyObject_HEAD
	gdImagePtr imagedata;
	int multiplier_x,origin_x;
	int multiplier_y,origin_y;
	struct i_o *current_brush;
	struct i_o *current_tile;
} imageobject;

typedef struct {
	char *name;
	gdFontPtr data;
} fontstruct;

extern gdFont gdFontTinyRep;
extern gdFont gdFontSmallRep;
extern gdFont gdFontMediumBoldRep;
extern gdFont gdFontLargeRep;
extern gdFont gdFontGiantRep;

static fontstruct fonts[] = {{"gdFontTiny",&gdFontTinyRep},
{"gdFontSmall",&gdFontSmallRep},
{"gdFontMediumBold",&gdFontMediumBoldRep},
{"gdFontLarge",&gdFontLargeRep},
{"gdFontGiant",&gdFontGiantRep},
{NULL,NULL}};

staticforward PyTypeObject Imagetype;

#define is_imageobject(v)		((v)->ob_type == &Imagetype)

#define MIN(x,y) ((x)<(y)?(x):(y))
#define X(x) ((x)*self->multiplier_x+self->origin_x)
#define Y(y) ((y)*self->multiplier_y+self->origin_y)
#define W(x) ((x)*self->multiplier_x)
#define H(y) ((y)*self->multiplier_y)

static imageobject *newimageobject();

/*
** Methods for the image type
*/
static PyObject *image_writegif(self, args)
	imageobject *self;
	PyObject *args;
{
	char *filename;
	PyObject *fileobj;
	FILE *fp;

	if (PyArg_ParseTuple(args, "O!", &PyFile_Type, &fileobj))
	{
		gdImageGif(self->imagedata, PyFile_AsFile(fileobj));
	}
	else if (PyErr_Clear(), PyArg_ParseTuple(args, "z", &filename))
	{
		if (fp = fopen(filename, "wb")) gdImageGif(self->imagedata, fp);
		else
		{
			PyErr_SetFromErrno(PyExc_IOError);
			return NULL;
		}
		fclose(fp);
	}
	else return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_writegd(self, args)
	imageobject *self;
	PyObject *args;
{
	char *filename;
	PyObject *fileobj;
	FILE *fp;

	if (PyArg_ParseTuple(args, "O!", &PyFile_Type, &fileobj))
	{
		gdImageGd(self->imagedata, PyFile_AsFile(fileobj));
	}
	else if (PyErr_Clear(), PyArg_ParseTuple(args, "z", &filename))
	{
		if (fp = fopen(filename, "wb")) gdImageGd(self->imagedata, fp);
		else
		{
			PyErr_SetFromErrno(PyExc_IOError);
			return NULL;
		}
		fclose(fp);
	}
	else return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_setpixel(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y,color;

	if (!PyArg_ParseTuple(args, "(ii)i", &x, &y, &color)) return NULL;
	gdImageSetPixel(self->imagedata, X(x), Y(y), color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_line(self, args)
	imageobject *self;
	PyObject *args;
{
	int sx,sy,ex,ey,color;

	if (!PyArg_ParseTuple(args, "(ii)(ii)i", &sx, &sy, &ex, &ey, &color)) return NULL;
	gdImageLine(self->imagedata, X(sx), Y(sy), X(ex), Y(ey), color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_lines(self, args)
	imageobject *self;
	PyObject *args;
{
	PyObject *point, *points, *bit;
	int color, i, x,y,ox,oy;

	if (!PyArg_ParseTuple(args, "O!i", &PyTuple_Type, &points, &color))
	{
		PyErr_Clear();
		if (PyArg_ParseTuple(args, "O!i", &PyList_Type, &points, &color))
			points = PyList_AsTuple(points);
		else return NULL;
	}

	point = PyTuple_GET_ITEM(points,0);
	ox = PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,0));
	oy = PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,1));
	for (i=1; i<PyTuple_Size(points); i++)
	{
		point = PyTuple_GET_ITEM(points,i);
		x = PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,0));
		y = PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,1));
		gdImageLine(self->imagedata, X(ox), Y(oy), X(x), Y(y), color);
		ox=x;oy=y;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_polygon(self, args)
	imageobject *self;
	PyObject *args;
{
	PyObject *point, *points, *bit;
	gdPointPtr gdpoints;
	int size, color, i, fillcolor=-1;

	if (!PyArg_ParseTuple(args, "O!i|i", &PyTuple_Type, &points, &color, &fillcolor))
	{
		PyErr_Clear();
		if (PyArg_ParseTuple(args, "O!i|i", &PyList_Type, &points, &color, &fillcolor))
			points=PyList_AsTuple(points);
		else return NULL;
	}

	size = PyTuple_Size(points);
	gdpoints = (gdPointPtr)calloc(size,sizeof(gdPoint));
	for (i=0; i<size; i++)
	{
		point = PyTuple_GET_ITEM(points,i);
		gdpoints[i].x = X(PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,0)));
		gdpoints[i].y = Y(PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,1)));
	}
	if (fillcolor != -1) gdImagePolygon(self->imagedata, gdpoints, size, color);
	gdImagePolygon(self->imagedata, gdpoints, size, color);
	free(gdpoints);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_rectangle(self, args)
	imageobject *self;
	PyObject *args;
{
	int tx,ty,bx,by,t,color,fillcolor,fill=0;

	if (PyArg_ParseTuple(args, "(ii)(ii)ii", &tx, &ty, &bx, &by, &color, &fillcolor)) fill=1;
	else if (PyErr_Clear(), !PyArg_ParseTuple(args, "(ii)(ii)i", &tx, &ty, &bx, &by, &color)) return NULL;
	tx = X(tx); ty = Y(ty);
	bx = X(bx); by = Y(by);
	if (tx > bx) {t=tx;tx=bx;bx=t;}
	if (ty > by) {t=ty;ty=by;by=t;}
	if (fill) gdImageFilledRectangle(self->imagedata, tx, ty, bx, by, fillcolor);
	gdImageRectangle(self->imagedata, tx, ty, bx, by, color);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_filledpolygon(self, args)
	imageobject *self;
	PyObject *args;
{
	PyObject *point, *points, *bit;
	gdPointPtr gdpoints;
	int size, color, i;

	if (!PyArg_ParseTuple(args, "O!i", &PyTuple_Type, &points, &color))
	{
		PyErr_Clear();
		if (PyArg_ParseTuple(args, "O!i", &PyList_Type, &points, &color))
			points=PyList_AsTuple(points);
		else return NULL;
	}

	size = PyTuple_Size(points);
	gdpoints = (gdPointPtr)calloc(size,sizeof(gdPoint));
	for (i=0; i<size; i++)
	{
		point = PyTuple_GET_ITEM(points,i);
		gdpoints[i].x = X(PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,0)));
		gdpoints[i].y = Y(PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(point,1)));
	}
	gdImageFilledPolygon(self->imagedata, gdpoints, size, color);
	free(gdpoints);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_filledrectangle(self, args)
	imageobject *self;
	PyObject *args;
{
	int tx,ty,bx,by,t,color;

	if (!PyArg_ParseTuple(args, "(ii)(ii)i", &tx, &ty, &bx, &by, &color)) return NULL;
	tx = X(tx); ty = Y(ty);
	bx = X(bx); by = Y(by);
	if (tx > bx) {t=tx;tx=bx;bx=t;}
	if (ty > by) {t=ty;ty=by;by=t;}
	gdImageFilledRectangle(self->imagedata, tx, ty, bx, by, color);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_arc(self, args)
	imageobject *self;
	PyObject *args;
{
	int cx, cy, w, h, s, e, color, i;

	if (!PyArg_ParseTuple(args, "(ii)(ii)iii", &cx, &cy, &w, &h, &s, &e, &color)) return NULL;
	if (e<s) {i=e;e=s;s=i;}
	gdImageArc(self->imagedata, X(cx), Y(cy), W(w), H(h), s, e, color);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_filltoborder(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y,border,color;

	if (!PyArg_ParseTuple(args, "(ii)ii", &x,&y,&border,&color)) return NULL;
	gdImageFillToBorder(self->imagedata, X(x),Y(y),border,color);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_fill(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y,color;

	if (!PyArg_ParseTuple(args, "(ii)i", &x,&y,&color)) return NULL;
	gdImageFill(self->imagedata, X(x),Y(y),color);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_setbrush(self, args)
	imageobject *self;
	PyObject *args;
{
	imageobject *brush;
	char *filename, *type; /* dummies */

	if (PyArg_ParseTuple(args, "z|z", &filename, &type)) brush = (imageobject *)newimageobject(args);
	else if (PyErr_Clear(), PyArg_ParseTuple(args, "O!", &Imagetype, &brush)) Py_INCREF(brush);
	else return NULL;
	if (self->current_brush) Py_DECREF(self->current_brush);
	self->current_brush = brush;
	gdImageSetBrush(self->imagedata, brush->imagedata);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_settile(self, args)
	imageobject *self;
	PyObject *args;
{
	imageobject *tile;
	char *filename, *type; /* dummies */

	if (PyArg_ParseTuple(args, "z|z", &filename, &type)) tile = (imageobject *)newimageobject(args);
	else if (PyErr_Clear(), PyArg_ParseTuple(args, "O!", &Imagetype, &tile)) Py_INCREF(tile);
	else return NULL;
	if (self->current_tile) Py_DECREF(self->current_tile);
	self->current_tile = tile;
	gdImageSetBrush(self->imagedata, tile->imagedata);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_setstyle(self, args)
	imageobject *self;
	PyObject *args;
{
	PyObject *style;
	int size, i, *stylearray;

	if (!PyArg_ParseTuple(args, "O!", &PyTuple_Type, &style))
	{
		PyErr_Clear();
		if (PyArg_ParseTuple(args, "O!", &PyList_Type, &style))
			style=PyList_AsTuple(style);
		else return NULL;
	}
	size = PyTuple_Size(style);
	stylearray = (int *)calloc(size,sizeof(int));
	for (i=0; i<size; i++) stylearray[i] = PyInt_AS_LONG((PyIntObject *)PyTuple_GET_ITEM(style,i));
	gdImageSetStyle(self->imagedata, stylearray, size);
	free(stylearray);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_getpixel(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y;

	if (!PyArg_ParseTuple(args, "(ii)", &x,&y)) return NULL;
	return Py_BuildValue("i",gdImageGetPixel(self->imagedata, X(x),Y(y)));
}

static PyObject *image_boundssafe(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y;

	if (!PyArg_ParseTuple(args, "(ii)", &x,&y)) return NULL;
	return Py_BuildValue("i",gdImageBoundsSafe(self->imagedata, X(x),Y(y)));
}

static PyObject *image_blue(self, args)
	imageobject *self;
	PyObject *args;
{
	int c;

	if (!PyArg_ParseTuple(args, "i", &c)) return NULL;
	return Py_BuildValue("i",gdImageBlue(self->imagedata,c));
}

static PyObject *image_green(self, args)
	imageobject *self;
	PyObject *args;
{
	int c;

	if (!PyArg_ParseTuple(args, "i", &c)) return NULL;
	return Py_BuildValue("i",gdImageGreen(self->imagedata,c));
}

static PyObject *image_red(self, args)
	imageobject *self;
	PyObject *args;
{
	int c;

	if (!PyArg_ParseTuple(args, "i", &c)) return NULL;
	return Py_BuildValue("i",gdImageRed(self->imagedata,c));
}

static PyObject *image_size(self)
	imageobject *self;
{
	return Py_BuildValue("(ii)",gdImageSX(self->imagedata),gdImageSY(self->imagedata));
}

static PyObject *image_char(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y,font,color;
	char c;

	if (!PyArg_ParseTuple(args, "i(ii)ii", &font,&x,&y,&c,&color)) return NULL;
	gdImageChar(self->imagedata, fonts[font].data, X(x), Y(y), c, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_charup(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y,font,color;
	char c;

	if (!PyArg_ParseTuple(args, "i(ii)si", &font,&x,&y,&c,&color)) return NULL;
	gdImageCharUp(self->imagedata, fonts[font].data, X(x), Y(y), c, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_string(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y,font,color;
	char *str;

	if (!PyArg_ParseTuple(args, "i(ii)si", &font,&x,&y,&str,&color)) return NULL;
	gdImageString(self->imagedata, fonts[font].data, X(x), Y(y), str, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_stringup(self, args)
	imageobject *self;
	PyObject *args;
{
	int x,y,font,color;
	char *str;

	if (!PyArg_ParseTuple(args, "i(ii)si", &font,&x,&y,&str,&color)) return NULL;
	gdImageStringUp(self->imagedata, fonts[font].data, X(x), Y(y), str, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_colorallocate(self, args)
	imageobject *self;
	PyObject *args;
{
	int r,g,b;

	if (!PyArg_ParseTuple(args, "(iii)", &r, &g, &b)) return NULL;
	return(Py_BuildValue("i",gdImageColorAllocate(self->imagedata, r, g, b)));
}

static PyObject *image_colorclosest(self, args)
	imageobject *self;
	PyObject *args;
{
	int r,g,b;

	if (!PyArg_ParseTuple(args, "(iii)", &r, &g, &b)) return NULL;
	return(Py_BuildValue("i",gdImageColorClosest(self->imagedata, r, g, b)));
}

static PyObject *image_colorexact(self, args)
	imageobject *self;
	PyObject *args;
{
	int r,g,b;

	if (!PyArg_ParseTuple(args, "(iii)", &r, &g, &b)) return NULL;
	return(Py_BuildValue("i",gdImageColorExact(self->imagedata, r, g, b)));
}

static PyObject *image_colorstotal(self)
	imageobject *self;
{
	return Py_BuildValue("i",gdImageColorsTotal(self->imagedata));
}

static PyObject *image_colorcomponents(self, args)
	imageobject *self;
	PyObject *args;
{
	int c,r,g,b;

	if (!PyArg_ParseTuple(args, "i", &c)) return NULL;
	r=gdImageRed(self->imagedata,c);
	g=gdImageGreen(self->imagedata,c);
	b=gdImageBlue(self->imagedata,c);
	return Py_BuildValue("(iii)",r,g,b);
}

static PyObject *image_getinterlaced(self)
	imageobject *self;
{
	return Py_BuildValue("i",gdImageGetInterlaced(self->imagedata));
}

static PyObject *image_gettransparent(self)
	imageobject *self;
{
	return Py_BuildValue("i",gdImageGetTransparent(self->imagedata));
}

static PyObject *image_colordeallocate(self, args)
	imageobject *self;
	PyObject *args;
{
	int c;

	if (!PyArg_ParseTuple(args, "i", &c)) return NULL;
	gdImageColorDeallocate(self->imagedata, c);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_colortransparent(self, args)
	imageobject *self;
	PyObject *args;
{
	int c;

	if (!PyArg_ParseTuple(args, "i", &c)) return NULL;
	gdImageColorTransparent(self->imagedata, c);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_copyto(self, args)
	imageobject *self;
	PyObject *args;
{
	imageobject *dest;
	int dx,dy,sx,sy,w,h,dw,dh;

	dx=dy=sx=sy=0;
	w = gdImageSX(self->imagedata);
	h = gdImageSY(self->imagedata);
	if (!PyArg_ParseTuple(args, "O!|(ii)(ii)(ii)", &Imagetype, &dest, &dx, &dy, &sx, &sy, &w, &h)) return NULL;
	dw = gdImageSX(dest->imagedata);
	dh = gdImageSY(dest->imagedata);
	gdImageCopy(dest->imagedata, self->imagedata, X(dx), Y(dy), X(sx), Y(sy), W(w), H(h));

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_copyresizedto(self, args)
	imageobject *self;
	PyObject *args;
{
	imageobject *dest;
	int dx,dy,sx,sy,dw,dh,sw,sh;

	dx=dy=sx=sy=0;
	sw = gdImageSX(self->imagedata);
	sh = gdImageSY(self->imagedata);
	if (PyArg_ParseTuple(args, "O!|(ii)(ii)", &Imagetype, &dest, &dx, &dy, &sx, &sy))
	{
		dw = gdImageSX(dest->imagedata);
		dh = gdImageSY(dest->imagedata);
	}
	else if (PyErr_Clear(), !PyArg_ParseTuple(args, "O!|(ii)(ii)(ii)(ii)", &Imagetype, &dest, &dx, &dy, &sx, &sy, &dw, &dh, &sw, &sh)) return NULL;
	gdImageCopyResized(dest->imagedata, self->imagedata, X(dx), Y(dy), X(sx), Y(sy), W(dw), H(dh), W(sw), H(sh));

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_interlace(self, args)
	imageobject *self;
	PyObject *args;
{
	int i;

	if (!PyArg_ParseTuple(args, "i", &i)) return NULL;
	gdImageInterlace(self->imagedata, i);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_origin(self, args)
	imageobject *self;
	PyObject *args;
{
	if (!PyArg_ParseTuple(args, "(ii)|ii",&self->origin_x,&self->origin_y,
		&self->multiplier_x,&self->multiplier_y)) return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *image_getorigin(self)
	imageobject *self;
{
	return Py_BuildValue("((ii)ii)",self->origin_x,self->origin_y,self->multiplier_x,self->multiplier_y);
}

static struct PyMethodDef image_methods[] = {
 {"writeGif",	(PyCFunction)image_writegif,	1, "writeGif(f)\n\
 write the image to f as a GIF, where f is either an open file object or a\n\
 file name."},
 {"writeGd",	(PyCFunction)image_writegd,	1, "writeGd(f)\n\
 write the image to f as a GD file, where f is either an open file object\n\
or a file name."},
 {"setPixel",	(PyCFunction)image_setpixel,	1, "setPixel((x,y), color)\n\
 set the pixel at (x,y) to color"},
 {"line",	(PyCFunction)image_line,	1, "line((x1,y1), (x2,y2), color)\n\
 draw a line from (x1,y1) to (x2,y2) in color"},
 {"lines",	(PyCFunction)image_lines,	1, "lines(((x1,y1), (x2,y2), ..., (xn, yn)), color)\n\
 draw a line along the sequence of points in the list or tuple using color"},
 {"polygon",	(PyCFunction)image_polygon,	1, "polygon(((x1,y1), (x2,y2), ..., (xn, yn)), color[, fillcolor])\n\
 draw a polygon using the list or tuple of points (minimum 3) in color,\n\
 optionally filled with fillcolor"},
 {"rectangle",	(PyCFunction)image_rectangle, 1,  "rectangle((x1,y1), (x2,y2), color[, fillcolor])\n\
 draw a rectangle with upper corner (x1,y1), lower corner (x2,y2) in color,\n\
 optionally filled with fillcolor"},
 {"filledPolygon",	(PyCFunction)image_filledpolygon, 1,  "filledPolygon(((x1,y1), (x2,y2), ..., (xn, yn)), color)\n\
 draw a filled polygon using the list or tuple of points (minimum 3) in color"},
 {"filledRectangle",	(PyCFunction)image_filledrectangle, 1,  "filledRectangle((x1,y1), (x2,y2), color)\n\
 draw a rectangle with upper corner (x1,y1), lower corner (x2,y2) in color"},
 {"arc",	(PyCFunction)image_arc,	1, "arc((x,y), (w,h), start, end, color)\n\
 draw an ellipse centered at (x,y) with width w, height h from start\n\
 degrees to end degrees in color."},
 {"fillToBorder",	(PyCFunction)image_filltoborder,	1, "fillToBorder((x,y), border, color)\n\
 flood from point (x,y) to border color in color"},
 {"fill",	(PyCFunction)image_fill,	1, "fill((x,y), color)\n\
 flood from point (x,y) in color for those pixels with the same color\n\
 as the starting point"},
 {"setBrush",	(PyCFunction)image_setbrush,	1, "setBrush(image)\n\
 set the drawing brush to <image> (use gdBrushed when drawing)"},
 {"setTile",	(PyCFunction)image_settile,	1, "setTile(image)\n\
 set the fill tile to <image> (use gdTiled when filling)"},
 {"setStyle",	(PyCFunction)image_setstyle,	1, "setStyle(tuple)\n\
 set the line bit-style to tuple of colors (use gdStyled when drawing)"},
 {"getPixel",	(PyCFunction)image_getpixel,	1, "getPixel((x,y))\n\
 color index of image at (x,y)"},
 {"boundsSafe",	(PyCFunction)image_boundssafe,	1, "boundsSafe((x,y))\n\
 returns true if (x,y) is within image"},
 {"size",	(PyCFunction)image_size,	1, "size()\n\
 return the 2-tuple size of image"},
 {"char",	(PyCFunction)image_char,1, "char(font, (x,y), c, color)\n\
 draw string c at (x,y) using one of the pre-defined gdmodule fonts (gdFont*)"},
 {"charUp",	(PyCFunction)image_charup,1, "charUp(font, (x,y), c, color)\n\
 vertically draw string c at (x,y) using one of the pre-defined gdmodule\n\
 fonts (gdFont*)"},
 {"string",	(PyCFunction)image_string,1, "string(font, (x,y), s, color)\n\
 draw string s at (x,y) using one of the pre-defined gdmodule fonts (gdFont*)"},
 {"stringUp",	(PyCFunction)image_stringup,1, "stringUp(font, (x,y), s, color)\n\
 vertically draw string s at (x,y) using one of the pre-defined gdmodule\n\
 fonts (gdFont*)"},
 {"colorAllocate",	(PyCFunction)image_colorallocate,1, "colorAllocate((r,g,b))\n\
 allocate a color index to (r,g,b) (returns -1 if unable to)"},
 {"colorClosest",	(PyCFunction)image_colorclosest,	1, "colorClosest((r,g,b))\n\
 return the color index closest to (r,g,b) (returns -1 if unable to)"},
 {"colorExact",	(PyCFunction)image_colorexact,	1, "colorExact((r,g,b))\n\
 return an exact color index match for (r,g,b) (returns -1 if unable to)"},
 {"colorsTotal",	(PyCFunction)image_colorstotal,	1, "colorsTotal()\n\
 returns the number of colors currently allocated"},
 {"colorComponents",	(PyCFunction)image_colorcomponents,	1, "colorComponents(color)\n\
 returns a 3-tulple of the (r,g,b) components of color"},
 {"getInterlaced",	(PyCFunction)image_getinterlaced,	1, "getInterlaced()\n\
 returns true if the image is interlaced"},
 {"getTransparent",	(PyCFunction)image_gettransparent,	1, "getTransparent()\n\
 returns transparent color index or -1"},
 {"colorDeallocate",	(PyCFunction)image_colordeallocate,	1, "colorDeallocate(color)\n\
 deallocate color from the image palette"},
 {"colorTransparent",	(PyCFunction)image_colortransparent,	1, "colorTransparent(color)\n\
 set the transparent color to color"},
 {"copyTo",	(PyCFunction)image_copyto,	1, "copyTo(image[, (dx,dy)[, (sx,sy)[, (w,h)]]])\n\
 copy from (sx,sy), width sw and height sh to destination image (dx,dy)"},
 {"copyResizedTo",	(PyCFunction)image_copyresizedto,1, "copyResizedTocopyTo(image[, (dx,dy)[, (sx,sy)[, (dw,dh)[, (sw,sh)]]]])\n\
 copy from (sx,sy), width sw and height sh to destination image (dx,dy), \n\
 width dw and height dh"},
 {"interlace",	(PyCFunction)image_interlace,	1, "interlace()\n\
 set the interlace bit"},
 {"origin",	(PyCFunction)image_origin,	1, "origin((x,y)[,xmult,ymult])\n\
 set the origin of the image to (x,y) and multiply all x, y, width and\n\
 height factors by xmult and ymult (typically either 1 or -1)"},
 {"getOrigin",	(PyCFunction)image_getorigin,	1, "getOrigin()\n\
 returns the origin parameters ((x,y),xmult,ymult)"},
 {NULL,		NULL}		/* sentinel */
};

/*
** Code to create the imageobject
*/
static imageobject *newimageobject(args)
	PyObject *args;
{
	imageobject *self, *srcimage;
	int xdim=0,ydim=0;
	char *filename,*ext=0;
	FILE *fp;
	
	if (!(self = PyObject_NEW(imageobject, &Imagetype))) return NULL;
	self->current_tile = self->current_brush = NULL;
	self->origin_x = self->origin_y = 0;
	self->multiplier_x = self->multiplier_y = 1;
	self->imagedata = NULL;
	if (PyArg_ParseTuple(args, ""))
	{
		PyErr_SetString(PyExc_ValueError, "image size or source filename required");
		Py_DECREF(self);
		return NULL;
	}
	else if (PyErr_Clear(), PyArg_ParseTuple(args, "O!|(ii)", &Imagetype, &srcimage, &xdim, &ydim))
	{
		if (!xdim) xdim = gdImageSX(srcimage->imagedata);
		if (!ydim) ydim = gdImageSY(srcimage->imagedata);
		if (!(self->imagedata = gdImageCreate(xdim,ydim))) {
			Py_DECREF(self);
			return NULL;
		}
		if (xdim == gdImageSX(srcimage->imagedata) &&
		    ydim == gdImageSY(srcimage->imagedata))
			gdImageCopy(self->imagedata,srcimage->imagedata,0,0,0,0,xdim,ydim);
		else
			gdImageCopyResized(self->imagedata,srcimage->imagedata,0,0,0,0,xdim,ydim,gdImageSX(srcimage->imagedata),gdImageSY(srcimage->imagedata));
	}
	else if (PyErr_Clear(), PyArg_ParseTuple(args, "(ii)", &xdim, &ydim))
	{
		if (!xdim || !ydim)
		{
			PyErr_SetString(PyExc_ValueError, "dimensions cannot be 0");
			Py_DECREF(self);
			return NULL;
		}
		if (!(self->imagedata = gdImageCreate(xdim,ydim))) {
			Py_DECREF(self);
			return NULL;
		}
	}
	else if (PyErr_Clear(), PyArg_ParseTuple(args, "s|s", &filename, &ext))
	{
		if (!ext)
		{
			if (!(ext = strrchr(filename,'.')))
			{
				PyErr_SetString(PyExc_IOError, "need an extension to determine file type (.gif|.gd|.xbm)");
				Py_DECREF(self);
				return NULL;
			}
			ext++;
		}
		if (!(fp = fopen(filename,"rb")))
		{
			PyErr_SetFromErrno(PyExc_IOError);
			Py_DECREF(self);
			return(NULL);
		}
		if (!strcmp(ext,"gif") && (!(self->imagedata = gdImageCreateFromGif(fp)))) {
			fclose(fp);
			Py_DECREF(self);
			return(NULL);
		}
		if (!strcmp(ext,"gd") && (!(self->imagedata = gdImageCreateFromGd(fp)))) {
			fclose(fp);
			Py_DECREF(self);
			return(NULL);
		}
		if (!strcmp(ext,"xbm") && (!(self->imagedata = gdImageCreateFromXbm(fp)))) {
			fclose(fp);
			Py_DECREF(self);
			return(NULL);
		}

		fclose(fp);
	}
	else {
		Py_DECREF(self);
		return(NULL);
	}

	return self;
}

static void image_dealloc(self)
	imageobject *self;
{
	if (self->current_brush) Py_DECREF(self->current_brush);
	if (self->current_tile) Py_DECREF(self->current_tile);
	if (self->imagedata) gdImageDestroy(self->imagedata);
	PyMem_DEL(self);
}

static PyObject *image_getattr(self, name)
	PyObject *self;
	char *name;
{
	return Py_FindMethod(image_methods, self, name);
}

static PyObject *image_print(self, fp, flags)
	PyObject *self;
	FILE *fp;
	int flags;
{
	gdImagePtr im;

	im = ((imageobject *)self)->imagedata;
	fprintf(fp,"<%dx%d image object at 0x%x>",gdImageSX(im),gdImageSY(im),self);
	return 0;
}


static PyTypeObject Imagetype = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,					/*ob_size*/
	"image",			/*tp_name*/
	sizeof(imageobject),/*tp_basicsize*/
	0,					/*tp_itemsize*/
	/* methods */
	(destructor)image_dealloc,		/*tp_dealloc*/
	(printfunc)image_print,		/*tp_print*/
	(getattrfunc)image_getattr,		/*tp_getattr*/
	0,					/*tp_setattr*/
	0,					/*tp_compare*/
	0,					/*tp_repr*/
	0,					/*tp_as_number*/
	0,					/*tp_as_sequence*/
	0,					/*tp_as_mapping*/
	0,					/*tp_hash*/
	0,					/*tp_call */
	0,					/*tp_str */
	0,0,0,0,			/*tp_xxx1-4 */
};


/*
** List of methods defined in the module
*/
static PyObject *gd_image(self, args)
	PyObject *self;
	PyObject *args;
{
	return (PyObject *)newimageobject(args);
}

static PyObject *gd_fontSSize(self, args)
	PyObject *self;
	PyObject *args;
{
	int font;
	char * str;
	int len;

	if (!PyArg_ParseTuple(args, "is", &font,&str)) return NULL;
	if (font < 0 && font >= (sizeof(fonts)-1)) {
		PyErr_SetString(PyExc_ValueError, "Font value not valid");
		return NULL;
	}
	len = strlen(str);
	return Py_BuildValue("(ii)", len*(fonts[font].data->w),
			     fonts[font].data->h);
}


static struct PyMethodDef gd_methods[] = {
	{"image", gd_image, 1, "image(image[,(w,h)] | file | file,type | (w,h))\n\
create GD image from file.(gif|gd|xbm), file,type (gif|gd|xbm),\n\
the existing image, optionally resized to width w and height h\n\
or blank with width w and height h"},
  {"fontstrsize", gd_fontSSize, 1, "fontstrsize(font, string)\n\
return a tuple containing the size in pixels of the given string in the\n\
 given font"},
  {NULL,		NULL}		/* sentinel */
};

/* Initialization function for the module (*must* be called initgd) */

void initgd()
{
	PyObject *m, *d, *v;
	int i=0;

	/* Create the module and add the functions */
	m = Py_InitModule("gd", gd_methods);

	/* Add some symbolic constants to the module */
	d = PyModule_GetDict(m);
	ErrorObject = PyString_FromString("gd.error");
	PyDict_SetItemString(d, "error", ErrorObject);

	/* ad din the dosctring */
	v = Py_BuildValue("s", gdModuleDocString);
	PyDict_SetItemString(d, "__doc__", v);

	/* add in the two font constants */
	while(fonts[i].name)
	{
		v = Py_BuildValue("i", i);
		PyDict_SetItemString(d, fonts[i++].name, v);
	}

	/* and the standard gd constants */
	v = Py_BuildValue("i", gdBrushed);
	PyDict_SetItemString(d, "gdBrushed", v);
	v = Py_BuildValue("i", gdMaxColors);
	PyDict_SetItemString(d, "gdMaxColors", v);
	v = Py_BuildValue("i", gdMaxColors);
	PyDict_SetItemString(d, "gdMaxColors", v);
	v = Py_BuildValue("i", gdStyled);
	PyDict_SetItemString(d, "gdStyled", v);
	v = Py_BuildValue("i", gdStyledBrushed);
	PyDict_SetItemString(d, "gdStyledBrushed", v);
	v = Py_BuildValue("i", gdDashSize);
	PyDict_SetItemString(d, "gdDashSize", v);
	v = Py_BuildValue("i", gdTiled);
	PyDict_SetItemString(d, "gdTiled", v);
	v = Py_BuildValue("i", gdTransparent);
	PyDict_SetItemString(d, "gdTransparent", v);

	/* Check for errors */
	if (PyErr_Occurred()) Py_FatalError("can't initialize module gd");
}
