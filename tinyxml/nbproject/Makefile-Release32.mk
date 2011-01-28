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
	${OBJECTDIR}/src/tinystr.o \
	${OBJECTDIR}/src/tinyxmlerror.o \
	${OBJECTDIR}/src/tinyxml.o \
	${OBJECTDIR}/src/tinyxmlparser.o


# C Compiler Flags
CFLAGS=

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
	"${MAKE}"  -f nbproject/Makefile-Release32.mk dist/Release32/GNU-Linux-x86/libtinyxml.a

dist/Release32/GNU-Linux-x86/libtinyxml.a: ${OBJECTFILES}
	${MKDIR} -p dist/Release32/GNU-Linux-x86
	${RM} dist/Release32/GNU-Linux-x86/libtinyxml.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libtinyxml.a ${OBJECTFILES} 
	$(RANLIB) dist/Release32/GNU-Linux-x86/libtinyxml.a

${OBJECTDIR}/src/tinystr.o: src/tinystr.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/tinystr.o src/tinystr.cpp

${OBJECTDIR}/src/tinyxmlerror.o: src/tinyxmlerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/tinyxmlerror.o src/tinyxmlerror.cpp

${OBJECTDIR}/src/tinyxml.o: src/tinyxml.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/tinyxml.o src/tinyxml.cpp

${OBJECTDIR}/src/tinyxmlparser.o: src/tinyxmlparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/tinyxmlparser.o src/tinyxmlparser.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release32
	${RM} dist/Release32/GNU-Linux-x86/libtinyxml.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
