#include <stdio.h>
#include <stdarg.h>
#include <pcre.h>

using namespace std;
#include <fstream>
#include <string>
#include <map>
#include <vector>

extern "C" {
void gasp( const char *fmt, ... );  /* fatal error handling */
}

#include "pcreux.h"
#include "xmlpe.h"
#include "charvec.h"
#include "projfile.h"


/*
void projfile::push_src( int c )	// allocation memoire incluse
{
if	( qsrc >= capsrc )
	{
	if	( capsrc == 0 )
		{
		capsrc = INIT_ALLOC;
		src_buf = (char *)malloc( capsrc );
		}
	else	{
		capsrc *= 2;
		src_buf = (char *)realloc( (void *)src_buf, capsrc );
		}
	if	( src_buf == NULL )
		gasp("echec alloc memoire %d", capsrc );
	}
src_buf[qsrc++] = c;
}
*/

// un scan generique qui affiche l'inner des elements et l'attribut 'id'
int projfile::scan_xml( FILE * sfil, FILE * logfil )
{
int status, c;
int level = 0;

// la boucle des chars
while	( ( c = fgetc( sfil ) ) != EOF )
	{
	status = lexml->proc1char( c );
	if	( status )			// status 0 : rien a faire; status 1, 2, ou 3 : ok; status < 0 : erreur 
		{
		elem = &lexml->stac.back();
		switch	( status )
			{
			case 1 :	// start-tag
				fprintf( logfil, "%*s", 1+(level++)*3, " " );	// indent!
				fprintf( logfil, "%s", elem->tag.c_str() );
				if	( elem->attr.count(string("id")) )
					fprintf( logfil, " id=%s", elem->attr[string("id")].c_str() );
				fprintf( logfil, "...\n");
			break;
			case 2 :	// end-tag
				fprintf( logfil, "%*s", 1+(--level)*3, " " );
				fprintf( logfil, "%s", elem->tag.c_str() );
				if	( elem->inner.size() )
					fprintf( logfil, " '%s'", elem->inner.c_str() );
				fprintf( logfil, "\n");
			break;
			case 3 :
				fprintf( logfil, "%*s", 1+(level)*3, " " );
				fprintf( logfil, "%s", elem->tag.c_str() );
				if	( elem->attr.count(string("id")) )
					fprintf( logfil, " id=%s", elem->attr[string("id")].c_str() );
				fprintf( logfil, "\n");
			break;
			default :
				return status;
			// gasp("%s ligne %d : syntaxe xml %d", lexml->filepath, (lexml->curlin+1), status );
			}
		}	// if status
	}	// for
return 0;

}

// un scan qui cherche les include paths dans un .cproject
// remplace eventuellement les chemins relatifs sur les Drivers
int projfile::scan_ac6_cproject( FILE * sfil, FILE * logfil, const char * patch )
{
int status, c;
int include_path_flag = 0;
string * s; int pos0, pos1, len, retval;

lareu1 = new pcreux( "gnu.c.compiler.option.include.paths" );
lareu2 = new pcreux( "value=\"(.*)Drivers" );

fprintf( logfil, "chemins des drivers :\n");

// la boucle des chars
while	( ( c = fgetc( sfil ) ) != EOF )
	{
	src_buf.push( c );
	status = lexml->proc1char( c );
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
						fprintf( logfil, "id=%s\n", elem->attr[string("id")].c_str() );
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
					//	fprintf( logfil, " %s\n", elem->attr[string("value")].c_str() );
					pos0 = elem->start_pos0; pos1 = elem->end_pos1; len = pos1 - pos0;
					// fprintf( logfil, "pos0=%d len=%d\n", pos0, len );
					if	( len > 0 )
						{
						lareu2->letexte = src_buf.data;		// LA SOLUTION qu'on cherchait !
						lareu2->lalen = pos1;			// pcre ne compte pas len a partir de start
						lareu2->start = pos0;			// mais a partir de 0
						retval = lareu2->matchav();
						if	( retval > 0 )
							{
							pos0 = lareu2->ovector[2];
							pos1 = lareu2->ovector[3];
							len = pos1 - pos0;
							if	( len > 0 )
								{
								fprintf( logfil, "  \"%.*s\"\n", len, src_buf.data + pos0 );
								++hits;
								if	( patch )
									{
									int ic = 0;
									while	( src_ic < pos0 )
										dst_buf.push( src_buf.data[src_ic++] );
									while	( ( c = patch[ic++] ) )
										dst_buf.push( c );
									src_ic = pos1;
									}
								}
							}
						// else	fprintf( logfil, "?%d?|%.*s|\n", retval, len, tbuf + pos0 );
						}
					}
			break;
				return status;
			}
		}	// if status
	}	// for
