--- gtk/test/run-test.rb.orig	2008-06-06 23:03:41.000000000 +0900
+++ gtk/test/run-test.rb	2008-07-11 23:51:33.000000000 +0900
@@ -11,9 +11,9 @@
 test_unit_ext_dir = File.join(glib_base, "test-unit-ext", "lib")
 
 [glib_base, atk_base, pango_base, gdk_pixbuf_base, gtk_base].each do |target|
-  if system("which make > /dev/null")
-    `make -C #{target.dump} > /dev/null` or exit(1)
-  end
+#  if system("which make > /dev/null")
+#    `make -C #{target.dump} > /dev/null` or exit(1)
+#  end
   $LOAD_PATH.unshift(File.join(target, "src"))
   $LOAD_PATH.unshift(File.join(target, "src", "lib"))
   $LOAD_PATH.unshift(File.join(target))
