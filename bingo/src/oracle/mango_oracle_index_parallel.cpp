#include "oracle/mango_oracle_index_parallel.h"

#include "oracle/bingo_oracle_context.h"

//
// MangoRegisterDispatcher
//
MangoRegisterDispatcher::MangoRegisterDispatcher (MangoOracleContext &context, 
                                                  OracleEnv &env,
                                                  const char *rowid)
   : BingoOracleDispatcher(OsCommandDispatcher::HANDLING_ORDER_SERIAL, true, 70),
   _context(context), _env(env)
{
   _molecules_prepared = 0;
   _molecules_saved = 0;
   _rowid = rowid;
}

OsCommand * MangoRegisterDispatcher::_allocateCommand ()
{
   return new MangoRegisterCommand(_env, 
      _context, _lock_for_exclusive_access, &_molecules_prepared);
}

OsCommandResult * MangoRegisterDispatcher::_allocateResult ()
{
   return new MangoRegisterResult;
}

bool MangoRegisterDispatcher::_setupCommand (OsCommand &command)
{
   bool ret = BingoOracleDispatcher::_setupCommand(command);

   return ret;
}

void MangoRegisterDispatcher::_addCurrentRecordToCommand (BingoOracleCommand &command)
{
   BingoOracleDispatcher::_addCurrentRecordToCommand(command);

   MangoRegisterCommand &cmd = (MangoRegisterCommand &)command;

   cmd.rowids.add(_rowid);
}

void MangoRegisterDispatcher::_handleResult (OsCommandResult &result)
{
   // Handle result
   MangoRegisterResult &res = (MangoRegisterResult &)result; 

   BingoFingerprints &fingerprints = _context.fingerprints;

   QS_DEF(Array<char>, prepared_data);
   for (int i = 0; i < res.valid_molecules; i++)
   {
      const char *rowid = (const char *)res.rowids.get(i);
      prepared_data.copy((char *)res.per_molecule_data.get(i), 
         res.per_molecule_data.getSize(i));

      mangoRegisterMolecule(_env, rowid, _context, 
         res.per_molecule_index[i], fingerprints, prepared_data);

      _molecules_saved++;

      if ((_molecules_saved % 100) == 0)
      {
         OsLocker locker(_lock_for_exclusive_access);
         _context.context().longOpUpdate(_env, _molecules_prepared);
      }
      
      if ((_molecules_saved % 2000) == 0)
      {
         {
            OsLocker locker(_lock_for_exclusive_access);
            _env.dbgPrintfTS("done %d molecules; flushing\n", _molecules_prepared);
         }
         _context.context().storage.flush(_env);
      }
   }
}

//
// MangoRegisterCommand
//

MangoRegisterCommand::MangoRegisterCommand (OracleEnv &env, 
                                            MangoOracleContext &context,
                                            OsLock &lock_for_exclusive_access,
                                            int *molecules_prepared_counter) :
   _context(context), _env(env), 
   _lock_for_exclusive_access(lock_for_exclusive_access)
{
   _molecules_prepared_counter = molecules_prepared_counter;
}

void MangoRegisterCommand::clear ()
{
   rowids.clear();
   BingoOracleCommand::clear();
}


void MangoRegisterCommand::execute (OsCommandResult &result)
{
   MangoRegisterResult &res = (MangoRegisterResult &)result;
   res.valid_molecules = 0;

   QS_DEF(Array<char>, molfile_buf);
   QS_DEF(Array<char>, prepared_data);

   for (int i = 0; i < blob_storage.count(); i++)
   {
      molfile_buf.copy((char *)blob_storage.get(i), blob_storage.getSize(i));
      const char *rowid = (const char *)rowids.get(i);

      {
         OsLocker locker(_lock_for_exclusive_access);

         _env.dbgPrintf("preparing molecule #%d with rowid %s\n", 
            *_molecules_prepared_counter, rowid);
         (*_molecules_prepared_counter)++;
      }

      try 
      {
         if (res.per_molecule_index.size() <= res.valid_molecules)
         {
            // Push new MangoIndex with parameter
            res.per_molecule_index.push(_context.context());
         }
         MangoIndex &index = res.per_molecule_index[res.valid_molecules];

         if (mangoPrepareMolecule(_env, rowid, molfile_buf, 
            _context, index, prepared_data, &_lock_for_exclusive_access)) 
         {
            res.per_molecule_data.add((byte*)prepared_data.ptr(), 
               prepared_data.size());
            res.valid_molecules++;
            res.rowids.add((byte *)rowid, rowids.getSize(i));
         }
      }
      catch (Exception &ex)
      {
         char buf[4096];
         snprintf(buf, NELEM(buf), "Failed on record with rowid=%s. Error message is '%s'",
            rowid, ex.message());

         throw Exception(buf);
      }
   }
}

//
// MangoRegisterResult
//

void MangoRegisterResult::clear ()
{
   rowids.clear();
   per_molecule_data.clear();
   valid_molecules = 0;

   // Don't clear per_molecule_index
}
