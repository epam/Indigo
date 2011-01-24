/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#include "indigo.h"
#include "indigo_jni_base.h"

CEXPORT void indigoThrowJNIException (JNIEnv *env, const char *message)
{
   jclass cls = (*env)->FindClass(env, "com/ggasoftware/indigo/IndigoException");

   if (cls == NULL)
      return;

   (*env)->ThrowNew(env, cls, message);
   // Not really sure if DeleteLocalRef() call is OK when an exception is pending;
   // however, it did not cause any harm so far.
   (*env)->DeleteLocalRef(env, cls);
}

static void _indigoErrorHandler (const char *message, void *context)
{
   JNIEnv *env = (JNIEnv *)context;
   indigoThrowJNIException(env, message);
}

CEXPORT void indigoJniSetSession (JNIEnv *env, jobject obj)
{
   jclass cls = (*env)->GetObjectClass(env, obj);
   jfieldID id = (*env)->GetFieldID(env, cls, "_sid", "J");
   jlong sid = (*env)->GetLongField(env, obj, id);

   indigoSetSessionId(sid);
   indigoSetErrorHandler(_indigoErrorHandler, env);
}


CEXPORT void indigoJniSetSession_NoErrorHandler (JNIEnv *env, jobject obj)
{
   jclass cls = (*env)->GetObjectClass(env, obj);
   jfieldID id = (*env)->GetFieldID(env, cls, "_sid", "J");
   jlong sid = (*env)->GetLongField(env, obj, id);

   indigoSetSessionId(sid);
}

/*
static const char *_className (JNIEnv *env, jobject obj)
{
   jclass cls = (*env)->GetObjectClass(env, obj);
   jclass classMethodAccess = (*env)->FindClass(env, "java/lang/Class");
   jmethodID classNameMethodID = (*env)->GetMethodID(env, classMethodAccess,  "getName", "()Ljava/lang/String;");
   jstring clsName=(jstring)((*env)->CallObjectMethod(env, cls, classNameMethodID));
   return (*env)->GetStringUTFChars(env, clsName, NULL);
}*/
