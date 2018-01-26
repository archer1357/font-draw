#include "test.h"

struct SimpleFont simpleFont;
GLuint fontTex;

void draw_back(float r,float g,float b) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS);
  glColor3f(r,g,b); glVertex3f( -1.0f,-1.0f,0.0f);
  glColor3f(r,g,b); glVertex3f( 1.0f,-1.0f,0.0f);
  glColor3f(r,g,b); glVertex3f( 1.0f,1.0f,0.0f);
  glColor3f(r,g,b); glVertex3f( -1.0f,1.0f,0.0f);
  glEnd();
}
void draw(bool persp,int windowWidth,int windowHeight) {


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if(persp) {
    double aspect=(double)windowWidth/(double)windowHeight;
    gluPerspective(45.0,aspect,1.0,1000.0);
  } else {
    // glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //
  if(persp) {
    glTranslatef(0.0f,0.0f,-500.0f);
    // gluLookAt(0.0,0.0,500.0, 0.0,0.0,0.0, 0.0,1.0,0.0);
  } else {
    // glScalef(1.0f/(float)windowWidth,1.0f/(float)windowHeight,1.0f);
  }

  //
  glBegin(GL_TRIANGLES);
  glColor3f(0.0,0.0,1.0); glVertex3f( 0.0f,0.0f,0.0f);
  glColor3f(0.0,1.0,0.0); glVertex3f( 1.0f, 0.0f,0.0f);
  glColor3f(1.0,0.0,0.0); glVertex3f(1.0f,1.0f,0.0f);
  glEnd();


  //
  const char *text="hello world!";
  int textSize=strlen(text);
  float verts[256*4];
  int vertsNum;
  bool waa=true;
  // waa=!bla;
  waa=false;

  font_simpleDrawVerts(text,textSize,0.0f,0.0f,
                       &simpleFont,waa,false,
                       verts,&vertsNum);

  if(!persp) {

    font_scaleDrawVertsToScreen(verts,vertsNum,windowWidth,windowHeight);
  }

  int i;



  // printf("%i\n",vertsNum);
  glEnable(GL_TEXTURE_2D);

  glBegin(GL_TRIANGLES);
  for(i=0;i<vertsNum;i++) {
    // printf("%f %f\n",verts[i*4+0],verts[i*4+1]);
    glTexCoord2f(verts[i*4+2],verts[i*4+3]);
    glVertex3f(verts[i*4+0],verts[i*4+1], 0.0f);
  }

  glEnd();
  glDisable(GL_TEXTURE_2D);

  // //

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if(persp) {
    glTranslatef(0.0f,0.0f,-50.0f);
    // gluLookAt(0.0,0.0,500.0, 0.0,0.0,0.0, 0.0,1.0,0.0);
  }



  glBegin(GL_LINES);
  glColor3f(1.0f,1.0f,1.0f);
  glVertex3f(-100.0f, 0.0f, 0.0f);
  glColor3f(1.0f,1.0f,1.0f);
  glVertex3f(100.0f, 0.0f, 0.0f);

  glColor3f(1.0f,1.0f,1.0f);
  glVertex3f( 0.0f, -100.0f,0.0f);
  glColor3f(1.0f,1.0f,1.0f);
  glVertex3f( 0.0f, 100.0f,0.0f);
  glEnd();

}

void displayCall() {

  glClearColor(0.0f,0.0f,0.0f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int windowWidth=SDL_GetVideoSurface()->w/2;
  int windowHeight=SDL_GetVideoSurface()->h/2;

  //
  glEnable(GL_CULL_FACE);


  //bottom left : persp, cull
  glViewport(0,0,windowWidth,windowHeight);
  draw_back(0.6f,0.4f,0.4f);
  draw(true,windowWidth,windowHeight);

  //bottom right : ortho, cull
  glViewport(windowWidth,0,windowWidth,windowHeight);
  draw_back(0.4f,0.4f,0.6f);
  draw(false,windowWidth,windowHeight);

  //
  glDisable(GL_CULL_FACE);

  //top left : persp, no cull
  glViewport(0,windowHeight,windowWidth,windowHeight);
  draw_back(0.4f,0.6f,0.4f);
  draw(true,windowWidth,windowHeight);

  //top right : orto, no cull
  glViewport(windowWidth,windowHeight,windowWidth,windowHeight);
  draw_back(0.7f,0.6f,0.1f);
  draw(false,windowWidth,windowHeight);

}

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_SetVideoMode(800,600, 32, SDL_OPENGL);
  SDL_WM_SetCaption( "Font Test", NULL );


  unsigned char *fontBitmap;

  //
  if(!font_simpleCreateFromFile("DroidSerif-Regular.ttf",22,&simpleFont,&fontBitmap)) {
    return 1;
  }



  glGenTextures(1,&fontTex);
  glBindTexture(GL_TEXTURE_2D,fontTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,simpleFont.texWidth,simpleFont.texHeight,0,GL_RED,GL_UNSIGNED_BYTE,fontBitmap);
// glTexImage2D(GL_TEXTURE_2D,0,GL_RED,simpleFont.texWidth,simpleFont.texHeight,0,GL_RED,GL_UNSIGNED_BYTE,fontBitmap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  free(fontBitmap);



  // glEnable(GL_DEPTH_TEST);

  // glCullFace(GL_FRONT);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);

  bool quit=false;

  while(!quit) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) ||
         event.type==SDL_QUIT) {
        quit=true;
      }
    }

    displayCall();
    SDL_GL_SwapBuffers();
  }


  SDL_Quit();
  return 0;
}
