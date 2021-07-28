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
    OsCommand* _allocateCommand() override;
    OsCommandResult* _allocateResult() override;

    bool _setupCommand(OsCommand& command) override;
    void _addCurrentRecordToCommand(BingoOracleCommand& command) override;
    void _handleResult(OsCommandResult& result) override;

    MangoOracleContext& _context;
    OracleEnv& _env;
    const char* _rowid;
    int _molecules_prepared, _molecules_saved;
    std::mutex _lock_for_exclusive_access;
};

class MangoRegisterCommand : public BingoOracleCommand
{
public:
    MangoRegisterCommand(OracleEnv& env, MangoOracleContext& context, std::mutex& lock_for_exclusive_access, int* molecules_prepared_counter);

    void execute(OsCommandResult& result) override;

    void clear() override;

    ChunkStorage rowids;

private:
    MangoOracleContext& _context;
    OracleEnv& _env;
    std::mutex& _lock_for_exclusive_access;
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

    void clear() override;

    ObjArray<MangoIndex> per_molecule_index;
    std::vector<MangoRegisterFailure> warnings;
    ChunkStorage per_molecule_data;
    ChunkStorage rowids;
};

bool mangoPrepareMolecule(OracleEnv& env, const char* rowid, const Array<char>& molfile_buf, MangoOracleContext& context, MangoIndex& index, Array<char>& data,
                          std::mutex* lock_for_exclusive_access, std::string& failure_message);

void mangoRegisterMolecule(OracleEnv& env, const char* rowid, MangoOracleContext& context, const MangoIndex& index, BingoFingerprints& fingerprints,
                           const Array<char>& prepared_data, bool append);

#endif // __mango_oracle_index_parallel_h__
