/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include <jni.h>

#include "indigo-renderer.h"
#include "indigo_jni_base.h"

#define JNINAME(name) JNICALL Java_com_gga_indigo_IndigoRenderer_##name

JNI_FUNC_jint_jint_jint(indigoRender)
JNI_FUNC_jint_jint_jstring(indigoRenderToFile)

JNIEXPORT jint JNINAME(indigoRenderGrid) (JNIEnv *env, jobject obj,
        jint objects, jintArray jrefAtoms, jint nColumns, jint output)
{
   jsize nrefatoms = 0;
   jint *refatoms = 0;
   int ret;

   indigoJniSetSession(env, obj);

   if (jrefAtoms != NULL)
   {
      nrefatoms = (*env)->GetArrayLength(env, jrefAtoms);
      refatoms = (*env)->GetIntArrayElements(env, jrefAtoms, 0);

      if (nrefatoms != indigoCount(objects))
         indigoThrowJNIException(env,
        "indigoRenderGrid(): refAtoms size does not match the number of objects");
   }

   ret = indigoRenderGrid(objects, refatoms, nColumns, output);
   if (refatoms != 0)
      (*env)->ReleaseIntArrayElements(env, jrefAtoms, refatoms, 0);
   return ret;
}

JNIEXPORT jint JNINAME(indigoRenderGridToFile) (JNIEnv *env, jobject obj,
        jint objects, jintArray jrefAtoms, jint nColumns, jstring jfilename)
{
   jsize nrefatoms = 0;
   jint *refatoms = 0;
   int ret;
   const char *filename;

   indigoJniSetSession(env, obj);

   filename = (*env)->GetStringUTFChars(env, jfilename, NULL);
   if (jrefAtoms != NULL)
   {
      nrefatoms = (*env)->GetArrayLength(env, jrefAtoms);
      refatoms = (*env)->GetIntArrayElements(env, jrefAtoms, 0);
      if (nrefatoms != indigoCount(objects))
         indigoThrowJNIException(env,
        "indigoRenderGridToFile(): refAtoms size does not match the number of objects");
   }

   ret = indigoRenderGridToFile(objects, refatoms, nColumns, filename);
   if (refatoms != 0)
      (*env)->ReleaseIntArrayElements(env, jrefAtoms, refatoms, 0);
   (*env)->ReleaseStringUTFChars(env, jfilename, filename);
   return ret;
}
