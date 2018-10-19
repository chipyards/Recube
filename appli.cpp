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
printf("usage : appli input_file elem\n");
exit(1);
}

int load_xml( char * xmlpath )
{
xmlobj * lexml = new xmlobj( xmlpath, NULL );
if	( !(lexml->is) )
	return 1;
int status;
// valeurs temporaires
xelem * elem; string id;

// la boucle des steps
while	( ( status = lexml->step() ) )
	{
	elem = &lexml->stac.back();
	switch	( status )
		{
		case 1 :
		id = elem->attr[string("id")];		// copier son nom
		printf("\n1) @ %d : %s : %s \n", lexml->curchar, elem->tag.c_str(), id.c_str() );
		break;
		case 2 :
		printf("\n2) @ %d\n", lexml->curchar );
		break;
		default :
		gasp("%s ligne %d : syntaxe xml %d", lexml->filepath, (lexml->curlin+1), status );
		}
	}	// while status
return 0;
}

/* ============================== the main ===================================== */
int main( int argc, char ** argv )
{
int retval;

if	( argc < 3 )
	usage();

retval = load_xml( argv[1] );
if	( retval )
	gasp("problem avec %s, code %d\n", argv[1], retval );
	
return 0;
}
