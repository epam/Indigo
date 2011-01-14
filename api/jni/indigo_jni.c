/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <jni.h>

#include "indigo.h"
#include "indigo_jni_base.h"

#define JNINAME(name) JNICALL Java_com_gga_indigo_Indigo_##name

JNIEXPORT jstring JNINAME(version) (JNIEnv *env, jobject obj)
{
   return (*env)->NewStringUTF(env, indigoVersion());
}

JNIEXPORT jlong JNINAME(allocSessionId) (JNIEnv *env, jclass cls)
{
   return indigoAllocSessionId();
}

JNIEXPORT void JNINAME(releaseSessionId) (JNIEnv *env, jclass cls, jlong id)
{
   indigoReleaseSessionId(id);
}

JNI_FUNC_jint_jint(indigoFree)
JNI_FUNC_jint_jint(indigoClone)
JNI_FUNC_jint(indigoCountReferences)

JNI_FUNC_jint_jstring_jstring(indigoSetOption)
JNI_FUNC_jint_jstring_jint(indigoSetOptionInt)
JNI_FUNC_jint_jstring_jint(indigoSetOptionBool)
JNI_FUNC_jint_jstring_jfloat(indigoSetOptionFloat)
JNI_FUNC_jint_jstring_jfloat_jfloat_jfloat(indigoSetOptionColor)
JNI_FUNC_jint_jstring_jint_jint(indigoSetOptionXY)

JNI_FUNC_jint_jstring(indigoReadFile)
JNI_FUNC_jint_jstring(indigoLoadString)
JNI_FUNC_jint_jbuf(indigoLoadBuffer)

JNI_FUNC_jint_jstring(indigoWriteFile)
JNI_FUNC_jint(indigoWriteBuffer)
JNI_FUNC_jint_jint(indigoClose)

JNI_FUNC_jint_jint(indigoLoadMolecule)
JNI_FUNC_jint_jstring(indigoLoadMoleculeFromString)
JNI_FUNC_jint_jstring(indigoLoadMoleculeFromFile)
JNI_FUNC_jint_jbuf(indigoLoadMoleculeFromBuffer)

JNI_FUNC_jint_jint(indigoLoadQueryMolecule)
JNI_FUNC_jint_jstring(indigoLoadQueryMoleculeFromString)
JNI_FUNC_jint_jstring(indigoLoadQueryMoleculeFromFile)
JNI_FUNC_jint_jbuf(indigoLoadQueryMoleculeFromBuffer)

JNI_FUNC_jint_jint(indigoLoadSmarts)
JNI_FUNC_jint_jstring(indigoLoadSmartsFromString)
JNI_FUNC_jint_jstring(indigoLoadSmartsFromFile)
JNI_FUNC_jint_jbuf(indigoLoadSmartsFromBuffer)

JNI_FUNC_jint_jint_jint(indigoSaveMolfile)
JNI_FUNC_jint_jint_jstring(indigoSaveMolfileToFile)
JNI_FUNC_jstring_jint(indigoMolfile)

JNI_FUNC_jint_jint_jint(indigoSaveCml)
JNI_FUNC_jint_jint_jstring(indigoSaveCmlToFile)
JNI_FUNC_jstring_jint(indigoCml)

JNI_FUNC_jint_jint_jint(indigoSaveMDLCT)

JNI_FUNC_jint_jint(indigoLoadReaction)
JNI_FUNC_jint_jstring(indigoLoadReactionFromString)
JNI_FUNC_jint_jstring(indigoLoadReactionFromFile)
JNI_FUNC_jint_jbuf(indigoLoadReactionFromBuffer)

JNI_FUNC_jint_jint(indigoLoadQueryReaction)
JNI_FUNC_jint_jstring(indigoLoadQueryReactionFromString)
JNI_FUNC_jint_jstring(indigoLoadQueryReactionFromFile)
JNI_FUNC_jint_jbuf(indigoLoadQueryReactionFromBuffer)

JNI_FUNC_jint(indigoCreateReaction)
JNI_FUNC_jint(indigoCreateQueryReaction)

JNI_FUNC_jint_jint_jint(indigoAddReactant)
JNI_FUNC_jint_jint_jint(indigoAddProduct)

JNI_FUNC_jint_jint(indigoCountReactants)
JNI_FUNC_jint_jint(indigoCountProducts)
JNI_FUNC_jint_jint(indigoCountMolecules)

