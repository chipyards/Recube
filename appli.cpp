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
xelem * elem;
int level = 0;

lexml->curchar = 0;
// la boucle des steps
while	( ( status = lexml->step() ) != 9 )
	{
	elem = &lexml->stac.back();
	switch	( status )
		{
		case 1 :	// start-tag
			printf("%5d ", lexml->curchar );
			indent( (level++) * 3 );
			printf("%s", elem->tag.c_str() );
			if	( elem->attr.count(string("id")) )
				printf(" id=%s", elem->attr[string("id")].c_str() );
			printf("...\n");
		break;
		case 2 :	// end-tag
			printf("%5d ", lexml->curchar );
			indent( (--level) * 3 );
			printf("%s", elem->tag.c_str() );
			if	( elem->inner.size() )
				printf(" '%s'", elem->inner.c_str() );
			printf("\n");
		break;
		case 3 :
			printf("%5d ", lexml->curchar );
			indent( level * 3 );
			printf("%s", elem->tag.c_str() );
			if	( elem->attr.count(string("id")) )
				printf(" id=%s", elem->attr[string("id")].c_str() );
			printf("\n");
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
