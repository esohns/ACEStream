#ifndef STREAM_VISUALIZATION_BASE_H
#define STREAM_VISUALIZATION_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Thread_Mutex.h"

#include "common_ui_ifullscreen.h"

#include "stream_vis_iresize.h"

class Stream_Visualization_Base
 : public Common_UI_IFullscreen
 , public Stream_Visualization_IResize
{
 public:
  Stream_Visualization_Base (ACE_Thread_Mutex*);
  inline virtual ~Stream_Visualization_Base () {}

  // implement Stream_Visualization_IResize
  inline virtual bool lock (bool block_in) { ACE_ASSERT (block_in && lock_2_); lock_2_->acquire (); return true; }
  inline virtual int unlock (bool unlockCompletely_in) { ACE_ASSERT (!unlockCompletely_in && lock_2_); return lock_2_->release (); }
  inline virtual void resize (const Common_Image_Resolution_t&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void resizing () { lock (true); resizing_ = true; unlock (false); }

 protected:
  ACE_Thread_Mutex* lock_2_;
  bool              resizing_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_Base ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_Base (const Stream_Visualization_Base&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_Base& operator= (const Stream_Visualization_Base&))
};

#endif // STREAM_VISUALIZATION_BASE_H