JNI_FUNC_jint_jint(indigoIterateReactants)
JNI_FUNC_jint_jint(indigoIterateProducts)
JNI_FUNC_jint_jint(indigoIterateMolecules)

JNI_FUNC_jint_jint_jint(indigoSaveRxnfile)
JNI_FUNC_jint_jint_jstring(indigoSaveRxnfileToFile)
JNI_FUNC_jstring_jint(indigoRxnfile)
JNI_FUNC_jint_jint_jstring(indigoAutomap);

JNI_FUNC_jint_jint(indigoIterateAtoms)
JNI_FUNC_jint_jint(indigoIteratePseudoatoms)
JNI_FUNC_jint_jint(indigoIterateRSites)
JNI_FUNC_jint_jint(indigoIterateStereocenters)
JNI_FUNC_jint_jint(indigoIterateRGroups)
JNI_FUNC_jint_jint(indigoIterateRGroupFragments)
JNI_FUNC_jint_jint(indigoIsPseudoatom)
JNI_FUNC_jint_jint(indigoIsRSite)
JNI_FUNC_jint_jint(indigoStereocenterType)
JNI_FUNC_jint_jint(indigoSingleAllowedRGroup)
JNI_FUNC_jstring_jint(indigoSymbol)

JNI_FUNC_jint_jint(indigoDegree)
JNI_FUNC_jobj_jint_intptr(indigoGetCharge)
JNI_FUNC_jobj_jint_intptr(indigoGetExplicitValence)
JNI_FUNC_jobj_jint_intptr(indigoGetRadicalElectrons)
JNI_FUNC_jint_jint(indigoAtomicNumber)
JNI_FUNC_jint_jint(indigoIsotope)

JNIEXPORT jfloatArray JNINAME(indigoXYZ) (JNIEnv *env, jobject obj, jint atom)
{
   float *xyz;
   jfloatArray jarr;

   indigoJniSetSession(env, obj);
   xyz = indigoXYZ(atom);
   jarr = (*env)->NewFloatArray(env, 3);
   (*env)->SetFloatArrayRegion(env, jarr, 0, 3, (jfloat *)xyz);

   return jarr;
}

JNI_FUNC_jint_jint(indigoResetCharge)
JNI_FUNC_jint_jint(indigoResetExplicitValence)
JNI_FUNC_jint_jint(indigoResetRadical)
JNI_FUNC_jint_jint(indigoResetIsotope)

JNI_FUNC_jint_jint_jint(indigoSetAttachmentPoint)

JNI_FUNC_jint_jint_jstring(indigoRemoveConstraints)
JNI_FUNC_jint_jint_jstring_jstring(indigoAddConstraint)
JNI_FUNC_jint_jint_jstring_jstring(indigoAddConstraintNot)

JNI_FUNC_jint_jint(indigoResetStereo)
JNI_FUNC_jint_jint(indigoInvertStereo)

JNI_FUNC_jint_jint(indigoCountAtoms)
JNI_FUNC_jint_jint(indigoCountBonds)
JNI_FUNC_jint_jint(indigoCountPseudoatoms)
JNI_FUNC_jint_jint(indigoCountRSites)

JNI_FUNC_jint_jint(indigoIterateBonds)
JNI_FUNC_jint_jint(indigoBondOrder)
JNI_FUNC_jint_jint(indigoBondStereo)
JNI_FUNC_jint_jint(indigoTopology)

JNI_FUNC_jint_jint(indigoIterateNeighbors)
JNI_FUNC_jint_jint(indigoBond)
JNI_FUNC_jint_jint_jint(indigoGetAtom)
JNI_FUNC_jint_jint_jint(indigoGetBond)
JNI_FUNC_jint_jint(indigoSource)
JNI_FUNC_jint_jint(indigoDestination)

JNI_FUNC_jint_jint(indigoClearCisTrans)
JNI_FUNC_jint_jint(indigoClearStereocenters)
JNI_FUNC_jint_jint(indigoCountStereocenters)

JNI_FUNC_jint_jint(indigoResetSymmetricCisTrans)

JNI_FUNC_jint_jint(indigoCountComponents)
JNI_FUNC_jint_jint(indigoComponentIndex)
JNI_FUNC_jint_jint(indigoIterateComponents)
JNI_FUNC_jint_jint_jint(indigoComponent)

