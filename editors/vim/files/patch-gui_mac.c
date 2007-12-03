--- src/gui_mac.c.orig
+++ src/gui_mac.c
@@ -127,6 +127,13 @@
 /* Keeping track of which scrollbar is being dragged */
 static ControlHandle dragged_sb = NULL;
 
+/* INLINE IM */
+#ifdef USE_IM_CONTROL
+TSMDocumentID s_TSMDocID = 0;
+TSMTERecHandle s_TSMTERecHandle = 0;
+TEHandle s_TEHandle = 0;
+#endif /* USE_IM_CONTROL */
+
 static struct
 {
     FMFontFamily family;
@@ -137,7 +144,12 @@
 
 #ifdef MACOS_CONVERT
 # define USE_ATSUI_DRAWING
+int         p_macatsui_last;
+int         p_antialias_last = -1;
 ATSUStyle   gFontStyle;
+#ifdef FEAT_MBYTE
+ATSUStyle   gWideFontStyle;
+#endif
 Boolean	    gIsFontFallbackSet;
 #endif
 
@@ -265,6 +277,18 @@
 static WindowRef drawer = NULL; // TODO: put into gui.h
 #endif
 
+#ifdef USE_ATSUI_DRAWING
+void gui_mac_set_font_attributes(GuiFont font);
+void gui_mac_dispose_atsui_style();
+void gui_mac_create_atsui_style();
+void gui_mac_change_antialias();
+#endif
+
+#ifdef USE_IM_CONTROL
+static pascal void tsmproc_pre(TEHandle textH, SInt32 refCon);
+static pascal void tsmproc_post(TEHandle textH, SInt32 fixLen, SInt32 inputAreaStart, SInt32 inputAreaEnd, SInt32 pinStart, SInt32 pinEnd, SInt32 refCon);
+#endif /* USE_IM_CONTROL */
+
 /*
  * ------------------------------------------------------------
  * Conversion Utility
@@ -1924,10 +1948,26 @@
     {
         ControlRef rootControl;
         GetRootControl(gui.VimWindow, &rootControl);
-        if ((event->modifiers) & activeFlag)
+        if ((event->modifiers) & activeFlag) {
             ActivateControl(rootControl);
-        else
+			/* INLINE IM */
+#ifdef USE_IM_CONTROL
+			if (s_TSMDocID) {
+				TEActivate(s_TEHandle);
+				ActivateTSMDocument(s_TSMDocID);
+			}
+#endif /* USE_IM_CONTROL */
+		} else {
             DeactivateControl(rootControl);
+			/* INLINE IM */
+#ifdef USE_IM_CONTROL
+			if (s_TSMDocID) {
+				FixTSMDocument(s_TSMDocID);
+				DeactivateTSMDocument(s_TSMDocID);
+				TEDeactivate(s_TEHandle);
+			}
+#endif /* USE_IM_CONTROL */
+		}
     }
 
     /* Activate */
@@ -3014,6 +3054,38 @@
     gui.scrollbar_height = gui.scrollbar_width = 15; /* cheat 1 overlap */
     gui.border_offset = gui.border_width = 2;
 
+    /* INLINE IM */
+#ifdef USE_IM_CONTROL
+    if (!s_TSMDocID)
+    {
+	OSErr		    err;
+	Rect		    rect;
+	InterfaceTypeList   supportedInterfaces;
+
+	memset(&rect, 0, sizeof(rect)); /* Just dummy initialization */
+	s_TEHandle = TEStyleNew(&rect, &rect);
+	supportedInterfaces[0] = kTSMTEInterfaceType;
+	err = NewTSMDocument(1, supportedInterfaces, &s_TSMDocID,
+		(SInt32)&s_TSMTERecHandle);
+	if (err == noErr && s_TSMDocID && s_TSMTERecHandle)
+	{
+	    TSMTERecPtr prec = *(s_TSMTERecHandle);
+
+	    prec->textH		    = s_TEHandle;
+	    prec->preUpdateProc	    = NewTSMTEPreUpdateUPP(tsmproc_pre);
+	    prec->postUpdateProc    = NewTSMTEPostUpdateUPP(tsmproc_post);
+	    prec->updateFlag	    = kTSMTEAutoScroll;
+	    prec->refCon	    = (SInt32)gui.VimWindow;
+
+	    err = UseInputWindow(s_TSMDocID, FALSE);
+	    if (err != noErr)
+		printf("UseInputWindow() error %d\n", err);
+	}
+	else
+	    printf("NewTSMDocument() error %d\n", err);
+    }
+#endif /* USE_IM_CONTROL */
+
     /* If Quartz-style text anti aliasing is available (see
        gui_mch_draw_string() below), enable it for all font sizes. */
     vim_setenv((char_u *)"QDTEXT_MINSIZE", (char_u *)"1");
