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
#include "stdafx.h"

#include "test_i_gtk_callbacks.h"

#include "ace/Log_Msg.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#include "test_i_camera_msa_common.h"
#include "test_i_camera_msa_defines.h"
#include "test_i_msafluidsolver2d.h"

gboolean
idle_initialize_GTK_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("idle_initialize_GTK_UI_cb"));

  // sanity check(s)
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    static_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
 
  // step2: (auto-)connect signals/slots
#if GTK_CHECK_VERSION (4,0,0)
#else
  gtk_builder_connect_signals ((*iterator).second.second,
                               ui_cb_data_p);
#endif // GTK_CHECK_VERSION (4,0,0)

  // step6a: connect default signals
#if GTK_CHECK_VERSION (4,0,0)
#else
  gulong result_2 =
      g_signal_connect (dialog_p,
                        ACE_TEXT_ALWAYS_CHAR ("destroy"),
                        G_CALLBACK (gtk_widget_destroyed),
                        &dialog_p);
  ACE_ASSERT (result_2);
#endif // GTK_CHECK_VERSION (4,0,0)

  // step9: draw main dialog
#if GTK_CHECK_VERSION (4,0,0)
  gtk_widget_show (dialog_p);
#else
  gtk_widget_show_all (dialog_p);
#endif // GTK_CHECK_VERSION (4,0,0)

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_GTK_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("idle_finalize_GTK_UI_cb"));

  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    static_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_UNUSED_ARG (ui_cb_data_p);

  return G_SOURCE_REMOVE;
}

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
scale_uv_cutoff_value_changed_cb (GtkRange* range_in,
                                  gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_uv_cutoff_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->UVCutoff_ =
    static_cast<float> (gtk_range_get_value (range_in));
}

void
scale_step_value_changed_cb (GtkRange* range_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_step_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->step_ =
    static_cast<int> (gtk_range_get_value (range_in));
}

void
scale_dt_value_changed_cb (GtkRange* range_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_dt_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->dt_ =
    static_cast<float> (gtk_range_get_value (range_in));
}

void
scale_fade_value_changed_cb (GtkRange* range_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_fade_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->fadeSpeed_ =
    static_cast<float> (gtk_range_get_value (range_in));
}

void
scale_iterations_value_changed_cb (GtkRange* range_in,
                                   gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_iterations_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->solverIterations_ =
    static_cast<int> (gtk_range_get_value (range_in));
}

void
scale_viscosity_value_changed_cb (GtkRange* range_in,
                                  gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_viscosity_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->viscosity_ =
    static_cast<float> (gtk_range_get_value (range_in));
}

void
scale_color_multiplier_value_changed_cb (GtkRange* range_in,
                                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_color_multiplier_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->colorMultiplier_ =
    static_cast<float> (gtk_range_get_value (range_in));
}

void scale_velocity_multiplier_value_changed_cb (GtkRange* range_in,
                                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("scale_velocity_multiplier_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (range_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->solver);

  ui_cb_data_p->solver->velocityMultiplier_ =
    static_cast<float> (gtk_range_get_value (range_in));
}

void
button_reset_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  // sanity check(s)
  ACE_ASSERT (button_in);
  struct Test_I_UI_GTK_CBData* ui_cb_data_p =
    reinterpret_cast<struct Test_I_UI_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT(iterator != ui_cb_data_p->UIState->builders.end ());

  GtkScale* scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_UVCUTOFF_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_UV_CUTOFF));

  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_STEP_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_STEP));

  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_DT_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_DT));

  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_FADE_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_FADESPEED));

  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_ITERATIONS_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_SOLVER_ITERATIONS));

  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_VISCOSITY_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_VISCOSITY));

  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_COLOR_MULTIPLIER_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_COLOR_MULTIPLIER));

  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCALE_VELOCITY_MULTIPLIER_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p), static_cast<double> (FLUID_DEFAULT_VELOCITY_MULTIPLIER));
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
