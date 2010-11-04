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

#include <jni.h>

#include "indigo-renderer.h"
#include "indigo_jni_base.h"

#define JNINAME(name) JNICALL Java_com_scitouch_indigo_IndigoRenderer_##name

JNI_FUNC_jint_jint_jint(indigoRender);
JNI_FUNC_jint_jint_jstring(indigoRenderToFile);
