/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __indigo_jni_base__
#define __indigo_jni_base__

JNIEXPORT void indigoJniSetSession (JNIEnv *env, jobject obj);

#define JNI_FUNC_jint_jint(name)                                     \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jint param1) \
{                                                                    \
   indigoJniSetSession(env, obj);                                             \
   return name(param1);                                              \
}

#define JNI_FUNC_jobj_jint_intptr(name)                                     \
JNIEXPORT jobject JNINAME(name) (JNIEnv *env, jobject obj, jint param1) \
{                                                                    \
   int rc, value; \
   jclass Integer; \
   jmethodID constructor; \
\
   indigoJniSetSession(env, obj);                                             \
   rc = name(param1, &value); \
   if (rc == 0) \
      return NULL; \
   Integer = (*env)->FindClass(env, "java/lang/Integer"); \
   constructor = (*env)->GetMethodID(env, Integer,  "<init>", "(I)V"); \
   \
   return (*env)->NewObject(env, Integer, constructor, value); \
}

#define JNI_FUNC_jfloat_jint(name)                                     \
JNIEXPORT jfloat JNINAME(name) (JNIEnv *env, jobject obj, jint param1) \
{                                                                      \
   indigoJniSetSession(env, obj);                                               \
   return name(param1);                                                \
}

#define JNI_FUNC_jint_jint_jint(name)                                     \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jint p1, jint p2) \
{                                                                         \
   indigoJniSetSession(env, obj);                                                  \
   return name(p1, p2);                                                   \
}

#define JNI_FUNC_jint_jstring(name)                                       \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jstring j_param1) \
{                                                                         \
   const char *param1;                                                    \
   int ret;                                                               \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   param1 = (*env)->GetStringUTFChars(env, j_param1, NULL);               \
   ret = name(param1);                                                    \
   (*env)->ReleaseStringUTFChars(env, j_param1, param1);                  \
   return ret;                                                            \
}

#define JNI_FUNC_jint_jstring_jstring(name)                               \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jstring j_p1, jstring j_p2) \
{                                                                         \
   const char *p1, *p2;                                                   \
   int ret;                                                               \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   p1 = (*env)->GetStringUTFChars(env, j_p1, NULL);                       \
   p2 = (*env)->GetStringUTFChars(env, j_p2, NULL);                       \
   ret = name(p1, p2);                                                    \
   (*env)->ReleaseStringUTFChars(env, j_p1, p1);                          \
   (*env)->ReleaseStringUTFChars(env, j_p2, p2);                          \
   return ret;                                                            \
}

#define JNI_FUNC_jint_jint_jstring_jstring(name)                          \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jint i, jstring j_p1, jstring j_p2) \
{                                                                         \
   const char *p1, *p2;                                                   \
   int ret;                                                               \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   p1 = (*env)->GetStringUTFChars(env, j_p1, NULL);                       \
   p2 = (*env)->GetStringUTFChars(env, j_p2, NULL);                       \
   ret = name(i, p1, p2);                                                 \
   (*env)->ReleaseStringUTFChars(env, j_p1, p1);                          \
   (*env)->ReleaseStringUTFChars(env, j_p2, p2);                          \
   return ret;                                                            \
}

#define JNI_FUNC_jint_jstring_jint(name)                                  \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jstring j_p1, jint p2) \
{                                                                         \
   const char *param1;                                                    \
   int ret;                                                               \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   param1 = (*env)->GetStringUTFChars(env, j_p1, NULL);                   \
   ret = name(param1, p2);                                                \
   (*env)->ReleaseStringUTFChars(env, j_p1, param1);                      \
   return ret;                                                            \
}

#define JNI_FUNC_jint_jstring_jint_jint(name)                             \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jstring j_p1, jint p2, jint p3) \
{                                                                         \
   const char *param1;                                                    \
   int ret;                                                               \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   param1 = (*env)->GetStringUTFChars(env, j_p1, NULL);                   \
   ret = name(param1, p2, p3);                                            \
   (*env)->ReleaseStringUTFChars(env, j_p1, param1);                      \
   return ret;                                                            \
}

