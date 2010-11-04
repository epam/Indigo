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
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/gray_codes.o \
	${OBJECTDIR}/src/molecule_3d_constraints.o \
	${OBJECTDIR}/src/max_common_submolecule.o \
	${OBJECTDIR}/src/molecule_electrons_localizer.o \
	${OBJECTDIR}/src/molecule.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/output.o \
	${OBJECTDIR}/src/molecule_fingerprint.o \
	${OBJECTDIR}/src/molecule_tautomer_chain.o \
	${OBJECTDIR}/src/molecule_exact_matcher.o \
	${OBJECTDIR}/src/molecule_decomposer.o \
	${OBJECTDIR}/src/molecule_chain_fingerprints.o \
	${OBJECTDIR}/src/cmf_saver.o \
	${OBJECTDIR}/src/gross_formula.o \
	${OBJECTDIR}/src/molecule_substructure_matcher.o \
	${OBJECTDIR}/src/molfile_loader.o \
	${OBJECTDIR}/src/molecule_tautomer_utils.o \
	${OBJECTDIR}/src/smiles_saver.o \
	${OBJECTDIR}/src/query_molecule.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_dictionary.o \
	${OBJECTDIR}/src/molecule_inchi_utils.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/vec2f.o \
	${OBJECTDIR}/src/molecule_mass.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/lseg3f.o \
	${OBJECTDIR}/src/molecule_auto_loader.o \
	${OBJECTDIR}/src/molecule_rgroups.o \
	${OBJECTDIR}/src/smiles_loader.o \
	${OBJECTDIR}/src/molecule_scaffold_detection.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/line3f.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/gzip/gzip_output.o \
	${OBJECTDIR}/src/molecule_dearom.o \
	${OBJECTDIR}/src/canonical_smiles_saver.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_encoder.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/plane3f.o \
	${OBJECTDIR}/src/cmf_loader.o \
	${OBJECTDIR}/src/molecule_stereocenters.o \
	${OBJECTDIR}/src/molecule_inchi_layers.o \
	${OBJECTDIR}/src/molecule_tautomer_match.o \
	${OBJECTDIR}/src/rdf_loader.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/transform3f.o \
	${OBJECTDIR}/src/icm_loader.o \
	${OBJECTDIR}/src/elements.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/scanner.o \
	${OBJECTDIR}/src/molecule_cis_trans.o \
	${OBJECTDIR}/src/molecule_tautomer_superstructure.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_decoder.o \
	${OBJECTDIR}/src/molecule_arom_match.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/vec3f.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/gzip/gzip_scanner.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/bitoutworker.o \
	${OBJECTDIR}/src/base_molecule.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/string_pool.o \
	${OBJECTDIR}/src/molecule_cml_saver.o \
	${OBJECTDIR}/src/molecule_neighbourhood_counters.o \
	${OBJECTDIR}/src/molfile_saver.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_c/bitarray.o \
	${OBJECTDIR}/src/molecule_exact_substructure_matcher.o \
	${OBJECTDIR}/src/molecule_inchi_component.o \
	${OBJECTDIR}/src/molecule_inchi.o \
	${OBJECTDIR}/src/molecule_tautomer_matcher.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/matr3x3d.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/bitinworker.o \
	${OBJECTDIR}/src/molecule_pi_systems_matcher.o \
	${OBJECTDIR}/src/molecule_arom.o \
	${OBJECTDIR}/src/sdf_loader.o \
	${OBJECTDIR}/src/molecule_automorphism_search.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/math/best_fit.o \
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
	${MAKE}  -f nbproject/Makefile-ReleaseIntel32.mk dist/ReleaseIntel32/ICC-Linux-x86/libmolecule.a

dist/ReleaseIntel32/ICC-Linux-x86/libmolecule.a: ${OBJECTFILES}
	${MKDIR} -p dist/ReleaseIntel32/ICC-Linux-x86
	${RM} dist/ReleaseIntel32/ICC-Linux-x86/libmolecule.a
	${AR} rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libmolecule.a ${OBJECTFILES} 
	$(RANLIB) dist/ReleaseIntel32/ICC-Linux-x86/libmolecule.a

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/gray_codes.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/gray_codes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/gray_codes.o ../common/base_cpp/gray_codes.cpp

${OBJECTDIR}/src/molecule_3d_constraints.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_3d_constraints.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_3d_constraints.o src/molecule_3d_constraints.cpp

${OBJECTDIR}/src/max_common_submolecule.o: nbproject/Makefile-${CND_CONF}.mk src/max_common_submolecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/max_common_submolecule.o src/max_common_submolecule.cpp

