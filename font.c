#define STB_TRUETYPE_IMPLEMENTATION
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void font_createBitmap(const stbtt_fontinfo *fontInfo,
                         const struct FontGlyph *glyphs,
                         int glyphsNum,float scale,int texWidth,
                         int texHeight,unsigned char *pBits) {
  //Originally taken from the Urho3D Engine
  int i,y,x;
  int Pitch=texWidth;
  memset(pBits,0,texWidth*texHeight);

  // Render glyphs into texture, and find out a scaling value in case font uses less than full opacity (thin outlines)
  int sumOpacity = 0;
  int nonEmptyGlyphs = 0;

  for(i = 0; i < glyphsNum; ++i)  {
    unsigned char *output;
    output=(unsigned char*)pBits + Pitch * glyphs[i].y + glyphs[i].x;
    stbtt_MakeGlyphBitmap(fontInfo,output,glyphs[i].width,glyphs[i].height,
                          Pitch,scale,scale,glyphs[i].index);

    int glyphMaxOpacity = 0;

    for( y = 0; y < glyphs[i].height; ++y) {
      unsigned char* pixels;
      pixels=(unsigned char*)pBits+Pitch*(y+glyphs[i].y)+glyphs[i].x;

      for(x = 0; x < glyphs[i].width; ++x) {
        if((int)pixels[x] > glyphMaxOpacity) {
          glyphMaxOpacity=(int)pixels[x];
        }
      }
    }

    if(glyphMaxOpacity > 0) {
      sumOpacity += glyphMaxOpacity;
      ++nonEmptyGlyphs;
    }
  }

  // Apply the scaling if necessary
  int avgOpacity = nonEmptyGlyphs ? sumOpacity / nonEmptyGlyphs : 255;

  if(avgOpacity < 255) {
    float scale = 255.0f / avgOpacity;

    for(i = 0; i < glyphsNum; ++i)  {
      for(y = 0; y < glyphs[i].height; ++y) {
        unsigned char* dest;
        dest=(unsigned char*)pBits+Pitch*(y+glyphs[i].y)+glyphs[i].x;

        for(x = 0; x < glyphs[i].width; ++x) {
          int aaa=(int)(dest[x]*scale);
          dest[x]=(aaa < 255)?aaa:255;
        }
      }
    }
  }
}

int font_glyphCompare(const void *a,const void *b) {
  const struct FontGlyph *aa,*bb;
  aa=*((const struct FontGlyph **)a);
  bb=*((const struct FontGlyph **)b);

  if(aa->height<bb->height) {
    return 1;
  }

  if(aa->height==bb->height) {
    return 0;
  }

  //if(aa->mHeight>bb->mHeight) {
    return -1;
  //}
}

// void font_packer_getter(void *data,unsigned int *w,unsigned int *h) {
//   struct FontGlyph *glyph;
//   glyph=(struct FontGlyph*)data;

//   *w=(unsigned int)glyph->width;
//   *h=(unsigned int)glyph->height;
// }

// void font_packer_setter(void *data,unsigned int x,unsigned int y) {
//   struct FontGlyph *glyph;
//   glyph=(struct FontGlyph*)data;

//   glyph->x=(int)x;
//   glyph->y=(int)y;
// }

// void rect_packer(void *data,size_t count, size_t size,
//                  void (*getter)(void*,unsigned int*,unsigned int*),
//                  void (*setter)(void*,unsigned int,unsigned int)) {
//   while(data!=data+count*size) {
//     setter(data,5,5);

//     unsigned int w,h;
//     getter(data,&w,&h);

//     data+=size;
//   }
// }

