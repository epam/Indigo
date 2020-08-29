/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

package com.epam.indigo;

public class IndigoException extends RuntimeException {
    final Object obj;

    // You may wonder what object we are keeping here and why.
    // Here is the answer: we are keeping the object that raised the exception,
    // for prohibiting the garbage collector to destroy it while the method
    // is running.

    // Here is an example:
    // {
    //    IndigoObject object = ...;
    //    object.someMethod(); // does a native call
    // }

    // In this situation, the JVM is perfectly OK with deleting the object
    // *WHILE THE NATIVE CALL IS STILL RUNNING*. At least, it happened
    // on 64-bit Windows Server 2008 within KNIME.
    // To prevent that, we keep a reference to the object in the IndigoException
    // object (which can be thrown from every Indigo or IndigoObject method).
    // As long as the JVM sees that the reference is still used somewhere
    // afterwards the method call, it does not garbage-collect the object.
    public IndigoException(Object obj, String message) {
        super(message);
        this.obj = obj;
    }
}
