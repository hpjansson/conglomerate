bin_PROGRAMS = conglomerate

if ENABLE_PRINTING
printing_sources = \
  fo.h                      \
  fo-parser-result.c        \
  fo-print-context.c 	    \
  fo-rect.c                 \
  fo-solver-result.c        \
  fo-tests.c
else
printing_sources = 
endif

conglomerate_SOURCES =      \
  global.h                  \
  cong-app.h                \
  cong-dialog.h             \
  cong-dispspec.h           \
  cong-dispspec-registry.h  \
  cong-document.h           \
  cong-dtd.h                \
  cong-editor-widget-impl.h \
  cong-eel.h                \
  cong-error-dialog.h       \
  cong-fake-plugin-hooks.h  \
  cong-font.h               \
  cong-graph.h              \
  cong-location.h           \
  cong-node.h               \
  cong-overview-view.h      \
  cong-parser-error.h       \
  cong-plugin.h             \
  cong-progress-checklist.h \
  cong-tree-view.h          \
  cong-view.h               \
  fo.h                      \
  cong-app.c                \
  cong-debug-log-view.c     \
  cong-dialog.c             \
  cong-dispspec.c           \
  cong-dispspec-registry.c  \
  cong-document.c           \
  cong-dom-view.c           \
  cong-dtd.c                \
  cong-dummy-element-editor.c  \
  cong-editor-widget.c      \
  cong-eel.c                \
  cong-element-editor.c     \
  cong-error-dialog.c       \
  cong-error-file-open.c    \
  cong-error-file-save.c    \
  cong-error-tests.c        \
  cong-file-new.c           \
  cong-file-open.c          \
  cong-file-save.c          \
  cong-file-export.c        \
  cong-file-import.c        \
  cong-file-print.c         \
  cong-font.c               \
  cong-graph.c              \
  cong-location.c           \
  cong-menus.c              \
  cong-node-properties-dialog.c \
  cong-overview-view.c      \
  cong-parser-error.c       \
  cong-plugin.c             \
  cong-primary-window.c     \
  cong-progress-checklist.c \
  cong-section-head-editor.c \
  cong-source-view.c        \
  cong-span-text-editor.c   \
  cong-tree-view.c          \
  cong-view.c               \
  icon_assemble.c           \
  icon_connect.c            \
  icon_new.c                \
  icon_openfile.c           \
  icon_submit.c             \
  ilogo.c                   \
  auth_on.c                 \
  auth_off.c                \
  position.c                \
  cursor.c                  \
  select.c                  \
  popup.c                   \
  tree.c                    \
  tchoice.c                 \
  file.c                    \
  xmldata.c                 \
  xmledit.c                 \
  xmlload.c                 \
  xmlview.c                 \
  plugin-cleanup-source.c   \
  plugin-convert-case.c     \
  plugin-docbook.c          \
  plugin-dtd.c              \
  plugin-empty.c            \
  plugin-fo.c               \
  plugin-lists.c            \
  plugin-sgml.c             \
  plugin-tests.c            \
  plugin-validate.c         \
  plugin-website.c          \
  plugin-xsl.c              \
  main.c		    \
  $(printing_sources)

INCLUDES = $(CONGLOMERATE_CFLAGS) \
	$(PRINTING_CFLAGS) \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-DDATADIR=\""$(datadir)"\"
conglomerate_LDADD  = $(CONGLOMERATE_LIBS) \
	$(PRINTING_LIBS)
