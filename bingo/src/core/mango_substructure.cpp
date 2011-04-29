/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "core/mango_matchers.h"
#include "base_cpp/scanner.h"
#include "core/bingo_error.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_saver.h"
#include "core/bingo_context.h"
#include "base_c/nano.h"
#include "base_cpp/output.h"
#include "layout/molecule_layout.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"
#include "core/mango_index.h"

#include "base_cpp/profiling.h"

MangoSubstructure::MangoSubstructure (BingoContext &context) :
_context(context)
{
   match_3d = 0;
   rms_threshold = 0;
   preserve_bonds_on_highlighting = false;
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;
   _use_pi_systems_matcher = false;
}

void MangoSubstructure::loadQuery (Scanner &scanner)
{
   MoleculeAutoLoader loader(scanner);
   QS_DEF(QueryMolecule, source);

   loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           ignore_closing_bond_direction_mismatch;
   loader.loadQueryMolecule(source);

   if (!source.have_xyz && match_3d != 0)
      throw Error("cannot do 3D match without XYZ in the query");

   _initQuery(source, _query);
   _query_fp_valid = false;
   _query_extra_valid = false;
}

void MangoSubstructure::loadSMARTS (Scanner &scanner)
{
   SmilesLoader loader(scanner);
   QS_DEF(QueryMolecule, source);

   loader.loadSMARTS(source);

   if (!source.have_xyz && match_3d != 0)
      throw Error("cannot do 3D match without XYZ in the query");

   _initSmartsQuery(source, _query);
   _query_fp_valid = false;
   _query_extra_valid = false;
   _use_pi_systems_matcher = false;
}


void MangoSubstructure::loadQuery (const Array<char> &buf)
{
   BufferScanner scanner(buf);

   loadQuery(scanner);
}

void MangoSubstructure::loadQuery (const char *str)
{
   BufferScanner scanner(str);

   loadQuery(scanner);
}

void MangoSubstructure::loadSMARTS (const Array<char> &buf)
{
   BufferScanner scanner(buf);

   loadSMARTS(scanner);
}

void MangoSubstructure::loadSMARTS (const char *str)
{
   BufferScanner scanner(str);

   loadSMARTS(scanner);
}

void MangoSubstructure::_validateQueryFP ()
{
   if (_query_fp_valid)
      return;

   MoleculeFingerprintBuilder builder(_query, _context.fp_parameters);

   builder.query = true;
   builder.skip_sim = true;
   builder.skip_tau = true;

   // atom charges and bond types may not match in pi-systems
   if (_use_pi_systems_matcher)
   {
      builder.skip_ord = true;
      builder.skip_any_atoms = true;
      builder.skip_ext_charge = true;
   }
   
   builder.process();
   _query_fp.copy(builder.get(), _context.fp_parameters.fingerprintSize());

   _query_fp_valid = true;
}

void MangoSubstructure::_validateQueryExtraData ()
{
   if (_query_extra_valid)
      return;

   _query_has_stereocenters = _query.stereocenters.size() > 0;
   _query_has_stereocare_bonds = _query.cis_trans.count() > 0;
   _query_extra_valid = true;
}

void MangoSubstructure::loadTarget (const Array<char> &molfile_buf)
{
   BufferScanner scanner(molfile_buf);

   loadTarget(scanner);
}

void MangoSubstructure::loadTarget (const char *target)
{
   BufferScanner scanner(target);

   loadTarget(scanner);
}

void MangoSubstructure::loadTarget (Scanner &scanner)
{
   MoleculeAutoLoader loader(scanner);

   loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           ignore_closing_bond_direction_mismatch;
   loader.loadMolecule(_target);
   _initTarget(false);
   Molecule::checkForConsistency(_target);
}

bool MangoSubstructure::matchLoadedTarget ()
{
   MoleculeSubstructureMatcher matcher(_target);

   matcher.match_3d = match_3d;
   matcher.rms_threshold = rms_threshold;
   matcher.highlight = true;
   matcher.use_pi_systems_matcher = _use_pi_systems_matcher;
   matcher.setNeiCounters(&_nei_query_counters, &_nei_target_counters);
   matcher.fmcache = &_fmcache;

   _fmcache.clear();

   matcher.setQuery(_query);

   profTimerStart(temb, "match.embedding");
   bool res = matcher.find();
   profTimerStop(temb);

   if (res)
   {
      profIncTimer("match.embedding_found", profTimerGetTime(temb));
   }
   else
   {
      profIncTimer("match.embedding_not_found", profTimerGetTime(temb));
   }
   return res;
}

void MangoSubstructure::loadBinaryTargetXyz (Scanner &xyz_scanner)
{
   cmf_loader->loadXyz(xyz_scanner);
}

void MangoSubstructure::getHighlightedTarget (Array<char> &molfile_buf)
{
   ArrayOutput output(molfile_buf);
   MolfileSaver saver(output);

   if (!_target.have_xyz)
   {
      MoleculeLayout ml(_target);
      ml.make();
      _target.stereocenters.markBonds();
   }

   if (preserve_bonds_on_highlighting)
      Molecule::loadBondOrders(_target, _target_bond_types);

   saver.saveMolecule(_target);
}

