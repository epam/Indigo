using System;
using System.Text;
using System.Runtime.InteropServices;
using System.Collections;
using System.Drawing;

namespace com.epam.indigo
{
    [Flags]
    public enum ReactingCenter
    {
        NOT_CENTER = -1,
        UNMARKED = 0,
        CENTER = 1,
        UNCHANGED = 2,
        MADE_OR_BROKEN = 4,
        ORDER_CHANGED = 8
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public unsafe class Indigo : IDisposable
    {
        public const int ABS = 1;
        public const int OR = 2;
        public const int AND = 3;
        public const int EITHER = 4;
        public const int UP = 5;
        public const int DOWN = 6;
        public const int CIS = 7;
        public const int TRANS = 8;
        public const int CHAIN = 9;
        public const int RING = 10;
        public const int ALLENE = 11;
        public const int SINGLET = 101;
        public const int DOUBLET = 102;
        public const int TRIPLET = 103;

        public const int RC_NOT_CENTER = -1;
        public const int RC_UNMARKED = 0;
        public const int RC_CENTER = 1;
        public const int RC_UNCHANGED = 2;
        public const int RC_MADE_OR_BROKEN = 4;
        public const int RC_ORDER_CHANGED = 8;

        public const int SG_TYPE_GEN = 0;
        public const int SG_TYPE_DAT = 1;
        public const int SG_TYPE_SUP = 2;
        public const int SG_TYPE_SRU = 3;
        public const int SG_TYPE_MUL = 4;
        public const int SG_TYPE_MON = 5;
        public const int SG_TYPE_MER = 6;
        public const int SG_TYPE_COP = 7;
        public const int SG_TYPE_CRO = 8;
        public const int SG_TYPE_MOD = 9;
        public const int SG_TYPE_GRA = 10;
        public const int SG_TYPE_COM = 11;
        public const int SG_TYPE_MIX = 12;
        public const int SG_TYPE_FOR = 13;
        public const int SG_TYPE_ANY = 14;

        public const uint MAX_SIZE = 1000000000;

        private IndigoDllLoader dll_loader;
        
        private long _sid = -1;
        private string _dllpath;
        private int _dll_loader_id;
        public IndigoLib _indigo_lib = null;

        ~Indigo()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (dll_loader.isValid())
            {
                if (_sid >= 0)
                {
                    if (IndigoDllLoader.InstanceId == _dll_loader_id)
                        _indigo_lib.indigoReleaseSessionId(_sid);
                    _sid = -1;
                }
            }
        }

        public void setSessionID()
        {
            _indigo_lib.indigoSetSessionId(_sid);
        }

        public void dbgBreakpoint()
        {
            setSessionID();
            _indigo_lib.indigoDbgBreakpoint();
        }


