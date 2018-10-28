#include <stdio.h>
#include <pcre.h>

using namespace std;
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "pcreux.h"
#include "xmlpe.h"
#include "projfile.h"

static void indent( int n, FILE * fil )
{
int i;
for	( i = 0; i < n; ++i )
	fputc( ' ', fil );
}

// un scan generique qui affiche l'inner des elements et l'attribut 'id'
int projfile::scan_xml( FILE * sfil, FILE * logfil )
{
int status;
int level = 0;
int c;

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
				indent( (level++) * 3, logfil );
				fprintf( logfil, "%s", elem->tag.c_str() );
				if	( elem->attr.count(string("id")) )
					fprintf( logfil, " id=%s", elem->attr[string("id")].c_str() );
				fprintf( logfil, "...\n");
			break;
			case 2 :	// end-tag
				indent( (--level) * 3, logfil );
				fprintf( logfil, "%s", elem->tag.c_str() );
				if	( elem->inner.size() )
					fprintf( logfil, " '%s'", elem->inner.c_str() );
				fprintf( logfil, "\n");
			break;
			case 3 :
				indent( level * 3, logfil );
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