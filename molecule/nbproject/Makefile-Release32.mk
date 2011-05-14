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
	${OBJECTDIR}/src/molecule_dearom.o \
	${OBJECTDIR}/src/molecule_cml_loader.o \
	${OBJECTDIR}/src/molecule_arom_match.o \
	${OBJECTDIR}/_ext/1704922415/gzip_scanner.o \
	${OBJECTDIR}/_ext/380065930/gray_codes.o \
	${OBJECTDIR}/src/molecule_substructure_matcher.o \
	${OBJECTDIR}/src/elements.o \
	${OBJECTDIR}/src/molecule_3d_constraints.o \
	${OBJECTDIR}/src/molecule_tautomer_matcher.o \
	${OBJECTDIR}/src/gross_formula.o \
	${OBJECTDIR}/src/canonical_smiles_saver.o \
	${OBJECTDIR}/src/max_common_submolecule.o \
	${OBJECTDIR}/src/molecule_auto_loader.o \
	${OBJECTDIR}/src/molecule_mass.o \
	${OBJECTDIR}/src/molecule_scaffold_detection.o \
	${OBJECTDIR}/src/smiles_loader.o \
	${OBJECTDIR}/src/molecule_inchi_component.o \
	${OBJECTDIR}/src/base_molecule.o \
	${OBJECTDIR}/src/molecule_pi_systems_matcher.o \
	${OBJECTDIR}/src/molfile_loader.o \
	${OBJECTDIR}/src/molecule_electrons_localizer.o \
	${OBJECTDIR}/src/molecule_cml_saver.o \
	${OBJECTDIR}/_ext/380065930/bitinworker.o \
	${OBJECTDIR}/src/molecule_inchi_layers.o \
	${OBJECTDIR}/_ext/637734348/lzw_decoder.o \
	${OBJECTDIR}/_ext/380065930/bitoutworker.o \
	${OBJECTDIR}/src/query_molecule.o \
	${OBJECTDIR}/src/smiles_saver.o \
	${OBJECTDIR}/_ext/380065930/locale_guard.o \
	${OBJECTDIR}/src/molecule_rgroups.o \
	${OBJECTDIR}/src/molecule_automorphism_search.o \
	${OBJECTDIR}/src/molecule_tautomer_utils.o \
	${OBJECTDIR}/_ext/1704922415/gzip_output.o \
	${OBJECTDIR}/src/molecule.o \
	${OBJECTDIR}/_ext/1881957642/bitarray.o \
	${OBJECTDIR}/src/molecule_tautomer_match.o \
	${OBJECTDIR}/src/molecule_exact_matcher.o \
	${OBJECTDIR}/src/rdf_loader.o \
	${OBJECTDIR}/src/molecule_arom.o \
	${OBJECTDIR}/src/icm_loader.o \
	${OBJECTDIR}/src/molecule_neighbourhood_counters.o \
	${OBJECTDIR}/src/molecule_tautomer_chain.o \
	${OBJECTDIR}/_ext/637734348/lzw_encoder.o \
	${OBJECTDIR}/src/sdf_loader.o \
	${OBJECTDIR}/_ext/637734348/lzw_dictionary.o \
	${OBJECTDIR}/src/molecule_stereocenters.o \
	${OBJECTDIR}/src/molecule_chain_fingerprints.o \
	${OBJECTDIR}/src/molecule_inchi_utils.o \
	${OBJECTDIR}/src/molecule_exact_substructure_matcher.o \
	${OBJECTDIR}/_ext/380065930/scanner.o \
	${OBJECTDIR}/src/molecule_inchi.o \
	${OBJECTDIR}/src/cmf_loader.o \
	${OBJECTDIR}/src/molecule_fingerprint.o \
	${OBJECTDIR}/src/cmf_saver.o \
	${OBJECTDIR}/src/molecule_cis_trans.o \
	${OBJECTDIR}/src/molecule_tautomer_superstructure.o \
	${OBJECTDIR}/src/molfile_saver.o \
	${OBJECTDIR}/src/multiple_cml_loader.o \
	${OBJECTDIR}/src/icm_saver.o


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
	"${MAKE}"  -f nbproject/Makefile-Release32.mk dist/Release32/GNU-Linux-x86/libmolecule.a

dist/Release32/GNU-Linux-x86/libmolecule.a: ${OBJECTFILES}
	${MKDIR} -p dist/Release32/GNU-Linux-x86
	${RM} dist/Release32/GNU-Linux-x86/libmolecule.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libmolecule.a ${OBJECTFILES} 
	$(RANLIB) dist/Release32/GNU-Linux-x86/libmolecule.a

