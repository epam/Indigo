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
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/indigo_jni_base.o \
	${OBJECTDIR}/indigo_jni.o


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
LDLIBSOPTIONS=../dist/DebugStatic/GNU-Linux-x86/libindigo.a ../../render2d/dist/Debug/GNU-Linux-x86/librender2d.a ../../layout/dist/Debug/GNU-Linux-x86/liblayout.a ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a -lm -lz -lpthread -lstdc++ -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/libindigo-jni.so

dist/Debug/GNU-Linux-x86/libindigo-jni.so: ../dist/DebugStatic/GNU-Linux-x86/libindigo.a

dist/Debug/GNU-Linux-x86/libindigo-jni.so: ../../render2d/dist/Debug/GNU-Linux-x86/librender2d.a

dist/Debug/GNU-Linux-x86/libindigo-jni.so: ../../layout/dist/Debug/GNU-Linux-x86/liblayout.a

dist/Debug/GNU-Linux-x86/libindigo-jni.so: ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a

dist/Debug/GNU-Linux-x86/libindigo-jni.so: ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a

dist/Debug/GNU-Linux-x86/libindigo-jni.so: ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a

dist/Debug/GNU-Linux-x86/libindigo-jni.so: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	gcc -rdynamic -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libindigo-jni.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/indigo_jni_base.o: indigo_jni_base.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -I.. -I../jni-base -I../../common/jni -I../../common/jni/linux -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/indigo_jni_base.o indigo_jni_base.c

${OBJECTDIR}/indigo_jni.o: indigo_jni.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -I.. -I../jni-base -I../../common/jni -I../../common/jni/linux -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/indigo_jni.o indigo_jni.c

# Subprojects
.build-subprojects:
	cd .. && ${MAKE}  -f Makefile CONF=DebugStatic
	cd ../../render2d && ${MAKE}  -f Makefile CONF=Debug
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/libindigo-jni.so

# Subprojects
.clean-subprojects:
	cd .. && ${MAKE}  -f Makefile CONF=DebugStatic clean
	cd ../../render2d && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
