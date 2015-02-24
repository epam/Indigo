/****************************************************************************
 * Copyright (C) 2010-2015 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "indigo_iupac_inchi.h"

#include "molecule/molecule_iupac_inchi_core.h"

#include "indigo_internal.h"
#include "indigo_molecule.h"
#include "option_manager.h"


using namespace indigo;

CEXPORT const char* indigoInchiVersion ()
{
   return IndigoInchiEmbedded::version();
}

//
// Session Inchi instance
//
class IndigoInchiContext : public IndigoPluginContext
{
public:
   IndigoInchiEmbedded inchi;

   virtual void init ()
   {
      inchi.clear();
   }
};

_SessionLocalContainer<IndigoInchiContext> indigo_inchi_self;

IndigoInchiContext &indigoInchiGetInstance ()
{
   IndigoInchiContext &inst = indigo_inchi_self.getLocalCopy();
   inst.validate();
   return inst;
}

// 
// C interface functions
//

CEXPORT int indigoInchiResetOptions (void)
{
   IndigoInchiContext &indigo_inchi = indigoInchiGetInstance();
   indigo_inchi.init();
   return 0;
}

CEXPORT int indigoInchiLoadMolecule (const char *inchi_string)
{
   INDIGO_BEGIN
   {
   IndigoInchiEmbedded &indigo_inchi = indigoInchiGetInstance().inchi;

      AutoPtr<IndigoMolecule> mol_obj(new IndigoMolecule());

      const char *aux_prefix = "AuxInfo";
      if (strncmp(inchi_string, aux_prefix, strlen(aux_prefix)) == 0)
         indigo_inchi.loadMoleculeFromAux(inchi_string, mol_obj->mol);
      else
         indigo_inchi.loadMoleculeFromInchi(inchi_string, mol_obj->mol);
      return self.addObject(mol_obj.release());
   }
   INDIGO_END(-1)
}

CEXPORT const char* indigoInchiGetInchi (int molecule)
{
   INDIGO_BEGIN
   {
   IndigoInchiEmbedded &indigo_inchi = indigoInchiGetInstance().inchi;
      IndigoObject &obj = self.getObject(molecule);

      auto &tmp = self.getThreadTmpData();
      indigo_inchi.saveMoleculeIntoInchi(obj.getMolecule(), tmp.string);
      return tmp.string.ptr();
   }
   INDIGO_END(0)
}

CEXPORT const char* indigoInchiGetInchiKey (const char *inchi_string)
{
   INDIGO_BEGIN
   {
      auto &tmp = self.getThreadTmpData();
      IndigoInchiEmbedded::InChIKey(inchi_string, tmp.string);
      return tmp.string.ptr();
   }
   INDIGO_END(0)
}

CEXPORT const char* indigoInchiGetWarning ()
{
   IndigoInchiEmbedded &indigo_inchi = indigoInchiGetInstance().inchi;
   if (indigo_inchi.warning.size() != 0)
      return indigo_inchi.warning.ptr();
   return "";
}

CEXPORT const char* indigoInchiGetLog ()
{
   IndigoInchiEmbedded &indigo_inchi = indigoInchiGetInstance().inchi;
   if (indigo_inchi.log.size() != 0)
      return indigo_inchi.log.ptr();
   return "";
}

CEXPORT const char* indigoInchiGetAuxInfo ()
{
   IndigoInchiEmbedded &indigo_inchi = indigoInchiGetInstance().inchi;
   if (indigo_inchi.auxInfo.size() != 0)
      return indigo_inchi.auxInfo.ptr();
   return "";
}

//
// Options
//

void indigoInchiSetInchiOptions (const char *options)
{
   IndigoInchiEmbedded &inchi = indigoInchiGetInstance().inchi;
   inchi.setOptions(options);
}

class _IndigoInchiOptionsHandlersSetter
{
public:
   _IndigoInchiOptionsHandlersSetter ();
};

_IndigoInchiOptionsHandlersSetter::_IndigoInchiOptionsHandlersSetter ()
{
   OptionManager &mgr = indigoGetOptionManager();
   OsLocker locker(mgr.lock);
   
   mgr.setOptionHandlerString("inchi-options", indigoInchiSetInchiOptions);
}

_IndigoInchiOptionsHandlersSetter _indigo_inchi_options_handlers_setter;
