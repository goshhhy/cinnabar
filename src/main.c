#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <sulfur/sulfur.h>

#define PROGRAM_NAME "cinnabar"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_STRING "0.1"
#define VERSION_BUILDSTR "1"

#define FONT_NAME "fixed"

sulfurColor_t colorWhite;
sulfurColor_t colorBlack;

unsigned int fontContext;
xcb_font_t windowFont;

sulfurWindow_t window;
xcb_screen_t* screen;
xcb_connection_t* c;
xcb_pixmap_t icon;

const char* homeDir;

/*
=================
Support functions
=================
*/

void CreateIconPixmap( void ) {
	icon = xcb_generate_id( c );
	xcb_create_pixmap( c, screen->root_depth, icon, screen->root, 11, 14 );
	SGrafDrawFill( icon, SULFUR_COLOR_WHITE, 0, 0, 11, 14 );
	SGrafDrawFill( icon, SULFUR_COLOR_RED, 3, 2, 5, 10 );
	SGrafDrawFill( icon, SULFUR_COLOR_RED, 2, 4, 7, 6 );
	SGrafDrawLine( icon, SULFUR_COLOR_YELLOW, 5, 1, 1, 5 );
	SGrafDrawLine( icon, SULFUR_COLOR_YELLOW, 1, 6, 1, 8 );
	SGrafDrawLine( icon, SULFUR_COLOR_ORANGE, 9, 5, 9, 8 );
	SGrafDrawLine( icon, SULFUR_COLOR_ORANGE, 8, 9, 5, 12 );
	SGrafDrawLine( icon, SULFUR_COLOR_BLACK, 5, 0, 0, 5 );
	SGrafDrawLine( icon, SULFUR_COLOR_BLACK, 0, 6, 0, 8 );
	SGrafDrawLine( icon, SULFUR_COLOR_BLACK, 1, 9, 5, 13 );
	SGrafDrawLine( icon, SULFUR_COLOR_BLACK, 5, 0, 0, 5 );
	SGrafDrawLine( icon, SULFUR_COLOR_BLACK, 6, 12, 10, 8 );
	SGrafDrawLine( icon, SULFUR_COLOR_BLACK, 10, 7, 10, 5 );
	SGrafDrawLine( icon, SULFUR_COLOR_BLACK, 9, 4, 6, 1 );
	xcb_flush( c );
} 

void DrawBar( void ) {
	char timeString[16];
	time_t rawTime;
	struct tm* timeInfo;

	SGrafDrawFill( window, colorWhite, 0, 0, screen->width_in_pixels, 20 );
	SGrafDrawLine( window, colorBlack, 0, 19, screen->width_in_pixels, 19 );

	SGrafDrawLine( window, colorBlack, 0, 0, 0, 4 );
	SGrafDrawLine( window, colorBlack, 1, 0, 4, 0 );
	SGrafDrawLine( window, colorBlack, 1, 1, 1, 2 );
	SGrafDrawLine( window, colorBlack, 1, 1, 2, 1 );

	SGrafDrawLine( window, colorBlack, screen->width_in_pixels - 1, 0, screen->width_in_pixels - 5, 0 );
	SGrafDrawLine( window, colorBlack, screen->width_in_pixels - 1, 0, screen->width_in_pixels - 1, 4 );
	SGrafDrawLine( window, colorBlack, screen->width_in_pixels - 2, 1, screen->width_in_pixels - 2, 2 );
	SGrafDrawLine( window, colorBlack, screen->width_in_pixels - 2, 1, screen->width_in_pixels - 3, 1 );

	xcb_copy_area( c, icon, window, sulfurGc, 0, 0, 17, 2, 11, 14 );

	time( &rawTime );
	timeInfo = localtime( &rawTime );
	strftime( timeString, 16, "%H:%M:%S", timeInfo );

	xcb_image_text_8( c, strlen( timeString ), window, fontContext, screen->width_in_pixels - 56, 14, timeString );

	xcb_flush( c );
	return;
}

void SetupFonts() {
	unsigned int v[3];

	windowFont = xcb_generate_id( c );
	xcb_open_font( c, windowFont, strnlen( FONT_NAME, 256 ), FONT_NAME );

	v[2] = windowFont;

	fontContext = xcb_generate_id( c );
	v[0] = colorBlack;
	v[1] = colorWhite;
	xcb_create_gc( c, fontContext, screen->root, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT, v );
}

void Cleanup() {
	SulfurClose();
}

void Quit( int r ) {
	Cleanup();
	exit( r );
}

/*
==============
event handlers
==============
*/

void DoButtonRelease( xcb_button_release_event_t *e  ) {
	if ( e->event_x < 30 && e->event_x > 14 && e->event_y < 20 ) {
		if ( fork() == 0 ) {
			execlp( "xterm", "xterm", NULL );
		}
	}
}

/*
=============
Main function
=============
*/

int main( int argc, char** argv ) {
	unsigned int v[3] = { 	colorWhite,
						1,
						XCB_EVENT_MASK_EXPOSURE | 
						XCB_EVENT_MASK_BUTTON_PRESS | 
						XCB_EVENT_MASK_BUTTON_RELEASE };
	struct timespec t = { .tv_sec = 0, .tv_nsec = 100000000 };
	xcb_generic_event_t *e;
	char* display;
	int i;
	int lastTime;

	signal( SIGTERM, Quit );
	signal( SIGINT, Quit );

	printf( "%s %s, build %s\n\n", PROGRAM_NAME, VERSION_STRING, VERSION_BUILDSTR );

	if ( SulfurInit( NULL ) != 0 ) {
			printf( "Problem starting up. Is X running?\n" );
			Cleanup();
			return 1;
	}
	c = sulfurGetXcbConn();
	screen = sulfurGetXcbScreen();

	colorBlack = SULFUR_COLOR_BLACK;
	colorWhite = SULFUR_COLOR_WHITE;

	homeDir = getenv( "HOME" );
	if ( !homeDir ) {
		struct passwd *pw = getpwuid( getuid() );
		homeDir = pw->pw_dir;
	}
	if ( homeDir ) {
		chdir( homeDir );
	}

	CreateIconPixmap();
	SetupFonts();
	window = xcb_generate_id( c );
	xcb_create_window( c, screen->root_depth, window, screen->root, 
						0, 0, screen->width_in_pixels, 20, 
						0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
							XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, v );
	xcb_map_window( c, window );
	DrawBar();
	xcb_flush( c );

	while ( 1 ) {
		while( ( e = xcb_poll_for_event( c ) ) != NULL ) {
			switch( e->response_type & ~0x80 ) {
				case XCB_BUTTON_PRESS:
					break;
				case XCB_BUTTON_RELEASE:
					DoButtonRelease( (xcb_button_release_event_t*)e );
					break;
				case XCB_EXPOSE:
					DrawBar();
					break;
				case XCB_DESTROY_NOTIFY:
					Quit( 0 );
					break;
				default:
					printf( "Warning, unhandled event #%d\n", e->response_type & ~0x80 );
					break;
			}
			free( e );
		}
		nanosleep( &t, NULL );
		if ( time(NULL) != lastTime ) {
			lastTime = time( NULL );
			DrawBar();
		}
		
	}
	
	printf( "Looks like we're done here. See you next time!\n" );
	Cleanup();
	return 0;
}
