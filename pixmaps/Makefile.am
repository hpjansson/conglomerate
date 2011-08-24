# application icons
appiconsdir = $(datadir)/conglomerate/pixmaps
appicons_DATA = \
	conglomerate-icon-32.png \
	cong-docbook-set-16.png \
	cong-docbook-book-16.png \
	cong-docbook-article-16.png \
	cong-metadata-16.png \
	cong-title-16.png \
	cong-wrapper-16.png \
	cong-author-16.png \
	cong-editor-16.png \
	cong-publisher-16.png \
	cong-name-16.png \
	cong-address-16.png \
	cong-history-16.png \
	cong-paragraph-16.png \
	cong-faq-16.png \
	cong-question-16.png \
	cong-answer-16.png \
	cong-bibliography-16.png \
	cong-entry-16.png \
	cong-footnote-16.png \
	cong-subsection-16.png \
	cong-indexterm-16.png \
	cong-summary-16.png \
	cong-list-16.png \
	cong-bulleted-list-16.png \
	cong-numbered-list-16.png \
	cong-list-item-16.png \
	cong-definition-list-16.png \
	cong-code-fragment-16.png \
	cong-comment-16.png \
	cong-uncomment-16.png \
	cong-table-16.png \
	cong-caution-48.png \
	cong-important-48.png \
	cong-note-48.png \
	cong-tip-48.png \
	cong-warning-48.png \
	conglomerate-logo.png

EXTRA_DIST = $(appicons_DATA)

SUBDIRS = icons

#
# GTK icon cache
#

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
install-data-local: update-icon-cache

update-icon-cache:
	if test -z "$(DESTDIR)"; then \
	        echo "Updating Gtk icon cache."; \
	        $(gtk_update_icon_cache); \
	else \
	        echo "*** Icon cache not updated.  After (un)install, run this:"; \
	        echo "***   $(gtk_update_icon_cache)"; \
	        fi


-include $(top_srcdir)/git.mk
