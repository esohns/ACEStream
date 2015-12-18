ACLOCAL_AMFLAGS = \
  -I m4

DEFS += \
  -DSTREAM_BUILD_DLL=1 \
  -DPACKAGE=\"@PACKAGE_NAME@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ACE
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

noinst_HEADERS = \
  stdafx.h
include_HEADERS = \
  stream_allocatorbase.h \
  stream_allocatorbase.inl \
  stream_allocatorheap.h \
  stream_allocatorheap.inl \
  stream_base.h \
  stream_base.inl \
  stream_cachedallocatorheap.h \
  stream_cachedallocatorheap.inl \
  stream_cacheddatablockallocatorheap.h \
  stream_cachedmessageallocator.h \
  stream_cachedmessageallocator.inl \
  stream_cachedmessageallocatorheap.h \
  stream_cachedmessageallocatorheap_base.h \
  stream_cachedmessageallocatorheap_base.inl \
  stream_cachedmessageallocatorheap.h \
  stream_cacheddatablockallocatorheap.h \
  stream_common.h \
  stream_data_base.h \
  stream_data_base.inl \
  stream_datablockallocatorheap.h \
  stream_datablockallocatorheap.inl \
  stream_data_message_base.h \
  stream_data_message_base.inl \
  stream_defines.h \
  stream_exports.h \
  stream_headmoduletask_base.h \
  stream_headmoduletask_base.inl \
  stream_iallocator.h \
  stream_imessagequeue.h \
  stream_imodule.h \
  stream_istreamcontrol.h \
  stream_itask.h \
  stream_macros.h \
  stream_messageallocatorheap_base.h \
  stream_messageallocatorheap_base.inl \
  stream_message_base.h \
  stream_message_base.inl \
  stream_messagequeue.h \
  stream_messagequeue_base.h \
  stream_messagequeue_base.inl \
  stream_module_base.h \
  stream_module_base.inl \
  stream_resetcounterhandler.h \
  stream_session_data.h \
  stream_session_data.inl \
  stream_session_data_base.h \
  stream_session_data_base.inl \
  stream_session_message_base.h \
  stream_session_message_base.inl \
  stream_statemachine_control.h \
  stream_statemachine_control.inl \
  stream_statistichandler.h \
  stream_statistichandler.inl \
  stream_streammodule_base.h \
  stream_streammodule_base.inl \
  stream_task_base.h \
  stream_task_base.inl \
  stream_task_base_asynch.h \
  stream_task_base_asynch.inl \
  stream_task_base_synch.h \
  stream_task_base_synch.inl \
  stream_tools.h

lib_LTLIBRARIES = libACEStream.la

libACEStream_la_SOURCES = \
  stream_allocatorbase.cpp \
  stream_allocatorbase.h \
  stream_allocatorbase.inl \
  stream_allocatorheap.cpp \
  stream_allocatorheap.h \
  stream_allocatorheap.inl \
  stream_base.cpp \
  stream_base.h \
  stream_base.inl \
  stream_cachedallocatorheap.cpp \
  stream_cachedallocatorheap.h \
  stream_cachedallocatorheap.inl \
  stream_cacheddatablockallocatorheap.cpp \
  stream_cacheddatablockallocatorheap.h \
  stream_cachedmessageallocator.cpp \
  stream_cachedmessageallocator.h \
  stream_cachedmessageallocator.inl \
  stream_cachedmessageallocatorheap.cpp \
  stream_cachedmessageallocatorheap.h \
  stream_cachedmessageallocatorheap_base.cpp \
  stream_cachedmessageallocatorheap_base.h \
  stream_cachedmessageallocatorheap_base.inl \
  stream_cachedmessageallocatorheap.cpp \
  stream_cachedmessageallocatorheap.h \
  stream_cacheddatablockallocatorheap.cpp \
  stream_cacheddatablockallocatorheap.h \
  stream_common.h \
  stream_data_base.cpp \
  stream_data_base.h \
  stream_data_base.inl \
  stream_datablockallocatorheap.cpp \
  stream_datablockallocatorheap.h \
  stream_datablockallocatorheap.inl \
  stream_data_message_base.cpp \
  stream_data_message_base.h \
  stream_data_message_base.inl \
  stream_defines.h \
  stream_exports.h \
  stream_headmoduletask_base.cpp \
  stream_headmoduletask_base.h \
  stream_headmoduletask_base.inl \
  stream_iallocator.h \
  stream_imessagequeue.h \
  stream_imodule.h \
  stream_istreamcontrol.h \
  stream_itask.h \
  stream_macros.h \
  stream_messageallocatorheap_base.cpp \
  stream_messageallocatorheap_base.h \
  stream_messageallocatorheap_base.inl \
  stream_message_base.cpp \
  stream_message_base.h \
  stream_message_base.inl \
  stream_messagequeue.h \
  stream_messagequeue_base.cpp \
  stream_messagequeue_base.h \
  stream_messagequeue_base.inl \
  stream_module_base.cpp \
  stream_module_base.h \
  stream_module_base.inl \
  stream_resetcounterhandler.h \
  stream_session_data.cpp \
  stream_session_data.h \
  stream_session_data.inl \
  stream_session_data_base.cpp \
  stream_session_data_base.h \
  stream_session_data_base.inl \
  stream_session_message_base.cpp \
  stream_session_message_base.h \
  stream_session_message_base.inl \
  stream_statemachine_control.cpp \
  stream_statemachine_control.h \
  stream_statemachine_control.inl \
  stream_statistichandler.cpp \
  stream_statistichandler.h \
  stream_statistichandler.inl \
  stream_streammodule_base.cpp \
  stream_streammodule_base.h \
  stream_streammodule_base.inl \
  stream_task_base.cpp \
  stream_task_base.h \
  stream_task_base.inl \
  stream_task_base_asynch.cpp \
  stream_task_base_asynch.h \
  stream_task_base_asynch.inl \
  stream_task_base_synch.cpp \
  stream_task_base_synch.h \
  stream_task_base_synch.inl \
  stream_tools.cpp \
  stream_tools.h
#  stdafx.cpp \
#  stdafx.h

libACEStream_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src
libACEStream_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

SUBDIRS = \
  . \
  modules