@@ -3078,6 +3150,9 @@
 gui_mch_open(void)
 {
     ShowWindow(gui.VimWindow);
+#ifdef USE_TRANSPARENCY
+    gui_mch_set_transparency(p_transparency);
+#endif // USE_TRANSPARENCY
 
     if (gui_win_x != -1 && gui_win_y != -1)
 	gui_mch_set_winpos(gui_win_x, gui_win_y);
@@ -3095,9 +3170,38 @@
     return OK;
 }
 
+#ifdef USE_ATSUI_DRAWING
+    void
+gui_mac_dispose_atsui_style()
+{
+    if (p_macatsui && gFontStyle)
+	ATSUDisposeStyle(gFontStyle);
+#ifdef FEAT_MBYTE
+    if (p_macatsui && gWideFontStyle)
+	ATSUDisposeStyle(gWideFontStyle);
+#endif
+}
+#endif
+
     void
 gui_mch_exit(int rc)
 {
+    /* INLINE IM */
+#ifdef USE_IM_CONTROL
+    if (s_TSMDocID)
+    {
+	OSErr err;
+
+	err = DeactivateTSMDocument(s_TSMDocID);
+	if (err != noErr)
+	    printf("DeactivateTSMDocument() error %d\n", err);
+	err = DeleteTSMDocument(s_TSMDocID);
+	if (err != noErr)
+	    printf("DeleteTSMDocument() error %d\n", err);
+	s_TSMDocID = NULL;
+    }
+#endif
+
     /* TODO: find out all what is missing here? */
     DisposeRgn(cursorRgn);
 
@@ -3110,8 +3214,7 @@
 	DisposeEventHandlerUPP(mouseWheelHandlerUPP);
 
 #ifdef USE_ATSUI_DRAWING
-    if (p_macatsui && gFontStyle)
-	ATSUDisposeStyle(gFontStyle);
+    gui_mac_dispose_atsui_style();
 #endif
 
     /* Exit to shell? */
@@ -3251,6 +3354,65 @@
     return selected_font;
 }
 
