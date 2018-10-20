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
printf("usage : appli input_file\n");
exit(1);
}

void indent( int n )
{
int i;
for	( i = 0; i < n; ++i )
	putchar(' ');
}

int load_xml( char * xmlpath )
{
xmlobj * lexml = new xmlobj( xmlpath, NULL );
if	( !(lexml->is) )
	return 1;
int status;
// valeurs temporaires
xelem * elem; string id;
int level = 0;

lexml->curchar = 0;
// la boucle des steps
while	( ( status = lexml->step() ) != 9 )
	{
	elem = &lexml->stac.back();
	switch	( status )
		{
		case 1 :	// start-tag
			id = elem->attr[string("id")];		// copier son id
			printf("%5d ", lexml->curchar );
			indent( (level++) * 3 );
			printf("%s id=%s...\n", elem->tag.c_str(), id.c_str() );
		break;
		case 2 :	// end-tag
			printf("%5d ", lexml->curchar );
			indent( (--level) * 3 );
			printf("%s <>%s<>\n", elem->tag.c_str(), elem->inner.c_str() );
		break;
		case 3 :
			id = elem->attr[string("id")];		// copier son id
			printf("%5d ", lexml->curchar );
			indent( level * 3 );
			printf("%s : %s\n", elem->tag.c_str(), id.c_str() );
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

if	( argc < 2 )
	usage();

retval = load_xml( argv[1] );
if	( retval )
	gasp("problem avec %s, code %d\n", argv[1], retval );
	
return 0;
}
