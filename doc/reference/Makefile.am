## Process this file with automake to produce Makefile.in

# See the GTK+ Makefiles in gtk+/docs/reference for examples
# of using the various options.

# The name of the module.
DOC_MODULE=conglomerate

# The top-level SGML file.
DOC_MAIN_SGML_FILE=conglomerate-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting functions and macros.
DOC_SOURCE_DIR=../../src

# Extra options to pass to gtkdoc-scanobj or gtkdoc-scangobj.
SCANOBJ_OPTIONS=

# Extra options to supply to gtkdoc-scan.
SCAN_OPTIONS=

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=--sgml-mode --output-format=XML

# Extra options to supply to gtkdoc-fixref.
FIXXREF_OPTIONS=

# Used for dependencies.
HFILE_GLOB=
CFILE_GLOB=

# Header files to ignore when scanning.
IGNORE_HFILES=

# Images to copy into HTML directory.
HTML_IMAGES =

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = version.xml

# Other files to distribute.
extra_files = version.xml.in

# CFLAGS and LDFLAGS for compiling scan program. Only needed if your app/lib
# contains GtkObjects/GObjects and you want to document signals and properties.
GTKDOC_CFLAGS = $(CONGLOMERATE_CFLAGS) -I$(top_srcdir)/src
GTKDOC_LIBS = $(CONGLOMERATE_LIBS) $(GTKSOURCEVIEW_LIBS) $(PRINTING_LIBS) \
	$(filter-out ../../src/main.o,$(wildcard ../../src/*.o))

GTKDOC_CC=$(LIBTOOL) --mode=compile $(CC)
GTKDOC_LD=$(LIBTOOL) --mode=link $(CC)

# If you need to override some of the declarations, place them in the
# $(DOC_MODULE)-overrides.txt file and uncomment the second line here.
DOC_OVERRIDES =
# DOC_OVERRIDES = $(DOC_MODULE)-overrides.txt



###########################################################################
# Everything below here is generic and you shouldn't need to change it.
###########################################################################

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

EXTRA_DIST = 				\
	$(content_files)		\
	$(extra_files)			\
	$(HTML_IMAGES)			\
	$(DOC_MAIN_SGML_FILE)		\
	$(DOC_MODULE).types		\
	$(DOC_MODULE)-sections.txt	\
	$(DOC_OVERRIDES)

DOC_STAMPS=scan-build.stamp tmpl-build.stamp sgml-build.stamp html-build.stamp \
	   $(srcdir)/tmpl.stamp $(srcdir)/sgml.stamp $(srcdir)/html.stamp

SCANOBJ_FILES = 		 \
	$(DOC_MODULE).args 	 \
	$(DOC_MODULE).hierarchy  \
	$(DOC_MODULE).interfaces \
	$(DOC_MODULE).prerequisites \
	$(DOC_MODULE).signals

if ENABLE_GTK_DOC
all-local: html-build.stamp

#### scan ####

scan-build.stamp: $(HFILE_GLOB)
	@echo '*** Building conglomerate.types ***'
	@echo '#include "global.h"' > conglomerate.types
	for i in $(top_srcdir)/src/*.h ; do \
	    if [[  $$i != "$(top_srcdir)/src/cong-editor-widget3-impl.h" && $$i != "$(top_srcdir)/src/global.h" ]] ; then \
		echo "#include \"`basename $$i`\"" >> conglomerate.types ; \
	    fi ; \
	done
	echo >> conglomerate.types
# here we have a hack that outputs cong_editor_widget3_get_type istead of cong_editor_widget_get_type
# (found in cong-editor-widget.h)
# also, we ignore cong-editor-widget3-impl.h and global.h
	for i in $(top_srcdir)/src/*.h ; do \
	    if [[ $$i != "$(top_srcdir)/src/cong-editor-widget3-impl.h" && $$i != "$(top_srcdir)/src/global.h" ]] ; \
	    then \
		A=`grep 'get_type' $$i | grep -v '#define'` ; \
		if [[ $$A != "" ]] ; \
		then \
		    B=`echo $$A | sed '{s/\ .void.;//g}'` ; \
		    if [[ $$B = "cong_editor_widget_get_type" ]] ; \
		    then \
			echo 'cong_editor_widget3_get_type' >> conglomerate.types ; \
		    else \
			echo $$B >> conglomerate.types ; \
		    fi ; \
		fi ; \
	    fi ; \
	done
	@echo '*** Scanning header files ***'
	if grep -l '^..*$$' $(srcdir)/$(DOC_MODULE).types > /dev/null ; then \
	    CC="$(GTKDOC_CC)" LD="$(GTKDOC_LD)" CFLAGS="$(GTKDOC_CFLAGS)" LDFLAGS="$(GTKDOC_LIBS)" gtkdoc-scangobj $(SCANOBJ_OPTIONS) --module=$(DOC_MODULE) --output-dir=$(srcdir) ;\
	else \
	    cd $(srcdir) ; \
	    for i in $(SCANOBJ_FILES) ; do \
               test -f $$i || touch $$i ; \
	    done \
	fi
	cd $(srcdir) && \
	  gtkdoc-scan --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR) --ignore-headers="$(IGNORE_HFILES)" $(SCAN_OPTIONS) $(EXTRA_HFILES)
	touch scan-build.stamp

$(DOC_MODULE)-decl.txt $(SCANOBJ_FILES): scan-build.stamp
	@true

#### templates ####

tmpl-build.stamp: $(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(DOC_MODULE)-sections.txt $(DOC_OVERRIDES)
	@echo '*** Rebuilding template files ***'
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)
	@echo '*** Building conglomerate-overrides.txt ***'
	@test -e $(srcdir)/conglomerate-overrides.txt && rm -f $(srcdir)/conglomerate-overrides.txt ; \
	let CONGOBJNUM=0 ; \
	for i in $(top_srcdir)/src/*.h ; \
	do \
	    ibase=`basename $$i` ; \
	    if [[ $$ibase != "global.h" && $$ibase != "*.h" && $$ibase != "cong-editor-widget3-impl.h" ]] ; \
	    then \
		if [[ `grep -c 'get_type' $$i` -ne 0 ]] ; \
		then \
		    A=`grep -e 'get_type' $$i | grep -v '#define'` ; \
		    B=$${A%%_get_type (void);} ; \
		    C=`echo $$B | sed -e {s/_//g}` ; \
		    D=`grep -i "struct $$C" $$i | grep -v "Class" | grep -v "Details" | grep -v "typedef" | sed {s/struct\ //1}` ; \
		    if [[ $$D != "" ]] ; \
		    then \
			echo '<STRUCT>' >> $(srcdir)/conglomerate-overrides.txt ; \
			echo '<NAME>'$$D'</NAME>' >> $(srcdir)/conglomerate-overrides.txt ; \
			echo '</STRUCT>' >> $(srcdir)/conglomerate-overrides.txt ; \
			echo 'Adding '$$D ; \
			let CONGOBJNUM+=1 ; \
		    fi ; \
		fi ; \
	    fi ; \
	done ; \
	echo "*** $$CONGOBJNUM object declarations found ***"
	@echo '*** Adding object declarations to conglomerate-sections.txt ***'
	mv $(srcdir)/conglomerate-sections.txt $(srcdir)/conglomerate-sections.txt.template
	@FILE_LINE= ; \
	let CONGOBJNUM=0 ; \
	CONG_SECTIONS=`cat $(srcdir)/conglomerate-sections.txt.template` ; \
	for line in $$CONG_SECTIONS ; \
	do \
	    [[ `echo "$$line" | grep "<FILE>"` != "" ]] && FILE_LINE=`echo "$$line" | grep "<FILE>"` ; \
	    A=`expr match "$$line" '.*_TYPE'` ; \
	    if [[ $$A != "0" ]] ; \
	    then \
		FILENAME=$(top_srcdir)/src/`echo $$FILE_LINE | sed {s/\<FILE\>//g} | sed {s/\<.FILE\>//g}`.h ; \
		OBJNAME=`echo $$line | sed {s/_//g} | sed {s/TYPE//g}` ; \
		D=`grep -i "<.*>$$OBJNAME<.*>" $(srcdir)/conglomerate-overrides.txt | sed {s/\<NAME\>//g} | sed {s/\<.*NAME\>//g}` ; \
		echo '<TITLE>'$$D'</TITLE>' >> $(srcdir)/conglomerate-sections.txt ; \
		echo $$D >> $(srcdir)/conglomerate-sections.txt ; \
		echo 'Adding '$$D ; \
		let CONGOBJNUM+=1 ; \
	    else \
		echo $$line >> $(srcdir)/conglomerate-sections.txt ; \
	    fi ; \
	done ; \
	echo '*** Added '$$CONGOBJNUM' objects to conglomerate-sections.txt ***'
	rm -f $(srcdir)/conglomerate-sections.txt.template
	touch tmpl-build.stamp

tmpl.stamp: tmpl-build.stamp
	@true

#### sgml ####

sgml-build.stamp: tmpl.stamp $(CFILE_GLOB) $(srcdir)/tmpl/*.sgml
	@echo '*** Building SGML ***'
	cd $(srcdir) && \
	gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR) $(MKDB_OPTIONS)
	touch sgml-build.stamp

sgml.stamp: sgml-build.stamp
	@true

#### html ####

html-build.stamp: sgml.stamp $(DOC_MAIN_SGML_FILE) $(content_files)
	@echo '*** Building HTML ***'
	test -d $(srcdir)/html || mkdir $(srcdir)/html
	cd $(srcdir)/html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)
	test "x$(HTML_IMAGES)" = "x" || ( cd $(srcdir) && cp $(HTML_IMAGES) html )
	@echo '-- Fixing Crossreferences' 
	cd $(srcdir) && gtkdoc-fixxref --module-dir=html --html-dir=$(HTML_DIR) $(FIXXREF_OPTIONS)
	touch html-build.stamp
endif

##############

clean-local:
	rm -f *~ *.bak $(SCANOBJ_FILES) *-unused.txt $(DOC_STAMPS)

maintainer-clean-local: clean
	cd $(srcdir) && rm -rf sgml html $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(TARGET_DIR)
	(installfiles=`echo $(srcdir)/html/*.html`; \
	if test "$$installfiles" = '$(srcdir)/html/*.html'; \
	then echo '-- Nothing to install' ; \
	else \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	  done; \
	  echo '-- Installing $(srcdir)/html/index.sgml' ; \
	  $(INSTALL_DATA) $(srcdir)/html/index.sgml $(DESTDIR)$(TARGET_DIR); \
	fi)

#
# Require gtk-doc when making dist
#
if ENABLE_GTK_DOC
dist-check-gtkdoc:
else
dist-check-gtkdoc:
	@echo "*** gtk-doc must be installed and enabled in order to make dist"
	@false
endif

dist-hook: dist-check-gtkdoc dist-hook-local
	mkdir $(distdir)/tmpl
	mkdir $(distdir)/xml
	mkdir $(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl
	-cp $(srcdir)/sgml/*.xml $(distdir)/xml
	-cp $(srcdir)/html/index.sgml $(distdir)/html
	-cp $(srcdir)/html/*.html $(srcdir)/html/*.css $(distdir)/html

	images=$(HTML_IMAGES) ;    	      \
	for i in $$images ; do		      \
	  cp $(srcdir)/$$i $(distdir)/html ;  \
	done

.PHONY : dist-hook-local