JNI_FUNC_jint_jint(indigoCountHeavyAtoms)

JNI_FUNC_jint_jint(indigoGrossFormula)
JNI_FUNC_jfloat_jint(indigoMolecularWeight)
JNI_FUNC_jfloat_jint(indigoMostAbundantMass)
JNI_FUNC_jfloat_jint(indigoMonoisotopicMass)

JNI_FUNC_jstring_jint(indigoCanonicalSmiles)
JNI_FUNC_jstring_jint(indigoLayeredCode)

JNI_FUNC_jint_jint(indigoHasZCoord)
JNI_FUNC_jint_jint(indigoIsChiral)

#define JNI_FUNC_jint_jint_jintarr(name) \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jint param1, jintArray param2) \
{ \
   jsize n; \
   jint *arr; \
   int ret; \
\
   indigoJniSetSession(env, obj); \
   n = (*env)->GetArrayLength(env, param2); \
   arr = (*env)->GetIntArrayElements(env, param2, 0); \
   ret = name(param1, n, arr); \
   (*env)->ReleaseIntArrayElements(env, param2, arr, 0); \
   return ret; \
}

JNI_FUNC_jint_jint_jintarr(indigoCreateSubmolecule)

JNIEXPORT jint JNINAME(indigoCreateEdgeSubmolecule) (JNIEnv *env, jobject obj, jint mol,
        jintArray jvertices, jintArray jedges)
{
   jsize nvertices, nedges;
   jint *vertices, *edges;
   int ret;

   indigoJniSetSession(env, obj);
   nvertices = (*env)->GetArrayLength(env, jvertices);
   nedges = (*env)->GetArrayLength(env, jedges);
   vertices = (*env)->GetIntArrayElements(env, jvertices, 0);
   edges = (*env)->GetIntArrayElements(env, jedges, 0);
   ret = indigoCreateEdgeSubmolecule(mol, nvertices, vertices, nedges, edges);
   (*env)->ReleaseIntArrayElements(env, jvertices, vertices, 0);
   (*env)->ReleaseIntArrayElements(env, jedges, edges, 0);
   return ret;
}

JNI_FUNC_jint_jint_jintarr(indigoRemoveAtoms)

JNIEXPORT jfloat JNINAME(indigoAlignAtoms) (JNIEnv *env, jobject obj,
        jint molecule, jintArray jatom_ids, jfloatArray jdesired_xyz)
{
   int natoms, nxyz;
   jint *atoms;
   jfloat *xyz;
   float ret;

   indigoJniSetSession(env, obj);
   natoms = (*env)->GetArrayLength(env, jatom_ids);
   nxyz = (*env)->GetArrayLength(env, jdesired_xyz);

   if (natoms * 3 != nxyz)
      indigoThrowJNIException(env, "alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]");

   atoms = (*env)->GetIntArrayElements(env, jatom_ids, 0);
   xyz = (*env)->GetFloatArrayElements(env, jdesired_xyz, 0);

   ret = indigoAlignAtoms(molecule, natoms, atoms, xyz);
   (*env)->ReleaseIntArrayElements(env, jatom_ids, atoms, 0);
   (*env)->ReleaseFloatArrayElements(env, jdesired_xyz, xyz, 0);
   return ret;
}


JNI_FUNC_jint_jint(indigoAromatize);
JNI_FUNC_jint_jint(indigoDearomatize);
JNI_FUNC_jint_jint(indigoFoldHydrogens);
JNI_FUNC_jint_jint(indigoUnfoldHydrogens);
JNI_FUNC_jint_jint(indigoLayout);

JNI_FUNC_jstring_jint(indigoSmiles);

JNI_FUNC_jint_jint_jint(indigoExactMatch);

JNI_FUNC_jstring_jint(indigoName);
JNI_FUNC_jint_jint_jstring(indigoSetName);

JNI_FUNC_jint_jint_jstring(indigoHasProperty);
JNI_FUNC_jstring_jint_jstring(indigoGetProperty);
JNI_FUNC_jint_jint_jstring_jstring(indigoSetProperty);
JNI_FUNC_jint_jint_jstring(indigoRemoveProperty);
JNI_FUNC_jint_jint(indigoIterateProperties);

JNI_FUNC_jstring_jint(indigoCheckBadValence);
JNI_FUNC_jstring_jint(indigoCheckAmbiguousH);

