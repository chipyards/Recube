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

/*
// un scan qui cherche les include paths dans un .cproject
int scan_ac6_cproject( char * tbuf, int qbuf )
{
int status, srcpos;
xelem * elem;
xmlobj * lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu

int include_path_flag = 0;
pcreux * lareu1;
pcreux * lareu2;
string * s; int pos0, pos1, len, retval, total;

lareu1 = new pcreux( "gnu.c.compiler.option.include.paths" );
lareu2 = new pcreux( "value=\"(.*)Drivers" );

printf("chemins des drivers :\n");
total = 0;
// la boucle des chars
for	( srcpos = 0; srcpos < qbuf; ++srcpos )
	{
	status = lexml->proc1char( tbuf[srcpos] );
	if	( status )			// status 0 : rien a faire; status 1, 2, ou 3 : ok; status < 0 : erreur 
		{
		elem = &lexml->stac.back();
		switch	( status )
			{
			case 1 :	// start-tag : on cherche un tag <option> dont l'id contient "include.paths"
				if	(
					( elem->tag.c_str() == string("option") ) &&
					( elem->attr.count(string("id")) )
					)
					{
					s = &elem->attr[string("id")];
					lareu1->letexte = s->c_str();
					lareu1->lalen = s->size();
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
			case 3 :	// empty element tag : on cherche les tags <listOptionValue> a l'interieur du tag <option>
				if	(
					( include_path_flag ) &&
					( elem->tag.c_str() == string("listOptionValue") )
					)
					{
					// if	( elem->attr.count(string("value")) )	// pour verif seulemment
					//	printf(" %s\n", elem->attr[string("value")].c_str() );
					pos0 = elem->start_pos0; pos1 = elem->end_pos1; len = pos1 - pos0;
					// printf("pos0=%d len=%d\n", pos0, len );
					if	( len > 0 )
						{
						// lareu2->letexte = tbuf;		// solution qui ne marche pas
						// lareu2->lalen = len;			// pcre ne comptepas len a partir de pos
						// lareu2->start = pos0;		// mais a partir de 0
						// lareu2->letexte = tbuf + pos0;	// solution qui marche
						// lareu2->lalen = len;			// mais lareu2->ovector est relatif
						// lareu2->start = 0;			// a tbuf + pos0
						lareu2->letexte = tbuf;			// LA SOLUTION qu'on cherchait !
						lareu2->lalen = pos1;
						lareu2->start = pos0;
						retval = lareu2->matchav();
						if	( retval > 0 )
							{
							pos0 = lareu2->ovector[2];
							pos1 = lareu2->ovector[3];
							len = pos1 - pos0;
							if	( len > 0 )
								{
								printf("  \"%.*s\"\n", len, tbuf + pos0 );
								++total;
								}
							}
						// else	printf("?%d?|%.*s|\n", retval, len, tbuf + pos0 );
						}
					}
			break;
			default :
			gasp("%s ligne %d : syntaxe xml %d", lexml->filepath, (lexml->curlin+1), status );
			}
		}	// if status
	}	// for
return total;
}

// un scan qui cherche le nom et les gros links relatifs ou absolus dans un .project
int scan_ac6_project( char * tbuf, int qbuf )
{
int status, srcpos;
xelem * elem;
xmlobj * lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu

int name_flag = 0, location_flag = 0;
pcreux * lareu1, * lareu2;
string * s; int pos0, pos1, len, retval, total;

lareu1 = new pcreux( "PARENT-[5-9]-PROJECT_LOC" );	// "gros" link relatif
lareu2 = new pcreux( "[A-Z]:.*[/\\\\]" );	// le compilateur C va deja enlever deux '\', pcre va enlever encore 1

total = 0;
// la boucle des chars
for	( srcpos = 0; srcpos < qbuf; ++srcpos )
	{
	status = lexml->proc1char( tbuf[srcpos] );
	if	( status )			// status 0 : rien a faire; status 1, 2, ou 3 : ok; status < 0 : erreur 
		{
		elem = &lexml->stac.back();
		switch	( status )
			{
			case 1 :	// start-tag : on cherche un tag <name> dont le parent est un tag <projectDescription>
					// 	ou un tag <location> dont le parent est un tag <link>
				if	( elem->tag.c_str() == string("name") )
					{
					len = lexml->stac.size();
					if	( len >= 2 )
						{
						s = &lexml->stac[len-2].tag;
						if	( *s == string("projectDescription") )
							name_flag = 1;
						}
					}
				else if	( elem->tag.c_str() == string("location") )
					{
					len = lexml->stac.size();
					if	( len >= 2 )
						{
						s = &lexml->stac[len-2].tag;
						if	( *s == string("link") )
							location_flag = 1;
						}
					}
			break;
			case 2 :	// end-tag
				if	(
					( elem->tag.c_str() == string("name") ) &&
					( name_flag )
					)
					{
					// printf("name = %s\n", elem->inner.c_str() );	// pour verif seulemment
					// on delimite l'inner :
					pos0 = elem->start_pos1 + 1;	// enlever le '>'
					pos1 = elem->end_pos0 - 1;	// enlever le '<'
					len = pos1 - pos0;
					if	( len > 0 )
						printf("name = \"%.*s\"\n", len, tbuf + pos0 );
					name_flag = 0;
					}
				else if	(
					( elem->tag.c_str() == string("location") ) &&
					( location_flag )
					)
					{
					// printf("location = %s\n", elem->inner.c_str() );	// pour verif seulemment
					// on delimite l'inner :
					pos0 = elem->start_pos1 + 1;	// enlever le '>'
					pos1 = elem->end_pos0 - 1;	// enlever le '<'
					len = pos1 - pos0;
					if	( len > 0 )
						{
						// printf("location = \"%.*s\"\n", len, tbuf + pos0 );
						lareu1->letexte = tbuf;
						lareu1->lalen = pos1;
						lareu1->start = pos0;
						retval = lareu1->matchav();
						if	( retval > 0 )
							{
							pos0 = lareu1->ovector[0];
							pos1 = lareu1->ovector[1];
							len = pos1 - pos0;
							if	( len > 0 )
								{
								printf("gros link relatif : \"%.*s\"\n", len, tbuf + pos0 );
								++total;
								}
							}
						else	{
							//printf("?%d?|%.*s|\n", retval, len, tbuf + pos0 );
							lareu2->letexte = tbuf;
							lareu2->lalen = pos1;
							lareu2->start = pos0;
							retval = lareu2->matchav();
							if	( retval > 0 )
								{
								pos0 = lareu2->ovector[0];
								pos1 = lareu2->ovector[1];
								len = pos1 - pos0;
								if	( len > 0 )
									{
									printf("link absolu : \"%.*s\"\n", len, tbuf + pos0 );
									++total;
									}
								}
							// else	printf("?%d?|%.*s|\n", retval, len, tbuf + pos0 );
							}
						}
					location_flag = 0;
					}
			break;
			case 3 :	// empty element tag : sans objet dans .project
			break;
			default :
			gasp("%s ligne %d : syntaxe xml %d", lexml->filepath, (lexml->curlin+1), status );
			}
		}	// if status
	}	// for
return total;
}

*/

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

retval = leproj->scan_xml( srcfil, stdout );

if	( retval )
	gasp("oups %d ligne %d", retval, leproj->lexml->curlin+1 );

/*
string fnam = string( argv[1] ); 
if	( fnam == string(".cproject") )
	{
	retval = scan_ac6_cproject( xmlbuf, qxml );
	printf("vu %d chemins de drivers\n", retval );
	}
else if	( fnam == string(".project") )
	{
	retval = scan_ac6_project( xmlbuf, qxml );
	printf("vu %d links relatif de plus de 4 niveaux\n", retval );
	}
*/
return 0;
}
