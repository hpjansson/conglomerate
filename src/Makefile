CC = gcc

CFLAGS = -g `pkg-config --cflags gtk+-2.0 libxml-2.0 libbonoboui-2.0 libgnomeui-2.0` -Wall

OBJ = icon_assemble.o icon_connect.o icon_new.o icon_openfile.o icon_submit.o \
      ilogo.o auth_on.o auth_off.o dispspec.o new_doc.o \
      position.o cursor.o select.o popup.o xmledit.o xmlview.o \
      tree.o tchoice.o xmldata.o file.o open.o save.o cong-document.o \
      cong-location.o xmlload.o \
	cong-error-dialog.o cong-error-file-open.o cong-error-tests.o

LIBS = -lflux-old

all: conge

clean:
	rm -f conge conge.exe *.o

conge: $(OBJ) main.c global.h
	gcc main.c $(OBJ) `pkg-config --libs gtk+-2.0 libxml-2.0 libbonoboui-2.0 libgnomeui-2.0` $(LIBS) $(CFLAGS) -o conge
