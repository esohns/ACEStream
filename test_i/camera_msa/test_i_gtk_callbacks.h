/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef TEST_I_GTK_CALLBACKS_H
#define TEST_I_GTK_CALLBACKS_H

#include "gtk/gtk.h"

//------------------------------------------------------------------------------

// idle routines
gboolean idle_initialize_GTK_UI_cb (gpointer);
gboolean idle_finalize_GTK_UI_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
G_MODULE_EXPORT void scale_uv_cutoff_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_step_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_dt_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_fade_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_iterations_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_viscosity_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_color_multiplier_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_velocity_multiplier_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void button_reset_clicked_cb (GtkButton*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
