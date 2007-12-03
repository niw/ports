--- src/gui_mac.m.orig
+++ src/gui_mac.m
@@ -21,6 +21,7 @@
 
 #include "vim.h"
 #import <Cocoa/Cocoa.h>
+#import <Carbon/Carbon.h>
 
 /* Internal Data Structures {{{ */
 
@@ -359,6 +360,9 @@
     fprintf(stderr, "gui_mch_open: %d %d\n", gui_win_x, gui_win_y);
 
     gui_mac_open_window();
+#ifdef USE_TRANSPARENCY
+    gui_mch_set_transparency(p_transparency);
+#endif	// USE_TRANSPARENCY
 
     if (gui_win_x != -1 && gui_win_y != -1)
         gui_mch_set_winpos(gui_win_x, gui_win_y);
@@ -514,16 +518,24 @@
 
 /* Input Method Handling {{{ */
 
+#if defined(FEAT_UIMFEP)
+int gui_im_get_status()
+#else // FEAT_UIMFEP
 int im_get_status()
+#endif // FEAT_UIMFEP
 {
-    if (! gui.in_use)
-            return 0;
-
-    return 0;
+    SInt32 script = GetScriptManagerVariable(smKeyScript);
+    return (script != smRoman
+	    && script == GetScriptManagerVariable(smSysScript)) ? 1 : 0;
 }
 
+#if defined(FEAT_UIMFEP)
+void gui_im_set_active(int active)
+#else // FEAT_UIMFEP
 void im_set_active(int active)
+#endif // FEAT_UIMFEP
 {
+    KeyScript(active ? smKeySysScript : smKeyRoman);
 }
 
 void im_set_position(int row, int col)
@@ -2379,6 +2391,13 @@
     [gui_mac.content_window makeKeyAndOrderFront: nil];
 }
 
+#if defined(USE_TRANSPARENCY)
+void gui_mch_set_transparency(int alpha)
+{
+	[gui_mac.content_window setAlphaValue:((float)alpha / 255.0f)];
+}
+#endif	// USE_TRANSPARENCY
+
 /* Window related Utilities 2}}} */
 
 /* View related Utilities 2{{{ */
@@ -2457,6 +2476,13 @@
 
     if (markedRange.length > 0)
     {
+        int idx = syn_name2id((char_u *)"IMLine");
+        [self setMarkedTextAttribute:NSColorFromGuiColor(
+            highlight_gui_color_rgb(idx, TRUE), 1.0)
+            forKey:NSBackgroundColorAttributeName];
+        [self setMarkedTextAttribute:NSColorFromGuiColor(
+            highlight_gui_color_rgb(idx, FALSE), 1.0)
+            forKey:NSForegroundColorAttributeName];
     } else
     {
         NSLog(@"clear markedText");
@@ -2909,6 +2909,8 @@
             break;
 
         case MOD_MASK_CTRL: 
+            if (original_char == 'h' && [self hasMarkedText])
+                goto insert_text;
             result[len++] = modified_char;
             add_to_input_buf(result, len);
             // NSLog(@"CTRL-%c, add_to_input_buf: %d", original_char, len);
