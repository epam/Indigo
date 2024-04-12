using System.Runtime.InteropServices;
using System.Security;

namespace com.epam.indigo
{
    public unsafe class IndigoLib
    {
        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoVersion();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoVersionInfo();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern long indigoAllocSessionId();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern void indigoSetSessionId(long id);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern void indigoReleaseSessionId(long id);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetLastError();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern void indigoSetErrorMessage(string message);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoFree(int id);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClone(int id);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountReferences();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetOption(string name, string value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetOptionInt(string name, int value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetOptionBool(string name, int value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetOptionFloat(string name, float value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetOptionColor(string name, float r, float g, float b);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetOptionXY(string name, int x, int y);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetOption(string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetOptionInt(string name, int* value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetOptionBool(string name, int* value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetOptionFloat(string name, float* value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetOptionType(string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetOptions();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoReadFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoReadString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoReadBuffer(byte[] buffer, int size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadBuffer(byte[] buffer, int size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoWriteFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoWriteBuffer();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClose(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetOriginalFormat(int id);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateMolecule();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateQueryMolecule();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadMoleculeFromString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadMoleculeFromFile(string path);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadMoleculeFromBuffer(byte[] buf, int bufsize);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadQueryMoleculeFromString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadQueryMoleculeFromFile(string path);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadQueryMoleculeFromBuffer(byte[] buf, int bufsize);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadSmartsFromString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadSmartsFromFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadSequenceFromString(string str, string seq_type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadFastaFromString(string str, string seq_type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadIdtFromString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadSequenceFromFile(string filename, string seq_type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadFastaFromFile(string filename, string seq_type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadIdtFromFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadSmartsFromBuffer(byte[] buffer, int size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadStructureFromString(string str, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadStructureFromFile(string path, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadStructureFromBuffer(byte[] buf, int bufsize, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveMolfile(int molecule, int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveSequenceToFile(int molecule, int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveMolfileToFile(int molecule, string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoMolfile(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoSequence(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoFasta(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoIdt(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveCml(int molecule, int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveCmlToFile(int molecule, string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCml(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoJson(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveCdxml(int molecule, int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveCdxmlToFile(int molecule, string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCdxml(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCdxBase64(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveMDLCT(int item, int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateSaver(int output, string format);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateFileSaver(string filename, string format);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAppend(int saver, int obj);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadReactionFromString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadReactionFromFile(string path);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadReactionFromBuffer(byte[] buf, int bufsize);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadQueryReactionFromString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadQueryReactionFromFile(string path);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadQueryReactionFromBuffer(byte[] buf, int bufsize);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadReactionSmartsFromString(string str);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadReactionSmartsFromFile(string path);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadReactionSmartsFromBuffer(byte[] buf, int bufsize);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateReaction();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateQueryReaction();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddReactant(int reaction, int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddProduct(int reaction, int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddCatalyst(int reaction, int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountReactants(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountProducts(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountCatalysts(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountMolecules(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateReactants(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateProducts(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateCatalysts(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateMolecules(int reader);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveRxnfile(int reaction, int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSaveRxnfileToFile(int reaction, string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoRxnfile(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAutomap(int reaction, string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetAtomMappingNumber(int reaction, int reaction_atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetAtomMappingNumber(int reaction, int reaction_atom, int number);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearAAM(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCorrectReactingCenters(int reaction);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetReactingCenter(int reaction, int reaction_bond, int* rc);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetReactingCenter(int reaction, int reaction_bond, int rc);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoOptimize(int query, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoNormalize(int structure, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoStandardize(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIonize(int item, float pH, float pH_toll);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern float* indigoGetAcidPkaValue(int item, int atom, int level, int min_level);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern float* indigoGetBasicPkaValue(int item, int atom, int level, int min_level);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoBuildPkaModel(int level, float theshold, string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateAtoms(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIteratePseudoatoms(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateRSites(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateStereocenters(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateAlleneCenters(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateRGroups(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateRGroupFragments(int rgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountRGroups(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountAttachmentPoints(int rgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIsPseudoatom(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIsRSite(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIsTemplateAtom(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoStereocenterType(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int* indigoStereocenterPyramid(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSingleAllowedRGroup(int rsite);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoChangeStereocenterType(int atom, int type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddStereocenter(int atom, int type, int v1, int v2, int v3, int v4);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoStereocenterGroup(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetStereocenterGroup(int atom, int group);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoSymbol(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDegree(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetCharge(int atom, int* charge);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetRadical(int atom, int* radical);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetExplicitValence(int atom, int* valence);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetRadicalElectrons(int atom, int* electrons);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAtomicNumber(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIsotope(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoValence(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetHybridization(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCheckValence(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCheckQuery(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCheckRGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountHydrogens(int atom, int* hydro);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountImplicitHydrogens(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountSuperatoms(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountDataSGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountGenericSGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountRepeatingUnits(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountMultipleGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSuperatoms(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateDataSGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateGenericSGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateRepeatingUnits(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateMultipleGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateTGroups(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetDataSGroup(int mol, int idx);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSuperatom(int mol, int idx);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetGenericSGroup(int molecule, int index);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetMultipleGroup(int molecule, int index);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetRepeatingUnit(int molecule, int index);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoDescription(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoData(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddDataSGroup(int molecule, int natoms, int[] atoms, int nbonds, int[] bonds, string description, string data);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetDataSGroupXY(int sgroup, float x, float y, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddSuperatom(int molecule, int natoms, int[] atoms, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetRepeatingUnitSubscript(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetRepeatingUnitConnectivity(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateSGroup(string type, int mapping, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupClass(int sgroup, string sgclass);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupName(int sgroup, string sgname);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetSGroupClass(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetSGroupName(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupNumCrossBonds(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddSGroupAttachmentPoint(int sgroup, int aidx, int lvidx, string apid);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDeleteSGroupAttachmentPoint(int sgroup, int apidx);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupDisplayOption(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupDisplayOption(int sgroup, int option);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupSeqId(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern float* indigoGetSGroupCoords(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupMultiplier(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupMultiplier(int sgroup, int mult);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupData(int sgroup, string data);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupCoords(int sgroup, float x, float y);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupDescription(int sgroup, string description);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupFieldName(int sgroup, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupQueryCode(int sgroup, string querycode);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupQueryOper(int sgroup, string queryoper);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupDisplay(int sgroup, string option);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupLocation(int sgroup, string option);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupTag(int sgroup, string tag);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupTagAlign(int sgroup, int tag_align);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupDataType(int sgroup, string type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupXCoord(int sgroup, float x);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupYCoord(int sgroup, float y);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupBrackets(int sgroup, int brk_style, float x1, float y1, float x2, float y2,
                                                         float x3, float y3, float x4, float y4);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoFindSGroups(int molecule, string property, string value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupType(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupIndex(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupOriginalId(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupOriginalId(int sgroup, int original);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSGroupParentId(int sgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetSGroupParentId(int sgroup, int parent);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddTemplate(int molecule, int templates, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRemoveTemplate(int molecule, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoFindTemplate(int molecule, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetTGroupClass(int tgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetTGroupName(int tgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetTGroupAlias(int tgroup);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoTransformSCSRtoCTAB(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoTransformCTABtoSCSR(int molecule, int templates);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetTemplateAtomClass(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetTemplateAtomClass(int atom, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern float* indigoXYZ(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetXYZ(int atom, float x, float y, float z);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetCharge(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetExplicitValence(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetRadical(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetIsotope(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetAttachmentPoint(int atom, int order);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateAttachmentPoints(int item, int order);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearAttachmentPoints(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRemoveConstraints(int item, string type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddConstraint(int item, string type, string value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddConstraintNot(int item, string type, string value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddConstraintOr(int item, string type, string value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoInvertStereo(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetStereo(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountAtoms(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountBonds(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountPseudoatoms(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountRSites(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateBonds(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoBondOrder(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoBondStereo(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoTopology(int bond);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateNeighbors(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoBond(int nei);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetAtom(int molecule, int idx);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetBond(int molecule, int idx);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetMolecule(int reaction, int idx);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSource(int bond);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDestination(int bond);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearCisTrans(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearStereocenters(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearAlleneCenters(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountStereocenters(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountAlleneCenters(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetSymmetricCisTrans(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetSymmetricStereocenters(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoMarkEitherCisTrans(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoMarkStereobonds(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoValidateChirality(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddAtom(int molecule, string symbol);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoResetAtom(int atom, string symbol);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddRSite(int molecule, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetRSite(int atom, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetCharge(int atom, int charge);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetExplicitValence(int atom, int valence);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetIsotope(int atom, int isotope);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetImplicitHCount(int atom, int implh);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetRadical(int atom, int radical);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddBond(int source, int destination, int order);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetBondOrder(int bond, int order);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoMerge(int where_to, int what);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoHighlight(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoUnhighlight(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIsHighlighted(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountComponents(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoComponentIndex(int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateComponents(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoComponent(int molecule, int index);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountSSSR(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSSSR(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSubtrees(int molecule, int min_atoms, int max_atoms);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateRings(int molecule, int min_atoms, int max_atoms);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateEdgeSubmolecules(int molecule, int min_bonds, int max_bonds);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountHeavyAtoms(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGrossFormula(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern double indigoMolecularWeight(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern double indigoMostAbundantMass(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern double indigoMonoisotopicMass(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoMassComposition(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern double indigoTPSA(int molecule, bool includeSP);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoNumRotatableBonds(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoNumHydrogenBondAcceptors(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoNumHydrogenBondDonors(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern double indigoLogP(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern double indigoMolarRefractivity(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern double indigoPka(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCanonicalSmiles(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern long indigoHash(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoLayeredCode(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoHasCoord(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoHasZCoord(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIsChiral(int molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIsPossibleFischerProjection(int molecule, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateSubmolecule(int molecule, int nvertices, int[] vertices);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateEdgeSubmolecule(int molecule, int nvertices, int[] vertices, int nedges, int[] edges);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRemoveAtoms(int molecule, int nvertices, int[] vertices);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRemoveBonds(int molecule, int nbonds, int[] bonds);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetSubmolecule(int molecule, int nvertices, int[] vertices);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern float indigoAlignAtoms(int molecule, int natoms, int[] atom_ids, float[] desired_xyz);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAromatize(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDearomatize(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoFoldHydrogens(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoUnfoldHydrogens(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoFoldUnfoldHydrogens(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearXYZ(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLayout(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClean2d(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoSmiles(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoSmarts(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCanonicalSmarts(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoExactMatch(int item1, int item2, string flags);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetTautomerRule(int id, string beg, string end);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRemoveTautomerRule(int id);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearTautomerRules();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoName(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetName(int item, string name);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSerialize(int handle, byte** buf, int* size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoUnserialize(byte[] buf, int size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoHasProperty(int handle, string field);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoGetProperty(int handle, string field);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSetProperty(int handle, string field, string value);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRemoveProperty(int item, string prop);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateProperties(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClearProperties(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCheckBadValence(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCheckAmbiguousH(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCheckChirality(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCheck3DStereo(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCheckStereo(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCheck(string item, string type, string load_params);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCheckObj(int item, string type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoCheckStructure(string structure, string type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoFingerprint(int item, string type);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoOneBitsList(int fingerprint);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadFingerprintFromBuffer(byte[] buffer, int size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoLoadFingerprintFromDescriptors(double[] arr, int arr_len, int size, double density);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountBits(int fingerprint);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCommonBits(int fingerprint1, int fingerprint2);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern float indigoSimilarity(int molecule1, int molecule2, string metrics);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSDF(int reader);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateRDF(int reader);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSmiles(int reader);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateCML(int reader);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateCDX(int reader);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSDFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateRDFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateSmilesFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateCMLFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateCDXFile(string filename);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoRawData(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoTell(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSdfAppend(int output, int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSmilesAppend(int output, int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRdfHeader(int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRdfAppend(int output, int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCmlHeader(int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCmlAppend(int output, int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCmlFooter(int output);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateArray();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoArrayAdd(int arr, int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAt(int item, int index);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCount(int arr);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoClear(int arr);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateArray(int arr);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoSubstructureMatcher(int target, string mode);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIgnoreAtom(int matcher, int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoUnignoreAtom(int matcher, int atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoUnignoreAllAtoms(int matcher);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoMatch(int matcher, int query);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountMatches(int matcher, int query);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCountMatchesWithLimit(int matcher, int query, int embeddings_limit);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateMatches(int matcher, int query);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoHighlightedTarget(int match);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoMapAtom(int match, int query_atom);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoMapBond(int match, int query_bond);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoMapMolecule(int match, int query_reaction_molecule);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoExtractCommonScaffold(int structures, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRGroupComposition(int molecule, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoGetFragmentedMolecule(int molecule, string options);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAllScaffolds(int extracted);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDecomposeMolecules(int scaffold, int structures);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDecomposedMoleculeScaffold(int decomp);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateDecomposedMolecules(int decomp);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDecomposedMoleculeHighlighted(int decomp);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDecomposedMoleculeSubstituents(int decomp);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDecomposedMoleculeWithRGroups(int decomp);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCreateDecomposer(int scaffold);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDecomposeMolecule(int decomp, int mol);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateDecompositions(int deco_item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoAddDecomposition(int decomp, int q_match);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoNext(int iter);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoHasNext(int iter);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIndex(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoRemove(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoToString(int handle);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoToBuffer(int handle, byte** buf, int* size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int* indigoSymmetryClasses(int molecule, int* size);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoReactionProductEnumerate(int reaction, int monomers);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoTransform(int reaction, int monomers);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoExpandAbbreviations(int structure);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoIterateTautomers(int structure, string parameters);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoNameToStructure(string name, string parameters);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoTransformHELMtoSCSR(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoDbgInternalType(int item);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern byte* indigoDbgProfiling(int whole_sessoin);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDbgResetProfiling(int whole_sessoin);

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoDbgBreakpoint();

        [DllImport("indigo"), SuppressUnmanagedCodeSecurity]
        public static extern int indigoCopyRGroups(int molecule_from, int molecule_to);
    }
}
