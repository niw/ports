--- src/MacVim/MMTextView.m.orig
+++ src/MacVim/MMTextView.m
@@ -349,7 +349,8 @@
     // in a separate popup window this behaviour is not wanted.  By setting the
     // location of the returned range to NSNotFound NSTextView does nothing.
     // This hack is continued in 'firstRectForCharacterRange:'.
-    return NSMakeRange(NSNotFound, 0);
+    //return NSMakeRange(NSNotFound, 0);
+    return NSMakeRange(0, [[markedTextField stringValue] length]);
 }
 
 - (void)setMarkedText:(id)text selectedRange:(NSRange)range
@@ -370,7 +370,7 @@
         [markedTextField setEditable:NO];
         [markedTextField setSelectable:NO];
         [markedTextField setBezeled:NO];
-        [markedTextField setBordered:YES];
+        [markedTextField setBordered:NO];
 
         NSPanel *panel = [[NSPanel alloc]
             initWithContentRect:cellRect
