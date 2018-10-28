
class projfile {
public:

char * src_buf;		// source buffer
char * dst_buf;		// dest buffer
int qsrc;		// contenu source buffer
int qdst;		// contenu dest buffer
int capsrc;		// contenu source buffer
int capdst;		// contenu dest buffer

int src_ic;		// index de copie dans src_buf
int dst_ic;		// index de copie dans dst_buf
int hits;

xelem * elem;		// un element xml
xmlobj * lexml;		// un parseur xml

pcreux * lareu1;	// matcheur de reg. exp.
pcreux * lareu2;

// constructeur
projfile() : qsrc(0), qdst(0), capsrc(0), capdst(0), src_ic(0), dst_ic(0), hits(0) {
lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu
}

// methodes

void push_src( int c );	// allocation memoire incluse
void push_dst( int c );

// un scan generique qui affiche l'inner des elements et l'attribut 'id'
#define INIT_ALLOC	1024
int scan_xml( FILE * sfil, FILE * logfil );
// un scan qui cherche les include paths dans un .cproject
int scan_ac6_cproject( FILE * sfil, FILE * logfil );
// un scan qui cherche le nom et les gros links relatifs ou absolus dans un .project
int scan_ac6_project( FILE * sfil, FILE * logfil );

};
