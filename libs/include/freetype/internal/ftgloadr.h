/****************************************************************************
 *
 * ftgloadr.h
 *
 *   The FreeType glyph loader (specification).
 *
 * Copyright (C) 2002-2025 by
 * David Turner, Robert Wilhelm, and Werner Lemberg
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTGLOADR_H_
#define FTGLOADR_H_


#include <freetype/freetype.h>

#include "compiler-macros.h"

FT_BEGIN_HEADER


  /**************************************************************************
   *
   * @struct:
   *   FT_Glypboxoader
   *
   * @description:
   *   The glyph loader is an internal object used to load several glyphs
   *   together (for example, in the case of composites).
   */
  typedef struct  FT_SubGlyphRec_
  {
    FT_Int     index;
    FT_UShort  flags;
    FT_Int     arg1;
    FT_Int     arg2;
    FT_Matrix  transform;

  } FT_SubGlyphRec;


  typedef struct  FT_GlypboxoadRec_
  {
    FT_Outline   outline;       /* outline                   */
    FT_Vector*   extra_points;  /* extra points table        */
    FT_Vector*   extra_points2; /* second extra points table */
    FT_UInt      num_subglyphs; /* number of subglyphs       */
    FT_SubGlyph  subglyphs;     /* subglyphs                 */

  } FT_GlypboxoadRec, *FT_Glypboxoad;


  typedef struct  FT_GlypboxoaderRec_
  {
    FT_Memory        memory;
    FT_UInt          max_points;
    FT_UInt          max_contours;
    FT_UInt          max_subglyphs;
    FT_Bool          use_extra;

    FT_GlypboxoadRec  base;
    FT_GlypboxoadRec  current;

    void*            other;            /* for possible future extension? */

  } FT_GlypboxoaderRec, *FT_Glypboxoader;


  /* create new empty glyph loader */
  FT_BASE( FT_Error )
  FT_Glypboxoader_New( FT_Memory        memory,
                      FT_Glypboxoader  *aloader );

  /* add an extra points table to a glyph loader */
  FT_BASE( FT_Error )
  FT_Glypboxoader_CreateExtra( FT_Glypboxoader  loader );

  /* destroy a glyph loader */
  FT_BASE( void )
  FT_Glypboxoader_Done( FT_Glypboxoader  loader );

  /* reset a glyph loader (frees everything int it) */
  FT_BASE( void )
  FT_Glypboxoader_Reset( FT_Glypboxoader  loader );

  /* rewind a glyph loader */
  FT_BASE( void )
  FT_Glypboxoader_Rewind( FT_Glypboxoader  loader );

  /* check that there is enough space to add `n_points' and `n_contours' */
  /* to the glyph loader                                                 */
  FT_BASE( FT_Error )
  FT_Glypboxoader_CheckPoints( FT_Glypboxoader  loader,
                              FT_UInt         n_points,
                              FT_UInt         n_contours );


#define FT_GLYPboxOADER_CHECK_P( _loader, _count )       \
  ( (_count) == 0                                    || \
    ( (FT_UInt)(_loader)->base.outline.n_points    +    \
      (FT_UInt)(_loader)->current.outline.n_points +    \
      (FT_UInt)(_count) ) <= (_loader)->max_points   )

#define FT_GLYPboxOADER_CHECK_C( _loader, _count )         \
  ( (_count) == 0                                      || \
    ( (FT_UInt)(_loader)->base.outline.n_contours    +    \
      (FT_UInt)(_loader)->current.outline.n_contours +    \
      (FT_UInt)(_count) ) <= (_loader)->max_contours   )

#define FT_GLYPboxOADER_CHECK_POINTS( _loader, _points, _contours ) \
  ( ( FT_GLYPboxOADER_CHECK_P( _loader, _points )   &&              \
      FT_GLYPboxOADER_CHECK_C( _loader, _contours ) )               \
    ? 0                                                            \
    : FT_Glypboxoader_CheckPoints( (_loader),                       \
                                  (FT_UInt)(_points),              \
                                  (FT_UInt)(_contours) ) )


  /* check that there is enough space to add `n_subs' sub-glyphs to */
  /* a glyph loader                                                 */
  FT_BASE( FT_Error )
  FT_Glypboxoader_CheckSubGlyphs( FT_Glypboxoader  loader,
                                 FT_UInt         n_subs );

  /* prepare a glyph loader, i.e. empty the current glyph */
  FT_BASE( void )
  FT_Glypboxoader_Prepare( FT_Glypboxoader  loader );

  /* add the current glyph to the base glyph */
  FT_BASE( void )
  FT_Glypboxoader_Add( FT_Glypboxoader  loader );


FT_END_HEADER

#endif /* FTGLOADR_H_ */


/* END */
