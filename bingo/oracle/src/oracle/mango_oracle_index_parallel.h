#ifndef __mango_oracle_index_parallel_h__
#define __mango_oracle_index_parallel_h__

#include "base_cpp/os_sync_wrapper.h"
#include "oracle/bingo_oracle_parallel.h"
#include "oracle/mango_oracle.h"

#include <string>
#include <vector>

using namespace indigo;

//
// Classes for parallelized index creation
//

class MangoRegisterDispatcher : public BingoOracleDispatcher
{
public:
    MangoRegisterDispatcher(MangoOracleContext& context, OracleEnv& env, const char* rowid);

protected:
    virtual OsCommand* _allocateCommand();
    virtual OsCommandResult* _allocateResult();

    virtual bool _setupCommand(OsCommand& command);
    virtual void _addCurrentRecordToCommand(BingoOracleCommand& command);
    virtual void _handleResult(OsCommandResult& result);

    MangoOracleContext& _context;
    OracleEnv& _env;
    const char* _rowid;
    int _molecules_prepared, _molecules_saved;
    OsLock _lock_for_exclusive_access;
};

class MangoRegisterCommand : public BingoOracleCommand
{
public:
    MangoRegisterCommand(OracleEnv& env, MangoOracleContext& context, OsLock& lock_for_exclusive_access, int* molecules_prepared_counter);

    virtual void execute(OsCommandResult& result);

    virtual void clear();

    ChunkStorage rowids;

private:
    MangoOracleContext& _context;
    OracleEnv& _env;
    OsLock& _lock_for_exclusive_access;
    int* _molecules_prepared_counter;
};

struct MangoRegisterFailure
{
    std::string rowid, message;
};

class MangoRegisterResult : public OsCommandResult
{
public:
    int valid_molecules;

    virtual void clear();

    ObjArray<MangoIndex> per_molecule_index;
    std::vector<MangoRegisterFailure> warnings;
    ChunkStorage per_molecule_data;
    ChunkStorage rowids;
};

bool mangoPrepareMolecule(OracleEnv& env, const char* rowid, const ArrayChar& molfile_buf, MangoOracleContext& context, MangoIndex& index, ArrayChar& data,
                          OsLock* lock_for_exclusive_access, std::string& failure_message);

void mangoRegisterMolecule(OracleEnv& env, const char* rowid, MangoOracleContext& context, const MangoIndex& index, BingoFingerprints& fingerprints,
                           const ArrayChar& prepared_data, bool append);

#endif // __mango_oracle_index_parallel_h__