+#ifdef USE_ATSUI_DRAWING
+    void 
+gui_mac_create_atsui_style()
+{
+    if (p_macatsui && gFontStyle == NULL)
+    {
+	if (ATSUCreateStyle(&gFontStyle) != noErr)
+	    gFontStyle = NULL;
+    }
+#ifdef FEAT_MBYTE
+    if (p_macatsui && gWideFontStyle == NULL)
+    {
+	if (ATSUCreateStyle(&gWideFontStyle) != noErr)
+	    gWideFontStyle = NULL;
+    }
+#endif
+
+    p_macatsui_last = p_macatsui;
+    p_antialias_last = -1;
+}
+
+    void 
+gui_mac_change_antialias()
+{
+    ATSStyleRenderingOptions	renderingOptions;
+    
+    if (gMacSystemVersion >= 0x1020) {
+	renderingOptions	= p_antialias ? kATSStyleApplyAntiAliasing : kATSStyleNoAntiAliasing;;
+    } else {
+	renderingOptions	= kATSStyleNoOptions;
+    }
+
+    ATSUAttributeTag attribTags[] =
+    {
+	kATSUStyleRenderingOptionsTag,
+    };
+
+    ByteCount attribSizes[] =
+    {
+	sizeof(ATSStyleRenderingOptions),
+    };
+
+    ATSUAttributeValuePtr attribValues[] =
+    {
+	&renderingOptions,
+    };
+
+    if (gFontStyle)
+	ATSUSetAttributes(gFontStyle,
+	    (sizeof attribTags) / sizeof(ATSUAttributeTag),
+		attribTags, attribSizes, attribValues);
+    if (gWideFontStyle)
+	ATSUSetAttributes(gWideFontStyle,
+	    (sizeof attribTags) / sizeof(ATSUAttributeTag),
+		attribTags, attribSizes, attribValues);
+
+    p_antialias_last = p_antialias;
+}
+#endif
 
 /*
  * Initialise vim to use the font with the given name.	Return FAIL if the font
@@ -3268,11 +3430,7 @@
     char_u	used_font_name[512];
 
 #ifdef USE_ATSUI_DRAWING
-    if (p_macatsui && gFontStyle == NULL)
-    {
-	if (ATSUCreateStyle(&gFontStyle) != noErr)
-	    gFontStyle = NULL;
-    }
+    gui_mac_create_atsui_style();
 #endif
 
     if (font_name == NULL)
@@ -3336,49 +3494,8 @@
     gui.char_height = font_info.ascent + font_info.descent + p_linespace;
 
 #ifdef USE_ATSUI_DRAWING
-    ATSUFontID			fontID;
-    Fixed			fontSize;
-    ATSStyleRenderingOptions	fontOptions;
-
     if (p_macatsui && gFontStyle)
-    {
-	fontID = font & 0xFFFF;
-	fontSize = Long2Fix(font >> 16);
-
-	/* No antialiasing by default (do not attempt to touch antialising
-	 * options on pre-Jaguar) */
-	fontOptions =
-	    (gMacSystemVersion >= 0x1020) ?
-	    kATSStyleNoAntiAliasing :
-	    kATSStyleNoOptions;
-
-	ATSUAttributeTag attribTags[] =
-	{
-	    kATSUFontTag, kATSUSizeTag, kATSUStyleRenderingOptionsTag,
-	    kATSUMaxATSUITagValue+1
-	};
-	ByteCount attribSizes[] =
-	{
-	    sizeof(ATSUFontID), sizeof(Fixed),
-	    sizeof(ATSStyleRenderingOptions), sizeof font
-	};
-	ATSUAttributeValuePtr attribValues[] =
-	{
-	    &fontID, &fontSize, &fontOptions, &font
-	};
-
-	/* Convert font id to ATSUFontID */
-	if (FMGetFontFromFontFamilyInstance(fontID, 0, &fontID, NULL) == noErr)
-	{
-	    if (ATSUSetAttributes(gFontStyle,
-			(sizeof attribTags)/sizeof(ATSUAttributeTag),
-			attribTags, attribSizes, attribValues) != noErr)
-	    {
-		ATSUDisposeStyle(gFontStyle);
-		gFontStyle = NULL;
-	    }
-	}
-    }
+	gui_mac_set_font_attributes(font);
 #endif
 
     return OK;
@@ -3435,6 +3552,68 @@
 }
 #endif
 
+#ifdef USE_ATSUI_DRAWING
+    void 
+gui_mac_set_font_attributes(GuiFont font)
+{
+    ATSUFontID	fontID;
+    Fixed	fontSize;
+    Fixed       fontWidth;
+    
+    fontID    = font & 0xFFFF;
+    fontSize  = Long2Fix(font >> 16);
+    fontWidth = Long2Fix(gui.char_width);
+
+    ATSUAttributeTag attribTags[] =
+    {
+        kATSUFontTag, kATSUSizeTag, kATSUImposeWidthTag, 
+        kATSUMaxATSUITagValue + 1
+    };
+
+    ByteCount attribSizes[] =
+    {
+        sizeof(ATSUFontID), sizeof(Fixed), sizeof(fontWidth), 
+        sizeof(font)
+    };
+
+    ATSUAttributeValuePtr attribValues[] =
+    {
+        &fontID, &fontSize, &fontWidth, &font
+    };
+
+    if (FMGetFontFromFontFamilyInstance(fontID, 0, &fontID, NULL) == noErr)
+    {
+        if (ATSUSetAttributes(gFontStyle,
+                    (sizeof attribTags) / sizeof(ATSUAttributeTag),
+                    attribTags, attribSizes, attribValues) != noErr)
+        {
+# ifndef NDEBUG
+            fprintf(stderr, "couldn't set font style\n");
+# endif
+            ATSUDisposeStyle(gFontStyle);
+            gFontStyle = NULL;
+        }
+
+#ifdef FEAT_MBYTE
+        if (has_mbyte)
+        {
+            /* FIXME: we should use a more mbyte sensitive way to support 
+             * wide font drawing */
+            fontWidth = Long2Fix(gui.char_width * 2);
+
+            if (ATSUSetAttributes(gWideFontStyle,
+                        (sizeof attribTags) / sizeof(ATSUAttributeTag),
+                        attribTags, attribSizes, attribValues) != noErr)
+            {
+                ATSUDisposeStyle(gWideFontStyle);
+                gWideFontStyle = NULL;
+            }
+        }
+#endif
+    }
+}
+#endif
+
 /*
  * Set the current text font.
  */
