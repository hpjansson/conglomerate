--- omf.make.tarball	2003-10-03 22:49:09.000000000 +0000
+++ omf.make	2003-10-03 22:49:44.000000000 +0000
@@ -42,7 +42,7 @@
 	for file in $(omffile); do \
 		$(INSTALL_DATA) $$file.out $(DESTDIR)$(omf_dest_dir)/$$file; \
 	done
-	-scrollkeeper-update -p $(scrollkeeper_localstate_dir) -o $(DESTDIR)$(omf_dest_dir)
+	##-scrollkeeper-update -p $(scrollkeeper_localstate_dir) -o $(DESTDIR)$(omf_dest_dir)
 
 uninstall-local-omf:
 	-for file in $(srcdir)/*.omf; do \
