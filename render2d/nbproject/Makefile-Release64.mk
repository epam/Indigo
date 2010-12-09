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
	${OBJECTDIR}/src/render_item_fragment.o \
	${OBJECTDIR}/src/render_internal.o \
	${OBJECTDIR}/src/render_item_factory.o \
	${OBJECTDIR}/src/render_grid.o \
	${OBJECTDIR}/src/render_item_hline.o \
	${OBJECTDIR}/src/render_item_aux.o \
	${OBJECTDIR}/src/render_fonts.o \
	${OBJECTDIR}/src/render_item_molecule.o \
	${OBJECTDIR}/src/render_params.o \
	${OBJECTDIR}/src/render.o \
	${OBJECTDIR}/src/render_item_reaction.o \
	${OBJECTDIR}/src/render_context.o \
	${OBJECTDIR}/src/render_common.o \
	${OBJECTDIR}/src/render_item_container.o \
	${OBJECTDIR}/src/render_single.o \
	${OBJECTDIR}/src/render_item.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m64 -fPIC
CXXFLAGS=-m64 -fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release64.mk dist/Release64/GNU-Linux-x86/librender2d.a

dist/Release64/GNU-Linux-x86/librender2d.a: ${OBJECTFILES}
	${MKDIR} -p dist/Release64/GNU-Linux-x86
	${RM} dist/Release64/GNU-Linux-x86/librender2d.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/librender2d.a ${OBJECTFILES} 
	$(RANLIB) dist/Release64/GNU-Linux-x86/librender2d.a

${OBJECTDIR}/src/render_item_fragment.o: src/render_item_fragment.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item_fragment.o src/render_item_fragment.cpp

${OBJECTDIR}/src/render_internal.o: src/render_internal.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_internal.o src/render_internal.cpp

${OBJECTDIR}/src/render_item_factory.o: src/render_item_factory.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item_factory.o src/render_item_factory.cpp

${OBJECTDIR}/src/render_grid.o: src/render_grid.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_grid.o src/render_grid.cpp

${OBJECTDIR}/src/render_item_hline.o: src/render_item_hline.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item_hline.o src/render_item_hline.cpp

${OBJECTDIR}/src/render_item_aux.o: src/render_item_aux.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item_aux.o src/render_item_aux.cpp

${OBJECTDIR}/src/render_fonts.o: src/render_fonts.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_fonts.o src/render_fonts.cpp

${OBJECTDIR}/src/render_item_molecule.o: src/render_item_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item_molecule.o src/render_item_molecule.cpp

${OBJECTDIR}/src/render_params.o: src/render_params.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_params.o src/render_params.cpp

${OBJECTDIR}/src/render.o: src/render.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render.o src/render.cpp

${OBJECTDIR}/src/render_item_reaction.o: src/render_item_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item_reaction.o src/render_item_reaction.cpp

${OBJECTDIR}/src/render_context.o: src/render_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_context.o src/render_context.cpp

${OBJECTDIR}/src/render_common.o: src/render_common.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_common.o src/render_common.cpp

${OBJECTDIR}/src/render_item_container.o: src/render_item_container.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item_container.o src/render_item_container.cpp

${OBJECTDIR}/src/render_single.o: src/render_single.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_single.o src/render_single.cpp

${OBJECTDIR}/src/render_item.o: src/render_item.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I. -I.. -I../common -I/usr/include/cairo -I/usr/include/freetype2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/render_item.o src/render_item.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release64
	${RM} dist/Release64/GNU-Linux-x86/librender2d.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
