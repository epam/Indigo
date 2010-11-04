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
CC=gcc
CCC=g++
CXX=g++
FC=
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=DebugShared
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/indigo_render2d.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-fPIC
CXXFLAGS=-fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../render2d/dist/Debug/GNU-Linux-x86/librender2d.a -lpthread -lz -lcairo

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-DebugShared.mk dist/DebugShared/GNU-Linux-x86/libindigo-renderer.so

dist/DebugShared/GNU-Linux-x86/libindigo-renderer.so: ../../render2d/dist/Debug/GNU-Linux-x86/librender2d.a

dist/DebugShared/GNU-Linux-x86/libindigo-renderer.so: ${OBJECTFILES}
	${MKDIR} -p dist/DebugShared/GNU-Linux-x86
	${LINK.cc} -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libindigo-renderer.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/indigo_render2d.o: src/indigo_render2d.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../.. -I../../common -I.. -I../src -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_render2d.o src/indigo_render2d.cpp

# Subprojects
.build-subprojects:
	cd ../../render2d && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/DebugShared
	${RM} dist/DebugShared/GNU-Linux-x86/libindigo-renderer.so

# Subprojects
.clean-subprojects:
	cd ../../render2d && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
