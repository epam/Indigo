/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#include "molecule/molecule_auto_loader.h"

#include "base_cpp/scanner.h"
#include "molecule/smiles_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/icm_loader.h"
#include "molecule/icm_saver.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "gzip/gzip_scanner.h"
#include "molecule/molecule_cml_loader.h"
#include "molecule/sdf_loader.h"
#include "molecule/molecule_cdx_loader.h"
#include "molecule/inchi_wrapper.h"

using namespace indigo;

void MoleculeAutoLoader::_init ()
{
   stereochemistry_options.reset();
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;
   ignore_noncritical_query_features = false;
   skip_3d_chirality = false;
   ignore_cistrans_errors = false;
}

IMPL_ERROR(MoleculeAutoLoader, "molecule auto loader");

CP_DEF(MoleculeAutoLoader);

MoleculeAutoLoader::MoleculeAutoLoader (Scanner &scanner) : CP_INIT, TL_CP_GET(properties)
{
   _scanner = &scanner;
   _own_scanner = false;
   _init();
}

MoleculeAutoLoader::MoleculeAutoLoader (const Array<char> &arr) : CP_INIT, TL_CP_GET(properties)
{
   _scanner = new BufferScanner(arr);
   _own_scanner = true;
   _init();
}

MoleculeAutoLoader::MoleculeAutoLoader (const char *str) : CP_INIT, TL_CP_GET(properties)
{
   _scanner = new BufferScanner(str);
   _own_scanner = true;
   _init();
}

MoleculeAutoLoader::~MoleculeAutoLoader ()
{
   if (_own_scanner)
      delete _scanner;
}

void MoleculeAutoLoader::loadMolecule (Molecule &mol)
{
   _loadMolecule(mol, false);
}

void MoleculeAutoLoader::loadQueryMolecule (QueryMolecule &qmol)
{
   _loadMolecule(qmol, true);
}

bool MoleculeAutoLoader::tryMDLCT (Scanner &scanner, Array<char> &outbuf)
{
   int pos = scanner.tell();
   bool endmark = false;
   QS_DEF(Array<char>, curline);

   outbuf.clear();
   while (!scanner.isEOF())
   {
      int len = scanner.readByte();

      if (len > 90) // Molfiles and Rxnfiles actually have 80 characters limit
      {
         scanner.seek(pos, SEEK_SET);
         // Garbage after endmark means end of data.
         // (See the note below about endmarks)
         if (endmark)
            return true;
         return false;
      }

      curline.clear();

      while (len-- > 0)
      {
         if (scanner.isEOF())
         {
            scanner.seek(pos, SEEK_SET);
            return false;
         }
         int c = scanner.readChar();
         curline.push(c);
      }

      curline.push(0);

      if (endmark)
      {
         // We can not properly read the data to the end as there
         // is often garbage after the actual MDLCT data.
         // Instead, we are doing this lousy check:
         // "M  END" or "$END MOL" can be followed only
         // by "$END CTAB" (in Markush queries), or
         // by "$MOL" (in Rxnfiles). Otherwise, this
         // is actually the end of data.
         if (strcmp(curline.ptr(), "$END CTAB") != 0 &&
             strcmp(curline.ptr(), "$MOL") != 0)
         {
            scanner.seek(pos, SEEK_SET);
            return true;
         }
      }

      if (strcmp(curline.ptr(), "M  END") == 0)
         endmark = true;
      else if (strcmp(curline.ptr(), "$END MOL") == 0)
         endmark = true;
      else
         endmark = false;

      outbuf.appendString(curline.ptr(), false);
      outbuf.push('\n');
   }
   scanner.seek(pos, SEEK_SET);
   // It happened once that a valid Molfile had successfully
   // made its way through the above while() cycle, and thus
   // falsely recognized as MDLCT. To fight this case, we include
   // here a check that the last line was actually an endmark
   return endmark;
}

