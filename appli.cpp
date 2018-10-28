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

#include "xmlpe.h"
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
printf("usage : appli input_file\n");
exit(1);
}

/* ============================== the main ===================================== */
int main( int argc, char ** argv )
{
int retval;
FILE * srcfil;
FILE * dstfil = stdout;
projfile * leproj;

if	( argc < 2 )
	usage();

srcfil = fopen( argv[1], "r" );
if	( srcfil == NULL )
	gasp("echec ouverture %s", argv[1] );

leproj = new projfile();


string fnam = string( argv[1] ); 
if	( fnam == string(".cproject") )
	{
	retval = leproj->scan_ac6_cproject( srcfil, stdout );
	printf("vu %d chemins de drivers\n", leproj->hits );
	}
else if	( fnam == string(".project") )
	{
	retval = leproj->scan_ac6_project( srcfil, stdout );
	printf("vu %d links relatif de plus de 4 niveaux\n", retval );
	}
else	retval = leproj->scan_xml( srcfil, stdout );

if	( retval )
	gasp("oups %d ligne %d", retval, leproj->lexml->curlin+1 );

return 0;
}
