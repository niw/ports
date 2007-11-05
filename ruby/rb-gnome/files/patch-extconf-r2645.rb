--- extconf.rb.orig	2007-10-26 22:25:41.000000000 +0900
+++ extconf.rb	2007-10-26 22:26:10.000000000 +0900
@@ -69,7 +69,7 @@
   topdir = File.join(*([".."] * subdir.split(/\/+/).size))
   /^\// =~ (dir = $topsrcdir) or dir = File.join(topdir, $topsrcdir)
   srcdir = File.join(dir, subdir)
-  args = ruby_args + ["-C", subdir, File.join(srcdir, "extconf.rb"),
+  args = ruby_args + ["-rvendor-specific", "-C", subdir, File.join(srcdir, "extconf.rb"),
                       "--topsrcdir=#{dir}", "--topdir=#{topdir}",
                       "--srcdir=#{srcdir}", *extra_args]
   ret = system(ruby, *args)
