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

#include "bingo_core_c_internal.h"

#include "base_cpp/profiling.h"

using namespace indigo::bingo_core;

BingoCore::BingoCore ()
{
   bingo_context = 0;
   mango_context = 0;
   ringo_context = 0;
   reset();
}

void BingoCore::reset ()
{
   if (bingo_context != 0)
   {
      int id = bingo_context->id;
      bingo_context->reset();
   }

   mango_search_type = _UNDEF;
   mango_search_type_non = false;
   bingo_context = 0;
   mango_context = 0;
   ringo_context = 0;
   error_handler = 0;
   error_handler_context = 0;
   skip_calculate_fp = false;
}

TL_DECL(BingoCore, self);

BingoCore& BingoCore::getInstance ()
{
   TL_GET(BingoCore, self);
   return self;
}

CEXPORT const char * bingoGetVersion ()
{
   return bingo_version_string;  
}                    

CEXPORT const char * bingoGetError ()
{
   BINGO_BEGIN
   {
      return self.error.ptr();
   }
   BINGO_END("", "")
}

CEXPORT const char * bingoGetWarning ()
{
   BINGO_BEGIN
   {
      return self.warning.ptr();
   }
   BINGO_END("", "")
}

CEXPORT qword bingoAllocateSessionID ()
{
   qword id = TL_ALLOC_SESSION_ID();

   TL_GET_BY_ID(BingoCore, self, id);
   self.reset();

   return id;
}

CEXPORT void bingoReleaseSessionID (qword session_id)
{
   return TL_RELEASE_SESSION_ID(session_id);
}

CEXPORT void bingoSetSessionID (qword session_id)
{
   TL_SET_SESSION_ID(session_id);
}

CEXPORT void bingoSetErrorHandler (BINGO_ERROR_HANDLER handler, void *context)
{
   BingoCore &self = BingoCore::getInstance();
   self.error_handler = handler;
   self.error_handler_context = context;
}

CEXPORT int bingoSetContext (int id)
{
   BINGO_BEGIN
   {
      self.bingo_context = BingoContext::get(id);
      self.mango_context = MangoContext::get(id);
      self.ringo_context = RingoContext::get(id);
   }
   BINGO_END(1, 0);
}

CEXPORT int bingoSetConfigInt (const char *name, int value)
{
   BINGO_BEGIN
   {
      if (self.bingo_context == 0)
         throw BingoError("context not set");
      if (strcasecmp(name, "treat-x-as-pseudoatom") == 0 || strcasecmp(name, "treat_x_as_pseudoatom") == 0)
      {
         self.bingo_context->treat_x_as_pseudoatom = (value != 0);
         self.bingo_context->treat_x_pseudo_ready = true;
      }
      else if (strcasecmp(name, "ignore-closing-bond-direction-mismatch") == 0 || strcasecmp(name, "ignore_closing_bond_direction_mismatch") == 0)
      {
         self.bingo_context->ignore_closing_bond_direction_mismatch = (value != 0);
         self.bingo_context->ignore_cbdm_ready = true;
      }
      else if (strcasecmp(name, "nthreads") == 0)
      {
         self.bingo_context->nthreads = value;
      }
      else
      {
         bool set = true;
         if (strcasecmp(name, "FP_ORD_SIZE") == 0)
            self.bingo_context->fp_parameters.ord_qwords = value;
         else if (strcasecmp(name, "FP_ANY_SIZE") == 0)
            self.bingo_context->fp_parameters.any_qwords = value;
         else if (strcasecmp(name, "FP_TAU_SIZE") == 0)
            self.bingo_context->fp_parameters.tau_qwords = value;
         else if (strcasecmp(name, "FP_SIM_SIZE") == 0)
            self.bingo_context->fp_parameters.sim_qwords = value;
         else if (strcasecmp(name, "SUB_SCREENING_MAX_BITS") == 0)
            self.sub_screening_max_bits = value;
         else if (strcasecmp(name, "SIM_SCREENING_PASS_MARK") == 0)
            self.sim_screening_pass_mark = value;
         else
            set = false;

         if (set)
            self.bingo_context->fp_parameters_ready = true;

         if (!set)
            throw BingoError("Unknown parameter name: '%s'", name);

      }
   }
   BINGO_END(1, 0);
}

CEXPORT int bingoGetConfigInt (const char *name, int *value)
{
   BINGO_BEGIN
   {
      if (self.bingo_context == 0)
         throw BingoError("context not set");

      if (strcasecmp(name, "treat-x-as-pseudoatom") == 0 || strcasecmp(name, "treat_x_as_pseudoatom") == 0)
         *value = (int)self.bingo_context->treat_x_as_pseudoatom;
      else if (strcasecmp(name, "fp-size-bytes") == 0 || strcasecmp(name, "fp_size_bytes") == 0)
         *value = self.bingo_context->fp_parameters.fingerprintSize();
      else if (strcasecmp(name, "reaction-fp-size-bytes") == 0 || strcasecmp(name, "reaction_fp_size_bytes") == 0)
         *value = self.bingo_context->fp_parameters.fingerprintSizeExtOrd() * 2;
      else if (strcasecmp(name, "SUB_SCREENING_MAX_BITS") == 0)
         *value = self.sub_screening_max_bits;
      else if (strcasecmp(name, "SIM_SCREENING_PASS_MARK") == 0)
         *value = self.sim_screening_pass_mark;
      else
         throw BingoError("unknown parameter name: %s", name);
   }
   BINGO_END(1, 0);
}

