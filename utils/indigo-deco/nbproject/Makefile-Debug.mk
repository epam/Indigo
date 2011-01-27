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
	${OBJECTDIR}/_ext/2043651719/os_dir_posix.o \
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
LDLIBSOPTIONS=../../api/dist/DebugStatic/GNU-Linux-x86/libindigo.a ../../layout/dist/Debug/GNU-Linux-x86/liblayout.a ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a ../../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a -lpthread -lz -lstdc++

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/indigo-deco

dist/Debug/GNU-Linux-x86/indigo-deco: ../../api/dist/DebugStatic/GNU-Linux-x86/libindigo.a

dist/Debug/GNU-Linux-x86/indigo-deco: ../../layout/dist/Debug/GNU-Linux-x86/liblayout.a

dist/Debug/GNU-Linux-x86/indigo-deco: ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a

dist/Debug/GNU-Linux-x86/indigo-deco: ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a

dist/Debug/GNU-Linux-x86/indigo-deco: ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a

dist/Debug/GNU-Linux-x86/indigo-deco: ../../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a

dist/Debug/GNU-Linux-x86/indigo-deco: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/indigo-deco ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/2043651719/os_dir_posix.o: ../../common/base_c/os_dir_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/2043651719
	${RM} $@.d
	$(COMPILE.c) -g -Wall -I../../api -I../../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2043651719/os_dir_posix.o ../../common/base_c/os_dir_posix.c

${OBJECTDIR}/main.o: main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -I../../api -I../../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

# Subprojects
.build-subprojects:
	cd ../../api && ${MAKE}  -f Makefile CONF=DebugStatic
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug
	cd ../../tinyxml && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/indigo-deco

# Subprojects
.clean-subprojects:
	cd ../../api && ${MAKE}  -f Makefile CONF=DebugStatic clean
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../tinyxml && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
