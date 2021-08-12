
#ifndef BGLBUILD_H_INCLUDED_
#define BGLBUILD_H_INCLUDED_

#ifdef USE_OPENGL

#if !defined GEKKO && !defined EDUKE32_GLES
# define DYNAMIC_GL
# define DYNAMIC_GLU
# define DYNAMIC_GLEXT
# define USE_GLEXT
#endif

#if defined EDUKE32_OSX
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif
#if defined EDUKE32_GLES
# include "jwzgles.h"
#endif

# ifdef _WIN32
#  define PR_CALLBACK __stdcall
# else
#  define PR_CALLBACK
# endif
// custom error checking

extern GLenum BuildGLError;
extern void BuildGLErrorCheck(void);


//////// dynamic/static API wrapping ////////

#if !defined RENDERTYPESDL && defined _WIN32 && defined DYNAMIC_GL
typedef HGLRC (WINAPI * bwglCreateContextProcPtr)(HDC);
extern bwglCreateContextProcPtr bwglCreateContext;
#define wglCreateContext bwglCreateContext
typedef BOOL (WINAPI * bwglDeleteContextProcPtr)(HGLRC);
extern bwglDeleteContextProcPtr bwglDeleteContext;
#define wglDeleteContext bwglDeleteContext
typedef PROC (WINAPI * bwglGetProcAddressProcPtr)(LPCSTR);
extern bwglGetProcAddressProcPtr bwglGetProcAddress;
#define wglGetProcAddress bwglGetProcAddress
typedef BOOL (WINAPI * bwglMakeCurrentProcPtr)(HDC,HGLRC);
extern bwglMakeCurrentProcPtr bwglMakeCurrent;
#define wglMakeCurrent bwglMakeCurrent

typedef int32_t (WINAPI * bwglChoosePixelFormatProcPtr)(HDC,CONST PIXELFORMATDESCRIPTOR*);
extern bwglChoosePixelFormatProcPtr bwglChoosePixelFormat;
#define wglChoosePixelFormat bwglChoosePixelFormat
typedef int32_t (WINAPI * bwglDescribePixelFormatProcPtr)(HDC,int32_t,UINT,LPPIXELFORMATDESCRIPTOR);
extern bwglDescribePixelFormatProcPtr bwglDescribePixelFormat;
#define wglDescribePixelFormat bwglDescribePixelFormat
typedef int32_t (WINAPI * bwglGetPixelFormatProcPtr)(HDC);
extern bwglGetPixelFormatProcPtr bwglGetPixelFormat;
#define wglGetPixelFormat bwglGetPixelFormat
typedef BOOL (WINAPI * bwglSetPixelFormatProcPtr)(HDC,int32_t,const PIXELFORMATDESCRIPTOR*);
extern bwglSetPixelFormatProcPtr bwglSetPixelFormat;
#define wglSetPixelFormat bwglSetPixelFormat
#endif

#if defined DYNAMIC_GLU

// GLU
typedef void             (APIENTRY * bgluPerspectiveProcPtr)(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
extern bgluPerspectiveProcPtr bgluPerspective;

typedef const GLubyte *  (APIENTRY * bgluErrorStringProcPtr)(GLenum error);
extern bgluErrorStringProcPtr bgluErrorString;

typedef GLint            (APIENTRY * bgluProjectProcPtr)(GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint	*view, GLdouble* winX, GLdouble* winY, GLdouble* winZ);
extern bgluProjectProcPtr bgluProject;
typedef GLint            (APIENTRY * bgluUnProjectProcPtr)(GLdouble winX, GLdouble winY, GLdouble winZ, const GLdouble * model, const GLdouble * proj, const GLint * view, GLdouble* objX, GLdouble* objY, GLdouble* objZ);
extern bgluUnProjectProcPtr bgluUnProject;

#else

#define bgluPerspective gluPerspective

#define bgluErrorString gluErrorString

#define bgluProject gluProject
#define bgluUnProject gluUnProject

#endif


//////// glGenTextures/glDeleteTextures debugging ////////
void texdbg_bglGenTextures(GLsizei n, GLuint *textures, const char *srcfn);
void texdbg_bglDeleteTextures(GLsizei n, const GLuint *textures, const char *srcfn);

//#define DEBUG_TEXTURE_NAMES

#if defined DEBUGGINGAIDS && defined DEBUG_TEXTURE_NAMES
# define glGenTextures(numtexs, texnamear) texdbg_bglGenTextures(numtexs, texnamear, __FILE__)
# define glDeleteTextures(numtexs, texnamear) texdbg_bglDeleteTextures(numtexs, texnamear, __FILE__)
#endif

#endif //USE_OPENGL

#if !defined RENDERTYPESDL && defined _WIN32 && defined DYNAMIC_GL
extern char *gldriver;

int32_t loadwgl(const char *driver);
int32_t unloadwgl(void);
#endif

#ifdef POLYMER
int32_t loadglulibrary(const char *driver);
int32_t unloadglulibrary(void);
#endif

#endif
