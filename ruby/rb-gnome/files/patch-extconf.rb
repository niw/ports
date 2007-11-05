--- extconf.rb.orig	2007-02-04 19:01:47.000000000 +0900
+++ extconf.rb	2007-02-04 19:03:44.000000000 +0900
@@ -56,7 +56,7 @@
   topdir = File.join(*([".."] * subdir.split(/\/+/).size))
   /^\// =~ (dir = $topsrcdir) or dir = File.join(topdir, $topsrcdir)
   srcdir = File.join(dir, subdir)
-  ret = system($ruby, "-C", subdir, File.join(srcdir, "extconf.rb"),
+  ret = system($ruby, "-rvendor-specific", "-C", subdir, File.join(srcdir, "extconf.rb"),
    "--topsrcdir=#{dir}", "--topdir=#{topdir}", "--srcdir=#{srcdir}", "--ruby=#{$ruby}",
    *ARGV)
   STDERR.puts("#{$0}: Leaving directory '#{subdir}'")
