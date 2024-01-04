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
import com.sun.jna.Pointer;
import com.sun.jna.ptr.FloatByReference;
import com.sun.jna.ptr.IntByReference;

import java.io.*;
import java.util.Collection;

public class Indigo {
    public static final int ABS = 1;
    public static final int OR = 2;
    public static final int AND = 3;
    public static final int EITHER = 4;
    public static final int UP = 5;
    public static final int DOWN = 6;
    public static final int CIS = 7;
    public static final int TRANS = 8;
    public static final int CHAIN = 9;
    public static final int RING = 10;
    public static final int ALLENE = 11;
    public static final int SINGLET = 101;
    public static final int DOUBLET = 102;
    public static final int TRIPLET = 103;
    public static final int RC_NOT_CENTER = -1;
    public static final int RC_UNMARKED = 0;
    public static final int RC_CENTER = 1;
    public static final int RC_UNCHANGED = 2;
    public static final int RC_MADE_OR_BROKEN = 4;
    public static final int RC_ORDER_CHANGED = 8;

    public static final int SG_TYPE_GEN = 0;
    public static final int SG_TYPE_DAT = 1;
    public static final int SG_TYPE_SUP = 2;
    public static final int SG_TYPE_SRU = 3;
    public static final int SG_TYPE_MUL = 4;
    public static final int SG_TYPE_MON = 5;
    public static final int SG_TYPE_MER = 6;
    public static final int SG_TYPE_COP = 7;
    public static final int SG_TYPE_CRO = 8;
    public static final int SG_TYPE_MOD = 9;
    public static final int SG_TYPE_GRA = 10;
    public static final int SG_TYPE_COM = 11;
    public static final int SG_TYPE_MIX = 12;
    public static final int SG_TYPE_FOR = 13;
    public static final int SG_TYPE_ANY = 14;



    // JNA does not allow throwing exception from callbacks, thus we can not
    // use the error handler and we have to check the error codes. Below are
    // four functions to ease checking them.
    public static final String INDIGO_DLL = "indigo.dll";
    public static final String LIBINDIGO_SO = "libindigo.so";
    public static final String LIBINDIGO_DYLIB = "libindigo.dylib";
    public static final String[] WIN_DLLS = {"vcruntime140.dll", "vcruntime140_1.dll", "msvcp140.dll"};
    private static final String dllpath;
    private static IndigoLib lib = null;

    static {
        dllpath = IndigoUtils.getDllPath();
    }

    private boolean session_released = false;
    private String path;
    private long sid;

    public Indigo(String path) {
        this.path = path;
        try {
            loadIndigo(path);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e.getMessage());
        }
        System.setProperty("jna.encoding", "UTF-8");

