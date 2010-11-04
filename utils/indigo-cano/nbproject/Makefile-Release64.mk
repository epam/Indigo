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
CND_CONF=Release64
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=-m64

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../api/dist/ReleaseStatic64/GNU-Linux-x86/libindigo.a ../../layout/dist/Release64/GNU-Linux-x86/liblayout.a ../../reaction/dist/Release64/GNU-Linux-x86/libreaction.a ../../molecule/dist/Release64/GNU-Linux-x86/libmolecule.a ../../graph/dist/Release64/GNU-Linux-x86/libgraph.a -lpthread -lz -lstdc++

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release64.mk dist/Release64/GNU-Linux-x86/indigo-cano

dist/Release64/GNU-Linux-x86/indigo-cano: ../../api/dist/ReleaseStatic64/GNU-Linux-x86/libindigo.a

dist/Release64/GNU-Linux-x86/indigo-cano: ../../layout/dist/Release64/GNU-Linux-x86/liblayout.a

dist/Release64/GNU-Linux-x86/indigo-cano: ../../reaction/dist/Release64/GNU-Linux-x86/libreaction.a

dist/Release64/GNU-Linux-x86/indigo-cano: ../../molecule/dist/Release64/GNU-Linux-x86/libmolecule.a

dist/Release64/GNU-Linux-x86/indigo-cano: ../../graph/dist/Release64/GNU-Linux-x86/libgraph.a

dist/Release64/GNU-Linux-x86/indigo-cano: ${OBJECTFILES}
	${MKDIR} -p dist/Release64/GNU-Linux-x86
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/indigo-cano ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/main.o: main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -s -I../../api -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

# Subprojects
.build-subprojects:
	cd ../../api && ${MAKE}  -f Makefile CONF=ReleaseStatic64
	cd ../../layout && ${MAKE}  -f Makefile CONF=Release64
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Release64
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Release64
	cd ../../graph && ${MAKE}  -f Makefile CONF=Release64

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release64
	${RM} dist/Release64/GNU-Linux-x86/indigo-cano

# Subprojects
.clean-subprojects:
	cd ../../api && ${MAKE}  -f Makefile CONF=ReleaseStatic64 clean
	cd ../../layout && ${MAKE}  -f Makefile CONF=Release64 clean
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Release64 clean
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Release64 clean
	cd ../../graph && ${MAKE}  -f Makefile CONF=Release64 clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
