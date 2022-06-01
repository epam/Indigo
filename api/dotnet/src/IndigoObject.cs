using System;
using System.Collections;

namespace com.epam.indigo
{
    public unsafe class IndigoObject : IDisposable, IEnumerable
    {
        public int self;
        private readonly Indigo dispatcher;
        private readonly object parent; // to prevent GC killing the parent object

        public IndigoObject(Indigo dispatcher, int id) : this(dispatcher, id, null)
        {
        }

        public IndigoObject(Indigo dispatcher, int id, object parent)
        {
            this.dispatcher = dispatcher;
            this.self = id;
            this.parent = parent;
        }

        ~IndigoObject()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (dispatcher == null)
            {
                // This happens exclusively in 32-bit .NET environment
                // after an IndigoObject constructor throws an exception.
                // In fact, the object is not created in this case,
                // but for some reason the .NET VM disposes it, despite it
                // has not been initialized.
                return;
            }
            if (self >= 0)
            {
                // Check that the session is still alive
                // (.NET has no problem disposing referenced
                // objects before the objects that reference to them)
                if (dispatcher.getSID() >= 0)
                {
                    dispatcher.setSessionID();
                    dispatcher.free(self);
                    self = -1;
                }
            }
        }

        public void dispose()
        {
            Dispose();
        }

        public Indigo indigo()
        {
            return dispatcher;
        }

