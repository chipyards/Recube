
class projfile {
public:

vector <char> src_buf;		// source buffer
vector <char> dst_buf;		// dest buffer

int src_ic;		// index de copie dans src_buf
int dst_ic;		// index de copie dans dst_buf

xelem * elem;		// un element xml
xmlobj * lexml;		// un parseur xml

pcreux * lareu1;	// matcheur de reg. exp.
pcreux * lareu2;

// constructeur
projfile() : src_ic(0), dst_ic(0) {
lexml = new xmlobj();	// ici on ne demande pas l'ouverture d'un istream, le fichier est deja lu
src_buf.reserve( 1 << 16 );
dst_buf.reserve( 1 << 16 );
}

// methodes

// un scan generique qui affiche l'inner des elements et l'attribut 'id'
int scan_xml( FILE * sfil, FILE * logfil );

};