${OBJECTDIR}/src/molecule_dearom.o: src/molecule_dearom.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_dearom.o src/molecule_dearom.cpp

${OBJECTDIR}/src/molecule_cml_loader.o: src/molecule_cml_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_cml_loader.o src/molecule_cml_loader.cpp

${OBJECTDIR}/src/molecule_arom_match.o: src/molecule_arom_match.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_arom_match.o src/molecule_arom_match.cpp

${OBJECTDIR}/_ext/1704922415/gzip_scanner.o: ../common/gzip/gzip_scanner.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1704922415
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1704922415/gzip_scanner.o ../common/gzip/gzip_scanner.cpp

${OBJECTDIR}/_ext/380065930/gray_codes.o: ../common/base_cpp/gray_codes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/gray_codes.o ../common/base_cpp/gray_codes.cpp

${OBJECTDIR}/src/molecule_substructure_matcher.o: src/molecule_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_substructure_matcher.o src/molecule_substructure_matcher.cpp

${OBJECTDIR}/src/elements.o: src/elements.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/elements.o src/elements.cpp

${OBJECTDIR}/src/molecule_3d_constraints.o: src/molecule_3d_constraints.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_3d_constraints.o src/molecule_3d_constraints.cpp

${OBJECTDIR}/src/molecule_tautomer_matcher.o: src/molecule_tautomer_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_matcher.o src/molecule_tautomer_matcher.cpp

${OBJECTDIR}/src/gross_formula.o: src/gross_formula.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/gross_formula.o src/gross_formula.cpp

${OBJECTDIR}/src/canonical_smiles_saver.o: src/canonical_smiles_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/canonical_smiles_saver.o src/canonical_smiles_saver.cpp

${OBJECTDIR}/src/max_common_submolecule.o: src/max_common_submolecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/max_common_submolecule.o src/max_common_submolecule.cpp

${OBJECTDIR}/src/molecule_auto_loader.o: src/molecule_auto_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_auto_loader.o src/molecule_auto_loader.cpp

${OBJECTDIR}/src/molecule_mass.o: src/molecule_mass.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_mass.o src/molecule_mass.cpp

${OBJECTDIR}/src/molecule_scaffold_detection.o: src/molecule_scaffold_detection.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_scaffold_detection.o src/molecule_scaffold_detection.cpp

${OBJECTDIR}/src/smiles_loader.o: src/smiles_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/smiles_loader.o src/smiles_loader.cpp

${OBJECTDIR}/src/molecule_inchi_component.o: src/molecule_inchi_component.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi_component.o src/molecule_inchi_component.cpp

${OBJECTDIR}/src/base_molecule.o: src/base_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/base_molecule.o src/base_molecule.cpp

${OBJECTDIR}/src/molecule_pi_systems_matcher.o: src/molecule_pi_systems_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_pi_systems_matcher.o src/molecule_pi_systems_matcher.cpp

${OBJECTDIR}/src/molfile_loader.o: src/molfile_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molfile_loader.o src/molfile_loader.cpp

${OBJECTDIR}/src/molecule_electrons_localizer.o: src/molecule_electrons_localizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_electrons_localizer.o src/molecule_electrons_localizer.cpp

${OBJECTDIR}/src/molecule_cml_saver.o: src/molecule_cml_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_cml_saver.o src/molecule_cml_saver.cpp

${OBJECTDIR}/_ext/380065930/bitinworker.o: ../common/base_cpp/bitinworker.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/bitinworker.o ../common/base_cpp/bitinworker.cpp

${OBJECTDIR}/src/molecule_inchi_layers.o: src/molecule_inchi_layers.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi_layers.o src/molecule_inchi_layers.cpp

${OBJECTDIR}/_ext/637734348/lzw_decoder.o: ../common/lzw/lzw_decoder.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/637734348
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/637734348/lzw_decoder.o ../common/lzw/lzw_decoder.cpp

${OBJECTDIR}/_ext/380065930/bitoutworker.o: ../common/base_cpp/bitoutworker.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/bitoutworker.o ../common/base_cpp/bitoutworker.cpp

${OBJECTDIR}/src/query_molecule.o: src/query_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/query_molecule.o src/query_molecule.cpp

${OBJECTDIR}/src/smiles_saver.o: src/smiles_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/smiles_saver.o src/smiles_saver.cpp

${OBJECTDIR}/_ext/380065930/locale_guard.o: ../common/base_cpp/locale_guard.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/locale_guard.o ../common/base_cpp/locale_guard.cpp

${OBJECTDIR}/src/molecule_rgroups.o: src/molecule_rgroups.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_rgroups.o src/molecule_rgroups.cpp

