#include "stdafx.h"

#include "stream_vis_base.h"

Stream_Visualization_Base::Stream_Visualization_Base (ACE_Thread_Mutex* lock_in)
 : lock_2_ (lock_in)
 , resizing_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Base::Stream_Visualization_Base"));

  // sanity check(s)
  ACE_ASSERT (lock_2_);
}