${OBJECTDIR}/src/molecule_electrons_localizer.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_electrons_localizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_electrons_localizer.o src/molecule_electrons_localizer.cpp

${OBJECTDIR}/src/molecule.o: nbproject/Makefile-${CND_CONF}.mk src/molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule.o src/molecule.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/output.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/output.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/output.o ../common/base_cpp/output.cpp

${OBJECTDIR}/src/molecule_fingerprint.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_fingerprint.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_fingerprint.o src/molecule_fingerprint.cpp

${OBJECTDIR}/src/molecule_tautomer_chain.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_tautomer_chain.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_chain.o src/molecule_tautomer_chain.cpp

${OBJECTDIR}/src/molecule_exact_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_exact_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_exact_matcher.o src/molecule_exact_matcher.cpp

${OBJECTDIR}/src/molecule_decomposer.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_decomposer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_decomposer.o src/molecule_decomposer.cpp

${OBJECTDIR}/src/molecule_chain_fingerprints.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_chain_fingerprints.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_chain_fingerprints.o src/molecule_chain_fingerprints.cpp

${OBJECTDIR}/src/cmf_saver.o: nbproject/Makefile-${CND_CONF}.mk src/cmf_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cmf_saver.o src/cmf_saver.cpp

${OBJECTDIR}/src/gross_formula.o: nbproject/Makefile-${CND_CONF}.mk src/gross_formula.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/gross_formula.o src/gross_formula.cpp

${OBJECTDIR}/src/molecule_substructure_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_substructure_matcher.o src/molecule_substructure_matcher.cpp

${OBJECTDIR}/src/molfile_loader.o: nbproject/Makefile-${CND_CONF}.mk src/molfile_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molfile_loader.o src/molfile_loader.cpp

${OBJECTDIR}/src/molecule_tautomer_utils.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_tautomer_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_utils.o src/molecule_tautomer_utils.cpp

${OBJECTDIR}/src/smiles_saver.o: nbproject/Makefile-${CND_CONF}.mk src/smiles_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/smiles_saver.o src/smiles_saver.cpp

${OBJECTDIR}/src/query_molecule.o: nbproject/Makefile-${CND_CONF}.mk src/query_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/query_molecule.o src/query_molecule.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_dictionary.o: nbproject/Makefile-${CND_CONF}.mk ../common/lzw/lzw_dictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/lzw
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_dictionary.o ../common/lzw/lzw_dictionary.cpp

${OBJECTDIR}/src/molecule_inchi_utils.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_inchi_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi_utils.o src/molecule_inchi_utils.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/vec2f.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/vec2f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/vec2f.o ../common/math/vec2f.cpp

${OBJECTDIR}/src/molecule_mass.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_mass.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_mass.o src/molecule_mass.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/lseg3f.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/lseg3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/lseg3f.o ../common/math/lseg3f.cpp

${OBJECTDIR}/src/molecule_auto_loader.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_auto_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_auto_loader.o src/molecule_auto_loader.cpp

${OBJECTDIR}/src/molecule_rgroups.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_rgroups.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_rgroups.o src/molecule_rgroups.cpp

${OBJECTDIR}/src/smiles_loader.o: nbproject/Makefile-${CND_CONF}.mk src/smiles_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/smiles_loader.o src/smiles_loader.cpp

${OBJECTDIR}/src/molecule_scaffold_detection.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_scaffold_detection.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_scaffold_detection.o src/molecule_scaffold_detection.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/line3f.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/line3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/line3f.o ../common/math/line3f.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/gzip/gzip_output.o: nbproject/Makefile-${CND_CONF}.mk ../common/gzip/gzip_output.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/gzip
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/gzip/gzip_output.o ../common/gzip/gzip_output.cpp

${OBJECTDIR}/src/molecule_dearom.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_dearom.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_dearom.o src/molecule_dearom.cpp

${OBJECTDIR}/src/canonical_smiles_saver.o: nbproject/Makefile-${CND_CONF}.mk src/canonical_smiles_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/canonical_smiles_saver.o src/canonical_smiles_saver.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_encoder.o: nbproject/Makefile-${CND_CONF}.mk ../common/lzw/lzw_encoder.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/lzw
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_encoder.o ../common/lzw/lzw_encoder.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/plane3f.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/plane3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/plane3f.o ../common/math/plane3f.cpp

${OBJECTDIR}/src/cmf_loader.o: nbproject/Makefile-${CND_CONF}.mk src/cmf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cmf_loader.o src/cmf_loader.cpp

${OBJECTDIR}/src/molecule_stereocenters.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_stereocenters.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_stereocenters.o src/molecule_stereocenters.cpp

