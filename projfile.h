
class projfile {
public:

charvec src_buf;	// source buffer
charvec dst_buf;	// dest buffer

int src_ic;		// index de copie dans src_buf
int dst_ic;		// index de copie dans dst_buf
int hits;

xelem * elem;		// un element xml
xmlobj * lexml;		// un parseur xml

pcreux * lareu1;	// matcheur de reg. exp.
pcreux * lareu2;

// constructeur
projfile() : src_ic(0), dst_ic(0), hits(0) {
  lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu
  }

// un scan generique qui affiche l'inner des elements et l'attribut 'id'
#define INIT_ALLOC	1024
int scan_xml( FILE * sfil, FILE * logfil );
// un scan qui cherche les include paths dans un .cproject et eventuellement les remplace par le patch
int scan_ac6_cproject( FILE * sfil, FILE * logfil, const char * patch = NULL );
// un scan qui cherche le nom et les gros links relatifs ou absolus dans un .project
int scan_ac6_project( FILE * sfil, FILE * logfil, const char * patch = NULL, const char * patch2 = NULL );

};
