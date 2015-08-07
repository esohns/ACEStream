ACLOCAL_AMFLAGS = \
  -I m4

DEFS += \
  -DSTREAM_BUILD_DLL=1 \
  -DMETA_PACKAGE=\"@PACKAGE_NAME@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ATCD/ACE
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

noinst_HEADERS = \
  stdafx.h
include_HEADERS = \
  stream_allocatorheap.h \
  stream_base.h \
  stream_base.inl \
  stream_cachedmessageallocator.h \
  stream_cachedmessageallocator.inl \
  stream_cachedmessageallocatorheap_base.h \
  stream_cachedmessageallocatorheap_base.inl \
  stream_cachedmessageallocatorheap.h \
  stream_cacheddatablockallocatorheap.h \
  stream_cachedallocatorheap.h \
  stream_common.h \
  stream_data_message_base.h \
  stream_data_message_base.inl \
  stream_datablockallocatorheap.h \
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
  stream_message_base.h \
  stream_message_base.inl \
  stream_messageallocatorheap_base.h \
  stream_messageallocatorheap_base.inl \
  stream_messagequeue.h \
  stream_messagequeue_base.h \
  stream_messagequeue_base.inl \
  stream_module_base.h \
  stream_module_base.inl \
  stream_module_filereader.h \
  stream_module_filereader.inl \
  stream_module_filewriter.h \
  stream_module_filewriter.inl \
  stream_module_messagehandler.h \
  stream_module_messagehandler.inl \
  stream_module_runtimestatistic.h \
  stream_module_runtimestatistic.inl \
  stream_resetcounterhandler.h \
  stream_session_data.h \
  stream_session_data.inl \
  stream_session_data_base.h \
  stream_session_data_base.inl \
  stream_session_message_base.h \
  stream_session_message_base.inl \
  stream_statemachine_control.h \
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
  stream_allocatorheap.cpp \
  stream_allocatorheap.h \
  stream_base.cpp \
  stream_base.h \
  stream_base.inl \
  stream_cachedmessageallocator.cpp \
  stream_cachedmessageallocator.h \
  stream_cachedmessageallocator.inl \
  stream_cachedmessageallocatorheap_base.cpp \
  stream_cachedmessageallocatorheap_base.h \
  stream_cachedmessageallocatorheap_base.inl \
  stream_cachedmessageallocatorheap.cpp \
  stream_cachedmessageallocatorheap.h \
  stream_cacheddatablockallocatorheap.cpp \
  stream_cacheddatablockallocatorheap.h \
  stream_cachedallocatorheap.cpp \
  stream_cachedallocatorheap.h \
  stream_common.h \
  stream_data_message_base.cpp \
  stream_data_message_base.h \
  stream_data_message_base.inl \
  stream_datablockallocatorheap.cpp \
  stream_datablockallocatorheap.h \
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
  stream_message_base.cpp \
  stream_message_base.h \
  stream_message_base.inl \
  stream_messageallocatorheap_base.cpp \
  stream_messageallocatorheap_base.h \
  stream_messageallocatorheap_base.inl \
  stream_messagequeue.cpp \
  stream_messagequeue.h \
  stream_messagequeue_base.cpp \
  stream_messagequeue_base.h \
  stream_messagequeue_base.inl \
  stream_module_base.cpp \
  stream_module_base.inl \
  stream_module_base.h \
  stream_module_filereader.cpp \
  stream_module_filereader.h \
  stream_module_filereader.inl \
  stream_module_filewriter.cpp \
  stream_module_filewriter.h \
  stream_module_filewriter.inl \
  stream_module_messagehandler.cpp \
  stream_module_messagehandler.h \
  stream_module_messagehandler.inl \
  stream_module_runtimestatistic.cpp \
  stream_module_runtimestatistic.h \
  stream_module_runtimestatistic.inl \
  stream_resetcounterhandler.cpp \
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