        sid = lib.indigoAllocSessionId();
    }

    public Indigo() {
        this(null);
    }

    public static int checkResult(Object obj, int result) {
        if (result < 0) throw new IndigoException(obj, lib.indigoGetLastError());
        return result;
    }

    public static long checkResultLong(Object obj, long result) {
        if (result < 0) throw new IndigoException(obj, lib.indigoGetLastError());
        return result;
    }

    public static int checkResult(Object obj, Object obj2, int result) {
        if (result < 0)
            throw new IndigoException(new Object[] {obj, obj2}, lib.indigoGetLastError());
        return result;
    }

    public static float checkResultFloat(Object obj, float result) {
        if (result < 0) throw new IndigoException(obj, lib.indigoGetLastError());
        return result;
    }

    public static double checkResultDouble(Object obj, double result) {
        if (result < 0) throw new IndigoException(obj, lib.indigoGetLastError());
        return result;
    }

    public static String checkResultString(Object obj, Pointer result) {
        if (result == Pointer.NULL) throw new IndigoException(obj, lib.indigoGetLastError());
        return result.getString(0);
    }

    public static Pointer checkResultPointer(Object obj, Pointer result) {
        if (result == Pointer.NULL) throw new IndigoException(obj, lib.indigoGetLastError());

        return result;
    }

    public static int[] toIntArray(Collection<Integer> collection) {
        if (collection == null) return new int[0];

        int[] res = new int[collection.size()];
        int i = 0;

        for (Integer x : collection) res[i++] = x;

        return res;
    }

    public static float[] toFloatArray(Collection<Float> collection) {
        if (collection == null) return new float[0];

        float[] res = new float[collection.size()];
        int i = 0;

        for (Float x : collection) res[i++] = x;

        return res;
    }

    private static synchronized void loadIndigo(String path) throws FileNotFoundException {
        if (lib != null) return;

        if (Platform.isLinux() || Platform.isSolaris())
            lib = Native.load(IndigoUtils.getPathToBinary(Indigo.class, dllpath, path, LIBINDIGO_SO), IndigoLib.class);
        else if (Platform.isMac())
            lib = Native.load(IndigoUtils.getPathToBinary(Indigo.class, dllpath, path, LIBINDIGO_DYLIB), IndigoLib.class);
        else if (Platform.isWindows()) {
            for (String dllName: WIN_DLLS) {
                try {
                    System.load(IndigoUtils.getPathToBinary(Indigo.class, dllpath, path, dllName));
                } catch (UnsatisfiedLinkError e) {
                    // File could have been already loaded
                } catch (FileNotFoundException e) {
                    // ignore, not all native windows dlls are available
                }
            }
            lib = Native.load(IndigoUtils.getPathToBinary(Indigo.class, dllpath, path, INDIGO_DLL), IndigoLib.class);
        }
    }

    public static String getPlatformDependentPath() {
        return dllpath;
    }

    public static IndigoLib getLibrary() {
        return lib;
    }

    public boolean sessionReleased() {
        return session_released;
    }

    public String version() {
        return lib.indigoVersion();
    }

    public String versionInfo() {
        return lib.indigoVersionInfo();
    }

    public int countReferences() {
        setSessionID();
        return checkResult(this, lib.indigoCountReferences());
    }

    public void setSessionID() {
        lib.indigoSetSessionId(sid);
    }

    public void setOption(String option, String value) {
        setSessionID();
        checkResult(this, lib.indigoSetOption(option, value));
    }

    public void setOption(String option, int value) {
        setSessionID();
        checkResult(this, lib.indigoSetOptionInt(option, value));
    }

    public void setOption(String option, int x, int y) {
        setSessionID();
        checkResult(this, lib.indigoSetOptionXY(option, x, y));
    }

    public void setOption(String option, float r, float g, float b) {
        setSessionID();
        checkResult(this, lib.indigoSetOptionColor(option, r, g, b));
    }

    public void setOption(String option, boolean value) {
        setSessionID();
        checkResult(this, lib.indigoSetOptionBool(option, value ? 1 : 0));
    }

    public void setOption(String option, float value) {
        setSessionID();
        checkResult(this, lib.indigoSetOptionFloat(option, value));
    }

    public void setOption(String option, double value) {
        setSessionID();
        checkResult(this, lib.indigoSetOptionFloat(option, (float) value));
    }

    public String getOption(String option) {
        setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetOption(option));
    }

    public Integer getOptionInt(String option) {
        setSessionID();
        IntByReference res = new IntByReference();
        if (Indigo.checkResult(this, lib.indigoGetOptionInt(option, res)) == 1) {
            return res.getValue();
        }
        return null;
    }

    public boolean getOptionBool(String option) {
        setSessionID();
        IntByReference res = new IntByReference();
        Indigo.checkResult(this, lib.indigoGetOptionBool(option, res));
        return res.getValue() > 0;
    }

    public Float getOptionFloat(String option) {
        setSessionID();
        FloatByReference res = new FloatByReference();
        if (Indigo.checkResult(this, lib.indigoGetOptionFloat(option, res)) == 1) {
            return res.getValue();
        }
        return null;
    }

    public String getOptionType(String option) {
        setSessionID();
        return Indigo.checkResultString(this, lib.indigoGetOptionType(option));
    }

    public void resetOptions() {
        setSessionID();
        checkResult(this, lib.indigoResetOptions());
    }

    public IndigoObject writeFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoWriteFile(filename)));
    }

    public IndigoObject writeBuffer() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoWriteBuffer()));
    }

    public IndigoObject createMolecule() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoCreateMolecule()));
    }

    public IndigoObject createQueryMolecule() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoCreateQueryMolecule()));
    }

    public IndigoObject loadMolecule(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadMoleculeFromString(str)));
    }

    public IndigoObject loadMolecule(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadMoleculeFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadMoleculeFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadMoleculeFromFile(path)));
    }

    public IndigoObject loadMoleculeFromBuffer(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadMoleculeFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadQueryMolecule(String str) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadQueryMoleculeFromString(str)));
    }

    public IndigoObject loadQueryMolecule(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadQueryMoleculeFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadQueryMoleculeFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadQueryMoleculeFromFile(path)));
    }

    public IndigoObject loadSmarts(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadSmartsFromString(str)));
    }

    public IndigoObject loadSmarts(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadSmartsFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadSmartsFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadSmartsFromFile(path)));
    }

    public IndigoObject loadSequence(String str, String seq_type) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadSequenceFromString(str, seq_type)));
    }

    public IndigoObject loadSequenceFromFile(String path, String seq_type) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadSequenceFromFile(path, seq_type)));
    }

    public IndigoObject loadReaction(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadReactionFromString(str)));
    }

    public IndigoObject loadReaction(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadReactionFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadReactionFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadReactionFromFile(path)));
    }

    public IndigoObject loadQueryReaction(String str) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadQueryReactionFromString(str)));
    }

    public IndigoObject loadQueryReaction(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadQueryReactionFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadQueryReactionFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadQueryReactionFromFile(path)));
    }

    public IndigoObject loadReactionSmarts(String str) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadReactionSmartsFromString(str)));
    }

    public IndigoObject loadReactionSmarts(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadReactionSmartsFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadReactionSmartsFromFile(String path) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadReactionSmartsFromFile(path)));
    }

    public String checkStructure(String str) {
        return checkStructure(str, "");
    }

    public String checkStructure(String str, String params) {
        setSessionID();
        return checkResultString(this, lib.indigoCheckStructure(str, params));
    }

    public String check(String str, String type, String params) {
        setSessionID();
        return checkResultString(this, lib.indigoCheck(str, type, params));
    }

    public IndigoObject loadStructure(String str) {
        return loadStructure(str, "");
    }

    public IndigoObject loadStructure(String str, String params) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadStructureFromString(str, params)));
    }

    public IndigoObject loadStructure(byte[] buf) {
        return loadStructure(buf, "");
    }

    public IndigoObject loadStructure(byte[] buf, String params) {
        setSessionID();
        return new IndigoObject(
                this,
                checkResult(this, lib.indigoLoadStructureFromBuffer(buf, buf.length, params)));
    }

    public IndigoObject loadStructureFromFile(String path) {
        return loadStructureFromFile(path, "");
    }

    public IndigoObject loadStructureFromFile(String path, String params) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadStructureFromFile(path, params)));
    }

    public IndigoObject loadStructureFromBuffer(byte[] buf) {
        return loadStructureFromBuffer(buf, "");
    }

    public IndigoObject loadStructureFromBuffer(byte[] buf, String params) {
        setSessionID();
        return new IndigoObject(
                this,
                checkResult(this, lib.indigoLoadStructureFromBuffer(buf, buf.length, params)));
    }

    public IndigoObject loadFingerprintFromBuffer(byte[] buf) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoLoadFingerprintFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadFingerprintFromDescriptors(
            double[] descriptors, int size, double density) {
        setSessionID();
        int result =
                lib.indigoLoadFingerprintFromDescriptors(
                        descriptors, descriptors.length, size, density);
        return new IndigoObject(this, checkResult(this, result));
    }

    public IndigoObject createReaction() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoCreateReaction()));
    }

    public IndigoObject createQueryReaction() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoCreateQueryReaction()));
    }

    public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2, String flags) {
        if (flags == null) flags = "";

        IndigoObject[] parent = new IndigoObject[] {obj1, obj2};
        setSessionID();
        int match = checkResult(this, parent, lib.indigoExactMatch(obj1.self, obj2.self, flags));

        if (match == 0) return null;

        return new IndigoObject(this, match, parent);
    }

    public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2) {
        return exactMatch(obj1, obj2, "");
    }

    public void setTautomerRule(int id, String beg, String end) {
        setSessionID();
        checkResult(this, lib.indigoSetTautomerRule(id, beg, end));
    }

    public void removeTautomerRule(int id) {
        setSessionID();
        checkResult(this, lib.indigoRemoveTautomerRule(id));
    }

    public void clearTautomerRules() {
        setSessionID();
        checkResult(this, lib.indigoClearTautomerRules());
    }

    public float similarity(IndigoObject obj1, IndigoObject obj2) {
        return similarity(obj1, obj2, "");
    }

    public float similarity(IndigoObject obj1, IndigoObject obj2, String metrics) {
        if (metrics == null) metrics = "";
        Object[] guard = new Object[] {this, obj1, obj2};
        setSessionID();
        return checkResultFloat(guard, lib.indigoSimilarity(obj1.self, obj2.self, metrics));
    }

    public int commonBits(IndigoObject fingerprint1, IndigoObject fingerprint2) {
        Object[] guard = new Object[] {this, fingerprint1, fingerprint2};
        setSessionID();
        return checkResult(guard, lib.indigoCommonBits(fingerprint1.self, fingerprint2.self));
    }

    /**
     * @deprecated Use {@link #deserialize(byte[])} instead
     */
    @Deprecated
    public IndigoObject unserialize(byte[] data) {
        return deserialize(data);
    }

    public IndigoObject deserialize(byte[] data) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoUnserialize(data, data.length)));
    }

    public IndigoObject createArray() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoCreateArray()));
    }

    public IndigoObject iterateSDFile(String filename) {
        setSessionID();
        int result = checkResult(this, lib.indigoIterateSDFile(filename));
        if (result == 0) return null;

        return new IndigoObject(this, result);
    }

    public IndigoObject iterateRDFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoIterateRDFile(filename)));
    }

    public IndigoObject iterateSmilesFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoIterateSmilesFile(filename)));
    }

    public IndigoObject iterateCMLFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoIterateCMLFile(filename)));
    }

    public IndigoObject iterateCDXFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoIterateCDXFile(filename)));
    }

    public IndigoObject substructureMatcher(IndigoObject target, String mode) {
        setSessionID();
        return new IndigoObject(
                this,
                checkResult(this, target, lib.indigoSubstructureMatcher(target.self, mode)),
                target);
    }

    public IndigoObject substructureMatcher(IndigoObject target) {
        return substructureMatcher(target, "");
    }

    public IndigoObject extractCommonScaffold(IndigoObject structures, String options) {
        setSessionID();
        int res =
                checkResult(
                        this,
                        structures,
                        lib.indigoExtractCommonScaffold(structures.self, options));

        if (res == 0) return null;

        return new IndigoObject(this, res);
    }

    public IndigoObject extractCommonScaffold(Collection<IndigoObject> structures, String options) {
        return extractCommonScaffold(toIndigoArray(structures), options);
    }

    public IndigoObject rgroupComposition(IndigoObject molecule, String options) {
        setSessionID();
        int res = checkResult(this, molecule, lib.indigoRGroupComposition(molecule.self, options));
        if (res == 0) return null;
        return new IndigoObject(this, res);
    }

    public IndigoObject getFragmentedMolecule(IndigoObject molecule, String options) {
        setSessionID();
        int res =
                checkResult(
                        this, molecule, lib.indigoGetFragmentedMolecule(molecule.self, options));
        if (res == 0) return null;
        return new IndigoObject(this, res);
    }

    /** Use createDecomposer() and decomposeMolecule() */
    @Deprecated
    public IndigoObject decomposeMolecules(IndigoObject scaffold, IndigoObject structures) {
        Object[] guard = new Object[] {this, scaffold, structures};
        setSessionID();
        int res = checkResult(guard, lib.indigoDecomposeMolecules(scaffold.self, structures.self));

        if (res == 0) return null;

        return new IndigoObject(this, res);
    }

    /** Use createDecomposer() and decomposeMolecule() */
    @Deprecated
    public IndigoObject decomposeMolecules(
            IndigoObject scaffold, Collection<IndigoObject> structures) {
        return decomposeMolecules(scaffold, toIndigoArray(structures));
    }

    public IndigoObject createDecomposer(IndigoObject scaffold) {
        Object[] guard = new Object[] {this, scaffold};
        setSessionID();
        int res = checkResult(guard, lib.indigoCreateDecomposer(scaffold.self));

        if (res == 0) return null;

        return new IndigoObject(this, res);
    }

    public IndigoObject reactionProductEnumerate(IndigoObject reaction, IndigoObject monomers) {
        Object[] guard = new Object[] {this, reaction, monomers};
        setSessionID();
        int res =
                checkResult(
                        guard, lib.indigoReactionProductEnumerate(reaction.self, monomers.self));

        if (res == 0) return null;

        return new IndigoObject(this, res);
    }

    public IndigoObject reactionProductEnumerate(
            IndigoObject reaction, Iterable<Iterable<IndigoObject>> monomers) {
        Object[] guard = new Object[] {this, reaction, monomers};

        IndigoObject monomersArrayArray = createArray();
        for (Iterable<IndigoObject> iter : monomers) {
            IndigoObject monomersArray = createArray();
            for (IndigoObject monomer : iter) {
                monomersArray.arrayAdd(monomer);
            }
            monomersArrayArray.arrayAdd(monomersArray);
        }
        setSessionID();
        int res =
                checkResult(
                        guard,
                        lib.indigoReactionProductEnumerate(reaction.self, monomersArrayArray.self));
        if (res == 0) return null;

        return new IndigoObject(this, res);
    }

    public IndigoObject transform(IndigoObject reaction, IndigoObject monomer) {
        Object[] guard = new Object[] {this, reaction, monomer};
        setSessionID();
        int res = checkResult(guard, lib.indigoTransform(reaction.self, monomer.self));
        if (res == 0) return null;
        return new IndigoObject(this, res);
    }

    public IndigoObject createSaver(IndigoObject output, String format) {
        setSessionID();
        return new IndigoObject(
                this,
                checkResult(this, output, lib.indigoCreateSaver(output.self, format)),
                output);
    }

    public IndigoObject createFileSaver(String filename, String format) {
        setSessionID();
        return new IndigoObject(
                this, checkResult(this, lib.indigoCreateFileSaver(filename, format)));
    }

    public void dbgBreakpoint() {
        setSessionID();
        lib.indigoDbgBreakpoint();
    }

    public IndigoObject toIndigoArray(Collection<IndigoObject> coll) {
        setSessionID();
        IndigoObject arr = createArray();
        for (IndigoObject obj : coll) arr.arrayAdd(obj);

        return arr;
    }

    public String getUserSpecifiedPath() {
        return path;
    }

    public long getSid() {
        return sid;
    }

    public IndigoObject loadBuffer(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadBuffer(buf, buf.length)));
    }

    public IndigoObject loadString(String string) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, lib.indigoLoadString(string)));
    }

    public IndigoObject iterateSDF(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, lib.indigoIterateSDF(reader.self));
        if (result == 0) return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateRDF(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, lib.indigoIterateRDF(reader.self));
        if (result == 0) return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateCML(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, lib.indigoIterateCML(reader.self));
        if (result == 0) return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateCDX(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, lib.indigoIterateCDX(reader.self));
        if (result == 0) return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateSmiles(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, lib.indigoIterateSmiles(reader.self));
        if (result == 0) return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateTautomers(IndigoObject molecule, String params) {
        setSessionID();
        int result = checkResult(this, lib.indigoIterateTautomers(molecule.self, params));
        if (result == 0) return null;

        return new IndigoObject(this, result, molecule);
    }

    public int buildPkaModel(int level, float threshold, String filename) {
        setSessionID();
        return checkResult(this, lib.indigoBuildPkaModel(level, threshold, filename));
    }

    public IndigoObject nameToStructure(String name) {
        return nameToStructure(name, "");
    }

    public IndigoObject nameToStructure(String name, String params) {
        if (params == null) {
            params = "";
        }
        setSessionID();
        int result = checkResult(this, lib.indigoNameToStructure(name, params));
        if (result == 0) return null;

        return new IndigoObject(this, result);
    }

    public IndigoObject transformHELMtoSCSR(IndigoObject item) {
        setSessionID();
        int result = checkResult(this, lib.indigoTransformHELMtoSCSR(item.self));
        if (result == 0) return null;

        return new IndigoObject(this, result);
    }

    @Override
    public void finalize() throws Throwable {
        if (!sessionReleased()) {
            lib.indigoReleaseSessionId(sid);
            session_released = true;
        }
        super.finalize();
    }
}
