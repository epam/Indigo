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
	${OBJECTDIR}/src/render_internal.o \
	${OBJECTDIR}/src/render_base.o \
	${OBJECTDIR}/src/render_fonts.o \
	${OBJECTDIR}/src/render_params.o \
	${OBJECTDIR}/src/render_reaction.o \
	${OBJECTDIR}/src/render_common.o \
	${OBJECTDIR}/src/render_context.o \
	${OBJECTDIR}/src/render_molecule.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/librender2d.a

dist/Debug/GNU-Linux-x86/librender2d.a: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${RM} dist/Debug/GNU-Linux-x86/librender2d.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/librender2d.a ${OBJECTFILES} 
	$(RANLIB) dist/Debug/GNU-Linux-x86/librender2d.a

${OBJECTDIR}/src/render_internal.o: src/render_internal.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_internal.o src/render_internal.cpp

${OBJECTDIR}/src/render_base.o: src/render_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_base.o src/render_base.cpp

${OBJECTDIR}/src/render_fonts.o: src/render_fonts.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_fonts.o src/render_fonts.cpp

${OBJECTDIR}/src/render_params.o: src/render_params.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_params.o src/render_params.cpp

${OBJECTDIR}/src/render_reaction.o: src/render_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_reaction.o src/render_reaction.cpp

${OBJECTDIR}/src/render_common.o: src/render_common.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_common.o src/render_common.cpp

${OBJECTDIR}/src/render_context.o: src/render_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_context.o src/render_context.cpp

${OBJECTDIR}/src/render_molecule.o: src/render_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_molecule.o src/render_molecule.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/librender2d.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