${OBJECTDIR}/src/molecule_automorphism_search.o: src/molecule_automorphism_search.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_automorphism_search.o src/molecule_automorphism_search.cpp

${OBJECTDIR}/src/molecule_tautomer_utils.o: src/molecule_tautomer_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_utils.o src/molecule_tautomer_utils.cpp

${OBJECTDIR}/_ext/1704922415/gzip_output.o: ../common/gzip/gzip_output.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1704922415
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1704922415/gzip_output.o ../common/gzip/gzip_output.cpp

${OBJECTDIR}/src/molecule.o: src/molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule.o src/molecule.cpp

${OBJECTDIR}/_ext/1881957642/bitarray.o: ../common/base_c/bitarray.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1881957642
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1881957642/bitarray.o ../common/base_c/bitarray.c

${OBJECTDIR}/src/molecule_tautomer_match.o: src/molecule_tautomer_match.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_match.o src/molecule_tautomer_match.cpp

${OBJECTDIR}/src/molecule_exact_matcher.o: src/molecule_exact_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_exact_matcher.o src/molecule_exact_matcher.cpp

${OBJECTDIR}/src/rdf_loader.o: src/rdf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rdf_loader.o src/rdf_loader.cpp

${OBJECTDIR}/src/molecule_arom.o: src/molecule_arom.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_arom.o src/molecule_arom.cpp

${OBJECTDIR}/src/icm_loader.o: src/icm_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icm_loader.o src/icm_loader.cpp

${OBJECTDIR}/src/molecule_neighbourhood_counters.o: src/molecule_neighbourhood_counters.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_neighbourhood_counters.o src/molecule_neighbourhood_counters.cpp

${OBJECTDIR}/src/molecule_tautomer_chain.o: src/molecule_tautomer_chain.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_chain.o src/molecule_tautomer_chain.cpp

${OBJECTDIR}/_ext/637734348/lzw_encoder.o: ../common/lzw/lzw_encoder.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/637734348
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/637734348/lzw_encoder.o ../common/lzw/lzw_encoder.cpp

${OBJECTDIR}/src/sdf_loader.o: src/sdf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sdf_loader.o src/sdf_loader.cpp

${OBJECTDIR}/_ext/637734348/lzw_dictionary.o: ../common/lzw/lzw_dictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/637734348
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/637734348/lzw_dictionary.o ../common/lzw/lzw_dictionary.cpp

${OBJECTDIR}/src/molecule_stereocenters.o: src/molecule_stereocenters.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_stereocenters.o src/molecule_stereocenters.cpp

${OBJECTDIR}/src/molecule_chain_fingerprints.o: src/molecule_chain_fingerprints.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_chain_fingerprints.o src/molecule_chain_fingerprints.cpp

${OBJECTDIR}/src/molecule_inchi_utils.o: src/molecule_inchi_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi_utils.o src/molecule_inchi_utils.cpp

${OBJECTDIR}/src/molecule_exact_substructure_matcher.o: src/molecule_exact_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_exact_substructure_matcher.o src/molecule_exact_substructure_matcher.cpp

${OBJECTDIR}/_ext/380065930/scanner.o: ../common/base_cpp/scanner.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/scanner.o ../common/base_cpp/scanner.cpp

${OBJECTDIR}/src/molecule_inchi.o: src/molecule_inchi.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi.o src/molecule_inchi.cpp

${OBJECTDIR}/src/cmf_loader.o: src/cmf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cmf_loader.o src/cmf_loader.cpp

${OBJECTDIR}/src/molecule_fingerprint.o: src/molecule_fingerprint.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_fingerprint.o src/molecule_fingerprint.cpp

${OBJECTDIR}/src/cmf_saver.o: src/cmf_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cmf_saver.o src/cmf_saver.cpp

${OBJECTDIR}/src/molecule_cis_trans.o: src/molecule_cis_trans.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_cis_trans.o src/molecule_cis_trans.cpp

${OBJECTDIR}/src/molecule_tautomer_superstructure.o: src/molecule_tautomer_superstructure.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_superstructure.o src/molecule_tautomer_superstructure.cpp

${OBJECTDIR}/src/molfile_saver.o: src/molfile_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molfile_saver.o src/molfile_saver.cpp

${OBJECTDIR}/src/multiple_cml_loader.o: src/multiple_cml_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/multiple_cml_loader.o src/multiple_cml_loader.cpp

${OBJECTDIR}/src/icm_saver.o: src/icm_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -I../tinyxml/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icm_saver.o src/icm_saver.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release32
	${RM} dist/Release32/GNU-Linux-x86/libmolecule.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
