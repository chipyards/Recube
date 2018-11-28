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

// petite fonction de comparaison rapide
static int txtncmp( const char* t1, const char* t2, int len )
{
for	( int i = 0; i < len; ++i )
	{
	if	( t1[i] == 0 ) return 1;
	if	( t2[i] == 0 ) return 2;
	if	( t1[i] != t2[i] ) return(3);
	}
return 0;
}

// traitement R/W d'un fichier projet, type selon fnam
int projfile::process_file( const char * fnam, const char * new_inc_path, const char * new_proj_name )
{
FILE * srcfil, *dstfil;
int retval = 0;
srcfil = fopen( fnam, "r" );
if	( srcfil == NULL )
	return 1;		// fichier non trouve, retour silencieux

printf("\n===== fichier %s ====\n", fnam );

if	( string(fnam) == string(".cproject") )
	{
	retval = scan_ac6_cproject( srcfil, stdout, new_inc_path );
	}
else if	( string(fnam) == string(".project") )
	{
	retval = scan_ac6_project( srcfil, stdout, new_inc_path, new_proj_name );
	}
else if	( string(fnam) == string("Project.uvprojx") )
	{
	retval = scan_keil_uvprojx( srcfil, stdout, new_inc_path );
	}
else if	( string(fnam) == string("Project.uvoptx") )
	{
	retval = scan_keil_uvoptx( srcfil, stdout, new_inc_path );
	}

if	( retval )
	gasp("erreur xml %d ligne %d", retval, lexml->curlin+1 );

if	( hits )
	printf("vu %d chemins de drivers\n", hits );

fclose( srcfil );

if	( ( hits == 0 ) && ( hits2 == 0 ) )
	return 2;		// aucun hit, retour silencieux
if	( dst_buf.size == 0 )
	return 3;		// aucune substitution, retour silencieux

if	( ( new_inc_path ) && ( hits ) )
	printf("a remplacer par %s\n", new_inc_path );
if	( ( new_proj_name ) && ( hits2 ) )
	printf("nouveau nom de projet %s\n", new_proj_name );

printf("sauver la version modifiee? (Y/N)\n");
fflush( stdout ); fflush( stdin );
if	( tolower(getchar()) == 'y' )
	{
	dstfil = fopen( fnam, "w" );
	if	( dstfil == NULL )
		gasp("echec ouverture %s", fnam );
	if	( fwrite( dst_buf.data, 1, dst_buf.size, dstfil ) != (size_t)dst_buf.size	)
		gasp("echec ecriture %s", fnam );
	fclose( dstfil );
	printf("c'est fait\n");
	}
else	printf("ok, bye\n");
return 0;
}

// un scan qui cherche les include paths et les drivers dans un .uvprojx
// remplace eventuellement les chemins sur les Drivers
int projfile::scan_keil_uvprojx( FILE * sfil, FILE * logfil, const char * patch )
{
int status, c;
int include_path_flag = 0, file_path_flag = 0;
int pos0, pos1, len, retval;

lareu1 = new pcreux( "([.][.][^;]*[/\\\\]|[A-Za-z]:[^;]*[/\\\\])Drivers" );	// link de Driver
lareu2 = new pcreux( "[A-Za-z]:.*[/\\\\]" );				// link absolu (pour info)
				// ^^^^ le compilateur C va deja enlever deux '\', pcre va enlever encore 1

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
			case 1 :	// start-tag : on cherche un tag <IncludePath> ou <FilePath> non vide
				if	( elem->tag == string("IncludePath") )
					include_path_flag = 1;
				if	( elem->tag == string("FilePath") )
					file_path_flag = 1;
			break;
			case 2 :	// end-tag
				if	(
					( elem->tag == string("IncludePath") ) &&
					( include_path_flag )
					)
					{
					include_path_flag = 0;
					// on delimite l'inner :
					pos0 = elem->start_pos1 + 1;	// enlever le '>'
					pos1 = elem->end_pos0 - 1;	// enlever le '<'
					len = pos1 - pos0;
					if	( len > 0 )
						{
						// fprintf( logfil, "file path = \"%.*s\"\n", len, src_buf.data + pos0 );
						lareu1->letexte = src_buf.data;
						lareu1->lalen = pos1;
						lareu1->start = pos0;
						do	{
							retval = lareu1->matchav();
							if	( retval > 0 )
								{
								pos0 = lareu1->ovector[2];
								pos1 = lareu1->ovector[3];
								len = pos1 - pos0;
								if	( len > 0 )
									{
									fprintf( logfil, "driver path : \"%.*s\"\n", len, src_buf.data + pos0 );
									++hits;
									if	( ( patch ) && ( txtncmp( src_buf.data + pos0, patch, len ) ) )
										{
										int ic = 0;
										while	( src_ic < pos0 )
											dst_buf.push( src_buf.data[src_ic++] );
										while	( ( c = patch[ic++] ) )
											dst_buf.push( c );
										src_ic = pos1;
										}
									}
								// matchav a mis a jour lareu1->start pour le prochain tour
								}
							} while ( retval > 0 );
						} // if len
					} // if tag
				if	(
					( elem->tag == string("FilePath") ) &&
					( file_path_flag )
					)
					{
					file_path_flag = 0;
					// on delimite l'inner :
					pos0 = elem->start_pos1 + 1;	// enlever le '>'
					pos1 = elem->end_pos0 - 1;	// enlever le '<'
					len = pos1 - pos0;
					if	( len > 0 )
						{
						// fprintf( logfil, "file path = \"%.*s\"\n", len, src_buf.data + pos0 );
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
								fprintf( logfil, "driver link : \"%.*s\"\n", len, src_buf.data + pos0 );
								++hits;
								if	( ( patch ) && ( txtncmp( src_buf.data + pos0, patch, len ) ) )
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
									fprintf( logfil, "link absolu : \"%.*s\" (pour info)\n", len, src_buf.data + pos0 );
									}
								}
							// else	fprintf( logfil, "?%d?|%.*s|\n", retval, len, src_buf.data + pos0 );
							}
						} // if len
					} // if tag
			break;
			case 3 :	// empty element tag : pas dans ce fichier
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

