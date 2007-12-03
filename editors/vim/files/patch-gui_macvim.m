--- src/MacVim/gui_macvim.m.orig
+++ src/MacVim/gui_macvim.m
@@ -1583,3 +1583,46 @@
 }
 
 #endif // MAC_CLIENTSERVER
+
+#if defined(USE_IM_CONTROL) || defined(PROTO)
+/*
+ * Input Method Control functions.
+ */
+
+/*
+ * Notify cursor position to IM.
+ */
+    void
+im_set_position(int row, int col)
+{
+}
+
+/*
+ * Set IM status on ("active" is TRUE) or off ("active" is FALSE).
+ */
+    void
+#if defined(FEAT_UIMFEP)
+gui_im_set_active(int active)
+#else // FEAT_UIMFEP
+im_set_active(int active)
+#endif // FEAT_UIMFEP
+{
+    KeyScript(active ? smKeySysScript : smKeyRoman);
+}
+
+/*
+ * Get IM status.  When IM is on, return not 0.  Else return 0.
+ */
+    int
+#if defined(FEAT_UIMFEP)
+gui_im_get_status(void)
+#else // FEAT_UIMFEP
+im_get_status(void)
+#endif // FEAT_UIMFEP
+{
+    SInt32 script = GetScriptManagerVariable(smKeyScript);
+    return (script != smRoman
+	    && script == GetScriptManagerVariable(smSysScript)) ? 1 : 0;
+}
+
+#endif /* defined(USE_IM_CONTROL) || defined(PROTO) */
