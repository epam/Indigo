using System;
using System.Collections.Generic;
using System.Text;

namespace com.epam.indigo
{
    public unsafe interface IndigoInchiLib
    {
        sbyte* indigoInchiVersion();
        int indigoInchiResetOptions();
        int indigoInchiLoadMolecule(String inchi_string);
        sbyte* indigoInchiGetInchi(int molecule);
        sbyte* indigoInchiGetInchiKey(String inchi_string);
        sbyte* indigoInchiGetWarning();
        sbyte* indigoInchiGetLog();
        sbyte* indigoInchiGetAuxInfo();
    }
}