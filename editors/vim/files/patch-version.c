--- src/version.c.orig
+++ src/version.c
@@ -1078,10 +1078,10 @@
 #      if defined(MSWIN)
     MSG_PUTS(_("with GUI."));
 #      else
-#	if defined (TARGET_API_MAC_CARBON) && TARGET_API_MAC_CARBON
+#	ifdef FEAT_GUI_CARBON
     MSG_PUTS(_("with Carbon GUI."));
 #	else
-#	 if defined (TARGET_API_MAC_OSX) && TARGET_API_MAC_OSX
+#	 ifdef FEAT_GUI_COCOA
     MSG_PUTS(_("with Cocoa GUI."));
 #	 else
 #	  if defined (MACOS)