if	( src_ic > 0 )
	{
	while	( src_ic < src_buf.size )
		dst_buf.push( src_buf.data[src_ic++] );
	}
return 0;
}

// un scan qui cherche le nom et les gros links relatifs ou absolus dans un .project
// remplace eventuellement les links relatifs sur les Drivers, et le nom
int projfile::scan_ac6_project( FILE * sfil, FILE * logfil, const char * patch, const char * patch2 )
{
int status, c;
int name_flag = 0, location_flag = 0;
string * s; int pos0, pos1, len, retval;

lareu1 = new pcreux( "(PARENT-[5-9]-PROJECT_LOC.)Drivers" );	// "gros" link relatif
lareu2 = new pcreux( "[A-Z]:.*[/\\\\]" );	// le compilateur C va deja enlever deux '\', pcre va enlever encore 1

// la boucle des chars
while	( ( c = fgetc( sfil ) ) != EOF )
	{
	src_buf.push( c );
	status = lexml->proc1char( c );
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
					// fprintf( logfil, "name = %s\n", elem->inner.c_str() );	// pour verif seulemment
					// on delimite l'inner :
					pos0 = elem->start_pos1 + 1;	// enlever le '>'
					pos1 = elem->end_pos0 - 1;	// enlever le '<'
					len = pos1 - pos0;
					if	( len > 0 )
						{
						fprintf( logfil, "name = \"%.*s\"\n", len, src_buf.data + pos0 );
						if	( patch2 )
							{
							int ic = 0;
							while	( src_ic < pos0 )
								dst_buf.push( src_buf.data[src_ic++] );
							while	( ( c = patch2[ic++] ) )
								dst_buf.push( c );
							src_ic = pos1;
							}
						}
					name_flag = 0;
					}
				else if	(
					( elem->tag.c_str() == string("location") ) &&
					( location_flag )
					)
					{
					// fprintf( logfil, "location = %s\n", elem->inner.c_str() );	// pour verif seulemment
					// on delimite l'inner :
					pos0 = elem->start_pos1 + 1;	// enlever le '>'
					pos1 = elem->end_pos0 - 1;	// enlever le '<'
					len = pos1 - pos0;
					if	( len > 0 )
						{
						// fprintf( logfil, "location = \"%.*s\"\n", len, src_buf.data + pos0 );
						lareu1->letexte = src_buf.data;
						lareu1->lalen = pos1;
						lareu1->start = pos0;
						retval = lareu1->matchav();
						if	( retval > 0 )
							{
							pos0 = lareu1->ovector[2];
							pos1 = lareu1->ovector[3];
							len = pos1 - pos0;
							if	( len > 0 )
								{
								fprintf( logfil, "driver link relatif : \"%.*s\"\n", len, src_buf.data + pos0 );
								++hits;
								if	( patch )
									{
									int ic = 0;
									while	( src_ic < pos0 )
										dst_buf.push( src_buf.data[src_ic++] );
									while	( ( c = patch[ic++] ) )
										dst_buf.push( c );
									src_ic = pos1;
									}
								}
							}
						else	{
							//fprintf( logfil, "?%d?|%.*s|\n", retval, len, src_buf.data + pos0 );
							lareu2->letexte = src_buf.data;
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
									fprintf( logfil, "link absolu : \"%.*s\"\n", len, src_buf.data + pos0 );
									}
								}
							// else	fprintf( logfil, "?%d?|%.*s|\n", retval, len, src_buf.data + pos0 );
							}
						}
					location_flag = 0;
					}
			break;
			case 3 :	// empty element tag : sans objet dans .project
			break;
			default :
				return status;
			}
		}	// if status
	}	// for
if	( src_ic > 0 )
	{
	while	( src_ic < src_buf.size )
		dst_buf.push( src_buf.data[src_ic++] );
	}
return 0;
}


