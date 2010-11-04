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
CND_CONF=ReleaseIntel32
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/reaction_enumerator_state.o \
	${OBJECTDIR}/src/reaction_product_enumerator.o \
	${OBJECTDIR}/src/rxnfile_saver.o \
	${OBJECTDIR}/src/icr_saver.o \
	${OBJECTDIR}/src/base_reaction_substructure_matcher.o \
	${OBJECTDIR}/src/reaction_exact_matcher.o \
	${OBJECTDIR}/src/rsmiles_saver.o \
	${OBJECTDIR}/src/rxnfile_loader.o \
	${OBJECTDIR}/src/crf_saver.o \
	${OBJECTDIR}/src/reaction_fingerprint.o \
	${OBJECTDIR}/src/query_reaction.o \
	${OBJECTDIR}/src/reaction_auto_loader.o \
	${OBJECTDIR}/src/icr_loader.o \
	${OBJECTDIR}/src/reaction_substructure_matcher.o \
	${OBJECTDIR}/src/crf_loader.o \
	${OBJECTDIR}/src/reaction.o \
	${OBJECTDIR}/src/reaction_highlighting.o \
	${OBJECTDIR}/src/base_reaction.o \
	${OBJECTDIR}/src/rsmiles_loader.o \
	${OBJECTDIR}/src/reaction_automapper.o \
	${OBJECTDIR}/src/reaction_neighborhood_counters.o

# C Compiler Flags
CFLAGS=-m32 -fPIC

# CC Compiler Flags
CCFLAGS=-m32 -fPIC
CXXFLAGS=-m32 -fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-ReleaseIntel32.mk dist/ReleaseIntel32/ICC-Linux-x86/libreaction.a

dist/ReleaseIntel32/ICC-Linux-x86/libreaction.a: ${OBJECTFILES}
	${MKDIR} -p dist/ReleaseIntel32/ICC-Linux-x86
	${RM} dist/ReleaseIntel32/ICC-Linux-x86/libreaction.a
	${AR} rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libreaction.a ${OBJECTFILES} 
	$(RANLIB) dist/ReleaseIntel32/ICC-Linux-x86/libreaction.a

${OBJECTDIR}/src/reaction_enumerator_state.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_enumerator_state.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_enumerator_state.o src/reaction_enumerator_state.cpp

${OBJECTDIR}/src/reaction_product_enumerator.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_product_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_product_enumerator.o src/reaction_product_enumerator.cpp

${OBJECTDIR}/src/rxnfile_saver.o: nbproject/Makefile-${CND_CONF}.mk src/rxnfile_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rxnfile_saver.o src/rxnfile_saver.cpp

${OBJECTDIR}/src/icr_saver.o: nbproject/Makefile-${CND_CONF}.mk src/icr_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icr_saver.o src/icr_saver.cpp

${OBJECTDIR}/src/base_reaction_substructure_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/base_reaction_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/base_reaction_substructure_matcher.o src/base_reaction_substructure_matcher.cpp

${OBJECTDIR}/src/reaction_exact_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_exact_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_exact_matcher.o src/reaction_exact_matcher.cpp

${OBJECTDIR}/src/rsmiles_saver.o: nbproject/Makefile-${CND_CONF}.mk src/rsmiles_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rsmiles_saver.o src/rsmiles_saver.cpp

${OBJECTDIR}/src/rxnfile_loader.o: nbproject/Makefile-${CND_CONF}.mk src/rxnfile_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rxnfile_loader.o src/rxnfile_loader.cpp

${OBJECTDIR}/src/crf_saver.o: nbproject/Makefile-${CND_CONF}.mk src/crf_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/crf_saver.o src/crf_saver.cpp

${OBJECTDIR}/src/reaction_fingerprint.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_fingerprint.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_fingerprint.o src/reaction_fingerprint.cpp

${OBJECTDIR}/src/query_reaction.o: nbproject/Makefile-${CND_CONF}.mk src/query_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/query_reaction.o src/query_reaction.cpp

${OBJECTDIR}/src/reaction_auto_loader.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_auto_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_auto_loader.o src/reaction_auto_loader.cpp

${OBJECTDIR}/src/icr_loader.o: nbproject/Makefile-${CND_CONF}.mk src/icr_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icr_loader.o src/icr_loader.cpp

${OBJECTDIR}/src/reaction_substructure_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_substructure_matcher.o src/reaction_substructure_matcher.cpp

${OBJECTDIR}/src/crf_loader.o: nbproject/Makefile-${CND_CONF}.mk src/crf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/crf_loader.o src/crf_loader.cpp

${OBJECTDIR}/src/reaction.o: nbproject/Makefile-${CND_CONF}.mk src/reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction.o src/reaction.cpp

${OBJECTDIR}/src/reaction_highlighting.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_highlighting.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_highlighting.o src/reaction_highlighting.cpp

${OBJECTDIR}/src/base_reaction.o: nbproject/Makefile-${CND_CONF}.mk src/base_reaction.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/base_reaction.o src/base_reaction.cpp

${OBJECTDIR}/src/rsmiles_loader.o: nbproject/Makefile-${CND_CONF}.mk src/rsmiles_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rsmiles_loader.o src/rsmiles_loader.cpp

${OBJECTDIR}/src/reaction_automapper.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_automapper.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_automapper.o src/reaction_automapper.cpp

${OBJECTDIR}/src/reaction_neighborhood_counters.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_neighborhood_counters.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_neighborhood_counters.o src/reaction_neighborhood_counters.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ReleaseIntel32
	${RM} dist/ReleaseIntel32/ICC-Linux-x86/libreaction.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
