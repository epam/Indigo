/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __mango_oracle_index_parallel_h__
#define __mango_oracle_index_parallel_h__

#include "base_cpp/os_sync_wrapper.h"
#include "oracle/bingo_oracle_parallel.h"
#include "oracle/mango_oracle.h"

#include <string>
#include <vector>

namespace indigo
{

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

} // namespace indigo

bool mangoPrepareMolecule(indigo::OracleEnv& env, const char* rowid, const indigo::Array<char>& molfile_buf, indigo::MangoOracleContext& context,
                          indigo::MangoIndex& index, indigo::Array<char>& data, std::mutex* lock_for_exclusive_access, std::string& failure_message);

void mangoRegisterMolecule(indigo::OracleEnv& env, const char* rowid, indigo::MangoOracleContext& context, const indigo::MangoIndex& index,
                           indigo::BingoFingerprints& fingerprints, const indigo::Array<char>& prepared_data, bool append);
#endif // __mango_oracle_index_parallel_h__
