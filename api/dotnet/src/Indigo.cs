using System;
using System.Collections;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;

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

    public enum Hybridization
    {
        S = 1,
        SP = 2,
        SP2 = 3,
        SP3 = 4,
        SP3D = 5,
        SP3D2 = 6,
        SP3D3 = 7,
        SP3D4 = 8,
        SP2D = 9
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

        public const uint CSTRING_MAX_SIZE = 1000000000;


        private long _sid = -1;
        private readonly string _dllpath;
        private readonly int _dll_loader_id;

        ~Indigo()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (_sid >= 0)
            {
                IndigoLib.indigoReleaseSessionId(_sid);
                _sid = -1;
            }
        }

        public void setSessionID()
        {
            IndigoLib.indigoSetSessionId(_sid);
        }

        public void dbgBreakpoint()
        {
            setSessionID();
            IndigoLib.indigoDbgBreakpoint();
        }


        public void free(int id)
        {
            setSessionID();
            checkResult(IndigoLib.indigoFree(id));
        }

        public string getDllPath()
        {
            return _dllpath;
        }

        public string dbgProfiling(bool whole_session)
        {
            setSessionID();
            return checkResult(IndigoLib.indigoDbgProfiling(whole_session ? 1 : 0));
        }

        public void dbgResetProfiling(bool whole_session)
        {
            setSessionID();
            checkResult(IndigoLib.indigoDbgResetProfiling(whole_session ? 1 : 0));
        }

        private static int strLen(byte* input)
        {
            var res = 0;
            do
            {
                if (input[res] == 0)
                {
                    break;
                }
                res++;
            } while (res < CSTRING_MAX_SIZE);

            if (res == CSTRING_MAX_SIZE)
            {
                throw new ArgumentException("Could not find terminate zero in c-style string");
            }

            return res;
        }

        public static string bytePtrToStringUtf8(byte* input)
        {
            int len = strLen(input);
            byte[] bytes = new byte[len];
            Marshal.Copy((IntPtr)input, bytes, 0, len);
            return Encoding.UTF8.GetString(bytes);
        }

        private static void _handleError(byte* message, Indigo self)
        {
            throw new IndigoException(bytePtrToStringUtf8(message));
        }

        private void init(string lib_path)
        {
            _sid = IndigoLib.indigoAllocSessionId();
            setSessionID();
        }

        public float checkResult(float result)
        {
            if (result < 0)
            {
                throw new IndigoException(bytePtrToStringUtf8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public double checkResult(double result)
        {
            if (result < 0)
            {
                throw new IndigoException(bytePtrToStringUtf8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public int checkResult(int result)
        {
            if (result < 0)
            {
                throw new IndigoException(bytePtrToStringUtf8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public long checkResult(long result)
        {
            if (result < 0)
            {
                throw new IndigoException(bytePtrToStringUtf8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public string checkResult(byte* result)
        {
            if (result == null)
            {
                throw new IndigoException(bytePtrToStringUtf8(IndigoLib.indigoGetLastError()));
            }

            return bytePtrToStringUtf8(result);
        }

        public float* checkResult(float* result)
        {
            if (result == null)
            {
                throw new IndigoException(bytePtrToStringUtf8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public int* checkResult(int* result)
        {
            if (result == null)
            {
                throw new IndigoException(bytePtrToStringUtf8(IndigoLib.indigoGetLastError()));
            }

            return result;
        }

        public long getSID()
        {
            return _sid;
        }

        public string version()
        {
            return checkResult(IndigoLib.indigoVersion());
        }

        public string versionInfo()
        {
            return checkResult(IndigoLib.indigoVersionInfo());
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
            return checkResult(IndigoLib.indigoCountReferences());
        }

        public void setOption(string name, string value)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOption(name, value));
        }

        public void setOption(string name, int x, int y)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionXY(name, x, y));
        }

        public void setOption(string name, bool value)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionBool(name, value ? 1 : 0));
        }

        public void setOption(string name, float value)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionFloat(name, value));
        }

        public void setOption(string name, double value)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionFloat(name, (float) value));
        }

        public void setOption(string name, int value)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionInt(name, value));
        }

        public void setOption(string name, float valuer, float valueg, float valueb)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionColor(name, valuer, valueg, valueb));
        }

        public void setOption(string name, double valuer, double valueg, double valueb)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionColor(name, (float) valuer, (float) valueg, (float) valueb));
        }

        public void setOption(string name, Color value)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetOptionColor(name, value.R / 255.0f, value.G / 255.0f, value.B / 255.0f));
        }


        public string getOption(string option)
        {
            setSessionID();
            return checkResult(IndigoLib.indigoGetOption(option));
        }

        public int? getOptionInt(string option)
        {
            setSessionID();
            int res;
            if (checkResult(IndigoLib.indigoGetOptionInt(option, &res)) == 1)
            {
                return res;
            }
            return null;
        }

        public bool getOptionBool(string option)
        {
            setSessionID();
            int res;
            checkResult(IndigoLib.indigoGetOptionBool(option, &res));
            return res > 0;
        }

        public float? getOptionFloat(string option)
        {
            setSessionID();
            float res;
            if (checkResult(IndigoLib.indigoGetOptionFloat(option, &res)) == 1)
            {
                return res;
            }
            return null;
        }

        public string getOptionType(string option)
        {
            setSessionID();
            return checkResult(IndigoLib.indigoGetOptionType(option));
        }

        public void resetOptions()
        {
            setSessionID();
            checkResult(IndigoLib.indigoResetOptions());
        }

        public IndigoObject writeFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoWriteFile(filename)));
        }

        public IndigoObject writeBuffer()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoWriteBuffer()));
        }

        public IndigoObject createMolecule()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoCreateMolecule()));
        }

        public IndigoObject createQueryMolecule()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoCreateQueryMolecule()));
        }

        public IndigoObject loadMolecule(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadMoleculeFromString(str)));
        }

        public IndigoObject loadMolecule(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadMoleculeFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadMoleculeFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadMoleculeFromFile(path)));
        }

        public IndigoObject loadMoleculeFromBuffer(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadMoleculeFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadQueryMolecule(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadQueryMoleculeFromString(str)));
        }

        public IndigoObject loadQueryMolecule(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadQueryMoleculeFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadQueryMoleculeFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadQueryMoleculeFromFile(path)));
        }

        public IndigoObject loadSmarts(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadSmartsFromString(str)));
        }

        public IndigoObject loadSequence(string str, string seq_type)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadSequenceFromString(str, seq_type)));
        }

        public IndigoObject loadFasta(string str, string seq_type)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadFastaFromString(str, seq_type)));
        }

        public IndigoObject loadIdt(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadIdtFromString(str)));
        }

        public IndigoObject loadSmarts(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadSmartsFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadSmartsFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadSmartsFromFile(path)));
        }

        public IndigoObject loadSequenceFromFile(string path, string seq_type)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadSequenceFromFile(path, seq_type)));
        }

        public IndigoObject loadFastaFromFile(string path, string seq_type)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadFastaFromFile(path, seq_type)));
        }

        public IndigoObject loadIdtFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadIdtFromFile(path)));
        }

        public IndigoObject loadReaction(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadReactionFromString(str)));
        }

        public IndigoObject loadReaction(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadReactionFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadReactionFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadReactionFromFile(path)));
        }

        public IndigoObject loadQueryReaction(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadQueryReactionFromString(str)));
        }

        public IndigoObject loadQueryReaction(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadQueryReactionFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadQueryReactionFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadQueryReactionFromFile(path)));
        }

        public IndigoObject loadReactionSmarts(string str)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadReactionSmartsFromString(str)));
        }

        public IndigoObject loadReactionSmarts(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadReactionSmartsFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadReactionSmartsFromFile(string path)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadReactionSmartsFromFile(path)));
        }

        public string checkStructure(string str)
        {
            return checkStructure(str, "");
        }

        public string checkStructure(string str, string options)
        {
            setSessionID();
            return checkResult(IndigoLib.indigoCheckStructure(str, options));
        }


        public string check(string str, string type, string options)
        {
            setSessionID();
            return checkResult(IndigoLib.indigoCheck(str, type, options));
        }


        public IndigoObject loadStructure(string str)
        {
            return loadStructure(str, "");
        }

        public IndigoObject loadStructure(string str, string options)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadStructureFromString(str, options)));
        }

        public IndigoObject loadStructure(byte[] buf)
        {
            return loadStructure(buf, "");
        }

        public IndigoObject loadStructure(byte[] buf, string options)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadStructureFromBuffer(buf, buf.Length, options)));
        }

        public IndigoObject loadStructureFromFile(string path)
        {
            return loadStructureFromFile(path, "");
        }

        public IndigoObject loadStructureFromFile(string path, string options)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadStructureFromFile(path, options)));
        }

        public IndigoObject loadStructureFromBuffer(byte[] buf)
        {
            return loadStructureFromBuffer(buf, "");
        }

        public IndigoObject loadStructureFromBuffer(byte[] buf, string options)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadStructureFromBuffer(buf, buf.Length, options)));
        }

        public IndigoObject loadFingerprintFromBuffer(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadFingerprintFromBuffer(buf, buf.Length)));
        }

        public IndigoObject loadFingerprintFromDescriptors(double[] descriptors, int size, double density)
        {
            setSessionID();
            int result = IndigoLib.indigoLoadFingerprintFromDescriptors(descriptors, descriptors.Length, size, density);
            return new IndigoObject(this, checkResult(result));
        }

        public IndigoObject createReaction()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoCreateReaction()));
        }

        public IndigoObject createQueryReaction()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoCreateQueryReaction()));
        }

        public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2, string flags)
        {
            if (flags == null)
            {
                flags = "";
            }

            setSessionID();
            int match = checkResult(IndigoLib.indigoExactMatch(obj1.self, obj2.self, flags));

            if (match == 0)
            {
                return null;
            }

            return new IndigoObject(this, match, new IndigoObject[] { obj1, obj2 });
        }

        public IndigoObject exactMatch(IndigoObject obj1, IndigoObject obj2)
        {
            return exactMatch(obj1, obj2, "");
        }

        public void setTautomerRule(int id, string beg, string end)
        {
            setSessionID();
            checkResult(IndigoLib.indigoSetTautomerRule(id, beg, end));
        }

        public void removeTautomerRule(int id)
        {
            setSessionID();
            checkResult(IndigoLib.indigoRemoveTautomerRule(id));
        }

        public void clearTautomerRules()
        {
            setSessionID();
            checkResult(IndigoLib.indigoClearTautomerRules());
        }
        
        public IndigoObject deserialize(byte[] buf)
        {
            return new IndigoObject(this, checkResult(IndigoLib.indigoUnserialize(buf, buf.Length)));
        }
        
        [Obsolete("unserialize() is deprecated, please use deserialize() instead.")] 
        public IndigoObject unserialize(byte[] buf)
        {
            return deserialize(buf);
        }

        public IndigoObject createArray()
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoCreateArray()));
        }

        public float similarity(IndigoObject obj1, IndigoObject obj2)
        {
            return similarity(obj1, obj2, "");
        }

        public float similarity(IndigoObject obj1, IndigoObject obj2, string metrics)
        {
            setSessionID();
            if (metrics == null)
            {
                metrics = "";
            }

            return checkResult(IndigoLib.indigoSimilarity(obj1.self, obj2.self, metrics));
        }

        public int commonBits(IndigoObject obj1, IndigoObject obj2)
        {
            setSessionID();
            return checkResult(IndigoLib.indigoCommonBits(obj1.self, obj2.self));
        }

        public IndigoObject iterateSDFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoIterateSDFile(filename)));
        }

        public IndigoObject iterateRDFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoIterateRDFile(filename)));
        }

        public IndigoObject iterateSmilesFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoIterateSmilesFile(filename)));
        }

        public IndigoObject iterateCMLFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoIterateCMLFile(filename)));
        }

        public IndigoObject iterateCDXFile(string filename)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoIterateCDXFile(filename)));
        }

        public IndigoObject substructureMatcher(IndigoObject target, string mode)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoSubstructureMatcher(target.self, mode)), target);
        }

        public IndigoObject substructureMatcher(IndigoObject target)
        {
            return substructureMatcher(target, "");
        }

        public IndigoObject extractCommonScaffold(IndigoObject structures, string options)
        {
            setSessionID();
            int res = checkResult(IndigoLib.indigoExtractCommonScaffold(structures.self, options));
            if (res == 0)
            {
                return null;
            }

            return new IndigoObject(this, res);
        }

        public IndigoObject rgroupComposition(IndigoObject structures, string options)
        {
            setSessionID();
            int res = checkResult(IndigoLib.indigoRGroupComposition(structures.self, options));
            if (res == 0)
            {
                return null;
            }

            return new IndigoObject(this, res);
        }

        public IndigoObject getFragmentedMolecule(IndigoObject structures, string options)
        {
            setSessionID();
            int res = checkResult(IndigoLib.indigoGetFragmentedMolecule(structures.self, options));
            if (res == 0)
            {
                return null;
            }

            return new IndigoObject(this, res);
        }

        public IndigoObject toIndigoArray(IEnumerable collection)
        {
            setSessionID();

            IndigoObject arr = createArray();
            foreach (IndigoObject obj in collection)
            {
                arr.arrayAdd(obj);
            }

            return arr;
        }

        public static int[] toIntArray(ICollection collection)
        {
            if (collection == null)
            {
                return new int[0];
            }

            int[] res = new int[collection.Count];
            int i = 0;

            foreach (object x in collection)
            {
                res[i++] = Convert.ToInt32(x);
            }

            return res;
        }

        public static float[] toFloatArray(ICollection collection)
        {
            if (collection == null)
            {
                return new float[0];
            }

            float[] res = new float[collection.Count];
            int i = 0;

            foreach (object x in collection)
            {
                res[i++] = Convert.ToSingle(x);
            }

            return res;
        }

        public IndigoObject extractCommonScaffold(IEnumerable structures, string options)
        {
            return extractCommonScaffold(toIndigoArray(structures), options);
        }

        public IndigoObject decomposeMolecules(IndigoObject scaffold, IndigoObject structures)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoDecomposeMolecules(scaffold.self, structures.self)));
        }

        public IndigoObject decomposeMolecules(IndigoObject scaffold, IEnumerable structures)
        {
            return decomposeMolecules(scaffold, toIndigoArray(structures));
        }

        public IndigoObject createDecomposer(IndigoObject scaffold)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoCreateDecomposer(scaffold.self)));
        }

        public IndigoObject reactionProductEnumerate(IndigoObject reaction, IndigoObject monomers)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoReactionProductEnumerate(reaction.self, monomers.self)));
        }

        public IndigoObject reactionProductEnumerate(IndigoObject reaction, IEnumerable monomers)
        {
            IndigoObject indigoArrayArray = createArray();
            foreach (IEnumerable iter in monomers)
            {
                IndigoObject indigoArray = createArray();
                foreach (IndigoObject monomer in iter)
                {
                    indigoArray.arrayAdd(monomer);
                }
                indigoArrayArray.arrayAdd(indigoArray);
            }
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoReactionProductEnumerate(reaction.self, indigoArrayArray.self)));
        }

        public IndigoObject createSaver(IndigoObject output, string format)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoCreateSaver(output.self, format)), output);
        }

        public IndigoObject createFileSaver(string filename, string format)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(checkResult(IndigoLib.indigoCreateFileSaver(filename, format))));
        }

        public IndigoObject transform(IndigoObject reaction, IndigoObject monomer)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoTransform(reaction.self, monomer.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result);
        }

        public IndigoObject loadBuffer(byte[] buf)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadBuffer(buf, buf.Length)));
        }

        public IndigoObject loadString(string s)
        {
            setSessionID();
            return new IndigoObject(this, checkResult(IndigoLib.indigoLoadString(s)));
        }

        public IndigoObject iterateSDF(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoIterateSDF(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateRDF(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoIterateRDF(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateCML(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoIterateCML(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateCDX(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoIterateCDX(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }

        public IndigoObject iterateSmiles(IndigoObject reader)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoIterateSmiles(reader.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, reader);
        }
        public IndigoObject tautomerEnumerate(IndigoObject molecule, string parameters)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoIterateTautomers(molecule.self, parameters));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result, molecule);
        }

        public IndigoObject transformHELMtoSCSR(IndigoObject item)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoTransformHELMtoSCSR(item.self));
            if (result == 0)
            {
                return null;
            }
            return new IndigoObject(this, result);
        }

        public IndigoObject iterateTautomers(IndigoObject molecule, string parameters)
        {
            setSessionID();
            int result = checkResult(IndigoLib.indigoIterateTautomers(molecule.self, parameters));
            if (result == 0)
            {
                return null;
            }

            return new IndigoObject(this, result, molecule);
        }

        public int buildPkaModel(int level, float threshold, string filename)
        {
            setSessionID();
            return checkResult(IndigoLib.indigoBuildPkaModel(level, threshold, filename));
        }

        public IndigoObject nameToStructure(string name, string parameters)
        {
            if (parameters == null)
            {
                parameters = "";
            }
            setSessionID();
            int result = checkResult(IndigoLib.indigoNameToStructure(name, parameters));
            if (result == 0)
            {
                return null;
            }

            return new IndigoObject(this, result);
        }

        public IndigoObject nameToStructure(string name)
        {
            return nameToStructure(name, "");
        }
    }
}
