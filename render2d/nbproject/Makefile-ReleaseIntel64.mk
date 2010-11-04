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
CCADMIN=CCadmin
RANLIB=ranlib
CC=icc
CCC=icc
CXX=icc
FC=
AS=

# Macros
CND_PLATFORM=ICC-Linux-x86
CND_CONF=ReleaseIntel64
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/render_context.o \
	${OBJECTDIR}/src/render_internal.o \
	${OBJECTDIR}/src/render_reaction.o \
	${OBJECTDIR}/src/render_molecule.o \
	${OBJECTDIR}/src/render_fonts.o \
	${OBJECTDIR}/src/render_params.o \
	${OBJECTDIR}/src/render_common.o \
	${OBJECTDIR}/src/render_base.o

# C Compiler Flags
CFLAGS=-fPIC

# CC Compiler Flags
CCFLAGS=-m64
CXXFLAGS=-m64

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-ReleaseIntel64.mk dist/ReleaseIntel64/ICC-Linux-x86/librender2d.a

dist/ReleaseIntel64/ICC-Linux-x86/librender2d.a: ${OBJECTFILES}
	${MKDIR} -p dist/ReleaseIntel64/ICC-Linux-x86
	${RM} dist/ReleaseIntel64/ICC-Linux-x86/librender2d.a
	${AR} rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/librender2d.a ${OBJECTFILES} 
	$(RANLIB) dist/ReleaseIntel64/ICC-Linux-x86/librender2d.a

${OBJECTDIR}/src/render_context.o: nbproject/Makefile-${CND_CONF}.mk src/render_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_context.o src/render_context.cpp

${OBJECTDIR}/src/render_internal.o: nbproject/Makefile-${CND_CONF}.mk src/render_internal.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_internal.o src/render_internal.cpp

${OBJECTDIR}/src/render_reaction.o: nbproject/Makefile-${CND_CONF}.mk src/render_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_reaction.o src/render_reaction.cpp

${OBJECTDIR}/src/render_molecule.o: nbproject/Makefile-${CND_CONF}.mk src/render_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_molecule.o src/render_molecule.cpp

${OBJECTDIR}/src/render_fonts.o: nbproject/Makefile-${CND_CONF}.mk src/render_fonts.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_fonts.o src/render_fonts.cpp

${OBJECTDIR}/src/render_params.o: nbproject/Makefile-${CND_CONF}.mk src/render_params.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_params.o src/render_params.cpp

${OBJECTDIR}/src/render_common.o: nbproject/Makefile-${CND_CONF}.mk src/render_common.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_common.o src/render_common.cpp

${OBJECTDIR}/src/render_base.o: nbproject/Makefile-${CND_CONF}.mk src/render_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_base.o src/render_base.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ReleaseIntel64
	${RM} dist/ReleaseIntel64/ICC-Linux-x86/librender2d.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
