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
	$(top_srcdir)/src/auth_off.o \
	$(top_srcdir)/src/auth_on.o \
	$(top_srcdir)/src/cong-app.o \
	$(top_srcdir)/src/cong-attribute-editor-cdata.o \
	$(top_srcdir)/src/cong-attribute-editor-enumeration.o \
	$(top_srcdir)/src/cong-attribute-editor.o \
	$(top_srcdir)/src/cong-attribute-wrapper-check-button.o \
	$(top_srcdir)/src/cong-attribute-wrapper.o \
	$(top_srcdir)/src/cong-attribute-wrapper-radio-button.o \
	$(top_srcdir)/src/cong-command-history.o \
	$(top_srcdir)/src/cong-command.o \
	$(top_srcdir)/src/cong-debug-log-view.o \
	$(top_srcdir)/src/cong-dialog.o \
	$(top_srcdir)/src/cong-dispspec.o \
	$(top_srcdir)/src/cong-dispspec-registry.o \
	$(top_srcdir)/src/cong-document.o \
	$(top_srcdir)/src/cong-dom-view.o \
	$(top_srcdir)/src/cong-dtd.o \
	$(top_srcdir)/src/cong-editor-area-bin.o \
	$(top_srcdir)/src/cong-editor-area-border.o \
	$(top_srcdir)/src/cong-editor-area-composer.o \
	$(top_srcdir)/src/cong-editor-area-container.o \
	$(top_srcdir)/src/cong-editor-area-entity-decl.o \
	$(top_srcdir)/src/cong-editor-area-entity-ref.o \
	$(top_srcdir)/src/cong-editor-area-flow-holder-blocks.o \
	$(top_srcdir)/src/cong-editor-area-flow-holder-inlines.o \
	$(top_srcdir)/src/cong-editor-area-flow-holder.o \
	$(top_srcdir)/src/cong-editor-area-flow-holder-single.o \
	$(top_srcdir)/src/cong-editor-area-line.o \
	$(top_srcdir)/src/cong-editor-area.o \
	$(top_srcdir)/src/cong-editor-area-pixbuf.o \
	$(top_srcdir)/src/cong-editor-area-spacer.o \
	$(top_srcdir)/src/cong-editor-area-span-tag.o \
	$(top_srcdir)/src/cong-editor-area-structural.o \
	$(top_srcdir)/src/cong-editor-area-structural-tag.o \
	$(top_srcdir)/src/cong-editor-area-text-comment.o \
	$(top_srcdir)/src/cong-editor-area-text-fragment.o \
	$(top_srcdir)/src/cong-editor-area-text.o \
	$(top_srcdir)/src/cong-editor-area-underline.o \
	$(top_srcdir)/src/cong-editor-area-unknown-tag.o \
	$(top_srcdir)/src/cong-editor-child-policy-flow-holder.o \
	$(top_srcdir)/src/cong-editor-child-policy-inline.o \
	$(top_srcdir)/src/cong-editor-child-policy.o \
	$(top_srcdir)/src/cong-editor-line-fragments.o \
	$(top_srcdir)/src/cong-editor-node-comment.o \
	$(top_srcdir)/src/cong-editor-node-document.o \
	$(top_srcdir)/src/cong-editor-node-dtd.o \
	$(top_srcdir)/src/cong-editor-node-element.o \
	$(top_srcdir)/src/cong-editor-node-element-span.o \
	$(top_srcdir)/src/cong-editor-node-element-structural.o \
	$(top_srcdir)/src/cong-editor-node-element-unknown.o \
	$(top_srcdir)/src/cong-editor-node-entity-decl.o \
	$(top_srcdir)/src/cong-editor-node-entity-ref.o \
	$(top_srcdir)/src/cong-editor-node.o \
	$(top_srcdir)/src/cong-editor-node-text.o \
	$(top_srcdir)/src/cong-editor-node-unimplemented.o \
	$(top_srcdir)/src/cong-editor-widget3.o \
	$(top_srcdir)/src/cong-eel.o \
	$(top_srcdir)/src/cong-enum-mapping.o \
	$(top_srcdir)/src/cong-error-dialog.o \
	$(top_srcdir)/src/cong-error-file-open.o \
	$(top_srcdir)/src/cong-error-file-save.o \
	$(top_srcdir)/src/cong-error-tests.o \
	$(top_srcdir)/src/cong-file-export.o \
	$(top_srcdir)/src/cong-file-import.o \
	$(top_srcdir)/src/cong-file-new.o \
	$(top_srcdir)/src/cong-file-open.o \
	$(top_srcdir)/src/cong-file-print.o \
	$(top_srcdir)/src/cong-file-properties.o \
	$(top_srcdir)/src/cong-file-save.o \
	$(top_srcdir)/src/cong-font.o \
	$(top_srcdir)/src/cong-graph.o \
	$(top_srcdir)/src/cong-location.o \
	$(top_srcdir)/src/cong-marshal.o \
	$(top_srcdir)/src/cong-menus.o \
	$(top_srcdir)/src/cong-modification-cursor-change.o \
	$(top_srcdir)/src/cong-modification.o \
	$(top_srcdir)/src/cong-modification-selection-change.o \
	$(top_srcdir)/src/cong-modification-set-dtd-ptr.o \
	$(top_srcdir)/src/cong-node-modification-add-after.o \
	$(top_srcdir)/src/cong-node-modification-add-before.o \
	$(top_srcdir)/src/cong-node-modification-make-orphan.o \
	$(top_srcdir)/src/cong-node-modification.o \
	$(top_srcdir)/src/cong-node-modification-remove-attribute.o \
	$(top_srcdir)/src/cong-node-modification-set-attribute.o \
	$(top_srcdir)/src/cong-node-modification-set-parent.o \
	$(top_srcdir)/src/cong-node-modification-set-text.o \
	$(top_srcdir)/src/cong-node.o \
	$(top_srcdir)/src/cong-node-properties-dialog.o \
	$(top_srcdir)/src/cong-overview-view.o \
	$(top_srcdir)/src/cong-parser-error.o \
	$(top_srcdir)/src/cong-plugin.o \
	$(top_srcdir)/src/cong-primary-window.o \
	$(top_srcdir)/src/cong-progress-checklist.o \
	$(top_srcdir)/src/cong-range.o \
	$(top_srcdir)/src/cong-source-view.o \
	$(top_srcdir)/src/cong-text-cache.o \
	$(top_srcdir)/src/cong-tree-view.o \
	$(top_srcdir)/src/cong-util.o \
	$(top_srcdir)/src/cong-vfs.o \
	$(top_srcdir)/src/cong-view.o \
	$(top_srcdir)/src/cursor.o \
	$(top_srcdir)/src/file.o \
	$(top_srcdir)/src/ilogo.o \
	$(top_srcdir)/src/plugin-cleanup-source.o \
	$(top_srcdir)/src/plugin-convert-case.o \
	$(top_srcdir)/src/plugin-docbook.o \
	$(top_srcdir)/src/plugin-doc-from-xds.o \
	$(top_srcdir)/src/plugin-dtd.o \
	$(top_srcdir)/src/plugin-empty.o \
	$(top_srcdir)/src/plugin-fo.o \
	$(top_srcdir)/src/plugin-lists-area-listitem.o \
	$(top_srcdir)/src/plugin-lists-node-element-listitem.o \
	$(top_srcdir)/src/plugin-lists.o \
	$(top_srcdir)/src/plugin-paragraph-area-paragraph.o \
	$(top_srcdir)/src/plugin-paragraph-node-element-paragraph.o \
	$(top_srcdir)/src/plugin-paragraph.o \
	$(top_srcdir)/src/plugin-save-dispspec.o \
	$(top_srcdir)/src/plugin-sgml.o \
	$(top_srcdir)/src/plugin-tests.o \
	$(top_srcdir)/src/plugin-validate.o \
	$(top_srcdir)/src/plugin-website.o \
	$(top_srcdir)/src/plugin-xsl.o \
	$(top_srcdir)/src/popup.o \
	$(top_srcdir)/src/position.o \
	$(top_srcdir)/src/select.o \
	$(top_srcdir)/src/tchoice.o \
	$(top_srcdir)/src/tree.o \
	$(top_srcdir)/src/xmldata.o \
	$(top_srcdir)/src/xmledit.o \
	$(top_srcdir)/src/xmlload.o \
	$(top_srcdir)/src/xmlview.o

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