        public IndigoObject clone()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoClone(self)));
        }

        public void close()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClose(self));
        }

        public string molfile()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoMolfile(self));
        }

        public void saveMolfile(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(IndigoLib.indigoWriteFile(filename));
            dispatcher.checkResult(IndigoLib.indigoSaveMolfile(self, s));
            dispatcher.checkResult(IndigoLib.indigoFree(s));
        }

        public string cml()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCml(self));
        }

        public string json()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoJson(self));
        }

        public void saveCml(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(IndigoLib.indigoWriteFile(filename));
            dispatcher.checkResult(IndigoLib.indigoSaveCml(self, s));
            dispatcher.checkResult(IndigoLib.indigoFree(s));
        }

        public string cdxml()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCdxml(self));
        }

        public void saveCdxml(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(IndigoLib.indigoWriteFile(filename));
            dispatcher.checkResult(IndigoLib.indigoSaveCdxml(self, s));
            dispatcher.checkResult(IndigoLib.indigoFree(s));
        }

        public byte[] mdlct()
        {
            dispatcher.setSessionID();
            IndigoObject buf = dispatcher.writeBuffer();
            dispatcher.checkResult(IndigoLib.indigoSaveMDLCT(self, buf.self));
            return buf.toBuffer();
        }

        public void addReactant(IndigoObject molecule)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddReactant(self, molecule.self));
        }

        public void addProduct(IndigoObject molecule)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddProduct(self, molecule.self));
        }

        public void addCatalyst(IndigoObject molecule)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddCatalyst(self, molecule.self));
        }

        public int countReactants()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountReactants(self));
        }

        public int countProducts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountProducts(self));
        }

        public int countCatalysts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountCatalysts(self));
        }

        public int countMolecules()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountMolecules(self));
        }

        public IEnumerable iterateReactants()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateReactants(self)), this);
        }

        public IEnumerable iterateProducts()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateProducts(self)), this);
        }

        public IEnumerable iterateCatalysts()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateCatalysts(self)), this);
        }

        public IEnumerable iterateMolecules()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateMolecules(self)), this);
        }

        public string rxnfile()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoRxnfile(self));
        }

        public void saveRxnfile(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(IndigoLib.indigoWriteFile(filename));
            dispatcher.checkResult(IndigoLib.indigoSaveRxnfile(self, s));
            dispatcher.checkResult(IndigoLib.indigoFree(s));
        }

        public void automap()
        {
            automap("");
        }

        public void automap(string mode)
        {
            if (mode == null)
            {
                mode = "";
            }

            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAutomap(self, mode));
        }

        public int atomMappingNumber(IndigoObject reaction_atom)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetAtomMappingNumber(self, reaction_atom.self));
        }

        public void setAtomMappingNumber(IndigoObject reaction_atom, int number)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetAtomMappingNumber(self, reaction_atom.self, number));
        }

        public ReactingCenter reactingCenter(IndigoObject bond)
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(IndigoLib.indigoGetReactingCenter(self, bond.self, &c)) == 1)
            {
                return (ReactingCenter)c;
            }

            throw new IndigoException("reactingCenter(): unexpected result");
        }

        public void setReactingCenter(IndigoObject bond, ReactingCenter type)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetReactingCenter(self, bond.self, (int)type));
        }

        public void clearAAM()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClearAAM(self));
        }

        public void correctReactingCenters()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoCorrectReactingCenters(self));
        }

        public IEnumerable iterateAtoms()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateAtoms(self)), this);
        }

        public IEnumerable iteratePseudoatoms()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIteratePseudoatoms(self)), this);
        }

        public IEnumerable iterateRSites()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateRSites(self)), this);
        }

        public IEnumerable iterateStereocenters()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateStereocenters(self)), this);
        }

        public IEnumerable iterateAlleneCenters()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateAlleneCenters(self)), this);
        }

        public IEnumerable iterateRGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateRGroups(self)), this);
        }

        public IEnumerable iterateRGroupFragments()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateRGroupFragments(self)), this);
        }

        public int countRGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountRGroups(self));
        }

        public int countAttachmentPoints()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountAttachmentPoints(self));
        }

        public bool isPseudoatom()
        {
            dispatcher.setSessionID();
            if (dispatcher.checkResult(IndigoLib.indigoIsPseudoatom(self)) == 1)
            {
                return true;
            }

            return false;
        }

        public bool isRSite()
        {
            dispatcher.setSessionID();
            if (dispatcher.checkResult(IndigoLib.indigoIsRSite(self)) == 1)
            {
                return true;
            }

            return false;
        }

        public bool isTemplateAtom()
        {
            dispatcher.setSessionID();
            if (dispatcher.checkResult(IndigoLib.indigoIsTemplateAtom(self)) == 1)
            {
                return true;
            }

            return false;
        }

        public int stereocenterType()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoStereocenterType(self));
        }

        public int stereocenterGroup()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoStereocenterGroup(self));
        }

        public int[] stereocenterPyramid()
        {
            dispatcher.setSessionID();
            int* pyramid_ptr = dispatcher.checkResult(IndigoLib.indigoStereocenterPyramid(self));

            int[] res = new int[4];
            for (int i = 0; i < 4; ++i)
            {
                res[i] = pyramid_ptr[i];
            }

            return res;
        }

        public void changeStereocenterType(int type)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoChangeStereocenterType(self, type));
        }

        public void setStereocenterGroup(int group)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetStereocenterGroup(self, group));
        }

        public int singleAllowedRGroup()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSingleAllowedRGroup(self));
        }

        public string symbol()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSymbol(self));
        }

        public int degree()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoDegree(self));
        }

        public int? charge()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(IndigoLib.indigoGetCharge(self, &c)) == 1)
            {
                return c;
            }

            return null;
        }

        public int? explicitValence()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(IndigoLib.indigoGetExplicitValence(self, &c)) == 1)
            {
                return c;
            }

            return null;
        }

        public int? radicalElectrons()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(IndigoLib.indigoGetRadicalElectrons(self, &c)) == 1)
            {
                return c;
            }

            return null;
        }

        public int? radical()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(IndigoLib.indigoGetRadical(self, &c)) == 1)
            {
                return c;
            }

            return null;
        }

        public int atomicNumber()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoAtomicNumber(self));
        }

        public int isotope()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoIsotope(self));
        }

        public int valence()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoValence(self));
        }

        public Hybridization getHybridization()
        {
            dispatcher.setSessionID();
            return (Hybridization)dispatcher.checkResult(IndigoLib.indigoGetHybridization(self));
        }

        public string getHybridizationStr()
        {
            return getHybridization().ToString();
        }

        public int checkValence()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckValence(self));
        }

        public int checkQuery()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckQuery(self));
        }

        public int checkRGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckRGroups(self));
        }

        public int? countHydrogens()
        {
            int h;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(IndigoLib.indigoCountHydrogens(self, &h)) == 1)
            {
                return h;
            }

            return null;
        }

        public int countImplicitHydrogens()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountImplicitHydrogens(self));
        }

        public int countSuperatoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountSuperatoms(self));
        }

        public int countDataSGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountDataSGroups(self));
        }

        public int countGenericSGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountGenericSGroups(self));
        }

        public int countRepeatingUnits()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountRepeatingUnits(self));
        }

        public int countMultipleGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountMultipleGroups(self));
        }

        public IndigoObject iterateSuperatoms()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateSuperatoms(self)), this);
        }

        public IndigoObject iterateAttachmentPoints(int order)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateAttachmentPoints(self, order)), this);
        }

        public IndigoObject iterateDataSGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateDataSGroups(self)), this);
        }

        public IndigoObject iterateGenericSGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateGenericSGroups(self)), this);
        }

        public IndigoObject iterateRepeatingUnits()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateRepeatingUnits(self)), this);
        }

        public IndigoObject iterateMultipleGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateMultipleGroups(self)), this);
        }

        public IndigoObject iterateSGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateSGroups(self)), this);
        }

        public IndigoObject iterateTGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateTGroups(self)), this);
        }

        public IndigoObject getDataSGroup(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetDataSGroup(self, index)), this);
        }

        public IndigoObject getSuperatom(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetSuperatom(self, index)), this);
        }

        public IndigoObject getGenericSGroup(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetGenericSGroup(self, index)), this);
        }

        public IndigoObject getMultipleGroup(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetMultipleGroup(self, index)), this);
        }

        public IndigoObject getRepeatingUnit(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetRepeatingUnit(self, index)), this);
        }

        public string description()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoDescription(self));
        }

        public string data()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoData(self));
        }

        public IndigoObject addDataSGroup(int[] atoms, int[] bonds, string description, string data)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoAddDataSGroup(self, atoms.Length, atoms, bonds.Length, bonds, description, data)));
        }

        public IndigoObject addDataSGroup(ICollection atoms, ICollection bonds, string description, string data)
        {
            return addDataSGroup(Indigo.toIntArray(atoms), Indigo.toIntArray(bonds), description, data);
        }

        public IndigoObject addSuperatom(int[] atoms, string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoAddSuperatom(self, atoms.Length, atoms, name)));
        }

        public IndigoObject addSuperatom(ICollection atoms, string name)
        {
            return addSuperatom(Indigo.toIntArray(atoms), name);
        }

        public IndigoObject createSGroup(string type, IndigoObject mapping, string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoCreateSGroup(type, mapping.self, name)));
        }

        public string getRepeatingUnitSubscript()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetRepeatingUnitSubscript(self));
        }

        public int getRepeatingUnitConnectivity()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetRepeatingUnitConnectivity(self));
        }

        public void setSGroupClass(string sgclass)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetSGroupClass(self, sgclass));
        }

        public void setSGroupName(string sgname)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetSGroupName(self, sgname));
        }

        public string getSGroupClass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupClass(self));
        }

        public string getSGroupName()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupName(self));
        }

        public int getSGroupNumCrossBonds()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupNumCrossBonds(self));
        }

        public int addSGroupAttachmentPoint(int aidx, int lvidx, string apid)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoAddSGroupAttachmentPoint(self, aidx, lvidx, apid));
        }

        public int deleteSGroupAttachmentPoint(int apidx)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoDeleteSGroupAttachmentPoint(self, apidx));
        }

        public int getSGroupDisplayOption()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupDisplayOption(self));
        }

        public int setSGroupDisplayOption(int option)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupDisplayOption(self, option));
        }

        public int getSGroupSeqId()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupSeqId(self));
        }

        public float[] getSGroupCoords()
        {
            dispatcher.setSessionID();
            float* ptr = dispatcher.checkResult(IndigoLib.indigoGetSGroupCoords(self));
            float[] res = new float[2];
            res[0] = ptr[0];
            res[1] = ptr[1];
            return res;
        }

        public int getSGroupMultiplier()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupMultiplier(self));
        }

        public int setSGroupMultiplier(int mult)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupMultiplier(self, mult));
        }

        public int setSGroupData(string data)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupData(self, data));
        }
        public int setSGroupCoords(float x, float y)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupCoords(self, x, y));
        }
        public int setSGroupDescription(string description)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupDescription(self, description));
        }
        public int setSGroupFieldName(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupFieldName(self, name));
        }
        public int setSGroupQueryCode(string querycode)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupQueryCode(self, querycode));
        }
        public int setSGroupQueryOper(string queryoper)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupQueryOper(self, queryoper));
        }
        public int setSGroupDisplay(string option)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupDisplay(self, option));
        }
        public int setSGroupLocation(string option)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupLocation(self, option));
        }
        public int setSGroupTag(string tag)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupTag(self, tag));
        }
        public int setSGroupTagAlign(int tag_align)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupTagAlign(self, tag_align));
        }
        public int setSGroupDataType(string data_type)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupDataType(self, data_type));
        }
        public int setSGroupXCoord(float x)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupXCoord(self, x));
        }
        public int setSGroupYCoord(float y)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupYCoord(self, y));
        }
        public int setSGroupBrackets(int brk_style, float x1, float y1, float x2, float y2,
                                     float x3, float y3, float x4, float y4)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupBrackets(self, brk_style, x1, y1, x2, y2, x3, y3, x4, y4));
        }

        public IndigoObject findSGroups(string property, string value)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoFindSGroups(self, property, value)), this);
        }

        public int getSGroupType()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupType(self));
        }

        public int getSGroupIndex()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupIndex(self));
        }

        public int getSGroupOriginalId()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupOriginalId(self));
        }

        public int setSGroupOriginalId(int original)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupOriginalId(self, original));
        }

        public int getSGroupParentId()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetSGroupParentId(self));
        }

        public int setSGroupParentId(int parent)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetSGroupParentId(self, parent));
        }

        public int addTemplate(IndigoObject templates, string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoAddTemplate(self, templates.self, name));
        }

        public int removeTemplate(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoRemoveTemplate(self, name));
        }

        public int findTemplate(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoFindTemplate(self, name));
        }

        public string getTGroupClass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetTGroupClass(self));
        }

        public string getTGroupName()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetTGroupName(self));
        }

        public string getTGroupAlias()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetTGroupAlias(self));
        }

        public int transformSCSRtoCTAB()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoTransformSCSRtoCTAB(self));
        }

        public int transformCTABtoSCSR(IndigoObject templates)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoTransformCTABtoSCSR(self, templates.self));
        }

        public string getTemplateAtomClass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetTemplateAtomClass(self));
        }

        public int setTemplateAtomClass(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSetTemplateAtomClass(self, name));
        }

        public void addStereocenter(int type, int v1, int v2, int v3)
        {
            addStereocenter(type, v1, v2, v3, -1);
        }

        public void addStereocenter(int type, int v1, int v2, int v3, int v4)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddStereocenter(self, type, v1, v2, v3, v4));
        }

        public void setDataSGroupXY(float x, float y)
        {
            setDataSGroupXY(x, y, "");
        }

        public void setDataSGroupXY(float x, float y, string options)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetDataSGroupXY(self, x, y, options));
        }

        public float[] xyz()
        {
            dispatcher.setSessionID();
            float* ptr = dispatcher.checkResult(IndigoLib.indigoXYZ(self));
            float[] res = new float[3];
            res[0] = ptr[0];
            res[1] = ptr[1];
            res[2] = ptr[2];
            return res;
        }

        public void setXYZ(float x, float y, float z)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetXYZ(self, x, y, z));
        }

        public void resetCharge()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoResetCharge(self));
        }

        public void resetExplicitValence()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoResetExplicitValence(self));
        }

        public void resetRadical()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoResetRadical(self));
        }

        public void resetIsotope()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoResetIsotope(self));
        }

        public void setAttachmentPoint(int order)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetAttachmentPoint(self, order));
        }

        public void clearAttachmentPoints()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClearAttachmentPoints(self));
        }

        public void removeConstraints(string type)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoRemoveConstraints(self, type));
        }

        public void addConstraint(string type, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddConstraint(self, type, value));
        }

        public void addConstraintNot(string type, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddConstraintNot(self, type, value));
        }

        public void addConstraintOr(string type, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddConstraintOr(self, type, value));
        }

        public void invertStereo()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoInvertStereo(self));
        }

        public void resetStereo()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoResetStereo(self));
        }

        public int countAtoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountAtoms(self));
        }

        public int countBonds()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountBonds(self));
        }

        public int countPseudoatoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountPseudoatoms(self));
        }

        public int countRSites()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountRSites(self));
        }

        public IEnumerable iterateBonds()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateBonds(self)), this);
        }

        public int bondOrder()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoBondOrder(self));
        }

        public int bondStereo()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoBondStereo(self));
        }

        public int topology()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoTopology(self));
        }

        public IEnumerable iterateNeighbors()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateNeighbors(self)), this);
        }

        public IndigoObject bond()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoBond(self)));
        }

        public IndigoObject getAtom(int idx)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetAtom(self, idx)), this);
        }

        public IndigoObject getBond(int idx)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetBond(self, idx)), this);
        }

        public IndigoObject getMolecule(int idx)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetMolecule(self, idx)), this);
        }

        public IndigoObject source()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoSource(self)));
        }

        public IndigoObject destination()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoDestination(self)));
        }

        public void clearCisTrans()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClearCisTrans(self));
        }

        public void clearStereocenters()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClearStereocenters(self));
        }

        public void clearAlleneCenters()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClearAlleneCenters(self));
        }

        public int countStereocenters()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountStereocenters(self));
        }

        public int countAlleneCenters()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountAlleneCenters(self));
        }

        public int resetSymmetricCisTrans()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoResetSymmetricCisTrans(self));
        }

        public int resetSymmetricStereocenters()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoResetSymmetricStereocenters(self));
        }

        public int markEitherCisTrans()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoMarkEitherCisTrans(self));
        }

        public int markStereobonds()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoMarkStereobonds(self));
        }

        public int validateChirality()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoValidateChirality(self));
        }

        public IndigoObject addAtom(string symbol)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoAddAtom(self, symbol)), this);
        }

        public IndigoObject resetAtom(string symbol)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoResetAtom(self, symbol)), this);
        }

        public IndigoObject addRSite(string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoAddRSite(self, name)), this);
        }

        public IndigoObject setRSite(string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoSetRSite(self, name)), this);
        }

        public void setCharge(int charge)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetCharge(self, charge));
        }

        public void setRadical(int radical)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetRadical(self, radical));
        }

        public void setExplicitValence(int valence)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetExplicitValence(self, valence));
        }

        public void setIsotope(int isotope)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetIsotope(self, isotope));
        }

        public void setImplicitHCount(int implh)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetImplicitHCount(self, implh));
        }

        public IndigoObject addBond(IndigoObject dest, int order)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoAddBond(self, dest.self, order)), this);
        }

        public void setBondOrder(int order)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetBondOrder(self, order));
        }

        public IndigoObject merge(IndigoObject what)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoMerge(self, what.self)), this);
        }

        public void highlight()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoHighlight(self));
        }

        public void unhighlight()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoUnhighlight(self));
        }

        public bool isHighlighted()
        {
            dispatcher.setSessionID();
            return (dispatcher.checkResult(IndigoLib.indigoIsHighlighted(self)) == 1);
        }

        public int countComponents()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountComponents(self));
        }

        public int componentIndex()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoComponentIndex(self));
        }

        public void remove()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoRemove(self));
        }

        public IndigoObject iterateComponents()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateComponents(self)), this);
        }

        public IndigoObject component(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoComponent(self, index)), this);
        }

        public int countSSSR()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountSSSR(self));
        }

        public IndigoObject iterateSSSR()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateSSSR(self)), this);
        }

        public IndigoObject iterateSubtrees(int min_vertices, int max_vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateSubtrees(self, min_vertices, max_vertices)), this);
        }

        public IndigoObject iterateRings(int min_vertices, int max_vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateRings(self, min_vertices, max_vertices)), this);
        }

        public IndigoObject iterateEdgeSubmolecules(int min_edges, int max_edges)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateEdgeSubmolecules(self, min_edges, max_edges)), this);
        }

        public int countHeavyAtoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountHeavyAtoms(self));
        }

        public string grossFormula()
        {
            int gf = -1;
            try
            {
                dispatcher.setSessionID();
                gf = dispatcher.checkResult(IndigoLib.indigoGrossFormula(self));
                string result = dispatcher.checkResult(IndigoLib.indigoToString(gf));
                return result;
            }
            finally
            {
                dispatcher.checkResult(IndigoLib.indigoFree(gf));
            }

        }

        public double molecularWeight()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoMolecularWeight(self));
        }

        public double mostAbundantMass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoMostAbundantMass(self));
        }

        public double monoisotopicMass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoMonoisotopicMass(self));
        }

        public string massComposition()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoMassComposition(self));
        }

        public double tpsa(bool includeSP = false)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoTPSA(self, includeSP));
        }

        public int numRotatableBonds()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoNumRotatableBonds(self));
        }

        public int numHydrogenBondAcceptors()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoNumHydrogenBondAcceptors(self));
        }

        public int numHydrogenBondDonors()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoNumHydrogenBondDonors(self));
        }

        public double cLogP()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCLogP(self));
        }

        public double cMolarRefractivity()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCMolarRefractivity(self));
        }

        public string canonicalSmiles()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCanonicalSmiles(self));
        }

        public int[] symmetryClasses()
        {
            dispatcher.setSessionID();
            int count;
            int* classes = dispatcher.checkResult(IndigoLib.indigoSymmetryClasses(self, &count));

            int[] res = new int[count];
            for (int i = 0; i < count; ++i)
            {
                res[i] = classes[i];
            }

            return res;
        }

        public string layeredCode()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoLayeredCode(self));
        }

        public bool hasCoord()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoHasCoord(self)) == 1;
        }

        public bool hasZCoord()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoHasZCoord(self)) == 1;
        }

        public bool isChiral()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoIsChiral(self)) == 1;
        }

        public bool isPossibleFischerProjection(string options)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoIsPossibleFischerProjection(self, options)) == 1;
        }

        public IndigoObject createSubmolecule(int[] vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoCreateSubmolecule(self, vertices.Length, vertices)));
        }

        public IndigoObject createSubmolecule(ICollection vertices)
        {
            return createSubmolecule(Indigo.toIntArray(vertices));
        }

        public IndigoObject getSubmolecule(int[] vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoGetSubmolecule(self, vertices.Length, vertices)));
        }

        public IndigoObject getSubmolecule(ICollection vertices)
        {
            return getSubmolecule(Indigo.toIntArray(vertices));
        }

        public IndigoObject createEdgeSubmolecule(int[] vertices, int[] edges)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoCreateEdgeSubmolecule(self, vertices.Length, vertices, edges.Length, edges)));
        }

        public IndigoObject createEdgeSubmolecule(ICollection vertices, ICollection edges)
        {
            return createEdgeSubmolecule(vertices, edges);
        }

        public void removeAtoms(int[] vertices)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoRemoveAtoms(self, vertices.Length, vertices));
        }

        public void removeAtoms(ICollection vertices)
        {
            removeAtoms(Indigo.toIntArray(vertices));
        }

        public void removeBonds(int[] bonds)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoRemoveBonds(self, bonds.Length, bonds));
        }

        public void removeBonds(ICollection bonds)
        {
            removeBonds(Indigo.toIntArray(bonds));
        }

        public float alignAtoms(int[] atom_ids, float[] desired_xyz)
        {
            if (atom_ids.Length * 3 != desired_xyz.Length)
            {
                throw new IndigoException("alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]");
            }

            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoAlignAtoms(self, atom_ids.Length, atom_ids, desired_xyz));
        }

        public float alignAtoms(ICollection atom_ids, ICollection desired_xyz)
        {
            return alignAtoms(Indigo.toIntArray(atom_ids), Indigo.toFloatArray(desired_xyz));
        }

        public void aromatize()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAromatize(self));
        }

        public void dearomatize()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoDearomatize(self));
        }

        public void foldHydrogens()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoFoldHydrogens(self));
        }

        public void unfoldHydrogens()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoUnfoldHydrogens(self));
        }

        public void layout()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoLayout(self));
        }

        public void clean2d()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClean2d(self));
        }


        public string smiles()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSmiles(self));
        }

        public string smarts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoSmarts(self));
        }

        public string canonicalSmarts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCanonicalSmarts(self));
        }

        public string name()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoName(self));
        }

        public void setName(string name)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetName(self, name));
        }

        public byte[] serialize()
        {
            dispatcher.setSessionID();
            byte* buf;
            int bufsize;
            dispatcher.checkResult(IndigoLib.indigoSerialize(self, &buf, &bufsize));

            byte[] res = new byte[bufsize];
            for (int i = 0; i < bufsize; ++i)
            {
                res[i] = buf[i];
            }

            return res;
        }

        public bool hasProperty(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoHasProperty(self, name)) == 1;
        }

        public string getProperty(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoGetProperty(self, name));
        }

        public void setProperty(string name, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSetProperty(self, name, value));
        }

        public void removeProperty(string name)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoRemoveProperty(self, name));
        }

        public IEnumerable iterateProperties()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateProperties(self)), this);
        }

        public void clearProperties()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClearProperties(self));
        }

        public string checkBadValence()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckBadValence(self));
        }

        public string checkAmbiguousH()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckAmbiguousH(self));
        }

        public int checkChirality()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckChirality(self));
        }

        public int check3DStereo()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheck3DStereo(self));
        }

        public int checkStereo()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckStereo(self));
        }

        public string check()
        {
            return check("");
        }

        public string check(string type)
        {
            if (type == null)
            {
                type = "";
            }

            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCheckObj(self, type));
        }

        public IndigoObject fingerprint()
        {
            return fingerprint("");
        }

        public IndigoObject fingerprint(string type)
        {
            if (type == null)
            {
                type = "";
            }

            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoFingerprint(self, type)));
        }

        public string oneBitsList()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoOneBitsList(self));
        }

        public int countBits()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountBits(self));
        }

        public string rawData()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoRawData(self));
        }

        public int tell()
        {

            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoTell(self));
        }

        public void sdfAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSdfAppend(self, item.self));
        }

        public void smilesAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoSmilesAppend(self, item.self));
        }

        public void rdfHeader()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoRdfHeader(self));
        }

        public void rdfAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoRdfAppend(self, item.self));
        }

        public void cmlHeader()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoCmlHeader(self));
        }

        public void cmlAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoCmlAppend(self, item.self));
        }

        public void cmlFooter()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoCmlFooter(self));
        }

        public int arrayAdd(IndigoObject item)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoArrayAdd(self, item.self));
        }

        public IndigoObject at(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoAt(self, index)), this);
        }

        public int count()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCount(self));
        }

        public void clear()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoClear(self));
        }

        public IEnumerable iterateArray()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateArray(self)), this);
        }

        public void ignoreAtom(IndigoObject atom)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoIgnoreAtom(self, atom.self));
        }

        public void unignoreAtom(IndigoObject atom)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoUnignoreAtom(self, atom.self));
        }

        public void unignoreAllAtoms()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoUnignoreAllAtoms(self));
        }

        public IndigoObject match(IndigoObject query)
        {
            dispatcher.setSessionID();
            int res = dispatcher.checkResult(IndigoLib.indigoMatch(self, query.self));
            if (res == 0)
            {
                return null;
            }

            return new IndigoObject(dispatcher, res, this);
        }

        public int countMatches(IndigoObject query)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountMatches(self, query.self));
        }

        public int countMatchesWithLimit(IndigoObject query, int embeddings_limit)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoCountMatchesWithLimit(self, query.self, embeddings_limit));
        }

        public IEnumerable iterateMatches(IndigoObject query)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateMatches(self, query.self)), this);
        }

        public IndigoObject highlightedTarget()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoHighlightedTarget(self)));
        }

        public IndigoObject mapAtom(IndigoObject query_atom)
        {
            dispatcher.setSessionID();
            int mapped = dispatcher.checkResult(IndigoLib.indigoMapAtom(self, query_atom.self));
            if (mapped == 0)
            {
                return null;
            }

            return new IndigoObject(dispatcher, mapped);
        }

        public IndigoObject mapMolecule(IndigoObject query_reaction_molecule)
        {
            dispatcher.setSessionID();
            int mapped = dispatcher.checkResult(IndigoLib.indigoMapMolecule(self, query_reaction_molecule.self));
            if (mapped == 0)
            {
                return null;
            }

            return new IndigoObject(dispatcher, mapped);
        }

        public IndigoObject mapBond(IndigoObject query_bond)
        {
            dispatcher.setSessionID();
            int mapped = dispatcher.checkResult(IndigoLib.indigoMapBond(self, query_bond.self));
            if (mapped == 0)
            {
                return null;
            }

            return new IndigoObject(dispatcher, mapped);
        }

        public IndigoObject allScaffolds()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoAllScaffolds(self)));
        }

        public IndigoObject decomposedMoleculeScaffold()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoDecomposedMoleculeScaffold(self)));
        }

        public IEnumerable iterateDecomposedMolecules()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoIterateDecomposedMolecules(self)), this);
        }

        public IndigoObject decomposedMoleculeHighlighted()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoDecomposedMoleculeHighlighted(self)));
        }

        public IndigoObject decomposedMoleculeWithRGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(IndigoLib.indigoDecomposedMoleculeWithRGroups(self)));
        }

        public IndigoObject decomposeMolecule(IndigoObject mol)
        {
            dispatcher.setSessionID();
            int res = dispatcher.checkResult(IndigoLib.indigoDecomposeMolecule(self, mol.self));
            if (res == 0)
            {
                return null;
            }

            return new IndigoObject(dispatcher, res, this);
        }

        public IEnumerable iterateDecompositions()
        {
            dispatcher.setSessionID();
            int res = dispatcher.checkResult(IndigoLib.indigoIterateDecompositions(self));
            if (res == 0)
            {
                return null;
            }

            return new IndigoObject(dispatcher, res, this);
        }

        public void addDecomposition(IndigoObject q_match)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAddDecomposition(self, q_match.self));
        }
        public IEnumerator GetEnumerator()
        {
            while (true)
            {
                dispatcher.setSessionID();
                int next = dispatcher.checkResult(IndigoLib.indigoNext(self));
                if (next == 0)
                {
                    break;
                }

                yield return new IndigoObject(dispatcher, next, this);
            }
        }

        public IndigoObject next()
        {
            dispatcher.setSessionID();
            int next = dispatcher.checkResult(IndigoLib.indigoNext(self));
            if (next == 0)
            {
                return null;
            }

            return new IndigoObject(dispatcher, next, this);
        }

        public bool hasNext()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoHasNext(self)) == 1;
        }

        public int index()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoIndex(self));
        }

        public string toString()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoToString(self));
        }

        public byte[] toBuffer()
        {
            byte* buf;
            int bufsize;
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoToBuffer(self, &buf, &bufsize));
            byte[] res = new byte[bufsize];
            for (int i = 0; i < bufsize; ++i)
            {
                res[i] = buf[i];
            }

            return res;
        }

        public void append(IndigoObject obj)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoAppend(self, obj.self));
        }

        public void optimize()
        {
            optimize(null);
        }

        public void optimize(string options)
        {
            if (options == null)
            {
                options = "";
            }

            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoOptimize(self, options));
        }

        public bool normalize()
        {
            return normalize(null);
        }

        public bool normalize(string options)
        {
            if (options == null)
            {
                options = "";
            }

            dispatcher.setSessionID();
            return (dispatcher.checkResult(IndigoLib.indigoNormalize(self, options)) == 1);
        }

        public void standardize()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoStandardize(self));
        }

        public void ionize(float pH, float pHToll)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(IndigoLib.indigoIonize(self, pH, pHToll));
        }

        public float getAcidPkaValue(IndigoObject atom, int level, int min_level)
        {
            dispatcher.setSessionID();
            float* ptr = dispatcher.checkResult(IndigoLib.indigoGetAcidPkaValue(self, atom.self, level, min_level));
            float pka = ptr[0];
            return pka;
        }

        public float getBasicPkaValue(IndigoObject atom, int level, int min_level)
        {
            dispatcher.setSessionID();
            float* ptr = dispatcher.checkResult(IndigoLib.indigoGetBasicPkaValue(self, atom.self, level, min_level));
            float pka = ptr[0];
            return pka;
        }

        public int buildPkaModel(int level, float threshold, string filename)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoBuildPkaModel(level, threshold, filename));
        }

        public int expandAbbreviations()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoExpandAbbreviations(self));
        }

        public int nameToStructure(string name, string parameters)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoNameToStructure(name, parameters));
        }

        public string dbgInternalType()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(IndigoLib.indigoDbgInternalType(self));
        }
    }
}
