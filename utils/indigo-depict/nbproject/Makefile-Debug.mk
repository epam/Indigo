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
	${OBJECTDIR}/main.o


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
LDLIBSOPTIONS=../../api/renderer/dist/DebugStatic/GNU-Linux-x86/libindigo-renderer.a ../../api/dist/DebugStatic/GNU-Linux-x86/libindigo.a ../../render2d/dist/Debug/GNU-Linux-x86/librender2d.a ../../layout/dist/Debug/GNU-Linux-x86/liblayout.a ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a ../../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a -lpthread -lz -lcairo -lstdc++

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/indigo-depict

dist/Debug/GNU-Linux-x86/indigo-depict: ../../api/renderer/dist/DebugStatic/GNU-Linux-x86/libindigo-renderer.a

dist/Debug/GNU-Linux-x86/indigo-depict: ../../api/dist/DebugStatic/GNU-Linux-x86/libindigo.a

dist/Debug/GNU-Linux-x86/indigo-depict: ../../render2d/dist/Debug/GNU-Linux-x86/librender2d.a

dist/Debug/GNU-Linux-x86/indigo-depict: ../../layout/dist/Debug/GNU-Linux-x86/liblayout.a

dist/Debug/GNU-Linux-x86/indigo-depict: ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a

dist/Debug/GNU-Linux-x86/indigo-depict: ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a

dist/Debug/GNU-Linux-x86/indigo-depict: ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a

dist/Debug/GNU-Linux-x86/indigo-depict: ../../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a

dist/Debug/GNU-Linux-x86/indigo-depict: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/indigo-depict ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/main.o: main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -I../../api -I../../api/renderer -I../../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

# Subprojects
.build-subprojects:
	cd ../../api/renderer && ${MAKE}  -f Makefile CONF=DebugStatic
	cd ../../api && ${MAKE}  -f Makefile CONF=DebugStatic
	cd ../../render2d && ${MAKE}  -f Makefile CONF=Debug
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug
	cd ../../tinyxml && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/indigo-depict

# Subprojects
.clean-subprojects:
	cd ../../api/renderer && ${MAKE}  -f Makefile CONF=DebugStatic clean
	cd ../../api && ${MAKE}  -f Makefile CONF=DebugStatic clean
	cd ../../render2d && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../tinyxml && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
