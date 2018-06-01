#include "cMovie.h"

#include <assert.h>

#include <exception>


cMovie::cMovie( const GLuint in_texno )
	: m_texno(in_texno),
	  m_texbuf(0),
	  m_data(0),
	  m_pdata(0)
{
	assert( m_texno );
	return;
}


cMovie::~cMovie() {
	CloseAVI();
}


void cMovie::OpenAVI( LPCSTR in_filename ) {
// note: opens an avi-file

	AVIFileInit();											// Opens The AVIFile Library
	
	// Opens The AVI Stream
	if( AVIStreamOpenFromFile( &m_pavi, in_filename, streamtypeVIDEO, 0, OF_READ, NULL)) {
		MessageBox( HWND_DESKTOP, "Failed To Open The AVI Stream", "Error", MB_OK | MB_ICONEXCLAMATION );
		exit(-1);
	}

	AVIStreamInfo( m_pavi, &m_psi, sizeof(m_psi) );						// Reads Information About The Stream Into psi
	m_width  = m_psi.rcFrame.right - m_psi.rcFrame.left;					// Width Is Right Side Of Frame Minus Left
	m_height = m_psi.rcFrame.bottom - m_psi.rcFrame.top;				// Height Is Bottom Of Frame Minus Top

	m_lastframe = AVIStreamLength( m_pavi );							// The Last Frame Of The Stream
	m_mpf = AVIStreamSampleToTime( m_pavi, m_lastframe ) / m_lastframe;	// Calculate Rough Milliseconds Per Frame

	m_pgf = AVIStreamGetFrameOpen(m_pavi, NULL);					// Create The PGETFRAME	Using Our Request Mode
	if( m_pgf==NULL ) {
		// An Error Occurred Opening The Frame
		MessageBox( HWND_DESKTOP, "Failed To Open The AVI Frame", "Error", MB_OK | MB_ICONEXCLAMATION );
		exit(-1);
	}

	// create pbo-objects for texture-upload
	glGenBuffersARB(1, &m_texbuf);
	glBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_texno );
	glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB8, m_width, m_height, 0, GL_BGR, GL_UNSIGNED_BYTE, 0 ); //note: allocate room, don't upload anything

	return;
}


inline
int clamp( int a, int in_min, int in_max ) {
	return min( in_max, max( a, in_min ));
}

void cMovie::GrabFrameByFramenumber( int frame ) {
	//note: safeguard
	frame = clamp( frame, 0, m_lastframe -1 );

	static int prev_frame = -1;
	if(prev_frame == frame) return;
	prev_frame = frame;

	LPVOID ptr = AVIStreamGetFrame( m_pgf, frame );
	if(!ptr) return;

	m_pdata = reinterpret_cast<unsigned char*>( ptr ) + sizeof(BITMAPINFOHEADER);

	//regular_upload();
	pbo_upload();

	return;
}




inline
void cMovie::regular_upload() {
	glBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_texno );
	glTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, m_width, m_height , GL_BGR, GL_UNSIGNED_BYTE, m_pdata);
	glBindTexture( GL_TEXTURE_RECTANGLE_ARB, 0 );

	return;
}


void cMovie::pbo_upload() {

	assert( m_pdata );
	assert( m_texbuf );

	const unsigned int texsiz = m_width * m_height * 3; //note: 3 is rgb

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_texbuf);
/**/
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, texsiz, NULL, GL_STREAM_DRAW);
	void * const pboMemory = glMapBufferARB( GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY );
	memcpy( pboMemory, m_pdata, texsiz );
	glUnmapBufferARB( GL_PIXEL_UNPACK_BUFFER_ARB );
/*/
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, texsiz, m_pdata, GL_STREAM_DRAW );
/**/
	glTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, 0 );

	return;
}


void cMovie::CloseAVI(void) {
// note: properly closes the avi file

	AVIStreamGetFrameClose( m_pgf );							// Deallocates The GetFrame Resources
	AVIStreamRelease( m_pavi );									// Release The Stream
	AVIFileExit();												// Release The File

	return;
}

