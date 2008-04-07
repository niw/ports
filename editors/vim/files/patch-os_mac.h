--- src/os_mac.h.orig
+++ src/os_mac.h
@@ -23,6 +23,7 @@
  * files have many conflicts).
  */
 #ifndef FEAT_X11
+# define __ASSERTMACROS__   /* workaround for check() */
 # include <Quickdraw.h>	    /* Apple calls it QuickDraw.h... */
 # include <ToolUtils.h>
 # include <LowMem.h>
