using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace com.epam.indigo
{
    public unsafe class IndigoObject : IEnumerable, IDisposable
    {
        public int self;
        private Indigo dispatcher;
        private object parent; // to prevent GC killing the parent object
        private IndigoLib _indigo_lib;
        private IndigoDllLoader dll_loader;

        public IndigoObject(Indigo dispatcher, int id) : this(dispatcher, id, null)
        {
            dll_loader = IndigoDllLoader.Instance;
        }

        public IndigoObject(Indigo dispatcher, int id, object parent)
        {
            this.dispatcher = dispatcher;
            this.self = id;
            this.parent = parent;
            _indigo_lib = dispatcher._indigo_lib;
            dll_loader = IndigoDllLoader.Instance;
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
                if (dll_loader.isValid())
                {
                    if (dispatcher.getSID() >= 0)
                    {
                        dispatcher.setSessionID();
                        dispatcher.free(self);
                        self = -1;
                    }
                }
            }
        }

        public void dispose()
        {
            Dispose();
        }

        public Indigo indigo ()
        {
            return dispatcher;
        }

        public IndigoObject clone()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoClone(self)));
        }

        public void close()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClose(self));
        }

        public string molfile()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoMolfile(self));
        }

        public void saveMolfile(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(_indigo_lib.indigoWriteFile(filename));
            dispatcher.checkResult(_indigo_lib.indigoSaveMolfile(self, s));
            dispatcher.checkResult(_indigo_lib.indigoFree(s));
        }

        public string cml()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCml(self));
        }

        public void saveCml(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(_indigo_lib.indigoWriteFile(filename));
            dispatcher.checkResult(_indigo_lib.indigoSaveCml(self, s));
            dispatcher.checkResult(_indigo_lib.indigoFree(s));
        }

        public string cdxml()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCdxml(self));
        }

        public void saveCdxml(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(_indigo_lib.indigoWriteFile(filename));
            dispatcher.checkResult(_indigo_lib.indigoSaveCdxml(self, s));
            dispatcher.checkResult(_indigo_lib.indigoFree(s));
        }

        public byte[] mdlct()
        {
            dispatcher.setSessionID();
            IndigoObject buf = dispatcher.writeBuffer();
            dispatcher.checkResult(_indigo_lib.indigoSaveMDLCT(self, buf.self));
            return buf.toBuffer();
        }

        public void addReactant(IndigoObject molecule)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddReactant(self, molecule.self));
        }

        public void addProduct(IndigoObject molecule)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddProduct(self, molecule.self));
        }

        public void addCatalyst(IndigoObject molecule)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddCatalyst(self, molecule.self));
        }

        public int countReactants()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountReactants(self));
        }

        public int countProducts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountProducts(self));
        }

        public int countCatalysts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountCatalysts(self));
        }

        public int countMolecules()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountMolecules(self));
        }

        public System.Collections.IEnumerable iterateReactants()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateReactants(self)), this);
        }

        public System.Collections.IEnumerable iterateProducts()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateProducts(self)), this);
        }

        public System.Collections.IEnumerable iterateCatalysts()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateCatalysts(self)), this);
        }

        public System.Collections.IEnumerable iterateMolecules()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateMolecules(self)), this);
        }

        public string rxnfile()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoRxnfile(self));
        }

        public void saveRxnfile(string filename)
        {
            dispatcher.setSessionID();
            int s = dispatcher.checkResult(_indigo_lib.indigoWriteFile(filename));
            dispatcher.checkResult(_indigo_lib.indigoSaveRxnfile(self, s));
            dispatcher.checkResult(_indigo_lib.indigoFree(s));
        }

        public void automap()
        {
            automap("");
        }

        public void automap(string mode)
        {
            if (mode == null)
                mode = "";
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAutomap(self, mode));
        }

        public int atomMappingNumber(IndigoObject reaction_atom)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetAtomMappingNumber(self, reaction_atom.self));
        }

        public void setAtomMappingNumber(IndigoObject reaction_atom, int number)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetAtomMappingNumber(self, reaction_atom.self, number));
        }

        public ReactingCenter reactingCenter(IndigoObject bond)
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(_indigo_lib.indigoGetReactingCenter(self, bond.self, &c)) == 1)
                return (ReactingCenter)c;
            throw new IndigoException("reactingCenter(): unexpected result");
        }

        public void setReactingCenter(IndigoObject bond, ReactingCenter type)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetReactingCenter(self, bond.self, (int)type));
        }

        public void clearAAM()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClearAAM(self));
        }

        public void correctReactingCenters()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoCorrectReactingCenters(self));
        }

        public System.Collections.IEnumerable iterateAtoms()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateAtoms(self)), this);
        }

        public System.Collections.IEnumerable iteratePseudoatoms()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIteratePseudoatoms(self)), this);
        }

        public System.Collections.IEnumerable iterateRSites()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateRSites(self)), this);
        }

        public System.Collections.IEnumerable iterateStereocenters()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateStereocenters(self)), this);
        }

        public System.Collections.IEnumerable iterateAlleneCenters()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateAlleneCenters(self)), this);
        }

        public System.Collections.IEnumerable iterateRGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateRGroups(self)), this);
        }

        public System.Collections.IEnumerable iterateRGroupFragments()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateRGroupFragments(self)), this);
        }

        public int countRGroups()                                          
        {                                                                   
            dispatcher.setSessionID();                                       
            return dispatcher.checkResult(_indigo_lib.indigoCountRGroups(self));  
        }     

        public int countAttachmentPoints()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountAttachmentPoints(self));
        }

        public bool isPseudoatom()
        {
            dispatcher.setSessionID();
            if (dispatcher.checkResult(_indigo_lib.indigoIsPseudoatom(self)) == 1)
                return true;
            return false;
        }

        public bool isRSite()
        {
            dispatcher.setSessionID();
            if (dispatcher.checkResult(_indigo_lib.indigoIsRSite(self)) == 1)
                return true;
            return false;
        }

        public bool isTemplateAtom()
        {
            dispatcher.setSessionID();
            if (dispatcher.checkResult(_indigo_lib.indigoIsTemplateAtom(self)) == 1)
                return true;
            return false;
        }

        public int stereocenterType()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoStereocenterType(self));
        }

        public int stereocenterGroup()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoStereocenterGroup(self));
        }

        public int[] stereocenterPyramid()
        {
            dispatcher.setSessionID();
            int* pyramid_ptr = dispatcher.checkResult(_indigo_lib.indigoStereocenterPyramid(self));

            int[] res = new int[4];
            for (int i = 0; i < 4; ++i)
                res[i] = pyramid_ptr[i];
            return res;
        }

        public void changeStereocenterType(int type)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoChangeStereocenterType(self, type));
        }

        public void setStereocenterGroup(int group)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetStereocenterGroup(self, group));
        }

        public int singleAllowedRGroup()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSingleAllowedRGroup(self));
        }

        public string symbol()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSymbol(self));
        }

        public int degree()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoDegree(self));
        }

        public int? charge()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(_indigo_lib.indigoGetCharge(self, &c)) == 1)
                return c;
            return null;
        }

        public int? explicitValence()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(_indigo_lib.indigoGetExplicitValence(self, &c)) == 1)
                return c;
            return null;
        }

        public int? radicalElectrons()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(_indigo_lib.indigoGetRadicalElectrons(self, &c)) == 1)
                return c;
            return null;
        }

        public int? radical()
        {
            int c;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(_indigo_lib.indigoGetRadical(self, &c)) == 1)
                return c;
            return null;
        }

        public int atomicNumber()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoAtomicNumber(self));
        }

        public int isotope()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoIsotope(self));
        }

        public int valence()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoValence(self));
        }

        public int checkValence()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheckValence(self));
        }

        public int checkQuery()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheckQuery(self));
        }

        public int checkRGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheckRGroups(self));
        }

        public int? countHydrogens()
        {
            int h;
            dispatcher.setSessionID();

            if (dispatcher.checkResult(_indigo_lib.indigoCountHydrogens(self, &h)) == 1)
                return h;
            return null;
        }

        public int countImplicitHydrogens()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountImplicitHydrogens(self));
        }

        public int countSuperatoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountSuperatoms(self));
        }

        public int countDataSGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountDataSGroups(self));
        }

        public int countGenericSGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountGenericSGroups(self));
        }

        public int countRepeatingUnits()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountRepeatingUnits(self));
        }

        public int countMultipleGroups()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountMultipleGroups(self));
        }

        public IndigoObject iterateSuperatoms()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateSuperatoms(self)), this);
        }

        public IndigoObject iterateAttachmentPoints(int order)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateAttachmentPoints(self, order)), this);
        }

        public IndigoObject iterateDataSGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateDataSGroups(self)), this);
        }

        public IndigoObject iterateGenericSGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateGenericSGroups(self)), this);
        }

        public IndigoObject iterateRepeatingUnits()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateRepeatingUnits(self)), this);
        }

        public IndigoObject iterateMultipleGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateMultipleGroups(self)), this);
        }

        public IndigoObject iterateSGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateSGroups(self)), this);
        }

        public IndigoObject iterateTGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateTGroups(self)), this);
        }

        public IndigoObject getDataSGroup(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetDataSGroup(self, index)), this);
        }

        public IndigoObject getSuperatom(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetSuperatom(self, index)), this);
        }

        public IndigoObject getGenericSGroup(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetGenericSGroup(self, index)), this);
        }

        public IndigoObject getMultipleGroup(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetMultipleGroup(self, index)), this);
        }

        public IndigoObject getRepeatingUnit(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetRepeatingUnit(self, index)), this);
        }

        public string description()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoDescription(self));
        }

        public string data()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoData(self));
        }

        public IndigoObject addDataSGroup(int[] atoms, int[] bonds, string description, string data)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoAddDataSGroup(self, atoms.Length, atoms, bonds.Length, bonds, description, data)));
        }

        public IndigoObject addDataSGroup(ICollection atoms, ICollection bonds, string description, string data)
        {
            return addDataSGroup(Indigo.toIntArray(atoms), Indigo.toIntArray(bonds), description, data);
        }

        public IndigoObject addSuperatom(int[] atoms, string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoAddSuperatom(self, atoms.Length, atoms, name)));
        }

        public IndigoObject addSuperatom(ICollection atoms, string name)
        {
            return addSuperatom(Indigo.toIntArray(atoms), name);
        }

        public IndigoObject createSGroup(string type, IndigoObject mapping, string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoCreateSGroup(type, mapping.self, name)));
        }

        public string getRepeatingUnitSubscript()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetRepeatingUnitSubscript(self));
        }

        public int getRepeatingUnitConnectivity()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetRepeatingUnitConnectivity(self));
        }

        public void setSGroupClass(string sgclass)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetSGroupClass(self, sgclass));
        }

        public void setSGroupName(string sgname)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetSGroupName(self, sgname));
        }

        public string getSGroupClass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupClass(self));
        }

        public string getSGroupName()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupName(self));
        }

        public int getSGroupNumCrossBonds()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupNumCrossBonds(self));
        }

        public int addSGroupAttachmentPoint(int aidx, int lvidx, string apid)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoAddSGroupAttachmentPoint(self, aidx, lvidx, apid));
        }

        public int deleteSGroupAttachmentPoint(int apidx)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoDeleteSGroupAttachmentPoint(self, apidx));
        }

        public int getSGroupDisplayOption()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupDisplayOption(self));
        }

        public int setSGroupDisplayOption(int option)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSetSGroupDisplayOption(self, option));
        }

        public int getSGroupSeqId()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupSeqId(self));
        }

        public float[] getSGroupCoords()
        {
            dispatcher.setSessionID();
            float* ptr = dispatcher.checkResult(_indigo_lib.indigoGetSGroupCoords(self));
            float[] res = new float[2];
            res[0] = ptr[0];
            res[1] = ptr[1];
            return res;
        }

        public int getSGroupMultiplier()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupMultiplier(self));
        }

        public int setSGroupMultiplier(int mult)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSetSGroupMultiplier(self, mult));
        }

        public int setSGroupData (string data)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupData(self, data));
        }
        public int setSGroupCoords (float x, float y)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupCoords(self, x, y));
        }
        public int setSGroupDescription (string description)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupDescription(self, description));
        }
        public int setSGroupFieldName (string name)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupFieldName(self, name));
        }
        public int setSGroupQueryCode (string querycode)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupQueryCode(self, querycode));
        }
        public int setSGroupQueryOper (string queryoper)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupQueryOper(self, queryoper));
        }
        public int setSGroupDisplay (string option)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupDisplay(self, option));
        }
        public int setSGroupLocation (string option)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupLocation(self, option));
        }
        public int setSGroupTag (string tag)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupTag(self, tag));
        }
        public int setSGroupTagAlign (int tag_align)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupTagAlign(self, tag_align));
        }
        public int setSGroupDataType (string data_type)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupDataType(self, data_type));
        }
        public int setSGroupXCoord (float x)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupXCoord(self, x));
        }
        public int setSGroupYCoord (float y)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupYCoord(self, y));
        }
        public int setSGroupBrackets(int brk_style, float x1, float y1, float x2, float y2,
                                     float x3, float y3, float x4, float y4)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoSetSGroupBrackets(self, brk_style, x1, y1, x2, y2, x3, y3, x4, y4));
        }

        public IndigoObject findSGroups(string property, string value)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoFindSGroups(self, property, value)), this);
        }

        public int getSGroupType()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupType(self));
        }

        public int getSGroupIndex()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupIndex(self));
        }

        public int getSGroupOriginalId()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupOriginalId(self));
        }

        public int setSGroupOriginalId(int original)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSetSGroupOriginalId(self, original));
        }

        public int getSGroupParentId()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetSGroupParentId(self));
        }

        public int setSGroupParentId(int parent)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSetSGroupParentId(self, parent));
        }

        public int addTemplate(IndigoObject templates, string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoAddTemplate(self, templates.self, name));
        }

        public int removeTemplate(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoRemoveTemplate(self, name));
        }

        public int findTemplate(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoFindTemplate(self, name));
        }

        public string getTGroupClass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetTGroupClass(self));
        }

        public string getTGroupName()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetTGroupName(self));
        }

        public string getTGroupAlias()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetTGroupAlias(self));
        }

        public int transformSCSRtoCTAB()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoTransformSCSRtoCTAB(self));
        }

        public int transformCTABtoSCSR(IndigoObject templates)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoTransformCTABtoSCSR(self, templates.self));
        }

        public string getTemplateAtomClass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetTemplateAtomClass(self));
        }

        public int setTemplateAtomClass(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSetTemplateAtomClass(self, name));
        }

        public void addStereocenter(int type, int v1, int v2, int v3)
        {
            addStereocenter(type, v1, v2, v3, -1);
        }

        public void addStereocenter(int type, int v1, int v2, int v3, int v4)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddStereocenter(self, type, v1, v2, v3, v4));
        }

        public void setDataSGroupXY(float x, float y)
        {
            setDataSGroupXY(x, y, "");
        }

        public void setDataSGroupXY(float x, float y, string options)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetDataSGroupXY(self, x, y, options));
        }

        public float[] xyz()
        {
            dispatcher.setSessionID();
            float* ptr = dispatcher.checkResult(_indigo_lib.indigoXYZ(self));
            float[] res = new float[3];
            res[0] = ptr[0];
            res[1] = ptr[1];
            res[2] = ptr[2];
            return res;
        }

        public void setXYZ(float x, float y, float z)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetXYZ(self, x, y, z));
        }

        public void resetCharge()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoResetCharge(self));
        }

        public void resetExplicitValence()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoResetExplicitValence(self));
        }

        public void resetRadical()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoResetRadical(self));
        }

        public void resetIsotope()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoResetIsotope(self));
        }

        public void setAttachmentPoint(int order)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetAttachmentPoint(self, order));
        }

        public void clearAttachmentPoints()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClearAttachmentPoints(self));
        }

        public void removeConstraints(string type)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoRemoveConstraints(self, type));
        }

        public void addConstraint(string type, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddConstraint(self, type, value));
        }

        public void addConstraintNot(string type, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddConstraintNot(self, type, value));
        }

        public void addConstraintOr(string type, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddConstraintOr(self, type, value));
        }

        public void invertStereo()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoInvertStereo(self));
        }

        public void resetStereo()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoResetStereo(self));
        }

        public int countAtoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountAtoms(self));
        }

        public int countBonds()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountBonds(self));
        }

        public int countPseudoatoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountPseudoatoms(self));
        }

        public int countRSites()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountRSites(self));
        }

        public System.Collections.IEnumerable iterateBonds()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateBonds(self)), this);
        }

        public int bondOrder()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoBondOrder(self));
        }

        public int bondStereo()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoBondStereo(self));
        }

        public int topology()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoTopology(self));
        }

        public System.Collections.IEnumerable iterateNeighbors()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateNeighbors(self)), this);
        }

        public IndigoObject bond()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoBond(self)));
        }

        public IndigoObject getAtom(int idx)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetAtom(self, idx)), this);
        }

        public IndigoObject getBond(int idx)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetBond(self, idx)), this);
        }

        public IndigoObject getMolecule(int idx)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetMolecule(self, idx)), this);
        }

        public IndigoObject source()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoSource(self)));
        }

        public IndigoObject destination()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoDestination(self)));
        }

        public void clearCisTrans()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClearCisTrans(self));
        }

        public void clearStereocenters()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClearStereocenters(self));
        }

        public void clearAlleneCenters()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClearAlleneCenters(self));
        }

        public int countStereocenters()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountStereocenters(self));
        }

        public int countAlleneCenters()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountAlleneCenters(self));
        }

        public int resetSymmetricCisTrans()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoResetSymmetricCisTrans(self));
        }

        public int resetSymmetricStereocenters()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoResetSymmetricStereocenters(self));
        }

        public int markEitherCisTrans()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoMarkEitherCisTrans(self));
        }

        public int markStereobonds()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoMarkStereobonds(self));
        }

        public int validateChirality()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoValidateChirality(self));
        }

        public IndigoObject addAtom(string symbol)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoAddAtom(self, symbol)), this);
        }

        public IndigoObject resetAtom(string symbol)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoResetAtom(self, symbol)), this);
        }

        public IndigoObject addRSite(string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoAddRSite(self, name)), this);
        }

        public IndigoObject setRSite(string name)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoSetRSite(self, name)), this);
        }

        public void setCharge(int charge)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetCharge(self, charge));
        }

        public void setRadical(int radical)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetRadical(self, radical));
        }

        public void setExplicitValence(int valence)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetExplicitValence(self, valence));
        }

        public void setIsotope(int isotope)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetIsotope(self, isotope));
        }

        public void setImplicitHCount(int implh)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetImplicitHCount(self, implh));
        }

        public IndigoObject addBond(IndigoObject dest, int order)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoAddBond(self, dest.self, order)), this);
        }

        public void setBondOrder(int order)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetBondOrder(self, order));
        }

        public IndigoObject merge(IndigoObject what)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoMerge(self, what.self)), this);
        }

        public void highlight()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoHighlight(self));
        }

        public void unhighlight()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoUnhighlight(self));
        }

        public bool isHighlighted()
        {
            dispatcher.setSessionID();
            return (dispatcher.checkResult(_indigo_lib.indigoIsHighlighted(self)) == 1);
        }

        public int countComponents()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountComponents(self));
        }

        public int componentIndex()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoComponentIndex(self));
        }

        public void remove()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoRemove(self));
        }

        public IndigoObject iterateComponents()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateComponents(self)), this);
        }

        public IndigoObject component(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoComponent(self, index)), this);
        }

        public int countSSSR()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountSSSR(self));
        }

        public IndigoObject iterateSSSR()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateSSSR(self)), this);
        }

        public IndigoObject iterateSubtrees(int min_vertices, int max_vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateSubtrees(self, min_vertices, max_vertices)), this);
        }

        public IndigoObject iterateRings(int min_vertices, int max_vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateRings(self, min_vertices, max_vertices)), this);
        }

        public IndigoObject iterateEdgeSubmolecules(int min_edges, int max_edges)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateEdgeSubmolecules(self, min_edges, max_edges)), this);
        }

        public int countHeavyAtoms()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountHeavyAtoms(self));
        }

        public string grossFormula()
        {
            int gf = -1;
            try
            {
                dispatcher.setSessionID();
                gf = dispatcher.checkResult(_indigo_lib.indigoGrossFormula(self));
                string result = dispatcher.checkResult(_indigo_lib.indigoToString(gf));
                return result;
            }
            finally
            {
                dispatcher.checkResult(_indigo_lib.indigoFree(gf));
            }

        }

        public double molecularWeight()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoMolecularWeight(self));
        }

        public double mostAbundantMass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoMostAbundantMass(self));
        }

        public double monoisotopicMass()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoMonoisotopicMass(self));
        }

        public string massComposition()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoMassComposition(self));
        }

        public string canonicalSmiles()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCanonicalSmiles(self));
        }

        public int[] symmetryClasses()
        {
            dispatcher.setSessionID();
            int count;
            int* classes = dispatcher.checkResult(_indigo_lib.indigoSymmetryClasses(self, &count));

            int[] res = new int[count];
            for (int i = 0; i < count; ++i)
                res[i] = classes[i];
            return res;
        }

        public string layeredCode()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoLayeredCode(self));
        }

        public bool hasCoord()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoHasCoord(self)) == 1;
        }

        public bool hasZCoord()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoHasZCoord(self)) == 1;
        }

        public bool isChiral()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoIsChiral(self)) == 1;
        }

        public bool isPossibleFischerProjection(string options)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoIsPossibleFischerProjection(self, options)) == 1;
        }

        public IndigoObject createSubmolecule(int[] vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoCreateSubmolecule(self, vertices.Length, vertices)));
        }

        public IndigoObject createSubmolecule(ICollection vertices)
        {
            return createSubmolecule(Indigo.toIntArray(vertices));
        }

        public IndigoObject getSubmolecule(int[] vertices)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoGetSubmolecule(self, vertices.Length, vertices)));
        }

        public IndigoObject getSubmolecule(ICollection vertices)
        {
            return getSubmolecule(Indigo.toIntArray(vertices));
        }

        public IndigoObject createEdgeSubmolecule(int[] vertices, int[] edges)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoCreateEdgeSubmolecule(self, vertices.Length, vertices, edges.Length, edges)));
        }

        public IndigoObject createEdgeSubmolecule(ICollection vertices, ICollection edges)
        {
            return createEdgeSubmolecule(vertices, edges);
        }

        public void removeAtoms(int[] vertices)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoRemoveAtoms(self, vertices.Length, vertices));
        }

        public void removeAtoms(ICollection vertices)
        {
            removeAtoms(Indigo.toIntArray(vertices));
        }

        public void removeBonds(int[] bonds)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoRemoveBonds(self, bonds.Length, bonds));
        }

        public void removeBonds(ICollection bonds)
        {
            removeBonds(Indigo.toIntArray(bonds));
        }

        public float alignAtoms(int[] atom_ids, float[] desired_xyz)
        {
            if (atom_ids.Length * 3 != desired_xyz.Length)
                throw new IndigoException("alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]");
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoAlignAtoms(self, atom_ids.Length, atom_ids, desired_xyz));
        }

        public float alignAtoms(ICollection atom_ids, ICollection desired_xyz)
        {
            return alignAtoms(Indigo.toIntArray(atom_ids), Indigo.toFloatArray(desired_xyz));
        }

        public void aromatize()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAromatize(self));
        }

        public void dearomatize()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoDearomatize(self));
        }

        public void foldHydrogens()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoFoldHydrogens(self));
        }

        public void unfoldHydrogens()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoUnfoldHydrogens(self));
        }

        public void layout()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoLayout(self));
        }

        public void clean2d()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClean2d(self));
        }


        public string smiles()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSmiles(self));
        }

        public string smarts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoSmarts(self));
        }

        public string canonicalSmarts()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCanonicalSmarts(self));
        }

        public string name()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoName(self));
        }

        public void setName(string name)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetName(self, name));
        }

        public byte[] serialize()
        {
            dispatcher.setSessionID();
            byte* buf;
            int bufsize;
            dispatcher.checkResult(_indigo_lib.indigoSerialize(self, &buf, &bufsize));

            byte[] res = new byte[bufsize];
            for (int i = 0; i < bufsize; ++i)
                res[i] = buf[i];
            return res;
        }

        public bool hasProperty(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoHasProperty(self, name)) == 1;
        }

        public string getProperty(string name)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoGetProperty(self, name));
        }

        public void setProperty(string name, string value)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSetProperty(self, name, value));
        }

        public void removeProperty(string name)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoRemoveProperty(self, name));
        }

        public System.Collections.IEnumerable iterateProperties()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateProperties(self)), this);
        }

        public void clearProperties()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClearProperties(self));
        }

        public string checkBadValence()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheckBadValence(self));
        }

        public string checkAmbiguousH()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheckAmbiguousH(self));
        }

        public int checkChirality()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheckChirality(self));
        }

        public int check3DStereo()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheck3DStereo(self));
        }

        public int checkStereo()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheckStereo(self));
        }

        public string check(string type)
        {
            if (type == null)
                type = "";
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCheck(self, type));
        }

        public IndigoObject fingerprint(string type)
        {
            if (type == null)
                type = "";
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoFingerprint(self, type)));
        }

        public int countBits()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountBits(self));
        }

        public string rawData()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoRawData(self));
        }

        public int tell()
        {

            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoTell(self));
        }

        public void sdfAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSdfAppend(self, item.self));
        }

        public void smilesAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoSmilesAppend(self, item.self));
        }

        public void rdfHeader()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoRdfHeader(self));
        }

        public void rdfAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoRdfAppend(self, item.self));
        }

        public void cmlHeader()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoCmlHeader(self));
        }

        public void cmlAppend(IndigoObject item)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoCmlAppend(self, item.self));
        }

        public void cmlFooter()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoCmlFooter(self));
        }

        public int arrayAdd(IndigoObject item)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoArrayAdd(self, item.self));
        }

        public IndigoObject at(int index)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoAt(self, index)), this);
        }

        public int count()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCount(self));
        }

        public void clear()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoClear(self));
        }

        public System.Collections.IEnumerable iterateArray()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateArray(self)), this);
        }

        public void ignoreAtom(IndigoObject atom)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoIgnoreAtom(self, atom.self));
        }

        public void unignoreAtom(IndigoObject atom)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoUnignoreAtom(self, atom.self));
        }

        public void unignoreAllAtoms()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoUnignoreAllAtoms(self));
        }

        public IndigoObject match(IndigoObject query)
        {
            dispatcher.setSessionID();
            int res = dispatcher.checkResult(_indigo_lib.indigoMatch(self, query.self));
            if (res == 0)
                return null;
            return new IndigoObject(dispatcher, res, this);
        }

        public int countMatches(IndigoObject query)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountMatches(self, query.self));
        }

        public int countMatchesWithLimit(IndigoObject query, int embeddings_limit)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoCountMatchesWithLimit(self, query.self, embeddings_limit));
        }

        public System.Collections.IEnumerable iterateMatches(IndigoObject query)
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateMatches(self, query.self)), this);
        }

        public IndigoObject highlightedTarget()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoHighlightedTarget(self)));
        }

        public IndigoObject mapAtom(IndigoObject query_atom)
        {
            dispatcher.setSessionID();
            int mapped = dispatcher.checkResult(_indigo_lib.indigoMapAtom(self, query_atom.self));
            if (mapped == 0)
                return null;
            return new IndigoObject(dispatcher, mapped);
        }

        public IndigoObject mapMolecule(IndigoObject query_reaction_molecule)
        {
            dispatcher.setSessionID();
            int mapped = dispatcher.checkResult(_indigo_lib.indigoMapMolecule(self, query_reaction_molecule.self));
            if (mapped == 0)
                return null;
            return new IndigoObject(dispatcher, mapped);
        }

        public IndigoObject mapBond(IndigoObject query_bond)
        {
            dispatcher.setSessionID();
            int mapped = dispatcher.checkResult(_indigo_lib.indigoMapBond(self, query_bond.self));
            if (mapped == 0)
                return null;
            return new IndigoObject(dispatcher, mapped);
        }

        public IndigoObject allScaffolds()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoAllScaffolds(self)));
        }

        public IndigoObject decomposedMoleculeScaffold()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoDecomposedMoleculeScaffold(self)));
        }

        public System.Collections.IEnumerable iterateDecomposedMolecules()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoIterateDecomposedMolecules(self)), this);
        }

        public IndigoObject decomposedMoleculeHighlighted()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoDecomposedMoleculeHighlighted(self)));
        }

        public IndigoObject decomposedMoleculeWithRGroups()
        {
            dispatcher.setSessionID();
            return new IndigoObject(dispatcher, dispatcher.checkResult(_indigo_lib.indigoDecomposedMoleculeWithRGroups(self)));
        }

        public IndigoObject decomposeMolecule(IndigoObject mol)
        {
            dispatcher.setSessionID();
            int res = dispatcher.checkResult(_indigo_lib.indigoDecomposeMolecule(self, mol.self));
            if (res == 0)
                return null;
            return new IndigoObject(dispatcher, res, this);
        }

        public System.Collections.IEnumerable iterateDecompositions()
        {
            dispatcher.setSessionID();
            int res = dispatcher.checkResult(_indigo_lib.indigoIterateDecompositions(self));
            if (res == 0)
                return null;
            return new IndigoObject(dispatcher, res, this);
        }

        public void addDecomposition(IndigoObject q_match)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAddDecomposition(self, q_match.self));
        }
        public IEnumerator GetEnumerator()
        {
            while (true)
            {
                dispatcher.setSessionID();
                int next = dispatcher.checkResult(_indigo_lib.indigoNext(self));
                if (next == 0)
                    break;
                yield return new IndigoObject(dispatcher, next, this);
            }
        }

        public IndigoObject next()
        {
            dispatcher.setSessionID();
            int next = dispatcher.checkResult(_indigo_lib.indigoNext(self));
            if (next == 0)
                return null;
            return new IndigoObject(dispatcher, next, this);
        }

        public bool hasNext()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoHasNext(self)) == 1;
        }

        public int index()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoIndex(self));
        }

        public string toString()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoToString(self));
        }

        public byte[] toBuffer()
        {
            byte* buf;
            int bufsize;
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoToBuffer(self, &buf, &bufsize));
            byte[] res = new byte[bufsize];
            for (int i = 0; i < bufsize; ++i)
                res[i] = buf[i];
            return res;
        }

        public void append(IndigoObject obj)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoAppend(self, obj.self));
        }

        public void optimize()
        {
            optimize(null);
        }

        public void optimize(string options)
        {
            if (options == null)
                options = "";
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoOptimize(self, options));
        }

        public bool normalize()
        {
            return normalize(null);
        }

        public bool normalize(string options)
        {
            if (options == null)
                options = "";
            dispatcher.setSessionID();
            return (dispatcher.checkResult(_indigo_lib.indigoNormalize(self, options)) == 1);
        }

        public void standardize()
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoStandardize(self));
        }

        public void ionize(float pH, float pHToll)
        {
            dispatcher.setSessionID();
            dispatcher.checkResult(_indigo_lib.indigoIonize(self, pH, pHToll));
        }

        public float getAcidPkaValue(IndigoObject atom, int level, int min_level)
        {
           dispatcher.setSessionID();
           float* ptr = dispatcher.checkResult(_indigo_lib.indigoGetAcidPkaValue(self, atom.self, level, min_level));
           float pka = ptr[0];
           return pka;
        }

        public float getBasicPkaValue(IndigoObject atom, int level, int min_level)
        {
           dispatcher.setSessionID();
           float* ptr = dispatcher.checkResult(_indigo_lib.indigoGetBasicPkaValue(self, atom.self, level, min_level));
           float pka = ptr[0];
           return pka;
        }

        public int buildPkaModel(int level, float threshold, string filename)
        {
           dispatcher.setSessionID();
           return dispatcher.checkResult(_indigo_lib.indigoBuildPkaModel(level, threshold, filename));
        }

        public int expandAbbreviations()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoExpandAbbreviations(self));
        }

        public int nameToStructure(string name, string parameters)
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoNameToStructure(name, parameters));
        }

        public string dbgInternalType()
        {
            dispatcher.setSessionID();
            return dispatcher.checkResult(_indigo_lib.indigoDbgInternalType(self));
        }
    }
}
