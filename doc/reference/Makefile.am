## Process this file with automake to produce Makefile.in

# This is a blank Makefile.am for using gtk-doc.
# Copy this to your project's API docs directory and modify the variables to
# suit your project. See the GTK+ Makefiles in gtk+/docs/reference for examples
# of using the various options.

# The name of the module, e.g. 'glib'.
DOC_MODULE=conglomerate

# The top-level SGML file. Change it if you want.
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
	../../src/auth_off.o \
	../../src/auth_on.o \
	../../src/cong-app.o \
	../../src/cong-attribute-editor-cdata.o \
	../../src/cong-attribute-editor-enumeration.o \
	../../src/cong-attribute-editor.o \
	../../src/cong-attribute-wrapper-check-button.o \
	../../src/cong-attribute-wrapper.o \
	../../src/cong-attribute-wrapper-radio-button.o \
	../../src/cong-command-history.o \
	../../src/cong-command.o \
	../../src/cong-debug-log-view.o \
	../../src/cong-dialog.o \
	../../src/cong-dispspec.o \
	../../src/cong-dispspec-registry.o \
	../../src/cong-document.o \
	../../src/cong-dom-view.o \
	../../src/cong-dtd.o \
	../../src/cong-editor-area-bin.o \
	../../src/cong-editor-area-border.o \
	../../src/cong-editor-area-composer.o \
	../../src/cong-editor-area-container.o \
	../../src/cong-editor-area-entity-decl.o \
	../../src/cong-editor-area-entity-ref.o \
	../../src/cong-editor-area-flow-holder-blocks.o \
	../../src/cong-editor-area-flow-holder-inlines.o \
	../../src/cong-editor-area-flow-holder.o \
	../../src/cong-editor-area-flow-holder-single.o \
	../../src/cong-editor-area-line.o \
	../../src/cong-editor-area.o \
	../../src/cong-editor-area-pixbuf.o \
	../../src/cong-editor-area-spacer.o \
	../../src/cong-editor-area-span-tag.o \
	../../src/cong-editor-area-structural.o \
	../../src/cong-editor-area-structural-tag.o \
	../../src/cong-editor-area-text-comment.o \
	../../src/cong-editor-area-text-fragment.o \
	../../src/cong-editor-area-text.o \
	../../src/cong-editor-area-underline.o \
	../../src/cong-editor-area-unknown-tag.o \
	../../src/cong-editor-child-policy-flow-holder.o \
	../../src/cong-editor-child-policy-inline.o \
	../../src/cong-editor-child-policy.o \
	../../src/cong-editor-line-fragments.o \
	../../src/cong-editor-node-comment.o \
	../../src/cong-editor-node-document.o \
	../../src/cong-editor-node-dtd.o \
	../../src/cong-editor-node-element.o \
	../../src/cong-editor-node-element-span.o \
	../../src/cong-editor-node-element-structural.o \
	../../src/cong-editor-node-element-unknown.o \
	../../src/cong-editor-node-entity-decl.o \
	../../src/cong-editor-node-entity-ref.o \
	../../src/cong-editor-node.o \
	../../src/cong-editor-node-text.o \
	../../src/cong-editor-node-unimplemented.o \
	../../src/cong-editor-widget3.o \
	../../src/cong-eel.o \
	../../src/cong-enum-mapping.o \
	../../src/cong-error-dialog.o \
	../../src/cong-error-file-open.o \
	../../src/cong-error-file-save.o \
	../../src/cong-error-tests.o \
	../../src/cong-file-export.o \
	../../src/cong-file-import.o \
	../../src/cong-file-new.o \
	../../src/cong-file-open.o \
	../../src/cong-file-print.o \
	../../src/cong-file-properties.o \
	../../src/cong-file-save.o \
	../../src/cong-font.o \
	../../src/cong-graph.o \
	../../src/cong-location.o \
	../../src/cong-marshal.o \
	../../src/cong-menus.o \
	../../src/cong-modification-cursor-change.o \
	../../src/cong-modification.o \
	../../src/cong-modification-selection-change.o \
	../../src/cong-modification-set-dtd-ptr.o \
	../../src/cong-node-modification-add-after.o \
	../../src/cong-node-modification-add-before.o \
	../../src/cong-node-modification-make-orphan.o \
	../../src/cong-node-modification.o \
	../../src/cong-node-modification-remove-attribute.o \
	../../src/cong-node-modification-set-attribute.o \
	../../src/cong-node-modification-set-parent.o \
	../../src/cong-node-modification-set-text.o \
	../../src/cong-node.o \
	../../src/cong-node-properties-dialog.o \
	../../src/cong-overview-view.o \
	../../src/cong-parser-error.o \
	../../src/cong-plugin.o \
	../../src/cong-primary-window.o \
	../../src/cong-progress-checklist.o \
	../../src/cong-range.o \
	../../src/cong-source-view.o \
	../../src/cong-text-cache.o \
	../../src/cong-tree-view.o \
	../../src/cong-util.o \
	../../src/cong-vfs.o \
	../../src/cong-view.o \
	../../src/cursor.o \
	../../src/file.o \
	../../src/ilogo.o \
	../../src/plugin-cleanup-source.o \
	../../src/plugin-convert-case.o \
	../../src/plugin-docbook.o \
	../../src/plugin-doc-from-xds.o \
	../../src/plugin-dtd.o \
	../../src/plugin-empty.o \
	../../src/plugin-fo.o \
	../../src/plugin-lists-area-listitem.o \
	../../src/plugin-lists-node-element-listitem.o \
	../../src/plugin-lists.o \
	../../src/plugin-paragraph-area-paragraph.o \
	../../src/plugin-paragraph-node-element-paragraph.o \
	../../src/plugin-paragraph.o \
	../../src/plugin-save-dispspec.o \
	../../src/plugin-sgml.o \
	../../src/plugin-tests.o \
	../../src/plugin-validate.o \
	../../src/plugin-website.o \
	../../src/plugin-xsl.o \
	../../src/popup.o \
	../../src/position.o \
	../../src/select.o \
	../../src/tchoice.o \
	../../src/tree.o \
	../../src/xmldata.o \
	../../src/xmledit.o \
	../../src/xmlload.o \
	../../src/xmlview.o

GTKDOC_CC=$(LIBTOOL) --mode=compile $(CC)
GTKDOC_LD=$(LIBTOOL) --mode=link $(CC)

# If you need to override some of the declarations, place them in the
# $(DOC_MODULE)-overrides.txt file and uncomment the second line here.
DOC_OVERRIDES =
#DOC_OVERRIDES = $(DOC_MODULE)-overrides.txt



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
	$(DOC_MODULE)-sections.txt	
#	$(DOC_OVERRIDES)

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
