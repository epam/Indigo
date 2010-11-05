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
	${OBJECTDIR}/_ext/2043651719/os_dir_posix.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=-m32

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../api/dist/ReleaseStatic32/GNU-Linux-x86/libindigo.a ../../layout/dist/Release32/GNU-Linux-x86/liblayout.a ../../reaction/dist/Release32/GNU-Linux-x86/libreaction.a ../../molecule/dist/Release32/GNU-Linux-x86/libmolecule.a ../../graph/dist/Release32/GNU-Linux-x86/libgraph.a -lz -lpthread -lstdc++

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release32.mk dist/Release32/GNU-Linux-x86/indigo-deco

dist/Release32/GNU-Linux-x86/indigo-deco: ../../api/dist/ReleaseStatic32/GNU-Linux-x86/libindigo.a

dist/Release32/GNU-Linux-x86/indigo-deco: ../../layout/dist/Release32/GNU-Linux-x86/liblayout.a

dist/Release32/GNU-Linux-x86/indigo-deco: ../../reaction/dist/Release32/GNU-Linux-x86/libreaction.a

dist/Release32/GNU-Linux-x86/indigo-deco: ../../molecule/dist/Release32/GNU-Linux-x86/libmolecule.a

dist/Release32/GNU-Linux-x86/indigo-deco: ../../graph/dist/Release32/GNU-Linux-x86/libgraph.a

dist/Release32/GNU-Linux-x86/indigo-deco: ${OBJECTFILES}
	${MKDIR} -p dist/Release32/GNU-Linux-x86
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/indigo-deco ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/2043651719/os_dir_posix.o: ../../common/base_c/os_dir_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/2043651719
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../../api -I../../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2043651719/os_dir_posix.o ../../common/base_c/os_dir_posix.c

${OBJECTDIR}/main.o: main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../../api -I../../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

# Subprojects
.build-subprojects:
	cd ../../api && ${MAKE}  -f Makefile CONF=ReleaseStatic32
	cd ../../layout && ${MAKE}  -f Makefile CONF=Release32
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Release32
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Release32
	cd ../../graph && ${MAKE}  -f Makefile CONF=Release32

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release32
	${RM} dist/Release32/GNU-Linux-x86/indigo-deco

# Subprojects
.clean-subprojects:
	cd ../../api && ${MAKE}  -f Makefile CONF=ReleaseStatic32 clean
	cd ../../layout && ${MAKE}  -f Makefile CONF=Release32 clean
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Release32 clean
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Release32 clean
	cd ../../graph && ${MAKE}  -f Makefile CONF=Release32 clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
