using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace com.ggasoftware.indigo
{
    public unsafe class IndigoInchi
    {
        private Indigo _indigo;
        private IndigoInchiLib _inchi_lib;

        public IndigoInchi(Indigo indigo)
        {
            String dllpath = indigo.getDllPath();

            IndigoDllLoader dll_loader = IndigoDllLoader.Instance;
            dll_loader.loadLibrary(dllpath, "indigo-inchi.dll",
               "com.ggasoftware.indigo.Properties.Resources", false);
            _inchi_lib = dll_loader.getInterface<IndigoInchiLib>("indigo-inchi.dll");

            _indigo = indigo;
        }

        public String version()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiVersion());
        }

        public int resetOptions()
        {
            _indigo.setSessionID();
            return _inchi_lib.indigoInchiResetOptions();
        }

        public IndigoObject loadMolecule(String inchi_string)
        {
            _indigo.setSessionID();
            return new IndigoObject(_indigo, _inchi_lib.indigoInchiLoadMolecule(inchi_string));
        }

        public String getInchi(IndigoObject molecule)
        {
               _indigo.setSessionID();
               return new String(_inchi_lib.indigoInchiGetInchi(molecule.self));
        }

        public String getInchiKey(String inchi_string)
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetInchiKey(inchi_string));
        }

        public String getWarning()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetWarning());
        }

        public String getLog()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetLog());
        }

        public String getAuxInfo()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetAuxInfo());
        }
    }
}