void font_packGlyphs(struct FontGlyph *glyphs,
                       int glyphsNum,int padding,
                       int *texWidth,int *texHeight) {

  //todo: when glyph.height>glyph.width then flip them sideways
  // add sideways : bool to glyph struct
  // update glyphQuad func

  int i=0;
  int x=padding;
  int y=padding;
  int width=32;
  int height=32;
  int xEnd=0; //furthest x+glyphWidth has been entirely
  int xStart=padding; //where to start from after moving y down
  int yEnd=padding; //furthest y+glyphHeight has been entirely
  int yStep=0; //furthest y+glyphHeight has been within current area
  struct FontGlyph **sortedGlyphs;
  int j;

  sortedGlyphs=(struct FontGlyph **)malloc(sizeof(void*)*glyphsNum);

  for(j=0;j<glyphsNum;j++) {
    sortedGlyphs[j]=&glyphs[j];
  }

  qsort(sortedGlyphs, glyphsNum, sizeof(void*), font_glyphCompare);

  while(i<glyphsNum) {
    struct FontGlyph *glyph=sortedGlyphs[i];

    if(x+glyph->width+padding <= width &&
       y+glyph->height+padding <= height) { //go across
      glyph->x=x;
      glyph->y=y;
      x+=glyph->width+padding;

      //set furthest down within area
      if(y+glyph->height+padding > yStep) {
        yStep=y+glyph->height+padding;
      }

      //set furthest across
      if(x > xEnd) {
        xEnd=x;
      }

      //set furthest down
      if(yStep>yEnd) {
        yEnd=yStep;
      }

      i++;
    } else if(x+glyph->width+padding > width &&
              yStep+glyph->height+padding<=height) { //can't go across, go down
      //set x to last starting point, like a typewriter
      x=xStart;

      //set y down to furthest down within area
      y=yStep;
    } else if(width<=height) { //can't go across, can't go down, expand rightward
      //set x to furthest across
      xStart=x=xEnd;

      //y and yStep goes to zero since we are expanding rightward
      y=yStep=padding;

      //expand across
      width*=2;
    } else { //can't go across, can't go down, expand downward
      //x and xStart goes to zero since we are expanding downward
      x=xStart=padding;

      //set y to furthest down
      y=yEnd;

      //expand downward
      height*=2;
    }
  }

  //
  free(sortedGlyphs);

  //
  *texWidth=width;
  *texHeight=height;


}

bool font_loadFile(const char *fn,unsigned char **data,
                     stbtt_fontinfo *fontInfo) {
  FILE *file;
  int dataLen;

  //load font file
  file = fopen(fn, "rb");

  if(!file) {
    fprintf(stderr,"font read file error : %s\n",fn);
    return false;
  }

  fseek(file,0L,SEEK_END);
  dataLen = ftell(file);
  fseek(file,0L,SEEK_SET);
  *data=(unsigned char*)malloc(dataLen);
  fread(*data,1,dataLen,file);
  fclose(file);

  //
  if(!stbtt_InitFont(fontInfo,*data,0)) {
    fprintf(stderr,"font load error : %s\n",fn);
    free(data);
    return false;
  }

  return true;
}

void font_glyphQuad(const struct FontGlyph *glyph,
                      bool flip,float rowAdvance,
                      float texInvWidth,float texInvHeight,
                      float x, float y,
                      stbtt_aligned_quad *q) {

  if(flip) {
    q->y0 = y+(float)glyph->offsetY; //use floorf?
    q->y1 = q->y0 + (float)glyph->height;
  } else {
    q->y0 = y+(float)(rowAdvance-glyph->offsetY); //use floorf?
    q->y1 = q->y0 - (float)glyph->height;
  }

  q->x0 = x + (float)glyph->offsetX; //use floorf?
  q->x1 = q->x0 + (float)glyph->width;
  q->s0 = glyph->x*texInvWidth;
  q->t0 = (glyph->y + glyph->height)*texInvHeight;
  q->s1 = (glyph->x + glyph->width)*texInvWidth;
  q->t1 = glyph->y*texInvHeight;


}

