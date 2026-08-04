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

// Loader-option conformance across every input path.
//
// Why this file exists (issue #3823): loader options used to reach only some
// input formats. MoleculeAutoLoader hand-copied a different subset of fields at
// each of its dispatch sites, so `valence-mode` was honoured for molfile but
// silently dropped for KET — Ketcher's native format — as well as CML and
// SMILES. Two further paths had no loader at all to copy from:
// createMolecule()/addAtom() and loadKetDocument().
//
// The guard is not any single assertion but the SHAPE of the table below: the
// same expectation stated for every format. A per-format gap cannot reappear
// without turning a row red.
//
// TO COVER A NEW OPTION: add rows to kCases. Add a probe or a format only if
// the option needs an observable or an input path that is not there yet.

#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "common.h"
#include <base_cpp/exception.h>
#include <indigo.h>

using namespace indigo;

namespace
{
    // ─── Inputs ──────────────────────────────────────────────────────────────
    // A single atom is all the chemistry needed: the two models diverge only
    // while an atom is under-coordinated.

    struct Structure
    {
        const char* symbol;
        int charge;
        bool explicit_h; // attach one explicitly drawn hydrogen
    };

    std::string asMolfile(const Structure& s)
    {
        std::ostringstream out;
        out << "\n  Indigo  0000000002D\n\n"
            << (s.explicit_h ? "  2  1" : "  1  0") << "  0  0  0  0  0  0  0  0999 V2000\n"
            << "    0.0000    0.0000    0.0000 " << std::string(s.symbol) + std::string(3 - strlen(s.symbol), ' ') << " 0  0  0  0  0  0  0  0  0  0  0  0\n";
        if (s.explicit_h)
        {
            out << "    1.5000    0.0000    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0\n";
            out << "  1  2  1  0  0  0  0\n";
        }
        if (s.charge != 0)
            out << "M  CHG  1   1 " << (s.charge < 0 ? "" : " ") << s.charge << "\n";
        out << "M  END\n";
        return out.str();
    }

    std::string asKet(const Structure& s)
    {
        std::ostringstream out;
        out << R"({"root":{"nodes":[{"$ref":"mol0"}]},"mol0":{"type":"molecule","atoms":[{"label":")" << s.symbol << R"(","location":[0,0,0])";
        if (s.charge != 0)
            out << R"(,"charge":)" << s.charge;
        out << "}";
        if (s.explicit_h)
            out << R"(,{"label":"H","location":[1.5,0,0]})";
        out << R"(],"bonds":[)";
        if (s.explicit_h)
            out << R"({"type":1,"atoms":[0,1]})";
        out << "]}}";
        return out.str();
    }

