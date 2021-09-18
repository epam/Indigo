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

public class IndigoInchi {
    public IndigoInchi(Indigo indigo) {
        loadLibrary(indigo.getUserSpecifiedPath());
        this.indigo = indigo;
        this.indigo.setSessionID();
        Indigo.checkResult(this, lib.indigoInchiInit());
        initialized = true;
    }

    @Override
    protected void finalize() throws Throwable {
        if (initialized)
        {
            indigo.setSessionID();
            Indigo.checkResult(this, lib.indigoInchiDispose());
            super.finalize();
            initialized = false;
        }
    }

    public String version() {
        indigo.setSessionID();
        return Indigo.checkResultString(this, lib.indigoInchiVersion());
    }

    public void resetOptions() {
        indigo.setSessionID();
        Indigo.checkResult(this, lib.indigoInchiResetOptions());
    }

    public IndigoObject loadMolecule(String inchi) {
        indigo.setSessionID();
        return new IndigoObject(indigo, Indigo.checkResult(this, lib.indigoInchiLoadMolecule(inchi)));
    }

    public String getInchi(IndigoObject molecule) {
        indigo.setSessionID();
        return Indigo.checkResultString(this, lib.indigoInchiGetInchi(molecule.self));
    }

    public String getInchiKey(String inchi) {
        indigo.setSessionID();
        return Indigo.checkResultString(this, lib.indigoInchiGetInchiKey(inchi));
    }

    public String getWarning() {
        indigo.setSessionID();
        return Indigo.checkResultString(this, lib.indigoInchiGetWarning());
    }

    public String getLog() {
        indigo.setSessionID();
        return Indigo.checkResultString(this, lib.indigoInchiGetLog());
    }

    public String getAuxInfo() {
        indigo.setSessionID();
        return Indigo.checkResultString(this, lib.indigoInchiGetAuxInfo());
    }

    private synchronized static void loadLibrary(String path) {
        if (lib != null)
            return;
        try {
            if (Platform.isLinux() || Platform.isSolaris())
                lib = Native.load(IndigoUtils.getPathToBinary(IndigoInchi.class, Indigo.getPlatformDependentPath(), path, "libindigo-inchi.so"), IndigoInchiLib.class);
            else if (Platform.isMac())
                lib = Native.load(IndigoUtils.getPathToBinary(IndigoInchi.class, Indigo.getPlatformDependentPath(), path, "libindigo-inchi.dylib"), IndigoInchiLib.class);
            else if (Platform.isWindows())
                lib = Native.load(IndigoUtils.getPathToBinary(IndigoInchi.class, Indigo.getPlatformDependentPath(), path, "indigo-inchi.dll"), IndigoInchiLib.class);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e.getMessage());
        }
    }

    final Indigo indigo;
    static IndigoInchiLib lib;
    boolean initialized;
}