CEXPORT int bingoGetConfigBin (const char *name, const char **value, int *len)
{
   BINGO_BEGIN
   {
      if (self.bingo_context == 0)
         throw BingoError("context not set");

      if (strcasecmp(name, "cmf-dict") == 0 || strcasecmp(name, "cmf_dict") == 0)
      {
         ArrayOutput output(self.buffer);
         self.bingo_context->cmf_dict.save(output);
         *value = self.buffer.ptr();
         *len = self.buffer.size();
      }            
      else
         throw BingoError("unknown parameter name: %s", name);
   }
   BINGO_END(1, 0);
}

CEXPORT int bingoSetConfigBin (const char *name, const char *value, int len)
{
   BINGO_BEGIN
   {
      if (self.bingo_context == 0)
         throw BingoError("context not set");

      if (strcasecmp(name, "cmf-dict") == 0 || strcasecmp(name, "cmf_dict") == 0)
      {
         BufferScanner scanner(value, len);
         self.bingo_context->cmf_dict.load(scanner);
      }
      else
         throw BingoError("unknown parameter name: %s", name);
   }
   BINGO_END(1, 0);
}

CEXPORT int bingoClearTautomerRules ()
{
   BINGO_BEGIN
   {
      if (self.bingo_context == 0)
         throw BingoError("context not set");

      self.bingo_context->tautomer_rules.clear();
      self.bingo_context->tautomer_rules_ready = false;
   }
   BINGO_END(1, 0);
}

CEXPORT int bingoAddTautomerRule (int n, const char *beg, const char *end)
{
   BINGO_BEGIN
   {
      if (self.bingo_context == 0)
         throw BingoError("context not set");

      if (n < 1 || n >= 32)
         throw BingoError("tautomer rule index %d is out of range", n);

      AutoPtr<TautomerRule> rule(new TautomerRule());

      bingoGetTauCondition(beg, rule->aromaticity1, rule->list1);
      bingoGetTauCondition(end, rule->aromaticity2, rule->list2);

      self.bingo_context->tautomer_rules.expand(n);
      self.bingo_context->tautomer_rules[n - 1] = rule.release();
   }
   BINGO_END(1, 0);
}

CEXPORT int bingoTautomerRulesReady (int n, const char *beg, const char *end)
{
   BINGO_BEGIN
   {
      if (self.bingo_context == 0)
         throw BingoError("context not set");

      self.bingo_context->tautomer_rules_ready = true;
   }
   BINGO_END(1, 0);
}


CEXPORT int bingoSDFImportOpen (const char *file_name)
{
   BINGO_BEGIN
   {
      self.file_scanner.create(file_name);
      self.sdf_loader.create(self.file_scanner.ref());
      return 1;
   }
   BINGO_END(-1, -1)
}

CEXPORT int bingoSDFImportClose ()
{
   BINGO_BEGIN
   {
      self.sdf_loader.free();
      self.file_scanner.free();
   }
   BINGO_END(0, -1);
}

CEXPORT int bingoSDFImportEOF ()
{
   BINGO_BEGIN
   {
      return self.sdf_loader->isEOF() ? 1 : 0;
   }
   BINGO_END(0, -1)
}
CEXPORT const char * bingoSDFImportGetNext ()
{
   BINGO_BEGIN
   {
      profTimerStart(t, "sdf_loader.readNext");
      self.sdf_loader->readNext();
      self.sdf_loader->data.push(0);
      return self.sdf_loader->data.ptr();
   }
   BINGO_END("", "")
}

CEXPORT const char * bingoSDFImportGetProperty (const char *param_name)
{
   BINGO_BEGIN
   {
      return self.sdf_loader->properties[param_name].ptr();
   }
   BINGO_END("", "")
}

CEXPORT int bingoRDFImportOpen (const char *file_name)
{
   BINGO_BEGIN
   {
      self.file_scanner.create(file_name);
      self.rdf_loader.create(self.file_scanner.ref());
      return 1;
   }
   BINGO_END(-1, -1)
}

CEXPORT int bingoRDFImportClose ()
{
   BINGO_BEGIN
   {
      self.rdf_loader.free();
      self.file_scanner.free();
   }
   BINGO_END(0, -1);
}

CEXPORT int bingoRDFImportEOF ()
{
   BINGO_BEGIN
   {
      return self.rdf_loader->isEOF() ? 1 : 0;
   }
   BINGO_END(0, -1)
}
CEXPORT const char * bingoRDFImportGetNext ()
{
   BINGO_BEGIN
   {
      self.rdf_loader->readNext();
      self.rdf_loader->data.push(0);
      return self.rdf_loader->data.ptr();
   }
   BINGO_END("", "")
}

