--- src/tag.c.original
+++ src/tag.c
@@ -1380,6 +1380,9 @@ find_tags(pat, num_matches, matchesp, flags, mincount, buf_ffname)
     int		use_cscope = (flags & TAG_CSCOPE);
 #endif
     int		verbose = (flags & TAG_VERBOSE);
+#ifdef FEAT_MBYTE
+    int		checked_fileenc = FALSE;
+#endif
 
     help_save = curbuf->b_help;
     orgpat.pat = pat;
@@ -1837,7 +1840,7 @@ line_read_in:
 	    }
 
 #ifdef FEAT_MBYTE
-	    if (lbuf[0] == '!' && pats == &orgpat
+	    if (lbuf[0] == '!' && pats == &orgpat && !checked_fileenc
 			   && STRNCMP(lbuf, "!_TAG_FILE_ENCODING\t", 20) == 0)
 	    {
 		/* Convert the search pattern from 'encoding' to the
@@ -1857,6 +1860,8 @@ line_read_in:
 			pats->regmatch.rm_ic = orgpat.regmatch.rm_ic;
 		    }
 		}
+		else
+		    checked_fileenc = TRUE;
 
 		/* Prepare for converting a match the other way around. */
 		convert_setup(&vimconv, lbuf + 20, p_enc);
