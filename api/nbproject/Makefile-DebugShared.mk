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
	${OBJECTDIR}/src/indigo_molecule.o \
	${OBJECTDIR}/src/indigo_loaders.o \
	${OBJECTDIR}/src/indigo_properties.o \
	${OBJECTDIR}/src/indigo_io.o \
	${OBJECTDIR}/src/indigo_mapping.o \
	${OBJECTDIR}/src/indigo_array.o \
	${OBJECTDIR}/src/indigo.o \
	${OBJECTDIR}/src/indigo_options.o \
	${OBJECTDIR}/src/indigo_scaffold.o \
	${OBJECTDIR}/src/indigo_match.o \
	${OBJECTDIR}/src/indigo_reaction.o \
	${OBJECTDIR}/src/indigo_product_enumerator.o \
	${OBJECTDIR}/src/indigo_fingerprints.o \
	${OBJECTDIR}/src/option_manager.o \
	${OBJECTDIR}/src/indigo_object.o \
	${OBJECTDIR}/src/indigo_basic.o \
	${OBJECTDIR}/src/indigo_macros.o \
	${OBJECTDIR}/src/indigo_misc.o \
	${OBJECTDIR}/src/indigo_layout.o \
	${OBJECTDIR}/src/indigo_stereo.o \
	${OBJECTDIR}/src/indigo_deconvolution.o


# C Compiler Flags
CFLAGS=-fPIC

# CC Compiler Flags
CCFLAGS=-fPIC
CXXFLAGS=-fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../layout/dist/Debug/GNU-Linux-x86/liblayout.a ../reaction/dist/Debug/GNU-Linux-x86/libreaction.a ../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a ../graph/dist/Debug/GNU-Linux-x86/libgraph.a ../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a -lpthread -lz

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-DebugShared.mk dist/DebugShared/GNU-Linux-x86/libindigo.so

dist/DebugShared/GNU-Linux-x86/libindigo.so: ../layout/dist/Debug/GNU-Linux-x86/liblayout.a

dist/DebugShared/GNU-Linux-x86/libindigo.so: ../reaction/dist/Debug/GNU-Linux-x86/libreaction.a

dist/DebugShared/GNU-Linux-x86/libindigo.so: ../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a

dist/DebugShared/GNU-Linux-x86/libindigo.so: ../graph/dist/Debug/GNU-Linux-x86/libgraph.a

dist/DebugShared/GNU-Linux-x86/libindigo.so: ../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a

dist/DebugShared/GNU-Linux-x86/libindigo.so: ${OBJECTFILES}
	${MKDIR} -p dist/DebugShared/GNU-Linux-x86
	${LINK.cc} -rdynamic -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libindigo.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/indigo_molecule.o: src/indigo_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_molecule.o src/indigo_molecule.cpp

${OBJECTDIR}/src/indigo_loaders.o: src/indigo_loaders.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_loaders.o src/indigo_loaders.cpp

${OBJECTDIR}/src/indigo_properties.o: src/indigo_properties.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_properties.o src/indigo_properties.cpp

${OBJECTDIR}/src/indigo_io.o: src/indigo_io.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_io.o src/indigo_io.cpp

${OBJECTDIR}/src/indigo_mapping.o: src/indigo_mapping.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_mapping.o src/indigo_mapping.cpp

${OBJECTDIR}/src/indigo_array.o: src/indigo_array.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_array.o src/indigo_array.cpp

${OBJECTDIR}/src/indigo.o: src/indigo.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo.o src/indigo.cpp

${OBJECTDIR}/src/indigo_options.o: src/indigo_options.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_options.o src/indigo_options.cpp

${OBJECTDIR}/src/indigo_scaffold.o: src/indigo_scaffold.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_scaffold.o src/indigo_scaffold.cpp

${OBJECTDIR}/src/indigo_match.o: src/indigo_match.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_match.o src/indigo_match.cpp

${OBJECTDIR}/src/indigo_reaction.o: src/indigo_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_reaction.o src/indigo_reaction.cpp

${OBJECTDIR}/src/indigo_product_enumerator.o: src/indigo_product_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_product_enumerator.o src/indigo_product_enumerator.cpp

${OBJECTDIR}/src/indigo_fingerprints.o: src/indigo_fingerprints.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_fingerprints.o src/indigo_fingerprints.cpp

${OBJECTDIR}/src/option_manager.o: src/option_manager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/option_manager.o src/option_manager.cpp

${OBJECTDIR}/src/indigo_object.o: src/indigo_object.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_object.o src/indigo_object.cpp

${OBJECTDIR}/src/indigo_basic.o: src/indigo_basic.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_basic.o src/indigo_basic.cpp

${OBJECTDIR}/src/indigo_macros.o: src/indigo_macros.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_macros.o src/indigo_macros.c

${OBJECTDIR}/src/indigo_misc.o: src/indigo_misc.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_misc.o src/indigo_misc.cpp

${OBJECTDIR}/src/indigo_layout.o: src/indigo_layout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_layout.o src/indigo_layout.cpp

${OBJECTDIR}/src/indigo_stereo.o: src/indigo_stereo.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_stereo.o src/indigo_stereo.cpp

${OBJECTDIR}/src/indigo_deconvolution.o: src/indigo_deconvolution.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I. -Isrc -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/indigo_deconvolution.o src/indigo_deconvolution.cpp

# Subprojects
.build-subprojects:
	cd ../layout && ${MAKE}  -f Makefile CONF=Debug
	cd ../reaction && ${MAKE}  -f Makefile CONF=Debug
	cd ../molecule && ${MAKE}  -f Makefile CONF=Debug
	cd ../graph && ${MAKE}  -f Makefile CONF=Debug
	cd ../tinyxml && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/DebugShared
	${RM} dist/DebugShared/GNU-Linux-x86/libindigo.so

# Subprojects
.clean-subprojects:
	cd ../layout && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../reaction && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../molecule && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../graph && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../tinyxml && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