void font_quadVerts(const stbtt_aligned_quad *q, float *verts,bool ccw) {
  if(ccw) {
    //0
    *(verts++)=q->x0; *(verts++)=q->y0;
    *(verts++)=q->s0; *(verts++)=q->t1;

    //1
    *(verts++)=q->x1; *(verts++)=q->y0;
    *(verts++)=q->s1; *(verts++)=q->t1;

    //2
    *(verts++)=q->x1; *(verts++)=q->y1;
    *(verts++)=q->s1; *(verts++)=q->t0;

    //0
    *(verts++)=q->x0; *(verts++)=q->y0;
    *(verts++)=q->s0; *(verts++)=q->t1;

    //2
    *(verts++)=q->x1; *(verts++)=q->y1;
    *(verts++)=q->s1; *(verts++)=q->t0;

    //3
    *(verts++)=q->x0; *(verts++)=q->y1;
    *(verts++)=q->s0; *(verts++)=q->t0;
  } else {
    //2
    *(verts++)=q->x1; *(verts++)=q->y1;
    *(verts++)=q->s1; *(verts++)=q->t0;

    //1
    *(verts++)=q->x1; *(verts++)=q->y0;
    *(verts++)=q->s1; *(verts++)=q->t1;

    //0
    *(verts++)=q->x0; *(verts++)=q->y0;
    *(verts++)=q->s0; *(verts++)=q->t1;

    //3
    *(verts++)=q->x0; *(verts++)=q->y1;
    *(verts++)=q->s0; *(verts++)=q->t0;

    //2
    *(verts++)=q->x1; *(verts++)=q->y1;
    *(verts++)=q->s1; *(verts++)=q->t0;

    //0
    *(verts++)=q->x0; *(verts++)=q->y0;
    *(verts++)=q->s0; *(verts++)=q->t1;


  }



  //   //0
  // verts[0]=q->x0; verts[1]=q->y0;
  //   verts[2]=q->s0; verts[3]=q->t1;


  //   //1
  //   verts[4]=q->x1; verts[5]=q->y0;
  //   verts[6]=q->s1; verts[7]=q->t1;

  //   //2
  //   verts[8]=q->x1; verts[9]=q->y1;
  //   verts[10]=q->s1; verts[11]=q->t0;

  //   //0
  //   verts[12]=q->x0; verts[13]=q->y0;
  //   verts[14]=q->s0; verts[15]=q->t1;

  //   //2
  //   verts[16]=q->x1; verts[17]=q->y1;
  //   verts[18]=q->s1; verts[19]=q->t0;

  //   //3
  //   verts[20]=q->x0; verts[21]=q->y1;
  //   verts[22]=q->s0; verts[23]=q->t0;


  //   // //0
  //   // *(verts++)=q->x0; *(verts++)=q->y0;
  //   // *(verts++)=q->s0; *(verts++)=q->t1;


  //   // //1
  //   // *(verts++)=q->x1; *(verts++)=q->y0;
  //   // *(verts++)=q->s1; *(verts++)=q->t1;

  //   // //2
  //   // *(verts++)=q->x1; *(verts++)=q->y1;
  //   // *(verts++)=q->s1; *(verts++)=q->t0;

  //   // //0
  //   // *(verts++)=q->x0; *(verts++)=q->y0;
  //   // *(verts++)=q->s0; *(verts++)=q->t1;

  //   // //2
  //   // *(verts++)=q->x1; *(verts++)=q->y1;
  //   // *(verts++)=q->s1; *(verts++)=q->t0;

  //   // //3
  //   // *(verts++)=q->x0; *(verts++)=q->y1;
  //   // *(verts++)=q->s0; *(verts++)=q->t0;

}

void font_asciiTextGlyphs(const stbtt_fontinfo *fontInfo,
                            float scale,int ascent,
                            struct FontGlyph *glyphs) {
  //Originally taken from the Urho3D Engine
  int i;
  //96 = 95 ascii + null = 126 - 32 + 1

  const int glyphsFrom=32;
  const int glyphsTo=127;

  int scaledAscent = (int)(scale * ascent);

  // Go through glyphs to get their dimensions & offsets
  for(i = glyphsFrom; i <= glyphsTo; ++i)  {

    //last one is for null
    int ii=(i==glyphsTo)?0:i;

    int mGlyphIndex = stbtt_FindGlyphIndex(fontInfo,ii);

    int ix0,iy0,ix1,iy1;
    int advanceWidth,leftSideBearing;

    stbtt_GetGlyphBitmapBox(fontInfo,mGlyphIndex,scale,scale,&ix0,&iy0,&ix1,&iy1);
    stbtt_GetGlyphHMetrics(fontInfo,mGlyphIndex,&advanceWidth,&leftSideBearing);
    glyphs[i-glyphsFrom].width = ix1 - ix0;
    glyphs[i-glyphsFrom].height = iy1 - iy0;
    glyphs[i-glyphsFrom].offsetX = (int)(leftSideBearing * scale);
    glyphs[i-glyphsFrom].offsetY = iy0+scaledAscent; //top align
    glyphs[i-glyphsFrom].colAdvance = (int)(advanceWidth * scale);
    glyphs[i-glyphsFrom].index=mGlyphIndex;
  }
}