void MangoSubstructure::getHighlightedTarget_Smiles (Array<char> &smiles_buf)
{
   ArrayOutput output(smiles_buf);
   SmilesSaver saver(output);

   if (preserve_bonds_on_highlighting)
      Molecule::loadBondOrders(_target, _target_bond_types);

   saver.saveMolecule(_target);
}

void MangoSubstructure::_correctQueryStereo (QueryMolecule &query)
{
   // Remove stereobond marks that are connected with R-groups
   for (int v = query.vertexBegin(); 
            v != query.vertexEnd(); 
            v = query.vertexNext(v))
   {
      if (!query.isRSite(v))
         continue;
      const Vertex &vertex = query.getVertex(v);      
      for (int nei = vertex.neiBegin(); 
               nei != vertex.neiEnd();
               nei = vertex.neiNext(nei))
      {
         int edge = vertex.neiEdge(nei);
         if (query.cis_trans.getParity(edge) != 0)
            query.cis_trans.setParity(edge, 0);
      }
   }

   MoleculeRGroups &rgroups = query.rgroups;
   int n_rgroups = rgroups.getRGroupCount();
   for (int i = 1; i <= n_rgroups; i++)
   {
      RGroup &rgroup = rgroups.getRGroup(i);
      if (rgroup.fragments.size() > 0)
      {
         for (int j = 0; j < rgroup.fragments.size(); j++)
         {
            QueryMolecule &fragment = *rgroup.fragments[j];
            _correctQueryStereo(fragment);
         }
      }
   }

}

void MangoSubstructure::_initQuery (QueryMolecule &query_in, QueryMolecule &query_out)
{
   _correctQueryStereo(query_in);

   QueryMoleculeAromatizer::aromatizeBonds(query_in);
   _nei_query_counters.calculate(query_in);

   QS_DEF(Array<int>, transposition);
  
   _nei_query_counters.makeTranspositionForSubstructure(query_in, transposition);

   query_out.makeSubmolecule(query_in, transposition, 0);
   _nei_query_counters.calculate(query_out);
}

void MangoSubstructure::_initSmartsQuery (QueryMolecule &query_in, QueryMolecule &query_out)
{
   QS_DEF(Array<int>, transposition);

   MoleculeSubstructureMatcher::makeTransposition(query_in, transposition);
   query_out.makeSubmolecule(query_in, transposition, 0);
   _nei_query_counters.calculate(query_out);
}

void MangoSubstructure::_initTarget (bool from_database)
{
   if (preserve_bonds_on_highlighting)
      Molecule::saveBondOrders(_target, _target_bond_types);

   if (!from_database)
      MoleculeAromatizer::aromatizeBonds(_target);

   _nei_target_counters.calculate(_target);
} 

bool MangoSubstructure::needCoords ()
{
   return MoleculeSubstructureMatcher::needCoords(match_3d, _query);
}

bool MangoSubstructure::matchBinary (const Array<char> &target_buf, const Array<char> *xyz_buf)
{
   BufferScanner scanner(target_buf);

   if (xyz_buf == 0)
      return matchBinary(scanner, 0);
   
   BufferScanner xyz_scanner(*xyz_buf);

   return matchBinary(scanner, &xyz_scanner);
}

bool MangoSubstructure::matchBinary (Scanner &scanner, Scanner *xyz_scanner)
{
   _validateQueryExtraData();
      
   profTimerStart(tcmf, "match.cmf");

   cmf_loader.free();
   cmf_loader.create(_context.cmf_dict, scanner);

   if (!_query_has_stereocare_bonds)
      cmf_loader->skip_cistrans = true;
   if (!_query_has_stereocenters)
      cmf_loader->skip_stereocenters = true;

   cmf_loader->loadMolecule(_target);
   if (xyz_scanner != 0)
      cmf_loader->loadXyz(*xyz_scanner);

   profTimerStop(tcmf);

   profTimerStart(tinit, "match.init_target");
   _initTarget(true);
   profTimerStop(tinit);

   return matchLoadedTarget();
}

bool MangoSubstructure::parse (const char *params)
{
   match_3d = 0;
   rms_threshold = 0;
   _use_pi_systems_matcher = false;
   preserve_bonds_on_highlighting = false;

   if (params == 0)
      return true;

   BufferScanner scanner(params);

   QS_DEF(Array<char>, word);

   scanner.skipSpace();
   while (!scanner.isEOF())
   {
      scanner.skipSpace();
      scanner.readWord(word, 0);

      bool is_aff = strcasecmp(word.ptr(), "AFF") == 0;
      bool is_conf = strcasecmp(word.ptr(), "CONF") == 0;
      if (is_aff || is_conf)
      {
         if (match_3d != 0)
            return false;
         if (is_aff)
            match_3d = MoleculeSubstructureMatcher::AFFINE;
         if (is_conf)
            match_3d = MoleculeSubstructureMatcher::CONFORMATION;

         scanner.skipSpace();
         rms_threshold = scanner.readFloat();
      }
      else if (strcasecmp(word.ptr(), "RES") == 0)
         _use_pi_systems_matcher = true;
      else
         return false;
   }


   return true;
}

const byte * MangoSubstructure::getQueryFingerprint ()
{
   _validateQueryFP();
   return _query_fp.ptr();
}