#define JNI_FUNC_jint_jstring_jfloat(name)                                  \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jstring j_p1, jfloat p2) \
{                                                                         \
   const char *param1;                                                    \
   int ret;                                                               \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   param1 = (*env)->GetStringUTFChars(env, j_p1, NULL);                   \
   ret = name(param1, p2);                                                \
   (*env)->ReleaseStringUTFChars(env, j_p1, param1);                      \
   return ret;                                                            \
}

#define JNI_FUNC_jint_jstring_jfloat_jfloat_jfloat(name)                  \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jstring j_p1, jfloat p2, jfloat p3, jfloat p4) \
{                                                                         \
   const char *param1;                                                    \
   int ret;                                                               \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   param1 = (*env)->GetStringUTFChars(env, j_p1, NULL);                   \
   ret = name(param1, p2, p3, p4);                                        \
   (*env)->ReleaseStringUTFChars(env, j_p1, param1);                      \
   return ret;                                                            \
}

#define JNI_FUNC_jint_jbuf(name)                                          \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jbyteArray jbuf)  \
{                                                                         \
   int length, ret;                                                       \
   jboolean isCopy = JNI_FALSE;                                           \
   jbyte* buf;                                                            \
                                                                          \
   indigoJniSetSession(env, obj);                                                  \
   length = (*env)->GetArrayLength(env, jbuf);                            \
   buf = (*env)->GetByteArrayElements(env, jbuf, &isCopy);                \
   ret = name((const char *)buf, length);                                 \
   (*env)->ReleaseByteArrayElements(env, jbuf, buf, isCopy);              \
                                                                          \
   return ret;                                                            \
}

#define JNI_FUNC_jint(name)                                               \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj)                   \
{                                                                         \
   indigoJniSetSession(env, obj);                                                  \
   return name();                                                         \
}

#define JNI_FUNC_jstring_jint(name)                                     \
JNIEXPORT jstring JNINAME(name) (JNIEnv *env, jobject obj, jint param)  \
{                                                                       \
   indigoJniSetSession(env, obj);                                                \
   return (*env)->NewStringUTF(env, name(param));                       \
}

#define JNI_FUNC_jint_jint_jstring(name)                                       \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jint p1, jstring j_p2) \
{                                                                              \
   const char *p2;                                                             \
   int ret;                                                                    \
                                                                               \
   indigoJniSetSession(env, obj);                                                       \
   p2 = (*env)->GetStringUTFChars(env, j_p2, NULL);                            \
   ret = name(p1, p2);                                                         \
   (*env)->ReleaseStringUTFChars(env, j_p2, p2);                               \
   return ret;                                                                 \
}

#define JNI_FUNC_jint_jint_jint_jstring(name)                                       \
JNIEXPORT jint JNINAME(name) (JNIEnv *env, jobject obj, jint p1, jint p2, jstring j_p3) \
{                                                                              \
   const char *p3;                                                             \
   int ret;                                                                    \
                                                                               \
   indigoJniSetSession(env, obj);                                                       \
   p3 = (*env)->GetStringUTFChars(env, j_p3, NULL);                            \
   ret = name(p1, p2, p3);                                                         \
   (*env)->ReleaseStringUTFChars(env, j_p3, p3);                               \
   return ret;                                                                 \
}

#define JNI_FUNC_jstring_jint_jstring(name)                                    \
JNIEXPORT jstring JNINAME(name) (JNIEnv *env, jobject obj, jint p1, jstring j_p2) \
{                                                                              \
   const char *p2;                                                             \
   const char *ret;                                                            \
                                                                               \
   indigoJniSetSession(env, obj);                                                       \
   p2 = (*env)->GetStringUTFChars(env, j_p2, NULL);                            \
   ret = name(p1, p2);                                                         \
   (*env)->ReleaseStringUTFChars(env, j_p2, p2);                               \
   return (*env)->NewStringUTF(env, ret);                                      \
}

#endif
