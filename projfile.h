
class projfile {
public:

charvec src_buf;	// source buffer
charvec dst_buf;	// dest buffer

int src_ic;		// index de copie dans src_buf
int dst_ic;		// index de copie dans dst_buf
int hits;		// hits de chemins de bibliotheque
int hits2;		// hits de nom de projet

xelem * elem;		// un element xml
xmlobj * lexml;		// un parseur xml

pcreux * lareu1;	// matcheur de reg. exp.
pcreux * lareu2;

// constructeur
projfile() : src_ic(0), dst_ic(0), hits(0), hits2(0), lareu1(NULL), lareu2(NULL) {
  lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu
  }

// destructeur
~projfile() {
  if	( lareu1 ) delete lareu1;
  if	( lareu2 ) delete lareu2;
  }

#define INIT_ALLOC	1024

// un scan generique qui affiche l'inner des elements et l'attribut 'id'
// int scan_xml( FILE * sfil, FILE * logfil );
// un scan qui cherche les include paths dans un .cproject et eventuellement les remplace par le patch
int scan_ac6_cproject( FILE * sfil, FILE * logfil, const char * patch = NULL );
// un scan qui cherche le nom de projet et les links de drivers dans un .project
int scan_ac6_project( FILE * sfil, FILE * logfil, const char * patch = NULL, const char * patch2 = NULL );
// un scan qui cherche les include paths et links de drivers dans un .uvprojx
int scan_keil_uvprojx( FILE * sfil, FILE * logfil, const char * patch );
// un scan qui cherche les drivers dans un .uvoptx
int scan_keil_uvoptx( FILE * sfil, FILE * logfil, const char * patch );

// traitement R/W d'un fichier projet, type selon fnam
int process_file( const char * fnam, const char * new_inc_path, const char * new_proj_name );

};
