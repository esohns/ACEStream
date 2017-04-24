#ifndef STREAM_MODULE_VIS_COMMON_H
#define STREAM_MODULE_VIS_COMMON_H

#include <deque>

#if defined (GTKGL_SUPPORT)
#include <gtk/gtk.h>
#endif

#include <ace/config-lite.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
enum Stream_Module_Visualization_MediaFrameWork
{
  STREAM_MODULE_VIS_FRAMEWORK_DIRECTSHOW,
  STREAM_MODULE_VIS_FRAMEWORK_MEDIAFOUNDATION,
  ////////////////////////////////////////
  STREAM_MODULE_VIS_FRAMEWORK_MAX,
  STREAM_MODULE_VIS_FRAMEWORK_INVALID
};
#endif

enum Stream_Module_Visualization_AudioRenderer
{
  STREAM_MODULE_VIS_AUDIORENDERER_GTK_CAIRO_SPECTRUMANALYZER = 0,
  ////////////////////////////////////////
  STREAM_MODULE_VIS_AUDIORENDERER_MAX,
  STREAM_MODULE_VIS_AUDIORENDERER_INVALID
};

enum Stream_Module_Visualization_VideoRenderer
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_MODULE_VIS_VIDEORENDERER_DIRECT3D = 0,
  STREAM_MODULE_VIS_VIDEORENDERER_DIRECTSHOW,
  STREAM_MODULE_VIS_VIDEORENDERER_GTK_CAIRO,
  STREAM_MODULE_VIS_VIDEORENDERER_MEDIAFOUNDATION,
#else
  STREAM_MODULE_VIS_VIDEORENDERER_GTK_CAIRO = 0,
#endif
  STREAM_MODULE_VIS_VIDEORENDERER_NULL,
  STREAM_MODULE_VIS_VIDEORENDERER_GTK_PIXBUF,
  ////////////////////////////////////////
  STREAM_MODULE_VIS_VIDEORENDERER_MAX,
  STREAM_MODULE_VIS_VIDEORENDERER_INVALID
};

#if defined (GTKGL_SUPPORT)
enum Stream_Module_Visualization_OpenGLInstructionType
{
  STREAM_MODULE_VIS_OPENGLINSTRUCTION_SET_COLOR_BG,
  STREAM_MODULE_VIS_OPENGLINSTRUCTION_SET_COLOR_FG,
  ////////////////////////////////////////
  STREAM_MODULE_VIS_OPENGLINSTRUCTION_MAX,
  STREAM_MODULE_VIS_OPENGLINSTRUCTION_INVALID
};
struct Stream_Module_Visualization_OpenGLInstruction
{
  inline Stream_Module_Visualization_OpenGLInstruction ()
   : type (STREAM_MODULE_VIS_OPENGLINSTRUCTION_INVALID)
  {};

  enum Stream_Module_Visualization_OpenGLInstructionType type;
  union {
#if defined (GTK3_SUPPORT)
    GdkRGBA                                              color;
#else
    GdkColor                                             color;
#endif
  };
};
typedef std::deque<struct Stream_Module_Visualization_OpenGLInstruction> Stream_Module_Visualization_OpenGLInstructions_t;
typedef Stream_Module_Visualization_OpenGLInstructions_t::const_iterator Stream_Module_Visualization_OpenGLInstructionsIterator_t;
#endif /* GTKGL_SUPPORT */

class Stream_Module_Visualization_IFullscreen
{
 public:
  inline ~Stream_Module_Visualization_IFullscreen () {};

  virtual void toggle () = 0;
};

#endif
