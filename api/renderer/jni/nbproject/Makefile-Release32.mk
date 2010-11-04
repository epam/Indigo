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
CND_CONF=Release32
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/indigo_renderer_jni.o


# C Compiler Flags
CFLAGS=-m32 -fPIC

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../dist/ReleaseStatic32/GNU-Linux-x86/libindigo-renderer.a ../../../render2d/dist/Release32/GNU-Linux-x86/librender2d.a -lm -lz -lpthread -lcairo -lstdc++

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release32.mk dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so

dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so: ../dist/ReleaseStatic32/GNU-Linux-x86/libindigo-renderer.a

dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so: ../../../render2d/dist/Release32/GNU-Linux-x86/librender2d.a

dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so: ${OBJECTFILES}
	${MKDIR} -p dist/Release32/GNU-Linux-x86
	${LINK.c} -rdynamic -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libindigo-renderer-jni.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/indigo_renderer_jni.o: indigo_renderer_jni.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I.. -I../.. -I../../jni -I../../../common/jni -I../../../common/jni/linux -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/indigo_renderer_jni.o indigo_renderer_jni.c

# Subprojects
.build-subprojects:
	cd .. && ${MAKE}  -f Makefile CONF=ReleaseStatic32
	cd ../../../render2d && ${MAKE}  -f Makefile CONF=Release32

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release32
	${RM} dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so

# Subprojects
.clean-subprojects:
	cd .. && ${MAKE}  -f Makefile CONF=ReleaseStatic32 clean
	cd ../../../render2d && ${MAKE}  -f Makefile CONF=Release32 clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