    std::string asCml(const Structure& s)
    {
        std::ostringstream out;
        out << R"(<molecule><atomArray><atom id="a1" elementType=")" << s.symbol << "\"";
        if (s.charge != 0)
            out << R"( formalCharge=")" << s.charge << "\"";
        out << "/>";
        if (s.explicit_h)
            out << R"(<atom id="a2" elementType="H"/>)";
        out << "</atomArray><bondArray>";
        if (s.explicit_h)
            out << R"(<bond atomRefs2="a1 a2" order="1"/>)";
        out << "</bondArray></molecule>";
        return out.str();
    }

    // A bracket atom in SMILES carries an EXPLICIT hydrogen count by definition,
    // so this format pins the H count regardless of the model. Included anyway:
    // `valence` still tracks the option and would expose a dropped one.
    std::string asSmiles(const Structure& s)
    {
        std::ostringstream out;
        out << "[" << s.symbol;
        if (s.explicit_h)
            out << "H";
        for (int i = 0; i < std::abs(s.charge); ++i)
            out << (s.charge < 0 ? "-" : "+");
        out << "]";
        return out.str();
    }

    // ─── Cases ───────────────────────────────────────────────────────────────

    struct Case
    {
        Structure structure;
        const char* format; // mol | ket | cml | smi | api | ketdoc
        const char* probe;  // valence | implicit_h | gross_formula | smiles
        const char* mode_a;
        const char* expect_a;
        const char* mode_b;
        const char* expect_b;
    };

    constexpr Structure kAl{"Al", 0, false};
    constexpr Structure kLi{"Li", 0, false};
    constexpr Structure kMg{"Mg", 0, false};
    constexpr Structure kGa{"Ga", 0, false};
    constexpr Structure kC{"C", 0, false};
    constexpr Structure kAlMinus{"Al", -1, false};
    constexpr Structure kLiH{"Li", 0, true};

    const Case kCases[] = {
        // Under-coordinated main-group metals: 2017 intercepts them
        // (valence = conn, hyd = 0), 2009 fills the group valence with H.
        {kAl, "mol", "implicit_h", "biovia-2009", "3", "biovia-2017", "0"},
        {kAl, "ket", "implicit_h", "biovia-2009", "3", "biovia-2017", "0"},
        {kAl, "cml", "implicit_h", "biovia-2009", "3", "biovia-2017", "0"},
        {kAl, "smi", "valence", "biovia-2009", "3", "biovia-2017", "0"},
        {kAl, "api", "implicit_h", "biovia-2009", "3", "biovia-2017", "0"},
        {kAl, "ketdoc", "smiles", "biovia-2009", "[AlH3]", "biovia-2017", "[Al]"},

        {kLi, "mol", "implicit_h", "biovia-2009", "1", "biovia-2017", "0"},
        {kLi, "ket", "implicit_h", "biovia-2009", "1", "biovia-2017", "0"},
        {kLi, "cml", "implicit_h", "biovia-2009", "1", "biovia-2017", "0"},
        {kLi, "smi", "valence", "biovia-2009", "1", "biovia-2017", "0"},
        {kLi, "api", "implicit_h", "biovia-2009", "1", "biovia-2017", "0"},
        {kLi, "ketdoc", "smiles", "biovia-2009", "[LiH]", "biovia-2017", "[Li]"},
        {kLi, "mol", "gross_formula", "biovia-2009", "H Li", "biovia-2017", "Li"},
        {kLi, "ket", "gross_formula", "biovia-2009", "H Li", "biovia-2017", "Li"},

        {kMg, "ket", "valence", "biovia-2009", "2", "biovia-2017", "0"},
        {kGa, "ket", "implicit_h", "biovia-2009", "3", "biovia-2017", "0"},
        {kGa, "cml", "implicit_h", "biovia-2009", "3", "biovia-2017", "0"},

        // Negative controls. Each pins a case that LOOKS like the option is
        // broken but is correct by design; together they stop an over-eager
        // "fix" from making the models diverge where they must not.
        //
        // Carbon is in the BIOVIA-2017 22-element table, so never intercepted.
        {kC, "mol", "implicit_h", "biovia-2009", "4", "biovia-2017", "4"},
        {kC, "ket", "implicit_h", "biovia-2009", "4", "biovia-2017", "4"},
        {kC, "api", "implicit_h", "biovia-2009", "4", "biovia-2017", "4"},
        // Al(-1) is the documented carve-out. Cancelling the interception does
        // not raise the valence to 4 — the 2009 ladder already returns 4.
        {kAlMinus, "mol", "implicit_h", "biovia-2009", "4", "biovia-2017", "4"},
        {kAlMinus, "ket", "implicit_h", "biovia-2009", "4", "biovia-2017", "4"},
        {kAlMinus, "api", "implicit_h", "biovia-2009", "4", "biovia-2017", "4"},
        // A SATURATED metal is indistinguishable: interception gives
        // valence = conn = 1 and the 2009 group-1 ladder independently gives
        // valence 1 / hyd 0. This is the exact structure originally reported as
        // proof that the switch does not work.
        {kLiH, "mol", "implicit_h", "biovia-2009", "0", "biovia-2017", "0"},
        {kLiH, "ket", "valence", "biovia-2009", "1", "biovia-2017", "1"},
        {kLiH, "ket", "gross_formula", "biovia-2009", "H Li", "biovia-2017", "H Li"},

        // `default` is deliberately an alias whose target may move (2009 today,
        // 2017 later). These rows pin today's meaning on purpose: when the alias
        // is repointed they fail loudly and force the change to be acknowledged
        // rather than letting it happen silently.
        {kLi, "mol", "implicit_h", "default", "1", "biovia-2009", "1"},
        {kAl, "ket", "implicit_h", "default", "3", "biovia-2009", "3"},
    };

    // ─── Execution ───────────────────────────────────────────────────────────

    std::string label(const Case& c)
    {
        std::ostringstream out;
        out << c.structure.symbol;
        if (c.structure.charge)
            out << "(" << c.structure.charge << ")";
        if (c.structure.explicit_h)
            out << "H";
        out << "/" << c.format << "/" << c.probe;
        return out.str();
    }

    // Most formats go through a loader. Two deliberately do not:
    //   api    - built via indigoCreateMolecule/indigoAddAtom, so only the
    //            C-API layer can seed the session's model onto it.
    //   ketdoc - loaded as a KetDocument, whose conversion to a Molecule is
    //            deferred past the point where the session is still reachable.
    int buildMolecule(const Case& c)
    {
        const std::string fmt = c.format;
        if (fmt == "api")
        {
            const int mol = indigoCreateMolecule();
            const int atom = indigoAddAtom(mol, c.structure.symbol);
            if (c.structure.charge != 0)
                indigoSetCharge(atom, c.structure.charge);
            indigoFree(atom);
            return mol;
        }
        if (fmt == "ketdoc")
            return indigoLoadKetDocumentFromString(asKet(c.structure).c_str());
        if (fmt == "mol")
            return indigoLoadMoleculeFromString(asMolfile(c.structure).c_str());
        if (fmt == "ket")
            return indigoLoadMoleculeFromString(asKet(c.structure).c_str());
        if (fmt == "cml")
            return indigoLoadMoleculeFromString(asCml(c.structure).c_str());
        if (fmt == "smi")
            return indigoLoadMoleculeFromString(asSmiles(c.structure).c_str());
        ADD_FAILURE() << "unknown format " << fmt;
        return -1;
    }

    std::string probeMolecule(int mol, const std::string& probe, int atom_index)
    {
        if (probe == "gross_formula")
        {
            const int gross = indigoGrossFormula(mol);
            const std::string result = indigoToString(gross);
            indigoFree(gross);
            return result;
        }
        // The only observable reachable on a KetDocument object; reading it
        // forces the deferred document -> molecule conversion.
        if (probe == "smiles")
            return indigoSmiles(mol);

        const int atom = indigoGetAtom(mol, atom_index);
        std::string result;
        if (probe == "valence")
            result = std::to_string(indigoValence(atom));
        else if (probe == "implicit_h")
            result = std::to_string(indigoCountImplicitHydrogens(atom));
        else
            ADD_FAILURE() << "unknown probe " << probe;
        indigoFree(atom);
        return result;
    }

    std::string run(const Case& c, const char* option, const char* value)
    {
        // Bare metals under BIOVIA-2009 can trip the bad-valence check; these
        // cases are about which model is applied, not about error policy.
        indigoSetOption("ignore-bad-valence", "true");
        indigoSetOption(option, value);
        const int mol = buildMolecule(c);
        EXPECT_NE(-1, mol) << "failed to build " << label(c);
        if (mol == -1)
            return "<build-failed>";
        const std::string result = probeMolecule(mol, c.probe, 0);
        indigoFree(mol);
        return result;
    }
}

