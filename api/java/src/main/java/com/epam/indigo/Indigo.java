/****************************************************************************
 * Copyright (C) 2009-2016 EPAM Systems
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

package com.epam.indigo;

import com.sun.jna.Native;
import com.sun.jna.Pointer;

import java.io.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
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
    public static final int OS_WINDOWS = 1;
    public static final int OS_MACOS = 2;
    public static final int OS_LINUX = 3;
    public static final int OS_SOLARIS = 4;
    private boolean _session_released = false;
    private static int _os = 0;
    private static String _dllpath = "";
    private static IndigoLib _lib = null;
    private String _path;
    private long _sid;

    public Indigo(String path) {
        _path = path;
        loadIndigo(path);
        System.setProperty("jna.encoding", "UTF-8");

        _sid = _lib.indigoAllocSessionId();
    }

    public Indigo() {
        this(null);
    }

    static public int checkResult(Object obj, int result) {
        if (result < 0)
            throw new IndigoException(obj, _lib.indigoGetLastError());
        return result;
    }

    static public int checkResult(Object obj, Object obj2, int result) {
        if (result < 0)
            throw new IndigoException(new Object[]{obj, obj2}, _lib.indigoGetLastError());
        return result;
    }

    static public float checkResultFloat(Object obj, float result) {
        if (result < 0)
            throw new IndigoException(obj, _lib.indigoGetLastError());
        return result;
    }

    static public double checkResultDouble(Object obj, double result) {
        if (result < 0)
            throw new IndigoException(obj, _lib.indigoGetLastError());
        return result;
    }

    static public String checkResultString(Object obj, Pointer result) {
        if (result == Pointer.NULL)
            throw new IndigoException(obj, _lib.indigoGetLastError());
        return result.getString(0);
    }

    static public Pointer checkResultPointer(Object obj, Pointer result) {
        if (result == Pointer.NULL)
            throw new IndigoException(obj, _lib.indigoGetLastError());

        return result;
    }

    public static int[] toIntArray(Collection<Integer> collection) {
        if (collection == null)
            return new int[0];

        int[] res = new int[collection.size()];
        int i = 0;

        for (Integer x : collection)
            res[i++] = x.intValue();

        return res;
    }

    public static float[] toFloatArray(Collection<Float> collection) {
        if (collection == null)
            return new float[0];

        float[] res = new float[collection.size()];
        int i = 0;

        for (Float x : collection)
            res[i++] = x.floatValue();

        return res;
    }

    private static String getHashString(InputStream input) throws NoSuchProviderException, NoSuchAlgorithmException, IOException {
        String res = "";
        MessageDigest algorithm = MessageDigest.getInstance("MD5");
        algorithm.reset();
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        int nRead;
        byte[] data = new byte[4096];

        while ((nRead = input.read(data, 0, data.length)) != -1) {
            buffer.write(data, 0, nRead);
        }
        buffer.flush();

        algorithm.update(buffer.toByteArray());
        byte[] hashArray = algorithm.digest();
        String tmp = "";
        for (int i = 0; i < hashArray.length; i++) {
            tmp = (Integer.toHexString(0xFF & hashArray[i]));
            if (tmp.length() == 1) {
                res += "0" + tmp;
            } else {
                res += tmp;
            }
        }
        return res;
    }

    public static String extractFromJar(Class cls, String path, String filename) {
        InputStream stream = cls.getResourceAsStream(path + "/" + filename);

        if (stream == null)
            return null;

        String tmpdir_path;
        final File tmpdir;
        final File dllfile;

        try {
            // Clone input stream to calculate its hash and copy to temporary folder
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buffer = new byte[4096];
            int len;
            while ((len = stream.read(buffer)) > -1 ) {
                baos.write(buffer, 0, len);
            }
            baos.flush();
            InputStream is1 = new ByteArrayInputStream(baos.toByteArray());
            InputStream is2 = new ByteArrayInputStream(baos.toByteArray());
            baos.close();

            // Calculate md5 hash string to name temporary folder
            String streamHashString = getHashString(is1);
            is1.close();
            tmpdir_path = System.getProperty("java.io.tmpdir") + File.separator + "indigo" + streamHashString;
            tmpdir = new File(tmpdir_path);
            if (!tmpdir.exists()) {
                if (!tmpdir.mkdir()) {
                    return null;
                }
            }

            // Copy library to temporary folder
            dllfile = new File(tmpdir.getAbsoluteFile() + File.separator + filename);
            if (!dllfile.exists()) {
                FileOutputStream outstream = new FileOutputStream(dllfile);
                byte buf[] = new byte[4096];

                while ((len = is2.read(buf)) > 0)
                    outstream.write(buf, 0, len);

                outstream.close();
                is2.close();
            }
        } catch (IOException e) {
            return null;
        } catch (NoSuchAlgorithmException e) {
            return null;
        } catch (NoSuchProviderException e) {
            return null;
        }

        String p;

        try {
            p = dllfile.getCanonicalPath();
        } catch (IOException e) {
            return null;
        }

        final String fullpath = p;

        return fullpath;
    }

    private static String getPathToBinary(String path, String filename) {
        if (path == null) {
            String res = extractFromJar(Indigo.class, "/" + _dllpath, filename);
            if (res != null)
                return res;
            path = "lib";
        }
        path = path + File.separator + _dllpath + File.separator + filename;
        try {
            return (new File(path)).getCanonicalPath();
        } catch (IOException e) {
            return path;
        }
    }

    private synchronized static void loadIndigo(String path) {
        if (_lib != null)
            return;

        if (_os == OS_LINUX || _os == OS_SOLARIS)
            _lib = (IndigoLib) Native.loadLibrary(getPathToBinary(path, "libindigo.so"), IndigoLib.class);
        else if (_os == OS_MACOS)
            _lib = (IndigoLib) Native.loadLibrary(getPathToBinary(path, "libindigo.dylib"), IndigoLib.class);
        else // _os == OS_WINDOWS
        {
            if ((new File(getPathToBinary(path, "msvcr120.dll"))).exists()) {
                try {
                    System.load(getPathToBinary(path, "msvcr120.dll"));
                } catch (UnsatisfiedLinkError e) {
                    // File could have been already loaded
                }
            }
            if ((new File(getPathToBinary(path, "msvcp120.dll"))).exists()) {
                try {
                    System.load(getPathToBinary(path, "msvcp120.dll"));
                } catch (UnsatisfiedLinkError e) {
                    // File could have been already loaded
                }
            }
            if ((new File(getPathToBinary(path, "vcruntime140.dll"))).exists()) {
                try {
                    System.load(getPathToBinary(path, "vcruntime140.dll"));
                } catch (UnsatisfiedLinkError e) {
                    // File could have been already loaded
                }
            }
            if ((new File(getPathToBinary(path, "msvcp140.dll"))).exists()) {
                try {
                    System.load(getPathToBinary(path, "msvcp140.dll"));
                } catch (UnsatisfiedLinkError e) {
                    // File could have been already loaded
                }
            }
            _lib = (IndigoLib) Native.loadLibrary(getPathToBinary(path, "indigo.dll"), IndigoLib.class);
        }
    }

    static public String getPlatformDependentPath() {
        return _dllpath;
    }

    public boolean sessionReleased() {
        return _session_released;
    }

    public static IndigoLib getLibrary() {
        return _lib;
    }

    public static int getOs() {
        String namestr = System.getProperty("os.name");
        if (namestr.matches("^Windows.*"))
            return OS_WINDOWS;
        else if (namestr.matches("^Mac OS.*"))
            return OS_MACOS;
        else if (namestr.matches("^Linux.*"))
            return OS_LINUX;
        else if (namestr.matches("^SunOS.*"))
            return OS_SOLARIS;
        else
            throw new Error("Operating system not recognized");
    }

    private static String getDllPath() {
        String path = "";
        switch (_os) {
            case OS_WINDOWS:
                path += "Win";
                break;
            case OS_LINUX:
                path += "Linux";
                break;
            case OS_SOLARIS:
                path += "Sun";
                break;
            case OS_MACOS:
                path += "Mac";
                break;
            default:
                throw new Error("OS not set");
        }
        path += "/";

        if (_os == OS_MACOS) {
            String version = System.getProperty("os.version");
            int minorVersion = Integer.parseInt(version.split("\\.")[1]);
            Integer usingVersion = null;

            for (int i = minorVersion; i >= 5; i--) {
                if (Indigo.class.getResourceAsStream("/" + path + "10." + i + "/libindigo.dylib") != null) {
                    usingVersion = i;
                    break;
                }
            }
            if (usingVersion == null) {
                throw new Error("Indigo cannot find native libraries for Mac OS X 10." + minorVersion);
            }
            path += "10." + usingVersion;
        } else if (_os == OS_SOLARIS) {
            String model = System.getProperty("sun.arch.data.model");

            if (model.equals("32"))
                path += "sparc32";
            else
                path += "sparc64";
        } else {
            String archstr = System.getProperty("os.arch");
            if (archstr.equals("x86") || archstr.equals("i386"))
                path += "x86";
            else if (archstr.equals("x86_64") || archstr.equals("amd64"))
                path += "x64";
            else
                throw new Error("architecture not recognized");
        }

        return path;
    }

    static {
        _os = getOs();
        _dllpath = getDllPath();
    }

    public String version() {
        return _lib.indigoVersion();
    }

    public int countReferences() {
        setSessionID();
        return checkResult(this, _lib.indigoCountReferences());
    }

    public void setSessionID() {
        _lib.indigoSetSessionId(_sid);
    }

    public void setOption(String option, String value) {
        setSessionID();
        checkResult(this, _lib.indigoSetOption(option, value));
    }

    public void setOption(String option, int value) {
        setSessionID();
        checkResult(this, _lib.indigoSetOptionInt(option, value));
    }

    public void setOption(String option, int x, int y) {
        setSessionID();
        checkResult(this, _lib.indigoSetOptionXY(option, x, y));
    }

    public void setOption(String option, float r, float g, float b) {
        setSessionID();
        checkResult(this, _lib.indigoSetOptionColor(option, r, g, b));
    }

    public void setOption(String option, boolean value) {
        setSessionID();
        checkResult(this, _lib.indigoSetOptionBool(option, value ? 1 : 0));
    }

    public void setOption(String option, float value) {
        setSessionID();
        checkResult(this, _lib.indigoSetOptionFloat(option, value));
    }

    public void setOption(String option, double value) {
        setSessionID();
        checkResult(this, _lib.indigoSetOptionFloat(option, (float) value));
    }

    public void resetOptions() {
        setSessionID();
        checkResult(this, _lib.indigoResetOptions());
    }

    public IndigoObject writeFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoWriteFile(filename)));
    }

    public IndigoObject writeBuffer() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoWriteBuffer()));
    }

    public IndigoObject createMolecule() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoCreateMolecule()));
    }

    public IndigoObject createQueryMolecule() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoCreateQueryMolecule()));
    }

    public IndigoObject loadMolecule(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadMoleculeFromString(str)));
    }

    public IndigoObject loadMolecule(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadMoleculeFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadMoleculeFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadMoleculeFromFile(path)));
    }

    public IndigoObject loadMoleculeFromBuffer(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadMoleculeFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadQueryMolecule(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryMoleculeFromString(str)));
    }

    public IndigoObject loadQueryMolecule(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryMoleculeFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadQueryMoleculeFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryMoleculeFromFile(path)));
    }

    public IndigoObject loadSmarts(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadSmartsFromString(str)));
    }

    public IndigoObject loadSmarts(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadSmartsFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadSmartsFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadSmartsFromFile(path)));
    }

    public IndigoObject loadReaction(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionFromString(str)));
    }

    public IndigoObject loadReaction(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadReactionFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionFromFile(path)));
    }

    public IndigoObject loadQueryReaction(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryReactionFromString(str)));
    }

    public IndigoObject loadQueryReaction(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryReactionFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadQueryReactionFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadQueryReactionFromFile(path)));
    }

    public IndigoObject loadReactionSmarts(String str) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionSmartsFromString(str)));
    }

    public IndigoObject loadReactionSmarts(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionSmartsFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadReactionSmartsFromFile(String path) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadReactionSmartsFromFile(path)));
    }

    public IndigoObject loadStructure(String str, String params) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadStructureFromString(str, params)));
    }

    public IndigoObject loadStructure(byte[] buf, String params) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadStructureFromBuffer(buf, buf.length, params)));
    }

    public IndigoObject loadStructureFromFile(String path, String params) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadStructureFromFile(path, params)));
    }

    public IndigoObject loadStructureFromBuffer(byte[] buf, String params) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadStructureFromBuffer(buf, buf.length, params)));
    }

    public IndigoObject loadFingerprintFromBuffer (byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadFingerprintFromBuffer(buf, buf.length)));
    }

    public IndigoObject loadFingerprintFromDescriptors(double[] descriptors, int size, double density) {
        setSessionID();
        int result = _lib.indigoLoadFingerprintFromDescriptors(descriptors, descriptors.length, size, density);
        return new IndigoObject(this, checkResult(this, result));
    }

    public IndigoObject createReaction() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoCreateReaction()));
    }

    public IndigoObject createQueryReaction() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoCreateQueryReaction()));
    }

    public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2, String flags) {
        if (flags == null)
            flags = "";

        IndigoObject[] parent = new IndigoObject[]{obj1, obj2};
        setSessionID();
        int match = checkResult(this, parent, _lib.indigoExactMatch(obj1.self, obj2.self, flags));

        if (match == 0)
            return null;

        return new IndigoObject(this, match, parent);
    }

    public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2) {
        return exactMatch(obj1, obj2, "");
    }

    public void setTautomerRule(int id, String beg, String end) {
        setSessionID();
        checkResult(this, _lib.indigoSetTautomerRule(id, beg, end));
    }

    public void removeTautomerRule(int id) {
        setSessionID();
        checkResult(this, _lib.indigoRemoveTautomerRule(id));
    }

    public void clearTautomerRules() {
        setSessionID();
        checkResult(this, _lib.indigoClearTautomerRules());
    }

    public float similarity(IndigoObject obj1, IndigoObject obj2) {
        return similarity(obj1, obj2, "");
    }

    public float similarity(IndigoObject obj1, IndigoObject obj2, String metrics) {
        if (metrics == null)
            metrics = "";
        Object[] guard = new Object[]{this, obj1, obj2};
        setSessionID();
        return checkResultFloat(guard, _lib.indigoSimilarity(obj1.self, obj2.self, metrics));
    }

    public int commonBits(IndigoObject fingerprint1, IndigoObject fingerprint2) {
        Object[] guard = new Object[]{this, fingerprint1, fingerprint2};
        setSessionID();
        return checkResult(guard, _lib.indigoCommonBits(fingerprint1.self, fingerprint2.self));
    }

    public IndigoObject unserialize(byte[] data) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoUnserialize(data, data.length)));
    }

    public IndigoObject createArray() {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoCreateArray()));
    }

    public IndigoObject iterateSDFile(String filename) {
        setSessionID();
        int result = checkResult(this, _lib.indigoIterateSDFile(filename));
        if (result == 0)
            return null;

        return new IndigoObject(this, result);
    }

    public IndigoObject iterateRDFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoIterateRDFile(filename)));
    }

    public IndigoObject iterateSmilesFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoIterateSmilesFile(filename)));
    }

    public IndigoObject iterateCMLFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoIterateCMLFile(filename)));
    }

    public IndigoObject iterateCDXFile(String filename) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoIterateCDXFile(filename)));
    }

    public IndigoObject substructureMatcher(IndigoObject target, String mode) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, target, _lib.indigoSubstructureMatcher(target.self, mode)), target);
    }

    public IndigoObject substructureMatcher(IndigoObject target) {
        return substructureMatcher(target, "");
    }

    public IndigoObject extractCommonScaffold(IndigoObject structures, String options) {
        setSessionID();
        int res = checkResult(this, structures, _lib.indigoExtractCommonScaffold(structures.self, options));

        if (res == 0)
            return null;

        return new IndigoObject(this, res);
    }

    public IndigoObject extractCommonScaffold(Collection<IndigoObject> structures, String options) {
        return extractCommonScaffold(toIndigoArray(structures), options);
    }

    public IndigoObject rgroupComposition(IndigoObject molecule, String options) {
        setSessionID();
        int res = checkResult(this, molecule, _lib.indigoRGroupComposition(molecule.self, options));
        if (res == 0)
            return null;
        return new IndigoObject(this, res);
    }

    public IndigoObject getFragmentedMolecule(IndigoObject molecule, String options) {
        setSessionID();
        int res = checkResult(this, molecule, _lib.indigoGetFragmentedMolecule(molecule.self, options));
        if (res == 0)
            return null;
        return new IndigoObject(this, res);
    }

    /**
     * Use createDecomposer() and decomposeMolecule()
     */
    @Deprecated
    public IndigoObject decomposeMolecules(IndigoObject scaffold, IndigoObject structures) {
        Object[] guard = new Object[]{this, scaffold, structures};
        setSessionID();
        int res = checkResult(guard, _lib.indigoDecomposeMolecules(scaffold.self, structures.self));

        if (res == 0)
            return null;

        return new IndigoObject(this, res);
    }

    /**
     * Use createDecomposer() and decomposeMolecule()
     */
    @Deprecated
    public IndigoObject decomposeMolecules(IndigoObject scaffold, Collection<IndigoObject> structures) {
        return decomposeMolecules(scaffold, toIndigoArray(structures));
    }

    public IndigoObject createDecomposer(IndigoObject scaffold) {
        Object[] guard = new Object[]{this, scaffold};
        setSessionID();
        int res = checkResult(guard, _lib.indigoCreateDecomposer(scaffold.self));

        if (res == 0)
            return null;

        return new IndigoObject(this, res);
    }

    public IndigoObject reactionProductEnumerate(IndigoObject reaction, IndigoObject monomers) {
        Object[] guard = new Object[]{this, reaction, monomers};
        setSessionID();
        int res = checkResult(guard, _lib.indigoReactionProductEnumerate(reaction.self, monomers.self));

        if (res == 0)
            return null;

        return new IndigoObject(this, res);
    }

    public IndigoObject reactionProductEnumerate(IndigoObject reaction, Iterable<Iterable> monomers) {
        Object[] guard = new Object[]{this, reaction, monomers};

        IndigoObject monomersArrayArray = createArray();
        for (Iterable<IndigoObject> iter: monomers) {
            IndigoObject monomersArray = createArray();
            for (IndigoObject monomer: iter) {
                monomersArray.arrayAdd(monomer);
            }
            monomersArrayArray.arrayAdd(monomersArray);
        }
        setSessionID();
        int res = checkResult(guard, _lib.indigoReactionProductEnumerate(reaction.self, monomersArrayArray.self));
        if (res == 0)
            return null;

        return new IndigoObject(this, res);
    }

    public void transform(IndigoObject reaction, IndigoObject monomer) {
        Object[] guard = new Object[]{this, reaction, monomer};
        setSessionID();
        checkResult(guard, _lib.indigoTransform(reaction.self, monomer.self));
    }

    public IndigoObject createSaver(IndigoObject output, String format) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, output, _lib.indigoCreateSaver(output.self, format)), output);
    }

    public IndigoObject createFileSaver(String filename, String format) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoCreateFileSaver(filename, format)));
    }

    public void dbgBreakpoint() {
        setSessionID();
        _lib.indigoDbgBreakpoint();
    }

    public IndigoObject toIndigoArray(Collection<IndigoObject> coll) {
        setSessionID();
        IndigoObject arr = createArray();
        for (IndigoObject obj : coll)
            arr.arrayAdd(obj);

        return arr;
    }

    public String getUserSpecifiedPath() {
        return _path;
    }

    public long getSid() {
        return _sid;
    }

    public IndigoObject loadBuffer(byte[] buf) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadBuffer(buf, buf.length)));
    }

    public IndigoObject loadString(String string) {
        setSessionID();
        return new IndigoObject(this, checkResult(this, _lib.indigoLoadString(string)));
    }

    public IndigoObject iterateSDF(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, _lib.indigoIterateSDF(reader.self));
        if (result == 0)
            return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateRDF(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, _lib.indigoIterateRDF(reader.self));
        if (result == 0)
            return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateCML(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, _lib.indigoIterateCML(reader.self));
        if (result == 0)
            return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateCDX(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, _lib.indigoIterateCDX(reader.self));
        if (result == 0)
            return null;

        return new IndigoObject(this, result, reader);
    }

    public IndigoObject iterateSmiles(IndigoObject reader) {
        setSessionID();
        int result = checkResult(this, _lib.indigoIterateSmiles(reader.self));
        if (result == 0)
            return null;

        return new IndigoObject(this, result, reader);
    }
    public IndigoObject iterateTautomers(IndigoObject molecule, String params) {
        setSessionID();
        int result = checkResult(this, _lib.indigoIterateTautomers(molecule.self, params));
        if (result == 0)
            return null;

        return new IndigoObject(this, result, molecule);
    }

    public int buildPkaModel(int level, float threshold, String filename) {
        setSessionID();
        return checkResult(this, _lib.indigoBuildPkaModel(level, threshold, filename));
    }

    public IndigoObject nameToStructure(String name) {
        return nameToStructure(name, "");
    }

    public IndigoObject nameToStructure(String name, String params) {
        if (params == null) {
            params = "";
        }
        setSessionID();
        int result = checkResult(this, _lib.indigoNameToStructure(name, params));
        if(result == 0)
            return null;

        return new IndigoObject(this, result);
    }

    public IndigoObject transformHELMtoSCSR(IndigoObject item) {
        setSessionID();
        int result = checkResult(this, _lib.indigoTransformHELMtoSCSR(item.self));
        if (result == 0)
            return null;

        return new IndigoObject(this, result);
    }

    @Override
    protected void finalize() throws Throwable {
        if (!sessionReleased()) {
            _lib.indigoReleaseSessionId(_sid);
            _session_released = true;
        }
        super.finalize();
    }
}
