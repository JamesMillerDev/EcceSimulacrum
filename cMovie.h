#ifndef C_MOVIE_H__
#define C_MOVIE_H__

#ifdef WIN32
#include <windows.h>
#include <vfw.h>	// Header File For Video For Windows
#endif

#include "Video.h"


class cMovie {
protected:
	const GLuint m_texno;
	GLuint m_texbuf;

protected:
	AVISTREAMINFO				m_psi;			// Pointer To A Structure Containing Stream Info
	PAVISTREAM					m_pavi;			// Handle To An Open Stream
	PGETFRAME						m_pgf;			// Pointer To A GetFrame Object
	BITMAPINFOHEADER		m_bmih;			// Header Information For DrawDibDraw Decoding
	HDRAWDIB						m_hdd;			// Handle For Our Dib
	HBITMAP							m_hBitmap;		// Handle To A Device Dependant Bitmap
	LPBITMAPINFOHEADER	lpbi;			// Holds The Bitmap Header Information
	HDC m_hdc;

	unsigned char*	m_data;			// Pointer To Our Resized Image
	long						m_lastframe;	// Last Frame Of The Stream
	int							m_width;		// Video Width
	int							m_height;		// Video Height
	unsigned char		*m_pdata;		// Pointer To Texture Data
	int							m_mpf;			// Will Hold Rough Milliseconds Per Frame

public:
	static cMovie* Create();

protected:
	cMovie( const GLuint in_texno );
public:
	~cMovie();

public:
	void OpenAVI( LPCSTR in_filename );
	void GrabFrameByFramenumber( int frame );
	void GrabFrameByTime( long cur_time ) { GrabFrameByFramenumber( cur_time / m_mpf ); }
	void CloseAVI();

public:
	int getWidth()      { return m_width; }
	int getHeight()     { return m_height; }
	long getNumFrames() { return m_lastframe; }
	long getLength()    { return m_mpf * m_lastframe; }
	GLuint getTexture() { return m_texno; }

protected:
	void pbo_upload();
	void regular_upload();

};

//////////////////////////////////////////////////////////////////////////

inline
cMovie* cMovie::Create() {

	GLuint movtex;
	glGenTextures( 1, &movtex );

	glEnable( GL_TEXTURE_RECTANGLE_ARB );
	glBindTexture( GL_TEXTURE_RECTANGLE_ARB, movtex );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );

	return new cMovie(movtex);
}

#endif
