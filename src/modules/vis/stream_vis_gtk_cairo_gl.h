/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef STREAM_VISUALIZATION_GTK_CAIRO_GL_H
#define STREAM_VISUALIZATION_GTK_CAIRO_GL_H

#include <functional>
#include <random>

#include "ace/Global_Macros.h"

#include "gtk/gtk.h"

#include "stream_stat_common.h"

#include "stream_vis_gtk_common.h"

class Stream_Visualization_GTK_Cairo_OpenGL
 : public Common_IDispatch_T<enum Stream_Statistic_AnalysisEventType>
{
 public:
  Stream_Visualization_GTK_Cairo_OpenGL ();
  inline virtual ~Stream_Visualization_GTK_Cairo_OpenGL () {}

  // implement Common_IDispatch_T
  virtual void dispatch (const enum Stream_Statistic_AnalysisEventType&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_OpenGL (const Stream_Visualization_GTK_Cairo_OpenGL&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_OpenGL& operator= (const Stream_Visualization_GTK_Cairo_OpenGL&))

#if GTK_CHECK_VERSION(3,0,0)
  GdkRGBA                                            backgroundColor_;
  GdkRGBA                                            foregroundColor_;
#else
  GdkColor                                           backgroundColor_;
  GdkColor                                           foregroundColor_;
#endif /* GTK_CHECK_VERSION (3,0,0) */
//#if defined (GTKGL_SUPPORT)
//  enum Stream_Visualization_SpectrumAnalyzer_3DMode* mode3D_;
//#endif // GTKGL_SUPPORT

  // random number generator
  std::uniform_int_distribution<int>                 randomDistribution_;
  std::default_random_engine                         randomEngine_;
  std::function<int ()>                              randomGenerator_;
};

#endif
