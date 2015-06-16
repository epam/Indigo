/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

package com.ggasoftware.indigo;

public class BingoException extends RuntimeException
{
   Object obj;

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
   public BingoException (Object obj_, String message)
   {
      super(message);
      obj = obj_;
   }
}
