using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace com.epam.indigo
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public unsafe class IndigoInchi
    {
        private Indigo _indigo;

        public IndigoInchi(Indigo indigo)
        {
            _indigo = indigo;
            resetOptions(); // Preloads native library to register renderer options
        }

        public String version()
        {
            _indigo.setSessionID();
            return _indigo.checkResult(IndigoInchiLib.indigoInchiVersion());
        }

        public int resetOptions()
        {
            _indigo.setSessionID();
            return _indigo.checkResult(IndigoInchiLib.indigoInchiResetOptions());
        }

        public IndigoObject loadMolecule(String inchi_string)
        {
            _indigo.setSessionID();
            return new IndigoObject(_indigo, _indigo.checkResult(IndigoInchiLib.indigoInchiLoadMolecule(inchi_string)));
        }

        public String getInchi(IndigoObject molecule)
        {
            _indigo.setSessionID();
            return _indigo.checkResult(IndigoInchiLib.indigoInchiGetInchi(molecule.self));
        }

        public String getInchiKey(String inchi_string)
        {
            _indigo.setSessionID();
            return _indigo.checkResult(IndigoInchiLib.indigoInchiGetInchiKey(inchi_string));
        }

        public String getWarning()
        {
            _indigo.setSessionID();
            return _indigo.checkResult(IndigoInchiLib.indigoInchiGetWarning());
        }

        public String getLog()
        {
            _indigo.setSessionID();
            return _indigo.checkResult(IndigoInchiLib.indigoInchiGetLog());
        }

        public String getAuxInfo()
        {
            _indigo.setSessionID();
            return _indigo.checkResult(IndigoInchiLib.indigoInchiGetAuxInfo());
        }
    }
}