        public void free(int id)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoFree(id));
        }

        public string getDllPath()
        {
            return _dllpath;
        }

        public string dbgProfiling (bool whole_session)
        {
            setSessionID();
            return checkResult(_indigo_lib.indigoDbgProfiling(whole_session ? 1 : 0));
        }
        
        public void dbgResetProfiling (bool whole_session)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoDbgResetProfiling(whole_session ? 1 : 0));
        }

        private static int strLen(sbyte* input)
        {
            int res = 0;
            do
            {
                if (input[res] == 0)
                {
                    break;
                }
                res++;
            } while (res < MAX_SIZE);
            return res;
        }

        private static string _sbyteToStringUTF8(sbyte* input) 
        {
            return new string(input, 0, strLen(input), Encoding.UTF8);
        }

        private static void _handleError(sbyte* message, Indigo self)
        {
            throw new IndigoException(_sbyteToStringUTF8(message));
        }

        private void init(string lib_path)
        {
            string libraryName;
            string libraryPrefix;
            dll_loader = IndigoDllLoader.Instance;
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                    libraryPrefix = (IntPtr.Size == 8) ? "Win.x64" : "Win.x86";
                    libraryName = string.Format("{0}.{1}", libraryPrefix, "indigo.dll");
                    dll_loader.loadLibrary(string.Format("{0}.{1}", libraryPrefix, "vcruntime140.dll"));
                    dll_loader.loadLibrary(string.Format("{0}.{1}", libraryPrefix, "msvcp140.dll"));
                    dll_loader.loadLibrary(string.Format("{0}.{1}", libraryPrefix, "concrt140.dll"));
                    dll_loader.loadLibrary(libraryName);
                    break;
                case PlatformID.Unix:
                    if (IndigoDllLoader.isMac())
                    {
                        libraryName = "Mac.10.7.libindigo.dylib";
                        dll_loader.loadLibrary(libraryName);
                    }
                    else
                    {
                        libraryPrefix = (IntPtr.Size == 8) ? "Linux.x64" : "Linux.x86";
                        libraryName = string.Format("{0}.{1}", libraryPrefix, "libindigo.so");
                        dll_loader.loadLibrary(libraryName);
                    }
                    break;
                default:
                    throw new PlatformNotSupportedException(String.Format("Unsupported platform: {0}", Environment.OSVersion.Platform));
            }

            // Save instance id to check if session was allocated for this instance
            _dll_loader_id = IndigoDllLoader.InstanceId;

            _dllpath = lib_path;

            _indigo_lib = dll_loader.getInterface<IndigoLib>(libraryName);

            _sid = _indigo_lib.indigoAllocSessionId();
            _indigo_lib.indigoSetSessionId(_sid);
        }

        public float checkResult(float result)
        {
            if (result < 0)
            {
                throw new IndigoException(_sbyteToStringUTF8(_indigo_lib.indigoGetLastError()));
            }

            return result;
        }

        public double checkResult(double result)
        {
            if (result < 0)
            {
                throw new IndigoException(_sbyteToStringUTF8(_indigo_lib.indigoGetLastError()));
            }

            return result;
        }

        public int checkResult(int result)
        {
            if (result < 0)
            {
                throw new IndigoException(_sbyteToStringUTF8(_indigo_lib.indigoGetLastError()));
            }

            return result;
        }

        public string checkResult(sbyte* result)
        {
            if (result == null)
            {
                throw new IndigoException(_sbyteToStringUTF8(_indigo_lib.indigoGetLastError()));
            }

            return _sbyteToStringUTF8(result);
        }

        public float* checkResult(float* result)
        {
            if (result == null)
            {
                throw new IndigoException(_sbyteToStringUTF8(_indigo_lib.indigoGetLastError()));
            }

            return result;
        }

        public int* checkResult(int* result)
        {
            if (result == null)
            {
                throw new IndigoException(_sbyteToStringUTF8(_indigo_lib.indigoGetLastError()));
            }

            return result;
        }

        public long getSID()
        {
            return _sid;
        }

        public string version()
        {
            return checkResult(_indigo_lib.indigoVersion());
        }

        public Indigo(string lib_path)
        {
            init(lib_path);
        }

        public Indigo()
            : this(null)
        {
        }

        public int countReferences()
        {
            setSessionID();
            return checkResult(_indigo_lib.indigoCountReferences());
        }

        public void setOption(string name, string value)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetOption(name, value));
        }

        public void setOption(string name, int x, int y)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetOptionXY(name, x, y));
        }

        public void setOption(string name, bool value)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetOptionBool(name, value ? 1 : 0));
        }

        public void setOption(string name, float value)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetOptionFloat(name, value));
        }

        public void setOption(string name, int value)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetOptionInt(name, value));
        }

        public void setOption(string name, float valuer, float valueg, float valueb)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetOptionColor(name, valuer / 255.0f, valueg / 255.0f, valueb / 255.0f));
        }

        public void setOption(string name, Color value)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetOptionColor(name, value.R / 255.0f, value.G / 255.0f, value.B / 255.0f));
        }


        public string getOption(string option) 
        {
           setSessionID();
           return checkResult(_indigo_lib.indigoGetOption(option));
        }
        
        public int? getOptionInt(string option) 
        {
          setSessionID();
          int res;
          if (checkResult(_indigo_lib.indigoGetOptionInt(option, &res)) == 1) {
             return res;
          }
          return null;
        }
        
        public bool getOptionBool(string option) 
        {
          setSessionID();
          int res;
          checkResult(_indigo_lib.indigoGetOptionBool(option, &res));
          return res > 0;
        }
        
        public float? getOptionFloat(string option) 
        {
          setSessionID();
          float res;
          if (checkResult(_indigo_lib.indigoGetOptionFloat(option, &res)) == 1) {
             return res;
          }
          return null;
        }
        
        public string getOptionType(string option) 
        {
           setSessionID();
           return checkResult(_indigo_lib.indigoGetOptionType(option));
        }        

        public void resetOptions()
        {
            setSessionID();
            checkResult(_indigo_lib.indigoResetOptions());
        }

        public IndigoObject writeFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoWriteFile(filename)));
        }

        public IndigoObject writeBuffer()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoWriteBuffer()));
        }

        public IndigoObject createMolecule()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoCreateMolecule()));
        }

        public IndigoObject createQueryMolecule()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoCreateQueryMolecule()));
        }

        public IndigoObject loadMolecule(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadMoleculeFromString(str)));
        }

        public IndigoObject loadMolecule(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadMoleculeFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadMoleculeFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadMoleculeFromFile(path)));
        }

        public IndigoObject loadMoleculeFromBuffer(byte[] buf) {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadMoleculeFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadQueryMolecule(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadQueryMoleculeFromString(str)));
        }

        public IndigoObject loadQueryMolecule(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadQueryMoleculeFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadQueryMoleculeFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadQueryMoleculeFromFile(path)));
        }

        public IndigoObject loadSmarts(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadSmartsFromString(str)));
        }

        public IndigoObject loadSmarts(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadSmartsFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadSmartsFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadSmartsFromFile(path)));
        }

        public IndigoObject loadReaction(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadReactionFromString(str)));
        }

        public IndigoObject loadReaction(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadReactionFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadReactionFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadReactionFromFile(path)));
        }

        public IndigoObject loadQueryReaction(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadQueryReactionFromString(str)));
        }

        public IndigoObject loadQueryReaction(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadQueryReactionFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadQueryReactionFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadQueryReactionFromFile(path)));
        }

        public IndigoObject loadReactionSmarts(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadReactionSmartsFromString(str)));
        }

        public IndigoObject loadReactionSmarts(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadReactionSmartsFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadReactionSmartsFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadReactionSmartsFromFile(path)));
        }

        public string checkStructure(string str) 
        {
            return checkStructure(str, "");
        }

        public string checkStructure(string str, string options)
        {
            setSessionID();
            return checkResult(_indigo_lib.indigoCheckStructure(str, options));
        }

        public IndigoObject loadStructure(string str) 
        {
            return loadStructure(str, "");
        }

        public IndigoObject loadStructure(string str, string options)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadStructureFromString(str, options)));
        }

        public IndigoObject loadStructure(byte[] buf) 
        {
            return loadStructure(buf, "");
        }

        public IndigoObject loadStructure(byte[] buf, string options)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadStructureFromBuffer(buf, buf.Length, options)));
        }

        public IndigoObject loadStructureFromFile(string path) 
        {
            return loadStructureFromFile(path, "");
        }

        public IndigoObject loadStructureFromFile(string path, string options)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadStructureFromFile(path, options)));
        }

        public IndigoObject loadStructureFromBuffer(byte[] buf) {
            return loadStructureFromBuffer(buf, "");
        }

        public IndigoObject loadStructureFromBuffer(byte[] buf, string options) {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadStructureFromBuffer(buf, buf.Length, options)));
        }

        public IndigoObject loadFingerprintFromBuffer(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadFingerprintFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadFingerprintFromDescriptors(double[] descriptors, int size, double density)
        {
            setSessionID();
            int result = _indigo_lib.indigoLoadFingerprintFromDescriptors(descriptors, descriptors.Length, size, density);
            return new IndigoObject(this, checkResult(result));
        }

        public IndigoObject createReaction()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoCreateReaction()));
        }

        public IndigoObject createQueryReaction()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoCreateQueryReaction()));
        }

        public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2, string flags)
        {
            if (flags == null)
                flags = "";

            setSessionID();
            int match = checkResult(_indigo_lib.indigoExactMatch(obj1.self, obj2.self, flags));

            if (match == 0)
                return null;
            return new IndigoObject(this, match, new IndigoObject[] { obj1, obj2 });
        }

        public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2)
        {
            return exactMatch(obj1, obj2, "");
        }

        public void setTautomerRule(int id, string beg, string end)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoSetTautomerRule(id, beg, end));
        }

        public void removeTautomerRule(int id)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoRemoveTautomerRule(id));
        }

        public void clearTautomerRules()
        {
            setSessionID();
            checkResult(_indigo_lib.indigoClearTautomerRules());
        }

        public IndigoObject unserialize(byte[] buf)
        {
            return new IndigoObject(this, checkResult(_indigo_lib.indigoUnserialize(buf, buf.Length)));
        }

        public IndigoObject createArray()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoCreateArray()));
        }

        public float similarity(IndigoObject obj1, IndigoObject obj2)
        {
            return similarity(obj1, obj2, "");
        }

        public float similarity(IndigoObject obj1, IndigoObject obj2, string metrics)
        {
            setSessionID();
            if (metrics == null)
                metrics = "";
            return checkResult(_indigo_lib.indigoSimilarity(obj1.self, obj2.self, metrics));
        }

        public int commonBits(IndigoObject obj1, IndigoObject obj2)
        {
            setSessionID();
            return checkResult(_indigo_lib.indigoCommonBits(obj1.self, obj2.self));
        }

        public IndigoObject iterateSDFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoIterateSDFile(filename)));
        }

        public IndigoObject iterateRDFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoIterateRDFile(filename)));
        }

        public IndigoObject iterateSmilesFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoIterateSmilesFile(filename)));
        }

        public IndigoObject iterateCMLFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoIterateCMLFile(filename)));
        }

        public IndigoObject iterateCDXFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoIterateCDXFile(filename)));
        }

        public IndigoObject substructureMatcher(IndigoObject target, string mode)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoSubstructureMatcher(target.self, mode)), target);
        }

        public IndigoObject substructureMatcher(IndigoObject target)
        {
            return substructureMatcher(target, "");
        }

        public IndigoObject extractCommonScaffold(IndigoObject structures, string options)
        {
            setSessionID();
            int res = checkResult(_indigo_lib.indigoExtractCommonScaffold(structures.self, options));
            if (res == 0)
                return null;
            return new IndigoObject(this, res);
        }

        public IndigoObject rgroupComposition(IndigoObject structures, string options)
        {
            setSessionID();
            int res = checkResult(_indigo_lib.indigoRGroupComposition(structures.self, options));
            if (res == 0)
                return null;
            return new IndigoObject(this, res);
        }

        public IndigoObject getFragmentedMolecule(IndigoObject structures, string options)
        {
            setSessionID();
            int res = checkResult(_indigo_lib.indigoGetFragmentedMolecule(structures.self, options));
            if (res == 0)
                return null;
            return new IndigoObject(this, res);
        }

        public IndigoObject toIndigoArray(IEnumerable collection)
        {
            setSessionID();

            IndigoObject arr = createArray();
            foreach (IndigoObject obj in collection)
                arr.arrayAdd(obj);

            return arr;
        }

        public static int[] toIntArray(ICollection collection)
        {
            if (collection == null)
                return new int[0];

            int[] res = new int[collection.Count];
            int i = 0;

            foreach (object x in collection)
                res[i++] = Convert.ToInt32(x);

            return res;
        }

        public static float[] toFloatArray(ICollection collection)
        {
            if (collection == null)
                return new float[0];

            float[] res = new float[collection.Count];
            int i = 0;

            foreach (object x in collection)
                res[i++] = Convert.ToSingle(x);

            return res;
        }

        public IndigoObject extractCommonScaffold(IEnumerable structures, string options)
        {
            return extractCommonScaffold(toIndigoArray(structures), options);
        }

        public IndigoObject decomposeMolecules(IndigoObject scaffold, IndigoObject structures)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoDecomposeMolecules(scaffold.self, structures.self)));
        }

        public IndigoObject decomposeMolecules(IndigoObject scaffold, IEnumerable structures)
        {
            return decomposeMolecules(scaffold, toIndigoArray(structures));
        }

        public IndigoObject createDecomposer(IndigoObject scaffold)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoCreateDecomposer(scaffold.self)));
        }

        public IndigoObject reactionProductEnumerate(IndigoObject reaction, IndigoObject monomers)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoReactionProductEnumerate(reaction.self, monomers.self)));
        }

        public IndigoObject reactionProductEnumerate(IndigoObject reaction, IEnumerable monomers)
        {
            IndigoObject indigoArrayArray = createArray();
            foreach (IEnumerable iter in monomers) {
                IndigoObject indigoArray = createArray();
                foreach(IndigoObject monomer in iter) {
                    indigoArray.arrayAdd(monomer);
                }
                indigoArrayArray.arrayAdd(indigoArray);
            }
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoReactionProductEnumerate(reaction.self, indigoArrayArray.self)));
        }

        public IndigoObject createSaver(IndigoObject output, string format)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoCreateSaver(output.self, format)), output);
        }

        public IndigoObject createFileSaver(string filename, string format)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(checkResult(_indigo_lib.indigoCreateFileSaver(filename, format))));
        }

        public void transform(IndigoObject reaction, IndigoObject monomer)
        {
            setSessionID();
            checkResult(_indigo_lib.indigoTransform(reaction.self, monomer.self));
        }

        public IndigoObject loadBuffer(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadBuffer(buf, buf.Length)));
        }

        public IndigoObject loadString(string s)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(_indigo_lib.indigoLoadString(s)));
        }

        public IndigoObject iterateSDF(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoIterateSDF(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateRDF(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoIterateRDF(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateCML(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoIterateCML(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateCDX(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoIterateCDX(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateSmiles(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoIterateSmiles(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }
        public IndigoObject tautomerEnumerate(IndigoObject molecule, string parameters)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoIterateTautomers(molecule.self, parameters));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, molecule);
        }

        public IndigoObject transformHELMtoSCSR(IndigoObject item)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoTransformHELMtoSCSR(item.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result);
        }

        public IndigoObject iterateTautomers(IndigoObject molecule, string parameters)
        {
            setSessionID();
            int result = checkResult(_indigo_lib.indigoIterateTautomers(molecule.self, parameters));
            if (result == 0)
                return null;

            return new IndigoObject(this, result, molecule);
        }

        public int buildPkaModel(int level, float threshold, String filename) {
            setSessionID();
            return checkResult(_indigo_lib.indigoBuildPkaModel(level, threshold, filename));
        }

        public IndigoObject nameToStructure(string name, string parameters) 
        {
            if (parameters == null) {
                parameters = "";
            }
            setSessionID();
            int result = checkResult(_indigo_lib.indigoNameToStructure(name, parameters));
            if(result == 0)
                return null;

            return new IndigoObject(this, result);
        }

        public IndigoObject nameToStructure(string name) 
        {
            return nameToStructure(name, "");
        }
    }
}
