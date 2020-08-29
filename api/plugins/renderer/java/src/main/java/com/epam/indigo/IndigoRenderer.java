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

import com.sun.jna.Native;
import com.sun.jna.Platform;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

public class IndigoRenderer {
    public IndigoRenderer(Indigo indigo) {
        loadLibrary(indigo.getUserSpecifiedPath());
        this.indigo = indigo;
    }

    public void render(IndigoObject obj, IndigoObject output) {
        indigo.setSessionID();
        Object[] guard = new Object[]{this, obj, output};
        Indigo.checkResult(guard, lib.indigoRender(obj.self, output.self));
    }

    public void renderToFile(IndigoObject obj, String filename) {
        indigo.setSessionID();
        Indigo.checkResult(this, obj, lib.indigoRenderToFile(obj.self, filename));
    }

    public byte[] renderToBuffer(IndigoObject obj) {
        indigo.setSessionID();
        IndigoObject buf = indigo.writeBuffer();
        try {
            Indigo.checkResult(this, obj, lib.indigoRender(obj.self, buf.self));

            return buf.toBuffer();
        } finally {
            buf.dispose();
        }
    }

    public void renderGridToFile(IndigoObject objects, int[] refAtoms, int ncolumns, String filename) {
        indigo.setSessionID();
        if (refAtoms != null && objects.count() != refAtoms.length)
            throw new IndigoException(this, "refAtoms size does not match the number of objects");
        Indigo.checkResult(this, objects, lib.indigoRenderGridToFile(objects.self, refAtoms, ncolumns, filename));
    }

    public byte[] renderGridToBuffer(IndigoObject objects, int[] refAtoms, int ncolumns) {
        indigo.setSessionID();
        if (refAtoms != null && objects.count() != refAtoms.length)
            throw new IndigoException(this, "refAtoms size does not match the number of objects");
        IndigoObject buf = indigo.writeBuffer();
        try {
            Indigo.checkResult(this, objects, lib.indigoRenderGrid(objects.self, refAtoms, ncolumns, buf.self));
            return buf.toBuffer();
        } finally {
            buf.dispose();
        }
    }

    public void renderResetSettings() {
        indigo.setSessionID();
        lib.indigoRenderReset();
    }

    private static String getPathToBinary(String path, String filename) throws FileNotFoundException {
        String dllpath = Indigo.getPlatformDependentPath();

        if (path == null) {
            String res = Indigo.extractFromJar(IndigoRenderer.class, "/" + dllpath, filename);
            if (res != null)
                return res;
            throw new FileNotFoundException("Couldn't extract native lib " + filename + " from jar");
        }
        path = path + File.separator + dllpath + File.separator + filename;
        try {
            return (new File(path)).getCanonicalPath();
        } catch (IOException e) {
            return path;
        }
    }

    private synchronized static void loadLibrary(String path) {
        if (lib != null)
            return;
        try {
            if (Platform.isLinux() || Platform.isSolaris())
                lib = Native.load(getPathToBinary(path, "libindigo-renderer.so"), IndigoRendererLib.class);
            else if (Platform.isMac())
                lib = Native.load(getPathToBinary(path, "libindigo-renderer.dylib"), IndigoRendererLib.class);
            else if (Platform.isWindows())
                lib = Native.load(getPathToBinary(path, "indigo-renderer.dll"), IndigoRendererLib.class);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e.getMessage());
        }
    }

    final Indigo indigo;
    static IndigoRendererLib lib;
}
