#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pcre.h>
#include "pcreux.h"

using namespace std;
#include <string>
//#include <sstream>
#include <vector>
#include <map>

extern "C" {
void gasp( const char *fmt, ... );  /* fatal error handling */
}

#include "xmlpe.h"
#include "charvec.h"
#include "projfile.h"

extern "C" {
/* gasp classique --> stderr par defaut */
FILE * logfil=stderr;

void gasp( const char *fmt, ... )  /* fatal error handling */
{
  va_list  argptr;
  fprintf( logfil, "ERROR : " );
  va_start( argptr, fmt );
  vfprintf( logfil, fmt, argptr );
  va_end( argptr );
  fprintf( logfil, "\n" );
  fclose(logfil);
  exit(1);
}
}

void usage(void)
{
printf("usage :\n  recube input_file\n  recube input_file C:/STM32/CubeL4/\n");
exit(1);
}

/* ============================== the main ===================================== */
int main( int argc, char ** argv )
{
int retval;
FILE * srcfil;
FILE * dstfil = stdout;
const char * new_inc_path = "C:/STM32/CubeL4/";
const char * new_proj_name = NULL;

projfile * leproj;

if	( argc >= 2 )
	new_proj_name = argv[1];

srcfil = fopen( ".cproject", "r" );
if	( srcfil == NULL )
	gasp("echec ouverture %s", ".cproject" );

printf("===== fichier .cproject ====\n");

leproj = new projfile();

retval = leproj->scan_ac6_cproject( srcfil, stdout, new_inc_path );
printf("vu %d chemins de drivers\n", leproj->hits );
fclose( srcfil );
if	( retval )
	gasp("oups %d ligne %d", retval, leproj->lexml->curlin+1 );

if	( ( new_inc_path ) && ( leproj->hits ) )
	printf("a remplacer par %s\n", new_inc_path );

if	( leproj->dst_buf.size )
	{
	printf("sauver la version modifiee? (Y/N)\n");
	fflush( stdout );
	if	( ( getchar() == 'y' ) || ( getchar() == 'y' ) )
		{
		dstfil = fopen( ".cproject", "w" );
		if	( dstfil == NULL )
			gasp("echec ouverture %s", ".cproject" );
		if	(
			fwrite( leproj->dst_buf.data, 1, leproj->dst_buf.size, dstfil ) !=
			(size_t)leproj->dst_buf.size
			)
			gasp("echec ecriture %s", ".cproject" );
		fclose( dstfil );
		printf("c'est fait\n");
		}
	else	{
		printf("bye\n");
		}
	}
delete leproj;

srcfil = fopen( ".project", "r" );
if	( srcfil == NULL )
	gasp("echec ouverture %s", ".project" );

printf("===== fichier .project ====\n");

leproj = new projfile();

retval = leproj->scan_ac6_project( srcfil, stdout, new_inc_path, new_proj_name );
printf("vu %d links de drivers relatif, de plus de 4 niveaux\n", leproj->hits );
fclose( srcfil );
if	( retval )
	gasp("oups %d ligne %d", retval, leproj->lexml->curlin+1 );

if	( ( new_inc_path ) && ( leproj->hits ) )
	printf("a remplacer par %s\n", new_inc_path );
if	( ( new_proj_name ) && ( leproj->dst_buf.size ) )
	printf("nouveau nom de projet %s\n", new_proj_name );

if	( leproj->dst_buf.size )
	{
	printf("sauver la version modifiee? (Y/N)\n");
	fflush( stdout );
	if	( ( getchar() == 'y' ) || ( getchar() == 'y' ) )
		{
		dstfil = fopen( ".project", "w" );
		if	( dstfil == NULL )
			gasp("echec ouverture %s", ".project" );
		if	(
			fwrite( leproj->dst_buf.data, 1, leproj->dst_buf.size, dstfil ) !=
			(size_t)leproj->dst_buf.size
			)
			gasp("echec ecriture %s", ".project" );
		fclose( dstfil );
		printf("c'est fait\n");
		}
	else	{
		printf("bye\n");
		}
	}

return 0;
}
