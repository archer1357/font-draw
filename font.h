#ifndef FONT_H
#define FONT_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "stb_truetype.h"

struct FontGlyph {
  short x,y,width,height,offsetX,offsetY,colAdvance;
  int index;
};

#define FONT_ASCII_TEXT_GLYPHS 96

struct SimpleFont {
  struct FontGlyph glyphs[FONT_ASCII_TEXT_GLYPHS];
  int rowAdvance,texWidth,texHeight;
};

#ifdef __cplusplus
extern "C" {
#endif

  void font_createBitmap(const stbtt_fontinfo *fontInfo,
                           const struct FontGlyph *glyphs,
                           int glyphsNum,float scale,
                           int texWidth,int texHeight,
                           unsigned char *pBits) ;

  void font_packGlyphs(struct FontGlyph *glyphs,
                         int glyphsNum,int padding,
                         int *texWidth,int *texHeight);

  bool font_loadFile(const char *fn,unsigned char **data,
                     stbtt_fontinfo *fontInfo);

  void font_glyphQuad(const struct FontGlyph *glyph,
                      bool flip,float rowAdvance,
                      float texInvWidth,float texInvHeight,
                      float x, float y,
                      stbtt_aligned_quad *q);

  void font_quadVerts(const stbtt_aligned_quad *q, float *verts,bool ccw);


  void font_asciiTextGlyphs(const stbtt_fontinfo *fontInfo,
                            float scale,int ascent,
                            struct FontGlyph *glyphs);

  void font_simpleCreate(const stbtt_fontinfo *fontInfo,
                         int pointSize,
                         struct SimpleFont *simpleFont,
                         unsigned char **bits);

  bool font_simpleCreateFromFile(const char *fn,
                                 int pointSize,
                                 struct SimpleFont *simpleFont,
                                 unsigned char **bits);


  void font_simpleCharGlyph(const struct SimpleFont *simpleFont,
                            char c,struct FontGlyph *glyph);

  void font_simpleDrawVerts(const char *str,int strCount,float x,float y,
                            const struct SimpleFont *simpleFont,
                            bool flip,bool ccw,
                            float *verts, int *vertsNum);

  void font_scaleDrawVertsToScreen(float *verts, int vertsNum,int screenWidth,int screenHeight);



#ifdef __cplusplus
}
#endif
#endif