@@ -3444,66 +3623,22 @@
 #ifdef USE_ATSUI_DRAWING
     GuiFont			currFont;
     ByteCount			actualFontByteCount;
-    ATSUFontID			fontID;
-    Fixed			fontSize;
-    ATSStyleRenderingOptions	fontOptions;
 
     if (p_macatsui && gFontStyle)
     {
 	/* Avoid setting same font again */
-	if (ATSUGetAttribute(gFontStyle, kATSUMaxATSUITagValue+1, sizeof font,
-		    &currFont, &actualFontByteCount) == noErr &&
-		actualFontByteCount == (sizeof font))
+	if (ATSUGetAttribute(gFontStyle, kATSUMaxATSUITagValue + 1, sizeof(font),
+		             &currFont, &actualFontByteCount) == noErr &&
+	    actualFontByteCount == (sizeof font))
 	{
 	    if (currFont == font)
 		return;
 	}
 
-	fontID = font & 0xFFFF;
-	fontSize = Long2Fix(font >> 16);
-	/* Respect p_antialias setting only for wide font.
-	 * The reason for doing this at the moment is a bit complicated,
-	 * but it's mainly because a) latin (non-wide) aliased fonts
-	 * look bad in OS X 10.3.x and below (due to a bug in ATS), and
-	 * b) wide multibyte input does not suffer from that problem. */
-	/*fontOptions =
-	    (p_antialias && (font == gui.wide_font)) ?
-	    kATSStyleNoOptions : kATSStyleNoAntiAliasing;
-	*/
-	/*fontOptions = kATSStyleAntiAliasing;*/
-
-	ATSUAttributeTag attribTags[] =
-	{
-	    kATSUFontTag, kATSUSizeTag, kATSUStyleRenderingOptionsTag,
-	    kATSUMaxATSUITagValue+1
-	};
-	ByteCount attribSizes[] =
-	{
-	    sizeof(ATSUFontID), sizeof(Fixed),
-	    sizeof(ATSStyleRenderingOptions), sizeof font
-	};
-	ATSUAttributeValuePtr attribValues[] =
-	{
-	    &fontID, &fontSize, &fontOptions, &font
-	};
-
-	if (FMGetFontFromFontFamilyInstance(fontID, 0, &fontID, NULL) == noErr)
-	{
-	    if (ATSUSetAttributes(gFontStyle,
-			(sizeof attribTags)/sizeof(ATSUAttributeTag),
-			attribTags, attribSizes, attribValues) != noErr)
-	    {
-# ifndef NDEBUG
-		fprintf(stderr, "couldn't set font style\n");
-# endif
-		ATSUDisposeStyle(gFontStyle);
-		gFontStyle = NULL;
-	    }
-	}
-
+        gui_mac_set_font_attributes(font);
     }
 
