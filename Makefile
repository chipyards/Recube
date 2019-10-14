# note : pour un bon exemple de makefile GTK voir GTK/TEXTU2
# si pas de make, essayer mingw32-make

# listes
SOURCESC=
SOURCESCPP= xmlpe.cpp projfile.cpp appli.cpp
EXE= recube.exe
OBJS= $(SOURCESC:.c=.o) $(SOURCESCPP:.cpp=.o)
     
HEADERS= pcreux.h charvec.h xmlpe.h projfile.h

# INCS= -IC:\Appli\GnuWin32\include
INCS=

# LIBS= -LC:\Appli\GnuWin32\lib -lpcre
LIBS= -lpcre

# cibles

ALL : $(OBJS) 
	g++ -o $(EXE) $(OBJS) $(LIBS)

clean : 
	rm *.o

delo :
	del *.o

.cpp.o:
	g++ -Wall $(INCS) -c -O3 -Wno-misleading-indentation $<

.c.o:
	gcc -Wall $(INCS) -c -O3 $<

# dependances

appli.o : ${HEADERS}
xmlpe.o : ${HEADERS}
projfile.o : ${HEADERS}
