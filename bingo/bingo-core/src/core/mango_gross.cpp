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

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "bingo_context.h"
#include "mango_index.h"
#include "mango_matchers.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_gross_formula.h"

IMPL_ERROR(MangoGross, "gross formula");

MangoGross::MangoGross(BingoContext& context) : _context(context)
{
}

void MangoGross::parseQuery(const std::string& query)
{
    BufferScanner scanner(query);

    parseQuery(scanner);
}

void MangoGross::parseQuery(const char* query)
{
    BufferScanner scanner(query);

    parseQuery(scanner);
}

void MangoGross::parseQuery(Scanner& scanner)
{
    scanner.skipSpace();

    char c = scanner.readChar();

    if (c == '>')
    {
        if (scanner.readChar() != '=')
            throw Error("expecting '=' after '>'");
        _sign = 1;
    }
    else if (c == '<')
    {
        if (scanner.readChar() != '=')
            throw Error("expecting '=' after '<'");
        _sign = -1;
    }
    else if (c == '=')
        _sign = 0;
    else
        throw Error("expected one of '<= >= =', got %c", c);

    MoleculeGrossFormula::fromString(scanner, _query_gross);

    StringOutput out(_conditions);

    bool first = true;

    if (_sign == 0)
    {
        QS_DEF(std::string, query_gross_str);

        MoleculeGrossFormula::toString(_query_gross, query_gross_str);

        out.printf("gross = '%s'", query_gross_str.c_str());
    }
    else
        for (int i = 0; i < NELEM(MangoIndex::counted_elements); i++)
        {
            int elem = MangoIndex::counted_elements[i];

            if (_query_gross[elem] <= 0 && _sign == 1)
                continue;

            if (!first)
                out.printf(" AND ");

            first = false;

            if (_sign == 1)
                out.printf("cnt_%s >= %d", Element::toString(elem), _query_gross[elem]);
            else // _sign == -1
                out.printf("cnt_%s <= %d", Element::toString(elem), _query_gross[elem]);
        }
    out.writeChar(0);
}

const char* MangoGross::getConditions()
{
    return _conditions.c_str();
}

bool MangoGross::checkGross(const Array<int>& target_gross)
{
    if (_sign == 1)
        return MoleculeGrossFormula::geq(target_gross, _query_gross);
    else if (_sign == -1)
        return MoleculeGrossFormula::leq(target_gross, _query_gross);
    // _sign == 0
    return MoleculeGrossFormula::equal(target_gross, _query_gross);
}

bool MangoGross::checkGross(const char* target_gross_str)
{
    QS_DEF(Array<int>, target_gross);

    MoleculeGrossFormula::fromString(target_gross_str, target_gross);

    return checkGross(target_gross);
}

bool MangoGross::checkMolecule(const std::string& target_buf)
{
    BufferScanner scanner(target_buf);

    return checkMolecule(scanner);
}

bool MangoGross::checkMolecule(Scanner& scanner)
{
    QS_DEF(Molecule, target);
    QS_DEF(Array<int>, target_gross);

    MoleculeAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);
    loader.loadMolecule(target);

    MoleculeGrossFormula::collect(target, target_gross);

    return checkGross(target_gross);
}