void MoleculeAutoLoader::_loadMolecule (BaseMolecule &mol, bool query)
{
   properties.clear();

   // check for GZip format
   if (!query && _scanner->length() >= 2)
   {
      byte id[2];
      int pos = _scanner->tell();

      _scanner->readCharsFix(2, (char *)id);
      _scanner->seek(pos, SEEK_SET);

      if (id[0] == 0x1f && id[1] == 0x8b)
      {
         GZipScanner gzscanner(*_scanner);
         QS_DEF(Array<char>, buf);

         gzscanner.readAll(buf);
         MoleculeAutoLoader loader2(buf);

         loader2.stereochemistry_options = stereochemistry_options;
         loader2.ignore_noncritical_query_features = ignore_noncritical_query_features;
         loader2.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
         loader2.skip_3d_chirality = skip_3d_chirality;
         loader2.loadMolecule((Molecule &)mol);
         return;
      }
   }

   // check for MDLCT format
   {
      QS_DEF(Array<char>, buf);
      if (tryMDLCT(*_scanner, buf))
      {
         BufferScanner scanner2(buf);
         MolfileLoader loader(scanner2);
         loader.stereochemistry_options = stereochemistry_options;
         loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
         loader.skip_3d_chirality = skip_3d_chirality;
         loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;

         if (query)
            loader.loadQueryMolecule((QueryMolecule &)mol);
         else
            loader.loadMolecule((Molecule &)mol);
         return;
      }
   }

   // check for ICM format
   if (!query && _scanner->length() >= 4)
   {
      char id[3];
      int pos = _scanner->tell();

      _scanner->readCharsFix(3, id);
      _scanner->seek(pos, SEEK_SET);
      if (IcmSaver::checkVersion(id))
      {
         if (query)
            throw Error("cannot load query molecule from ICM format");

         IcmLoader loader(*_scanner);
         loader.loadMolecule((Molecule &)mol);
         return;
      }
   }

   // check for CML format
   {
      int pos = _scanner->tell();
      _scanner->skipSpace();

      if (_scanner->lookNext() == '<')
      {
         if (_scanner->findWord("<molecule"))
         {
            MoleculeCmlLoader loader(*_scanner);
            loader.stereochemistry_options = stereochemistry_options;
            if (query)
               throw Error("CML queries not supported");
            loader.loadMolecule(mol.asMolecule());
            return;
         }
      }

      _scanner->seek(pos, SEEK_SET);
   }

   // check for single line formats
   if (Scanner::isSingleLine(*_scanner)) {
      // check for InChI format
      {
         char prefix[6];
         int  start = _scanner->tell();
         _scanner->readCharsFix(6, prefix);
         _scanner->seek(start, SEEK_SET);

         if (!strncmp(prefix, "InChI=", 6)) {
            if (query) {
               throw Error("InChI input doesn't support query molecules");
            }

            Array<char> inchi;
            _scanner->readWord(inchi, " ");

            InchiWrapper loader;
            loader.loadMoleculeFromInchi(inchi.ptr(), (Molecule &)mol);
            return;
         }
      }

      // if not InChI then SMILES
      {
         SmilesLoader loader(*_scanner);

         loader.ignore_closing_bond_direction_mismatch =
            ignore_closing_bond_direction_mismatch;
         loader.stereochemistry_options = stereochemistry_options;
         loader.ignore_cistrans_errors = ignore_cistrans_errors;
         if (query)
            loader.loadQueryMolecule((QueryMolecule &)mol);
         else
            loader.loadMolecule((Molecule &)mol);
         return;
      }
   }

   // check for CDX format
/*
   {
      if (_scanner->findWord("VjCD0100"))
      {
         MoleculeCdxLoader loader(*_scanner);
         loader.stereochemistry_options = stereochemistry_options;
         if (query)
            throw Error("CDX queries not supported yet");
         loader.loadMolecule(mol.asMolecule());

         properties.copy(loader.properties);

         return;
      }
   }
*/
   // default is Molfile format
   {
      SdfLoader sdf_loader(*_scanner);
      sdf_loader.readNext();

      // Copy properties
      properties.copy(sdf_loader.properties);

      BufferScanner scanner2(sdf_loader.data);

      MolfileLoader loader(scanner2);
      loader.stereochemistry_options = stereochemistry_options;
      loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
      loader.skip_3d_chirality = skip_3d_chirality;
      loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;

      if (query)
         loader.loadQueryMolecule((QueryMolecule &)mol);
      else
         loader.loadMolecule((Molecule &)mol);
   }
}
