#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=cc
CCC=CC
CXX=CC
FC=f95
AS=as

# Macros
CND_PLATFORM=OracleSolarisStudio-Solaris-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1386528437/stdafx.o \
	${OBJECTDIR}/_ext/1386528437/stream.o \
	${OBJECTDIR}/_ext/1386528437/stream_allocatorheap.o \
	${OBJECTDIR}/_ext/1386528437/stream_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_cachedallocatorheap.o \
	${OBJECTDIR}/_ext/1386528437/stream_cacheddatablockallocatorheap.o \
	${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocator.o \
	${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocatorheap.o \
	${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocatorheap_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_data_message_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_datablockallocatorheap.o \
	${OBJECTDIR}/_ext/1386528437/stream_headmoduletask.o \
	${OBJECTDIR}/_ext/1386528437/stream_headmoduletask_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_message_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_messageallocatorheap.o \
	${OBJECTDIR}/_ext/1386528437/stream_messageallocatorheap_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_messagequeue.o \
	${OBJECTDIR}/_ext/1386528437/stream_messagequeue_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_module_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_resetcounterhandler.o \
	${OBJECTDIR}/_ext/1386528437/stream_session_data.o \
	${OBJECTDIR}/_ext/1386528437/stream_session_data_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_session_message.o \
	${OBJECTDIR}/_ext/1386528437/stream_session_message_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_statemachine_control.o \
	${OBJECTDIR}/_ext/1386528437/stream_statistichandler.o \
	${OBJECTDIR}/_ext/1386528437/stream_streammodule_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_task.o \
	${OBJECTDIR}/_ext/1386528437/stream_task_asynch.o \
	${OBJECTDIR}/_ext/1386528437/stream_task_base.o \
	${OBJECTDIR}/_ext/1386528437/stream_task_base_asynch.o \
	${OBJECTDIR}/_ext/1386528437/stream_task_base_synch.o \
	${OBJECTDIR}/_ext/1386528437/stream_task_synch.o \
	${OBJECTDIR}/_ext/1386528437/stream_tools.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACEStream.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACEStream.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACEStream.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -G -KPIC -norunpath -h liblibACEStream.${CND_DLIB_EXT}

${OBJECTDIR}/_ext/1386528437/stdafx.o: ../../../src/stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stdafx.o ../../../src/stdafx.cpp

${OBJECTDIR}/_ext/1386528437/stream.o: ../../../src/stream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream.o ../../../src/stream.cpp

${OBJECTDIR}/_ext/1386528437/stream_allocatorheap.o: ../../../src/stream_allocatorheap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_allocatorheap.o ../../../src/stream_allocatorheap.cpp

${OBJECTDIR}/_ext/1386528437/stream_base.o: ../../../src/stream_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_base.o ../../../src/stream_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_cachedallocatorheap.o: ../../../src/stream_cachedallocatorheap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_cachedallocatorheap.o ../../../src/stream_cachedallocatorheap.cpp

${OBJECTDIR}/_ext/1386528437/stream_cacheddatablockallocatorheap.o: ../../../src/stream_cacheddatablockallocatorheap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_cacheddatablockallocatorheap.o ../../../src/stream_cacheddatablockallocatorheap.cpp

${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocator.o: ../../../src/stream_cachedmessageallocator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocator.o ../../../src/stream_cachedmessageallocator.cpp

${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocatorheap.o: ../../../src/stream_cachedmessageallocatorheap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocatorheap.o ../../../src/stream_cachedmessageallocatorheap.cpp

${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocatorheap_base.o: ../../../src/stream_cachedmessageallocatorheap_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_cachedmessageallocatorheap_base.o ../../../src/stream_cachedmessageallocatorheap_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_data_message_base.o: ../../../src/stream_data_message_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_data_message_base.o ../../../src/stream_data_message_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_datablockallocatorheap.o: ../../../src/stream_datablockallocatorheap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_datablockallocatorheap.o ../../../src/stream_datablockallocatorheap.cpp

${OBJECTDIR}/_ext/1386528437/stream_headmoduletask.o: ../../../src/stream_headmoduletask.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_headmoduletask.o ../../../src/stream_headmoduletask.cpp

${OBJECTDIR}/_ext/1386528437/stream_headmoduletask_base.o: ../../../src/stream_headmoduletask_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_headmoduletask_base.o ../../../src/stream_headmoduletask_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_message_base.o: ../../../src/stream_message_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_message_base.o ../../../src/stream_message_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_messageallocatorheap.o: ../../../src/stream_messageallocatorheap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_messageallocatorheap.o ../../../src/stream_messageallocatorheap.cpp

${OBJECTDIR}/_ext/1386528437/stream_messageallocatorheap_base.o: ../../../src/stream_messageallocatorheap_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_messageallocatorheap_base.o ../../../src/stream_messageallocatorheap_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_messagequeue.o: ../../../src/stream_messagequeue.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_messagequeue.o ../../../src/stream_messagequeue.cpp

${OBJECTDIR}/_ext/1386528437/stream_messagequeue_base.o: ../../../src/stream_messagequeue_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_messagequeue_base.o ../../../src/stream_messagequeue_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_module_base.o: ../../../src/stream_module_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_module_base.o ../../../src/stream_module_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_resetcounterhandler.o: ../../../src/stream_resetcounterhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_resetcounterhandler.o ../../../src/stream_resetcounterhandler.cpp

${OBJECTDIR}/_ext/1386528437/stream_session_data.o: ../../../src/stream_session_data.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_session_data.o ../../../src/stream_session_data.cpp

${OBJECTDIR}/_ext/1386528437/stream_session_data_base.o: ../../../src/stream_session_data_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_session_data_base.o ../../../src/stream_session_data_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_session_message.o: ../../../src/stream_session_message.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_session_message.o ../../../src/stream_session_message.cpp

${OBJECTDIR}/_ext/1386528437/stream_session_message_base.o: ../../../src/stream_session_message_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_session_message_base.o ../../../src/stream_session_message_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_statemachine_control.o: ../../../src/stream_statemachine_control.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_statemachine_control.o ../../../src/stream_statemachine_control.cpp

${OBJECTDIR}/_ext/1386528437/stream_statistichandler.o: ../../../src/stream_statistichandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_statistichandler.o ../../../src/stream_statistichandler.cpp

${OBJECTDIR}/_ext/1386528437/stream_streammodule_base.o: ../../../src/stream_streammodule_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_streammodule_base.o ../../../src/stream_streammodule_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_task.o: ../../../src/stream_task.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_task.o ../../../src/stream_task.cpp

${OBJECTDIR}/_ext/1386528437/stream_task_asynch.o: ../../../src/stream_task_asynch.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_task_asynch.o ../../../src/stream_task_asynch.cpp

${OBJECTDIR}/_ext/1386528437/stream_task_base.o: ../../../src/stream_task_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_task_base.o ../../../src/stream_task_base.cpp

${OBJECTDIR}/_ext/1386528437/stream_task_base_asynch.o: ../../../src/stream_task_base_asynch.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_task_base_asynch.o ../../../src/stream_task_base_asynch.cpp

${OBJECTDIR}/_ext/1386528437/stream_task_base_synch.o: ../../../src/stream_task_base_synch.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_task_base_synch.o ../../../src/stream_task_base_synch.cpp

${OBJECTDIR}/_ext/1386528437/stream_task_synch.o: ../../../src/stream_task_synch.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_task_synch.o ../../../src/stream_task_synch.cpp

${OBJECTDIR}/_ext/1386528437/stream_tools.o: ../../../src/stream_tools.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -g -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stream_tools.o ../../../src/stream_tools.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACEStream.${CND_DLIB_EXT}
	${CCADMIN} -clean

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