-    if (p_macatsui && !gIsFontFallbackSet)
+    if (p_macatsui && ! gIsFontFallbackSet)
     {
 	/* Setup automatic font substitution. The user's guifontwide
 	 * is tried first, then the system tries other fonts. */
@@ -3524,7 +3659,9 @@
 			&fallbackFonts,
 			NULL) == noErr)
 	    {
-		ATSUSetFontFallbacks((sizeof fallbackFonts)/sizeof(ATSUFontID), &fallbackFonts, kATSUSequentialFallbacksPreferred);
+		ATSUSetFontFallbacks((sizeof fallbackFonts)/sizeof(ATSUFontID), 
+                                     &fallbackFonts, 
+                                     kATSUSequentialFallbacksPreferred);
 	    }
 /*
 	ATSUAttributeValuePtr fallbackValues[] = { };
@@ -3907,9 +4044,10 @@
     UniChar *tofree = mac_enc_to_utf16(s, len, (size_t *)&utf16_len);
     utf16_len /= sizeof(UniChar);
 
-    /* - ATSUI automatically antialiases text (Someone)
-     * - for some reason it does not work... (Jussi) */
-
+#ifdef MAC_ATSUI_DEBUG
+    fprintf(stderr, "row = %d, col = %d, len = %d: '%c'\n", 
+            row, col, len, len == 1 ? s[0] : ' ');
+#endif
     /*
      * When antialiasing we're using srcOr mode, we have to clear the block
      * before drawing the text.
@@ -3944,35 +4082,120 @@
     }
 
     {
-	/* Use old-style, non-antialiased QuickDraw text rendering. */
 	TextMode(srcCopy);
 	TextFace(normal);
 
-    /*  SelectFont(hdc, gui.currFont); */
-
+        /*  SelectFont(hdc, gui.currFont); */
 	if (flags & DRAW_TRANSP)
 	{
 	    TextMode(srcOr);
 	}
 
 	MoveTo(TEXT_X(col), TEXT_Y(row));
-	ATSUTextLayout textLayout;
 
-	if (ATSUCreateTextLayoutWithTextPtr(tofree,
-		    kATSUFromTextBeginning, kATSUToTextEnd,
-		    utf16_len,
-		    (gFontStyle ? 1 : 0), &utf16_len,
-		    (gFontStyle ? &gFontStyle : NULL),
-		    &textLayout) == noErr)
-	{
-	    ATSUSetTransientFontMatching(textLayout, TRUE);
-
-	    ATSUDrawText(textLayout,
-		    kATSUFromTextBeginning, kATSUToTextEnd,
-		    kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc);
+        if (gFontStyle && flags & DRAW_BOLD)
+        {
+            Boolean attValue = true;
+            ATSUAttributeTag attribTags[] = { kATSUQDBoldfaceTag };
+            ByteCount attribSizes[] = { sizeof(Boolean) };
+            ATSUAttributeValuePtr attribValues[] = { &attValue };
+            
+            ATSUSetAttributes(gFontStyle, 1, attribTags, attribSizes, attribValues);
+        }
 
-	    ATSUDisposeTextLayout(textLayout);
+#ifdef FEAT_MBYTE
+	if (has_mbyte)
+	{
+	    int n, width_in_cell, last_width_in_cell;
+            UniCharArrayOffset offset = 0;
+            UniCharCount yet_to_draw = 0;
+            ATSUTextLayout textLayout;
+            ATSUStyle      textStyle;
+            
+            last_width_in_cell = 1;
+            ATSUCreateTextLayout(&textLayout);
+            ATSUSetTextPointerLocation(textLayout, tofree, 
+                                       kATSUFromTextBeginning, 
+                                       kATSUToTextEnd, utf16_len);
+            /*
+            ATSUSetRunStyle(textLayout, gFontStyle, 
+                            kATSUFromTextBeginning, kATSUToTextEnd); */
+
+	    /* Compute the length in display cells. */
+	    for (n = 0; n < len; n += MB_BYTE2LEN(s[n]))
+            {
+                width_in_cell = (*mb_ptr2cells)(s + n);
+
+                /* probably we are switching from single byte character 
+                 * to multibyte characters (which requires more than one
+                 * cell to draw) */
+                if (width_in_cell != last_width_in_cell)
+                {
+#ifdef MAC_ATSUI_DEBUG
+                    fprintf(stderr, "\tn = %2d, (%d-%d), offset = %d, yet_to_draw = %d\n", 
+                            n, last_width_in_cell, width_in_cell, offset, yet_to_draw);
+#endif
+                    textStyle = last_width_in_cell > 1 ? gWideFontStyle : gFontStyle;
+
+                    ATSUSetRunStyle(textLayout, textStyle, offset, yet_to_draw);
+                    offset += yet_to_draw;
+                    yet_to_draw = 0;
+                    last_width_in_cell = width_in_cell;
+                }
+                
+                yet_to_draw++;
+            }
+            
+            if (yet_to_draw)
+            {
+#ifdef MAC_ATSUI_DEBUG
+                fprintf(stderr, "\tn = %2d, (%d-%d), offset = %d, yet_to_draw = %d\n", 
+                        n, last_width_in_cell, width_in_cell, offset, yet_to_draw);
+#endif
+                /* finish the rest style */
+                textStyle = width_in_cell > 1 ? gWideFontStyle : gFontStyle;
+                ATSUSetRunStyle(textLayout, textStyle, offset, kATSUToTextEnd);
+            }
+
+            ATSUSetTransientFontMatching(textLayout, TRUE);
+            ATSUDrawText(textLayout,
+                         kATSUFromTextBeginning, kATSUToTextEnd,
+                         kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc);
+            ATSUDisposeTextLayout(textLayout);
 	}
+        else
+#endif
+        {
+            ATSUTextLayout textLayout;
+
+            if (ATSUCreateTextLayoutWithTextPtr(tofree,
+                        kATSUFromTextBeginning, kATSUToTextEnd,
+                        utf16_len,
+                        (gFontStyle ? 1 : 0), &utf16_len,
+                        (gFontStyle ? &gFontStyle : NULL),
+                        &textLayout) == noErr)
+            {
+                ATSUSetTransientFontMatching(textLayout, TRUE);
+
+                ATSUDrawText(textLayout,
+                        kATSUFromTextBeginning, kATSUToTextEnd,
+                        kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc);
+
+                ATSUDisposeTextLayout(textLayout);
+            }
+        }
+
+        /* drawing is done, now reset bold to normal */
+        if (gFontStyle && flags & DRAW_BOLD)
+        {
+            Boolean attValue = false;
+            
+            ATSUAttributeTag attribTags[] = { kATSUQDBoldfaceTag };
+            ByteCount attribSizes[] = { sizeof(Boolean) };
+            ATSUAttributeValuePtr attribValues[] = { &attValue };
+            
+            ATSUSetAttributes(gFontStyle, 1, attribTags, attribSizes, attribValues);
+        }
     }
 
     if (flags & DRAW_UNDERC)