${OBJECTDIR}/src/molecule_inchi_layers.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_inchi_layers.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi_layers.o src/molecule_inchi_layers.cpp

${OBJECTDIR}/src/molecule_tautomer_match.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_tautomer_match.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_match.o src/molecule_tautomer_match.cpp

${OBJECTDIR}/src/rdf_loader.o: nbproject/Makefile-${CND_CONF}.mk src/rdf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/rdf_loader.o src/rdf_loader.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/transform3f.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/transform3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/transform3f.o ../common/math/transform3f.cpp

${OBJECTDIR}/src/icm_loader.o: nbproject/Makefile-${CND_CONF}.mk src/icm_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icm_loader.o src/icm_loader.cpp

${OBJECTDIR}/src/elements.o: nbproject/Makefile-${CND_CONF}.mk src/elements.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/elements.o src/elements.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/scanner.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/scanner.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/scanner.o ../common/base_cpp/scanner.cpp

${OBJECTDIR}/src/molecule_cis_trans.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_cis_trans.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_cis_trans.o src/molecule_cis_trans.cpp

${OBJECTDIR}/src/molecule_tautomer_superstructure.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_tautomer_superstructure.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_superstructure.o src/molecule_tautomer_superstructure.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_decoder.o: nbproject/Makefile-${CND_CONF}.mk ../common/lzw/lzw_decoder.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/lzw
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/lzw/lzw_decoder.o ../common/lzw/lzw_decoder.cpp

${OBJECTDIR}/src/molecule_arom_match.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_arom_match.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_arom_match.o src/molecule_arom_match.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/vec3f.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/vec3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/vec3f.o ../common/math/vec3f.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/gzip/gzip_scanner.o: nbproject/Makefile-${CND_CONF}.mk ../common/gzip/gzip_scanner.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/gzip
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/gzip/gzip_scanner.o ../common/gzip/gzip_scanner.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/bitoutworker.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/bitoutworker.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/bitoutworker.o ../common/base_cpp/bitoutworker.cpp

${OBJECTDIR}/src/base_molecule.o: nbproject/Makefile-${CND_CONF}.mk src/base_molecule.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/base_molecule.o src/base_molecule.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/string_pool.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/string_pool.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/string_pool.o ../common/base_cpp/string_pool.cpp

${OBJECTDIR}/src/molecule_cml_saver.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_cml_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_cml_saver.o src/molecule_cml_saver.cpp

${OBJECTDIR}/src/molecule_neighbourhood_counters.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_neighbourhood_counters.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_neighbourhood_counters.o src/molecule_neighbourhood_counters.cpp

${OBJECTDIR}/src/molfile_saver.o: nbproject/Makefile-${CND_CONF}.mk src/molfile_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molfile_saver.o src/molfile_saver.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_c/bitarray.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_c/bitarray.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_c
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_c/bitarray.o ../common/base_c/bitarray.c

${OBJECTDIR}/src/molecule_exact_substructure_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_exact_substructure_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_exact_substructure_matcher.o src/molecule_exact_substructure_matcher.cpp

${OBJECTDIR}/src/molecule_inchi_component.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_inchi_component.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi_component.o src/molecule_inchi_component.cpp

${OBJECTDIR}/src/molecule_inchi.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_inchi.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_inchi.o src/molecule_inchi.cpp

${OBJECTDIR}/src/molecule_tautomer_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_tautomer_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_tautomer_matcher.o src/molecule_tautomer_matcher.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/matr3x3d.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/matr3x3d.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/matr3x3d.o ../common/math/matr3x3d.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/bitinworker.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/bitinworker.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/bitinworker.o ../common/base_cpp/bitinworker.cpp

${OBJECTDIR}/src/molecule_pi_systems_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_pi_systems_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_pi_systems_matcher.o src/molecule_pi_systems_matcher.cpp

${OBJECTDIR}/src/molecule_arom.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_arom.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_arom.o src/molecule_arom.cpp

${OBJECTDIR}/src/sdf_loader.o: nbproject/Makefile-${CND_CONF}.mk src/sdf_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sdf_loader.o src/sdf_loader.cpp

${OBJECTDIR}/src/molecule_automorphism_search.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_automorphism_search.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_automorphism_search.o src/molecule_automorphism_search.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/math/best_fit.o: nbproject/Makefile-${CND_CONF}.mk ../common/math/best_fit.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/math
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/math/best_fit.o ../common/math/best_fit.cpp

${OBJECTDIR}/src/icm_saver.o: nbproject/Makefile-${CND_CONF}.mk src/icm_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/icm_saver.o src/icm_saver.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ReleaseIntel32
	${RM} dist/ReleaseIntel32/ICC-Linux-x86/libmolecule.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