// un scan qui cherche les drivers dans un .uvoptx
// remplace eventuellement les chemins sur les Drivers
int projfile::scan_keil_uvoptx( FILE * sfil, FILE * logfil, const char * patch )
{
int status, c;
int file_path_flag = 0;
int pos0, pos1, len, retval;

lareu1 = new pcreux( "([.][.][^;]*[/\\\\]|[A-Za-z]:[^;]*[/\\\\])Drivers" );	// link de Driver
lareu2 = new pcreux( "[A-Za-z]:.*[/\\\\]" );				// link absolu (pour info)
				// ^^^^ le compilateur C va deja enlever deux '\', pcre va enlever encore 1

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
			case 1 :	// start-tag : on cherche un tag <PathWithFileName> non vide
				if	( elem->tag == string("PathWithFileName") )
					file_path_flag = 1;
			break;
			case 2 :	// end-tag
				if	(
					( elem->tag == string("PathWithFileName") ) &&
					( file_path_flag )
					)
					{
					file_path_flag = 0;
					// on delimite l'inner :
					pos0 = elem->start_pos1 + 1;	// enlever le '>'
					pos1 = elem->end_pos0 - 1;	// enlever le '<'
					len = pos1 - pos0;
					if	( len > 0 )
						{
						// fprintf( logfil, "file path = \"%.*s\"\n", len, src_buf.data + pos0 );
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
								fprintf( logfil, "driver link : \"%.*s\"\n", len, src_buf.data + pos0 );
								++hits;
								if	( ( patch ) && ( txtncmp( src_buf.data + pos0, patch, len ) ) )
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
									fprintf( logfil, "link absolu : \"%.*s\" (pour info)\n", len, src_buf.data + pos0 );
									}
								}
							// else	fprintf( logfil, "?%d?|%.*s|\n", retval, len, src_buf.data + pos0 );
							}
						} // if len
					} // if tag
			break;
			case 3 :	// empty element tag : pas dans ce fichier
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

// un scan qui cherche les include paths dans un .cproject
// remplace eventuellement les chemins relatifs sur les Drivers
int projfile::scan_ac6_cproject( FILE * sfil, FILE * logfil, const char * patch )
{
int status, c;
int include_path_flag = 0;
string * s; int pos0, pos1, len, retval;

//			id chez AC6				id chez Atollic
lareu1 = new pcreux( "gnu.c.compiler.option.include.paths|gcc.directories.select" );
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
					( elem->tag == string("option") ) &&
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
					( elem->tag == string("option") ) &&
					( include_path_flag )
					)
					include_path_flag = 0;
			break;
			case 3 :	// empty element tag : on cherche les tags <listOptionValue> a l'interieur du tag <option>
				if	(
					( include_path_flag ) &&
					( elem->tag == string("listOptionValue") )
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
								if	( ( patch ) && ( txtncmp( src_buf.data + pos0, patch, len ) ) )
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

// un scan qui cherche le nom et les gros links relatifs ou absolus dans un .project
// remplace eventuellement les links relatifs sur les Drivers, et le nom
int projfile::scan_ac6_project( FILE * sfil, FILE * logfil, const char * patch, const char * patch2 )
{
int status, c;
int name_flag = 0, location_flag = 0;
string * s; int pos0, pos1, len, retval;

lareu1 = new pcreux( "(PARENT-[5-9]-PROJECT_LOC.|[A-Za-z]:.*[/\\\\])Drivers" );	// link de Driver
lareu2 = new pcreux( "[A-Za-z]:.*[/\\\\]" );					// link absolu pour info
				// ^^^^ le compilateur C va deja enlever deux '\', pcre va enlever encore 1

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
				if	( elem->tag == string("name") )
					{
					len = lexml->stac.size();
					if	( len >= 2 )
						{
						s = &lexml->stac[len-2].tag;
						if	( *s == string("projectDescription") )
							name_flag = 1;
						}
					}
				else if	( elem->tag == string("location") )
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
					( elem->tag == string("name") ) &&
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
						++hits2;
						if	( ( patch2 ) && ( txtncmp( src_buf.data + pos0, patch2, len ) ) )
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
					( elem->tag == string("location") ) &&
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
								fprintf( logfil, "driver link : \"%.*s\"\n", len, src_buf.data + pos0 );
								++hits;
								if	( ( patch ) && ( txtncmp( src_buf.data + pos0, patch, len ) ) )
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
									fprintf( logfil, "link absolu : \"%.*s\" (pour info)\n", len, src_buf.data + pos0 );
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

/* un scan generique qui affiche l'inner des elements et l'attribut 'id'
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
*/
