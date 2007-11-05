--- ../ruby-gnome2-all-0.14.1.org/rsvg/src/rbrsvg.c	2005-11-14 17:14:01.000000000 +0900
+++ rsvg/src/rbrsvg.c	2006-11-05 11:46:28.000000000 +0900
@@ -439,13 +439,13 @@
 
     rb_define_const(mRSVG, "VERSION",
                     rb_ary_new3(3,
-                                INT2FIX(librsvg_major_version),
-                                INT2FIX(librsvg_minor_version),
-                                INT2FIX(librsvg_micro_version)));
+                                INT2FIX(LIBRSVG_MAJOR_VERSION),
+                                INT2FIX(LIBRSVG_MINOR_VERSION),
+                                INT2FIX(LIBRSVG_MICRO_VERSION)));
   
-    rb_define_const(mRSVG, "MAJOR_VERSION", INT2FIX(librsvg_major_version));
-    rb_define_const(mRSVG, "MINOR_VERSION", INT2FIX(librsvg_minor_version));
-    rb_define_const(mRSVG, "MICRO_VERSION", INT2FIX(librsvg_micro_version));
+    rb_define_const(mRSVG, "MAJOR_VERSION", INT2FIX(LIBRSVG_MAJOR_VERSION));
+    rb_define_const(mRSVG, "MINOR_VERSION", INT2FIX(LIBRSVG_MINOR_VERSION));
+    rb_define_const(mRSVG, "MICRO_VERSION", INT2FIX(LIBRSVG_MICRO_VERSION));
   
 #if LIBRSVG_CHECK_VERSION(2, 9, 0)
     rb_define_module_function(mRSVG, "init", rb_rsvg_init, 0);
