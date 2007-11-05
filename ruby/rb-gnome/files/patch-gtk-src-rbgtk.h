--- ../ruby-gnome2-all-0.14.1.org/gtk/src/rbgtk.h	2005-11-14 17:14:00.000000000 +0900
+++ gtk/src/rbgtk.h	2006-11-05 10:59:50.000000000 +0900
@@ -50,7 +50,7 @@
 #define GTK_TYPE_ALLOCATION (gtk_allocation_get_type())
 #define GTK_TYPE_ACCEL_KEY (gtk_accel_key_get_type())
 #define GTK_TYPE_ACCEL_GROUP_ENTRY (gtk_accel_group_entry_get_type())
-#define GTK_TYPE_TARGET_LIST (gtk_target_list_get_type())
+#define GTK_TYPE_TARGET_LIST (gtkgtk_target_list_get_type())
 #define GTK_TYPE_BINDING_SET (gtk_bindingset_get_type())
 #define GTK_TYPE_TEXT_APPEARANCE (gtk_text_appearance_get_type())
 
@@ -94,7 +94,7 @@
 extern GType gtk_allocation_get_type();
 extern GType gtk_accel_key_get_type();
 extern GType gtk_accel_group_entry_get_type();
-extern GType gtk_target_list_get_type();
+extern GType gtkgtk_target_list_get_type();
 extern GType gtk_bindingset_get_type();
 extern GType gtk_text_appearance_get_type();
 