JNI_FUNC_jint_jint_jstring(indigoFingerprint);
JNI_FUNC_jint_jint(indigoCountBits);
JNI_FUNC_jint_jint_jint(indigoCommonBits);

JNIEXPORT jfloat JNINAME(indigoSimilarity) (JNIEnv *env, jobject obj,
                                            int item1, int item2, jstring jmetrics)
{
   const char *metrics;
   float ret;

   indigoJniSetSession(env, obj);
   if (jmetrics != NULL)
      metrics = (*env)->GetStringUTFChars(env, jmetrics, NULL);
   else
      metrics = 0;
   ret = indigoSimilarity(item1, item2, metrics);
   (*env)->ReleaseStringUTFChars(env, jmetrics, metrics);
   return ret;
}

JNI_FUNC_jint_jint(indigoIterateSDF)
JNI_FUNC_jint_jint(indigoIterateRDF)
JNI_FUNC_jint_jint(indigoIterateSmiles)

JNI_FUNC_jint_jstring(indigoIterateSDFile)
JNI_FUNC_jint_jstring(indigoIterateRDFile)
JNI_FUNC_jint_jstring(indigoIterateSmilesFile)

JNI_FUNC_jstring_jint(indigoRawData)
JNI_FUNC_jint_jint(indigoTell)

JNI_FUNC_jint_jint_jint(indigoSdfAppend)
JNI_FUNC_jint_jint_jint(indigoSmilesAppend)
JNI_FUNC_jint_jint(indigoRdfHeader)
JNI_FUNC_jint_jint_jint(indigoRdfAppend)

JNI_FUNC_jint(indigoCreateArray)
JNI_FUNC_jint_jint_jint(indigoArrayAdd)
JNI_FUNC_jint_jint_jint(indigoAt)
JNI_FUNC_jint_jint(indigoCount)
JNI_FUNC_jint_jint(indigoClear)
JNI_FUNC_jint_jint(indigoIterateArray)

JNI_FUNC_jint_jint_jstring(indigoSubstructureMatcher)
JNI_FUNC_jint_jint_jint(indigoMatch)
JNI_FUNC_jint_jint_jint(indigoCountMatches)
JNI_FUNC_jint_jint_jint(indigoIterateMatches)
JNI_FUNC_jint_jint(indigoHighlightedTarget)
JNI_FUNC_jint_jint_jint(indigoMapAtom)
JNI_FUNC_jint_jint_jint(indigoMapBond)

JNI_FUNC_jint_jint_jstring(indigoExtractCommonScaffold)
JNI_FUNC_jint_jint(indigoAllScaffolds)

JNI_FUNC_jint_jint_jint(indigoDecomposeMolecules)
JNI_FUNC_jint_jint(indigoDecomposedMoleculeScaffold)
JNI_FUNC_jint_jint(indigoIterateDecomposedMolecules)
JNI_FUNC_jint_jint(indigoDecomposedMoleculeHighlighted)
JNI_FUNC_jint_jint(indigoDecomposedMoleculeWithRGroups)

JNI_FUNC_jint_jint(indigoNext)
JNI_FUNC_jint_jint(indigoHasNext)
JNI_FUNC_jint_jint(indigoIndex)

JNI_FUNC_jstring_jint(indigoToString);

JNIEXPORT jbyteArray JNINAME(indigoToBuffer) (JNIEnv *env, jobject obj, jint handle)
{
   char *buf;
   int size;
   jbyteArray jbuf;

   indigoJniSetSession(env, obj);
   indigoToBuffer(handle, &buf, &size);
   jbuf = (*env)->NewByteArray(env, size);
   (*env)->SetByteArrayRegion(env, jbuf, 0, size, (jbyte *)buf);

   return jbuf;
}

JNI_FUNC_jint_jint_jint(indigoReactionProductEnumerate);

JNIEXPORT jint JNINAME(indigoRtldGlobal) (JNIEnv *env, jobject obj, jstring j_param1)
{
#ifndef _WIN32
   const char *path;

   indigoJniSetSession(env, obj);
   path = (*env)->GetStringUTFChars(env, j_param1, NULL);
   if (dlopen(path, RTLD_NOW | RTLD_NOLOAD | RTLD_GLOBAL) == NULL)
   {
      indigoThrowJNIException(env, dlerror());
      return -1;
   }
   (*env)->ReleaseStringUTFChars(env, j_param1, path);
#endif
   return 1;
}
