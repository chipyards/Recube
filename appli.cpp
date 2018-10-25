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

// un scan generique qui affiche l'inner des elements et l'attribut 'id'
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

// un scan qui cherche les include paths dans un .cproject
int scan_ac6_cproject( char * tbuf, int qbuf )
{
int status, srcpos;
xelem * elem;
xmlobj * lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu

int include_path_flag = 0;
pcreux * lareu1;
pcreux * lareu2;
string s;

lareu1 = new pcreux( "gnu.c.compiler.option.include.paths" );
lareu2 = new pcreux( "value=(.*)Drivers" );

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
				if	(
					( elem->tag.c_str() == string("option") ) &&
					( elem->attr.count(string("id")) )
					)
					{
					s = elem->attr[string("id")];
					lareu1->letexte = s.c_str();
					lareu1->lalen = s.size();
					lareu1->start = 0;
					if	( lareu1->matchav() > 0 )
						{
						printf("id=%s\n", elem->attr[string("id")].c_str() );
						include_path_flag = 1;
						}
					}
			break;
			case 2 :	// end-tag
				if	(
					( elem->tag.c_str() == string("option") ) &&
					( include_path_flag )
					)
					include_path_flag = 0;
			break;
			case 3 :
				if	(
					( include_path_flag ) &&
					( elem->tag.c_str() == string("listOptionValue") )
					)
					{
					if	( elem->attr.count(string("value")) )
						printf(" %s\n", elem->attr[string("value")].c_str() );
					// printf("pos0=%d pos1=%d\n", elem->start_pos0, elem->end_pos1 );
					lareu2->letexte = tbuf;
					lareu2->lalen = ( elem->end_pos1 - elem->start_pos0 );
					lareu2->start = elem->start_pos0;
					if	( lareu2->matchav() > 0 )
						{
						int len = lareu2->ovector[3] - lareu2->ovector[2];
						int start = lareu2->ovector[2];
						printf(" |%.*s|\n", len, tbuf + start );
						}
					else if	( elem->end_pos1 > elem->start_pos0 )
						printf(" ?|%.*s|\n", ( elem->end_pos1 - elem->start_pos0 ),
								   tbuf + elem->start_pos0 );
					}
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

string fnam = string( argv[1] ); 
if	( fnam == string(".cproject") )
	retval = scan_ac6_cproject( xmlbuf, qxml );
	
return 0;
}