void font_simpleCreate(const stbtt_fontinfo *fontInfo,
                        int pointSize,
                        struct SimpleFont *simpleFont,
                        unsigned char **bits) {

  int ascent,descent,lineGap;
  float scale;
  int w,h;

  //Get row height
  stbtt_GetFontVMetrics(fontInfo,&ascent,&descent,&lineGap);

  //Calculate scale (use ascent only)
  scale=(float)pointSize/ascent;

  //Calculate row advance
  simpleFont->rowAdvance=(int)(scale*(ascent-descent+lineGap));

  //
  font_asciiTextGlyphs(fontInfo,scale,ascent,simpleFont->glyphs);
  font_packGlyphs(simpleFont->glyphs,FONT_ASCII_TEXT_GLYPHS,1,&w,&h);

  //create bitmap
  *bits=(unsigned char*)malloc(w*h);
  font_createBitmap(fontInfo,simpleFont->glyphs,FONT_ASCII_TEXT_GLYPHS,scale,w,h,*bits);

  //
  simpleFont->texWidth=w;
  simpleFont->texHeight=h;
}
bool font_simpleCreateFromFile(const char *fn,
                                int pointSize,
                                struct SimpleFont *simpleFont,
                                unsigned char **bits) {
  unsigned char *data;
  stbtt_fontinfo fontInfo;

  if(!font_loadFile(fn,&data,&fontInfo)) {
    return false;
  }

  font_simpleCreate(&fontInfo,pointSize,
                     simpleFont,
                     bits);

  free(data);
  return true;
}

void font_simpleCharGlyph(const struct SimpleFont *simpleFont,
                          char c,struct FontGlyph *glyph) {
  const int glyphsFrom=32;
  const int glyphsTo=127;

  if(c >= glyphsFrom && c < glyphsTo) {
    *glyph=simpleFont->glyphs[c-glyphsFrom];
  } else { //else use null char
    *glyph=simpleFont->glyphs[glyphsTo-glyphsFrom];
  }

}

void font_simpleDrawVerts(const char *str,int strCount,float x,float y,
                           const struct SimpleFont *simpleFont,
                           bool flip,bool ccw,
                           float *verts, int *vertsNum) {

  float texInvWidth=1.0f/(float)simpleFont->texWidth;
  float texInvHeight=1.0f/(float)simpleFont->texHeight;
  float x2=x;
  const char *strEnd=&str[strCount];
  *vertsNum=0;

  while(str[0] && str < strEnd) {
    if(str[0]=='\n') {
      y+=(float)simpleFont->rowAdvance;
      x2=x;
    } else {
      struct FontGlyph glyph;
      font_simpleCharGlyph(simpleFont,str[0],&glyph);

      //
      stbtt_aligned_quad q;
      font_glyphQuad(&glyph,flip,(float)simpleFont->rowAdvance,
                       texInvWidth,texInvHeight,x2,y,&q);
      font_quadVerts(&q,verts,ccw);


      verts+=24;
      x2+=(float)glyph.colAdvance;
      (*vertsNum)+=6;
    }

    ++str;
  }
}

void font_scaleDrawVertsToScreen(float *verts, int vertsNum,int screenWidth,int screenHeight) {
  int i;
  float screenInvWidth,screenInvHeight;
  screenInvWidth=1.0f/(float)screenWidth;
  screenInvHeight=1.0f/(float)screenHeight;

  for(i=0;i<vertsNum;i++) {

    // verts[i*4+0]=-1.0f+2.0f*verts[i*4+0]*screenInvWidth;
    // verts[i*4+1]= 1.0f-2.0f*verts[i*4+1]*screenInvHeight;


    // verts[i*4+0]=2.0f*verts[i*4+0]*screenInvWidth-1.0f;
    // verts[i*4+1]=2.0f*verts[i*4+1]*screenInvHeight-1.0f;
    // verts[i*4+1]=1.7f+verts[i*4+1];


    verts[i*4+0]*=screenInvWidth;
    verts[i*4+1]*=screenInvHeight;
  }
}
