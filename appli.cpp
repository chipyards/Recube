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
const char * new_inc_path = "C:/STM32/CubeL4/";
const char * new_proj_name = NULL;

projfile * leproj;

if	( argc >= 2 )
	new_proj_name = argv[1];

leproj = new projfile();
retval = leproj->process_file( ".cproject", new_inc_path, NULL );
delete leproj;

leproj = new projfile();
retval = leproj->process_file( ".project", new_inc_path, new_proj_name );
delete leproj;

return 0;
}