class LoaderOptionsTest : public IndigoApiTest
{
};

TEST_F(LoaderOptionsTest, ValenceModeIsHonouredOnEveryInputPath)
{
    for (const auto& c : kCases)
    {
        EXPECT_EQ(c.expect_a, run(c, "valence-mode", c.mode_a)) << label(c) << " with " << c.mode_a;
        EXPECT_EQ(c.expect_b, run(c, "valence-mode", c.mode_b)) << label(c) << " with " << c.mode_b;
    }
}

TEST_F(LoaderOptionsTest, InvalidValueIsRejected)
{
    // A silently ignored bad value would look exactly like the original defect.
    // IndigoApiTest installs an error handler that rethrows, so the rejection
    // surfaces as an exception rather than a -1 return.
    EXPECT_THROW(indigoSetOption("valence-mode", "not-a-mode"), Exception);
    // A valid value must still be accepted afterwards — a rejected value must
    // not leave the session in a broken state.
    EXPECT_NO_THROW(indigoSetOption("valence-mode", "biovia-2017"));
}

// Ketcher reaches Indigo through channels that each filter the caller's option
// map before handing it to setOption. Every filter is a DENY-list, so an option
// is forwarded unless someone names it — which means a one-word edit in either
// file can disable an option for that channel with nothing else noticing.
TEST_F(LoaderOptionsTest, NoChannelDeniesTheOption)
{
    const std::string repo = std::string(DATA_PATH) + "/..";
    const std::pair<const char*, const char*> channels[] = {
        {"WASM binding (Ketcher in-process)", "/api/wasm/indigo-ketcher/indigo-ketcher.cpp"},
        {"legacy Flask service (/v2/indigo/*)", "/utils/indigo-service/backend/service/v2/indigo_api.py"},
    };
    const std::set<std::string> options{"valence-mode"};

    for (const auto& channel : channels)
    {
        std::ifstream stream(repo + channel.second);
        if (!stream.good())
            continue; // not present in a partial checkout
        std::ostringstream buffer;
        buffer << stream.rdbuf();
        const std::string source = buffer.str();
        for (const auto& option : options)
            EXPECT_EQ(std::string::npos, source.find("\"" + option + "\"")) << channel.first << ": option \"" << option << "\" appears in " << channel.second
                                                                            << " — if it was added to the skip list, that channel now drops it";
    }
}