@@ -3986,6 +4209,17 @@
 gui_mch_draw_string(int row, int col, char_u *s, int len, int flags)
 {
 #if defined(USE_ATSUI_DRAWING)
+    /* switch from macatsui to nomacatsui */
+    if (p_macatsui == 0 && p_macatsui_last != 0)
+        gui_mac_dispose_atsui_style();
+    else
+    /* switch from nomacatsui to macatsui */
+    if (p_macatsui != 0 && p_macatsui_last == 0)
+        gui_mac_create_atsui_style();
+
+    if (p_antialias != p_antialias_last)
+	gui_mac_change_antialias();
+
     if (p_macatsui)
 	draw_string_ATSUI(row, col, s, len, flags);
     else
@@ -4095,6 +4329,7 @@
 #endif
     rc.bottom = rc.top + gui.char_height;
 
+    PenNormal();
     gui_mch_set_fg_color(color);
 
     FrameRect(&rc);
@@ -4119,6 +4354,7 @@
     rc.right = rc.left + w;
     rc.bottom = rc.top + h;
 
+    PenNormal();
     gui_mch_set_fg_color(color);
 
     FrameRect(&rc);
@@ -6023,6 +6259,8 @@
 /*
  * Input Method Control functions.
  */
+static int im_row = 0;
+static int im_col = 0;
 
 /*
  * Notify cursor position to IM.
@@ -6030,14 +6268,19 @@
     void
 im_set_position(int row, int col)
 {
-    /* TODO: Implement me! */
+    im_row = row;
+    im_col = col;
 }
 
 /*
  * Set IM status on ("active" is TRUE) or off ("active" is FALSE).
  */
     void
+#if defined(FEAT_UIMFEP)
+gui_im_set_active(int active)
+#else // FEAT_UIMFEP
 im_set_active(int active)
+#endif // FEAT_UIMFEP
 {
     KeyScript(active ? smKeySysScript : smKeyRoman);
 }
