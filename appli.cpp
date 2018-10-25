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

int scan_xml( char * tbuf, int qbuf )
{
int status, srcpos;
xelem * elem;
int level = 0;
xmlobj * lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu


// la boucle des chars
for	( srcpos = 0; srcpos < qbuf; ++srcpos )
	{
	status = lexml->proc1char( tbuf[srcpos] );
	if	( status )			// status 0 : rien a faire; status 1, 2, ou 3 : ok; status < 0 : erreur 
		{
		elem = &lexml->stac.back();
		switch	( status )
			{
			case 1 :	// start-tag
				printf("%5d ", srcpos );
				indent( (level++) * 3 );
				printf("%s", elem->tag.c_str() );
				if	( elem->attr.count(string("id")) )
					printf(" id=%s", elem->attr[string("id")].c_str() );
				printf("...\n");
			break;
			case 2 :	// end-tag
				printf("%5d ", srcpos );
				indent( (--level) * 3 );
				printf("%s", elem->tag.c_str() );
				if	( elem->inner.size() )
					printf(" '%s'", elem->inner.c_str() );
				printf("\n");
			break;
			case 3 :
				printf("%5d ", srcpos );
				indent( level * 3 );
				printf("%s", elem->tag.c_str() );
				if	( elem->attr.count(string("id")) )
					printf(" id=%s", elem->attr[string("id")].c_str() );
				printf("\n");
			break;
			default :
			gasp("%s ligne %d : syntaxe xml %d", lexml->filepath, (lexml->curlin+1), status );
			}
		}	// if status
	}	// for
return 0;
}

#define QXML	(1<<16)

/* ============================== the main ===================================== */
int main( int argc, char ** argv )
{
int retval;
FILE * srcfil;
FILE * dstfil = stdout;
char xmlbuf[QXML];
int qxml = 0;

if	( argc < 2 )
	usage();

srcfil = fopen( argv[1], "r" );
if	( srcfil == NULL )
	gasp("echec ouverture %s", argv[1] );

qxml = fread( xmlbuf, 1, QXML, srcfil );

if	( qxml <= 0 )
	gasp("rien lu dans %s", argv[1] );
if	( qxml == QXML )
	gasp("%s trop gros (%d bytes)", argv[1], qxml );

printf("lu %d bytes\n", qxml ); 

retval = scan_xml( xmlbuf, qxml );
	
return 0;
}