CEXPORT const char * bingoRDFImportGetProperty (const char *param_name)
{
   BINGO_BEGIN
   {
      return self.rdf_loader->properties[param_name].ptr();
   }
   BINGO_END("", "")
}


CEXPORT void bingoProfilingReset (byte reset_whole_session)
{
   ProfilingSystem::getInstance().reset(reset_whole_session != 0);
}

CEXPORT const char* bingoProfilingGetStatistics (bool for_session)
{
   BINGO_BEGIN
   {
      ArrayOutput output(self.buffer);
      profGetStatistics(output, for_session);
      output.writeByte(0);
      return self.buffer.ptr();
   }
   BINGO_END("<unknown>", "<unknown>")
}

CEXPORT float bingoProfilingGetTime (const char *counter_name, byte for_session)
{
   BINGO_BEGIN
   {
      return ProfilingSystem::getInstance().getLabelExecTime(counter_name, 
         for_session != 0);
   }
   BINGO_END(-1, -1)
}

CEXPORT qword bingoProfilingGetValue (const char *counter_name, byte for_session)
{
   BINGO_BEGIN
   {
      return ProfilingSystem::getInstance().getLabelValue(counter_name, 
         for_session != 0);
   }
   BINGO_END(-1, -1)
}

CEXPORT qword bingoProfilingGetCount (const char *counter_name, byte for_session)
{
   BINGO_BEGIN
   {
      return ProfilingSystem::getInstance().getLabelCallCount(counter_name, 
         for_session != 0);
   }
   BINGO_END(-1, -1)
}

#include <exception>

CEXPORT int bingoCheckMemoryAllocate (int size)
{
   BINGO_BEGIN
   {
      try {
         self.test_ptr = 0;
         self.test_ptr = (byte*)malloc(size);

         if (self.test_ptr == 0)
         {
            self.error.readString("self.test_ptr == 0", true); 
            return -1;
         }
         for (int i = 0; i < size; i++)
            self.test_ptr[i] = i;

         return 1;
      }
      catch (std::exception &ex)
      {
         self.error.readString(ex.what(), true); 
         return -1;
      }
   }
   BINGO_END(-1, -1)
}

CEXPORT int bingoCheckMemoryFree ()
{
   BINGO_BEGIN
   {
      if (self.test_ptr != 0)
      {
         free(self.test_ptr);
      }

      self.test_ptr = 0;
      return 1;
   }
   BINGO_END(-1, -1)
}

CEXPORT qword bingoProfNanoClock ()
{
   return nanoClock();
}

CEXPORT void bingoProfIncTimer (const char *name, qword dt)
{
   ProfilingSystem &inst = ProfilingSystem::getInstance();
   int name_index = inst.getNameIndex(name);
   inst.addTimer(name_index, dt);
}

CEXPORT void bingoProfIncCounter (const char *name, int dv)
{
   ProfilingSystem &inst = ProfilingSystem::getInstance();
   int name_index = inst.getNameIndex(name);
   inst.addCounter(name_index, dv);
}

CEXPORT const char * bingoGetNameCore (const char *target_buf, int target_buf_len)
{
   BINGO_BEGIN
   {
      QS_DEF(Array<char>, source);
      QS_DEF(Array<char>, name);

      BufferScanner scanner(target_buf, target_buf_len);
      bingoGetName(scanner, self.buffer);
      self.buffer.push(0);
      return self.buffer.ptr();
   }
   BINGO_END(0, 0)
}

CEXPORT int bingoIndexMarkTermintate ()
{
   BINGO_BEGIN
   {
      if (self.parallel_indexing_dispatcher.get())
         self.parallel_indexing_dispatcher->markToTerminate();
      return 1;
   }
   BINGO_END(-2, -2)
}

CEXPORT int bingoIndexEnd ()
{
   BINGO_BEGIN
   {
      if (self.parallel_indexing_dispatcher.get())
      {
         self.parallel_indexing_dispatcher->terminate();
         self.parallel_indexing_dispatcher.reset(0);
      }

      if (self.single_mango_index.get())
         self.single_mango_index.free();
      if (self.single_ringo_index.get())
         self.single_ringo_index.free();

      self.mango_index = 0;
      self.ringo_index = 0;
      self.index_record_data_id = -1;
      self.index_record_data.free();

      return 1;
   }
   BINGO_END(-2, -2)
}

CEXPORT int bingoIndexBegin ()
{
   BINGO_BEGIN
   {
      if (!self.bingo_context->fp_parameters_ready)
         throw BingoError("fingerprint parameters not set");

      bingoIndexEnd();

      self.index_record_data.create();
      return 1;
   }
   BINGO_END(-2, -2)
}

CEXPORT int bingoIndexSetSkipFP (bool skip)
{
   BINGO_BEGIN
   {
      self.skip_calculate_fp = skip;
      return 1;
   }
   BINGO_END(-2, -2)
}