@@ -6046,13 +6289,108 @@
  * Get IM status.  When IM is on, return not 0.  Else return 0.
  */
     int
+#if defined(FEAT_UIMFEP)
+gui_im_get_status(void)
+#else // FEAT_UIMFEP
 im_get_status(void)
+#endif // FEAT_UIMFEP
 {
     SInt32 script = GetScriptManagerVariable(smKeyScript);
     return (script != smRoman
 	    && script == GetScriptManagerVariable(smSysScript)) ? 1 : 0;
 }
 
+static RGBColor s_saved_fg;
+static RGBColor s_saved_bg;
+
+/* INLINE IM */
+    static pascal void
+tsmproc_pre(TEHandle textH, SInt32 refCon)
+{
+    Rect rect;
+    TextStyle style;
+    RGBColor fore;
+	int idx = syn_name2id((char_u *)"IMLine");
+
+    /* Save current color and set IM color */
+    GetForeColor(&s_saved_fg);
+    GetBackColor(&s_saved_bg);
+    gui_mch_set_fg_color(highlight_gui_color_rgb(idx, TRUE));
+    gui_mch_set_bg_color(highlight_gui_color_rgb(idx, FALSE));
+    GetForeColor(&fore);
+    style.tsColor = fore;
+
+    /* Set IME font and color */
+    style.tsFont = gui.norm_font & 0xFFFF;
+    style.tsSize = gui.norm_font >> 16;
+    TESetStyle(doFont | doSize | doColor, &style, false, textH);
+
+    /* Set IME's rectangle */
+    {
+	int row = im_row, col = im_col;
+
+	if (col * 4 > gui.num_cols * 3)
+	{
+	    /* Slide inline area to 1 line down or up */
+	    if (row > gui.num_rows / 2)
+		--row;
+	    else
+		++row;
+	    col = 0;
+	}
+	rect.top	= FILL_Y(row);
+	rect.left	= FILL_X(col);
+	rect.right	= FILL_X(screen_Columns);
+	rect.bottom	= rect.top + gui.char_height;
+	(**textH).destRect = rect;
+	(**textH).viewRect = rect;
+	TECalText(textH);
+    }
+}
+
+    static pascal void
+tsmproc_post(TEHandle textH, SInt32 fixLen, SInt32 inputAreaStart,
+	SInt32 inputAreaEnd, SInt32 pinStart, SInt32 pinEnd, SInt32 refCon)
+{
+    char_u	*to = NULL;
+    vimconv_T	conv;
+    int		convlen = 0;
+
+    Rect rect = (**textH).viewRect;
+
+    /* Restore color */
+    RGBForeColor(&s_saved_fg);
+    RGBBackColor(&s_saved_bg);
+
+    /* Add IME's result string to input buffer */
+    if (fixLen > 0)
+    {
+	CharsHandle ch = TEGetText(textH);
+	conv.vc_type = CONV_NONE;
+	convlen = fixLen;
+	convert_setup(&conv, (char_u *)"sjis", p_enc? p_enc: (char_u *)"utf-8");
+	to = string_convert(&conv, (char_u *)*ch, &convlen);
+	if (to != NULL)
+	    add_to_input_buf_csi(to, convlen); 
+       	else
+	    add_to_input_buf_csi(*ch, fixLen); 
+
+	if (conv.vc_type != CONV_NONE)
+	    vim_free(to);
+	convert_setup(&conv, NULL, NULL);
+    }
+
+    if (inputAreaEnd < 0)
+    {
+	TESetText("", 0, textH);
+	/* Remove TSMTE garbage */
+	gui_redraw(rect.left, rect.top, rect.right - rect.left + 1,
+		rect.bottom - rect.top + 1);
+	gui_update_cursor(TRUE, FALSE);
+    }
+}
+
+
 #endif /* defined(USE_IM_CONTROL) || defined(PROTO) */
 
 
@@ -6456,3 +6794,11 @@
 }
 
 #endif // FEAT_GUI_TABLINE
+
+#if defined(USE_TRANSPARENCY)
+    void
+gui_mch_set_transparency(int alpha)
+{
+    SetWindowAlpha(gui.VimWindow, (float)alpha / 255.0f);
+}
+#endif	// USE_TRANSPARENCY
