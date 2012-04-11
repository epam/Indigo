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
            _inchi_lib = dll_loader.getInterface<IndigoInchiLib>("indigo-renderer.dll");

            _indigo = indigo;
        }

        public String indigoInchiVersion()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiVersion());
        }

        public int indigoInchiResetOptions()
        {
            _indigo.setSessionID();
            return _inchi_lib.indigoInchiResetOptions();
        }

        public String indigoInchiGetInchi(int molecule)
        {
               _indigo.setSessionID();
               return new String(_inchi_lib.indigoInchiGetInchi(molecule));
        }

        public String indigoInchiGetInchiKey(String inchi_string)
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetInchiKey(inchi_string));
        }

        public String indigoInchiGetWarning()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetWarning());
        }

        public String indigoInchiGetLog()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetLog());
        }

        public String indigoInchiGetAuxInfo()
        {
            _indigo.setSessionID();
            return new String(_inchi_lib.indigoInchiGetAuxInfo());
        }
    }
}
