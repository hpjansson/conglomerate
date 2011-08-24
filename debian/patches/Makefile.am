## an other Makefile.am

NULL =

# patches that travell with the package
patch_DISTS = \
	dont_install_generated_gtkdoc.dpatch \
	no_scrollkeeper-update.dpatch \
	package_version.dpatch \
	$(NULL)

# other files that travell with the package
EXTRA_DIST = 00list 00patch-opts \
	$(patch_DISTS) \
	README.4sarge \
	README.release \
	4sarge \
	next \
	release \
	sync \
	$(NULL)
## eof

-include $(top_srcdir)/git.mk
