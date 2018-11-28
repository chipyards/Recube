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
printf("usage : recube {-uC:} {-fF1} {-pC:/STM32/CubeL4} {-nPipo}");
exit(1);
}

/* ============================== the main ===================================== */
int main( int argc, char ** argv )
{
int retval;
const char * new_inc_path = NULL;
const char * new_proj_name = NULL;
const char * libpath = "/STM32/Cube";
char unit = 'C';
const char * family = "F1";

char inc_path[256];

inc_path[0] = 0;
for	( int i = 1; i < argc; ++i )
	{
	if	( argv[i][0] != '-' )
		usage();
	if	( argv[i][2] <= ' ' )
		usage();
	switch	( argv[i][1] )
		{
		case 'u' : unit = argv[i][2];
			snprintf( inc_path, sizeof( inc_path ), "%c:%s%s/", unit, libpath, family );
			break;
		case 'f' : family = argv[i] + 2;
			snprintf( inc_path, sizeof( inc_path ), "%c:%s%s/", unit, libpath, family );
			break;
		case 'p' :
			snprintf( inc_path, sizeof( inc_path ), "%s/", argv[i] + 2 );
			break;
		case 'n' : new_proj_name = argv[i] + 2;
			break;
		}
	}

if	( inc_path[0] )
	new_inc_path = inc_path;

projfile * leproj;

leproj = new projfile();
retval = leproj->process_file( ".cproject", new_inc_path, NULL );
delete leproj;

leproj = new projfile();
retval = leproj->process_file( ".project", new_inc_path, new_proj_name );
delete leproj;

return 0;
}
