bin_PROGRAMS = conge

conge_SOURCES =             \
  global.h                  \
  cong-dialog.h             \
  cong-dispspec.h           \
  cong-dispspec-registry.h  \
  cong-document.h           \
  cong-editor-widget-impl.h \
  cong-eel.h                \
  cong-error-dialog.h       \
  cong-plugin.h             \
  icon_assemble.c           \
  icon_connect.c            \
  icon_new.c                \
  icon_openfile.c           \
  icon_submit.c             \
  ilogo.c                   \
  auth_on.c                 \
  auth_off.c                \
  dispspec.c                \
  new_doc.c                 \
  position.c                \
  cursor.c                  \
  select.c                  \
  popup.c                   \
  xmledit.c                 \
  xmlview.c                 \
  tree.c                    \
  tchoice.c                 \
  xmldata.c                 \
  file.c                    \
  open.c                    \
  save.c                    \
  xmlload.c                 \
  cong-dialog.c             \
  cong-dispspec-registry.c  \
  cong-document.c           \
  cong-dummy-element-editor.c  \
  cong-editor-widget.c      \
  cong-eel.c                \
  cong-element-editor.c     \
  cong-error-dialog.c       \
  cong-error-file-open.c    \
  cong-error-file-save.c    \
  cong-error-tests.c        \
  cong-file-export.c        \
  cong-file-import.c        \
  cong-font.c               \
  cong-location.c           \
  cong-menus.c              \
  cong-plugin.c             \
  cong-primary-window.c     \
  cong-progress-checklist.c \
  cong-section-head-editor.c \
  cong-source-view.c        \
  cong-span-text-editor.c   \
  cong-test-view.c          \
  cong-tree-view.c          \
  fo.h                      \
  fo-parser-result.c        \
  fo-print-context.c        \
  fo-rect.c                 \
  fo-solver-result.c        \
  fo-tests.c                \
  plugin-docbook.c          \
  plugin-lists.c            \
  plugin-tests.c            \
  plugin-xsl.c              \
  main.c

INCLUDES = $(CONGLOMERATE_CFLAGS) \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-DDATADIR=\""$(datadir)"\"
conge_LDADD  = $(CONGLOMERATE_LIBS)

