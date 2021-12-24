using System;
using System.Runtime.InteropServices;

namespace com.epam.indigo
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public unsafe class IndigoInchi : IDisposable
    {
        private Indigo _indigo;
        private bool initialized;

        public IndigoInchi(Indigo indigo)
        {
            _indigo = indigo;
            _indigo.setSessionID();
             // Preloads native library to register options
            _indigo.checkResult(IndigoInchiLib.indigoInchiInit());
            initialized = true;
        }

        ~IndigoInchi()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (initialized)
            {
                _indigo.setSessionID();
                _indigo.checkResult(IndigoInchiLib.indigoInchiDispose());
                initialized = false;
            }
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
