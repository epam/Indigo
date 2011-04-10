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
	${OBJECTDIR}/src/rxnfile_loader.o \
	${OBJECTDIR}/src/reaction_exact_matcher.o \
	${OBJECTDIR}/src/base_reaction.o \
	${OBJECTDIR}/src/query_reaction.o \
	${OBJECTDIR}/src/rsmiles_loader.o \
	${OBJECTDIR}/src/reaction_enumerator_state.o \
	${OBJECTDIR}/src/reaction_auto_loader.o \
	${OBJECTDIR}/src/reaction.o \
	${OBJECTDIR}/src/reaction_neighborhood_counters.o \
	${OBJECTDIR}/src/rsmiles_saver.o \
	${OBJECTDIR}/src/crf_loader.o \
	${OBJECTDIR}/src/reaction_automapper.o \
	${OBJECTDIR}/src/icr_loader.o \
	${OBJECTDIR}/src/rxnfile_saver.o \
	${OBJECTDIR}/src/icr_saver.o \
	${OBJECTDIR}/src/reaction_fingerprint.o \
	${OBJECTDIR}/src/reaction_product_enumerator.o \
	${OBJECTDIR}/src/reaction_substructure_matcher.o \
	${OBJECTDIR}/src/crf_saver.o \
	${OBJECTDIR}/src/base_reaction_substructure_matcher.o \
	${OBJECTDIR}/src/reaction_cml_saver.o \
	${OBJECTDIR}/src/reaction_cml_loader.o


# C Compiler Flags
CFLAGS=-m64 -fPIC

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
	"${MAKE}"  -f nbproject/Makefile-Release64.mk dist/Release64/GNU-Linux-x86/libreaction.a

dist/Release64/GNU-Linux-x86/libreaction.a: ${OBJECTFILES}
	${MKDIR} -p dist/Release64/GNU-Linux-x86
	${RM} dist/Release64/GNU-Linux-x86/libreaction.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libreaction.a ${OBJECTFILES} 
	$(RANLIB) dist/Release64/GNU-Linux-x86/libreaction.a

${OBJECTDIR}/src/rxnfile_loader.o: src/rxnfile_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rxnfile_loader.o src/rxnfile_loader.cpp

${OBJECTDIR}/src/reaction_exact_matcher.o: src/reaction_exact_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_exact_matcher.o src/reaction_exact_matcher.cpp

${OBJECTDIR}/src/base_reaction.o: src/base_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/base_reaction.o src/base_reaction.cpp

${OBJECTDIR}/src/query_reaction.o: src/query_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/query_reaction.o src/query_reaction.cpp

${OBJECTDIR}/src/rsmiles_loader.o: src/rsmiles_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rsmiles_loader.o src/rsmiles_loader.cpp

${OBJECTDIR}/src/reaction_enumerator_state.o: src/reaction_enumerator_state.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_enumerator_state.o src/reaction_enumerator_state.cpp

${OBJECTDIR}/src/reaction_auto_loader.o: src/reaction_auto_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_auto_loader.o src/reaction_auto_loader.cpp

${OBJECTDIR}/src/reaction.o: src/reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction.o src/reaction.cpp

${OBJECTDIR}/src/reaction_neighborhood_counters.o: src/reaction_neighborhood_counters.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_neighborhood_counters.o src/reaction_neighborhood_counters.cpp

${OBJECTDIR}/src/rsmiles_saver.o: src/rsmiles_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rsmiles_saver.o src/rsmiles_saver.cpp

${OBJECTDIR}/src/crf_loader.o: src/crf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/crf_loader.o src/crf_loader.cpp

${OBJECTDIR}/src/reaction_automapper.o: src/reaction_automapper.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_automapper.o src/reaction_automapper.cpp

${OBJECTDIR}/src/icr_loader.o: src/icr_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icr_loader.o src/icr_loader.cpp

${OBJECTDIR}/src/rxnfile_saver.o: src/rxnfile_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rxnfile_saver.o src/rxnfile_saver.cpp

${OBJECTDIR}/src/icr_saver.o: src/icr_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icr_saver.o src/icr_saver.cpp

${OBJECTDIR}/src/reaction_fingerprint.o: src/reaction_fingerprint.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_fingerprint.o src/reaction_fingerprint.cpp

${OBJECTDIR}/src/reaction_product_enumerator.o: src/reaction_product_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_product_enumerator.o src/reaction_product_enumerator.cpp

${OBJECTDIR}/src/reaction_substructure_matcher.o: src/reaction_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_substructure_matcher.o src/reaction_substructure_matcher.cpp

${OBJECTDIR}/src/crf_saver.o: src/crf_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/crf_saver.o src/crf_saver.cpp

${OBJECTDIR}/src/base_reaction_substructure_matcher.o: src/base_reaction_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/base_reaction_substructure_matcher.o src/base_reaction_substructure_matcher.cpp

${OBJECTDIR}/src/reaction_cml_saver.o: src/reaction_cml_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_cml_saver.o src/reaction_cml_saver.cpp

${OBJECTDIR}/src/reaction_cml_loader.o: src/reaction_cml_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_cml_loader.o src/reaction_cml_loader.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release64
	${RM} dist/Release64/GNU-Linux-x86/libreaction.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
