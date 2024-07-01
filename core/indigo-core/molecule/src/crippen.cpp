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

#include "molecule/crippen.h"

#include "base_cpp/csv_reader.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/smiles_loader.h"
#include "pka_decision_tree.h"

using namespace std;

namespace
{
    using namespace indigo;

    const vector<pair<string, vector<string>>>& getQueries()
    {
        // clang-format off
        static vector<pair<string, vector<string>>> queries{
            {"C1", {"[CH4]", "[CH3]C", "[CH2](C)C"}},
            {"C2", {"[CH](C)(C)C", "[C](C)(C)(C)C"}},
            {"C3", {"[CH3][N,O,P,S,F,Cl,Br,I]", "[CH2X4]([N,O,P,S,F,Cl,Br,I])[A;!#1]"}},
            {"C4", {"[CH1X4]([N,O,P,S,F,Cl,Br,I])([A;!#1])[A;!#1]", "[CH0X4]([N,O,P,S,F,Cl,Br,I])([A;!#1])([A;!#1])[A;!#1]"}},
            {"C5", {"[C]=[!C;A;!#1]"}},
            {"C6", {"[CH2]=C", "[CH1](=C)[A;!#1]", "[CH0](=C)([A;!#1])[A;!#1]", "[C](=C)=C"}},
            {"C7", {"[CX2]#[A;!#1]"}},
            {"C8", {"[CH3]c"}},
            {"C9", {"[CH3]a"}},
            {"C11", {"[CHX4]a"}},
            {"C12", {"[CH0X4]a"}},
            {"C13", {"[cH0]-[A;!C;!N;!O;!S;!F;!Cl;!Br;!I;!#1]"}},
            {"C14", {"[c][#9]"}},
            {"C15", {"[c][#17]"}},
            {"C16", {"[c][#35]"}},
            {"C17", {"[c][#53]"}},
            {"C18", {"[cH]"}},
            {"C19", {"[c](:a)(:a):a"}},
            {"C20", {"[c](:a)(:a)-a"}},
            {"C21", {"[c](:a)(:a)-C"}},
            {"C22", {"[c](:a)(:a)-N"}},
            {"C23", {"[c](:a)(:a)-O"}},
            {"C24", {"[c](:a)(:a)-S"}},
            {"C25", {"[c](:a)(:a)=[C,N,O]"}},
            {"C26", {"[C](=C)(a)[A;!#1]", "[C](=C)(c)a", "[CH1](=C)a", "[C]=c"}},
            {"C27", {"[CX4][A;!C;!N;!O;!P;!S;!F;!Cl;!Br;!I;!#1]"}},
            {"C", {"[#6]"}},
            {"H1", {"[#1][#6,#1]"}},
            {"H2", {"[#1]O[CX4,c]", "[#1]O[!#6;!#7;!#8;!#16]", "[#1][!#6;!#7;!#8]"}},
            {"H3", {"[#1][#7]", "[#1]O[#7]"}},
            {"H4", {"[#1]OC=[#6,#7,O,S]", "[#1]O[O,S]"}},
            {"H", {"[#1]"}},
            {"N1", {"[NH2+0][A;!#1]"}},
            {"N2", {"[NH+0]([A;!#1])[A;!#1]"}},
            {"N3", {"[NH2+0]a"}},
            {"N4", {"[NH1+0]([!#1;A,a])a"}},
            {"N5", {"[NH+0]=[!#1;A,a]"}},
            {"N6", {"[N+0](=[!#1;A,a])[!#1;A,a]"}},
            {"N7", {"[N+0]([A;!#1])([A;!#1])[A;!#1]"}},
            {"N8", {"[N+0](a)([!#1;A,a])[A;!#1]", "[N+0](a)(a)a"}},
            {"N9", {"[N+0]#[A;!#1]"}},
            {"N10", {"[NH3,NH2,NH;+,+2,+3]"}},
            {"N11", {"[n+0]"}},
            {"N12", {"[n;+,+2,+3]"}},
            {"N13", {"[NH0;+,+2,+3]([A;!#1])([A;!#1])([A;!#1])[A;!#1]", "[NH0;+,+2,+3](=[A;!#1])([A;!#1])[!#1;A,a]", "[NH0;+,+2,+3](=[#6])=[#7]"}},
            {"N14", {"[N;+,+2,+3]#[A;!#1]", "[N;-,-2,-3]", "[N;+,+2,+3](=[N;-,-2,-3])=N"}},
            {"N", {"[#7]"}},
            {"O1", {"[o]"}},
            {"O2", {"[OH,OH2]"}},
            {"O3", {"[O]([A;!#1])[A;!#1]"}},
            {"O4", {"[O](a)[!#1;A,a]"}},
            {"O5", {"[O]=[#7,#8]", "[OX1;-,-2,-3][#7]"}},
            {"O6", {"[OX1;-,-2,-2][#16]", "[O;-0]=[#16;-0]"}},
            {"O12", {"[O-]C(=O)"}},
            {"O7", {"[OX1;-,-2,-3][!#1;!N;!S]"}},
            {"O8", {"[O]=c"}},
            {"O9", {"[O]=[CH]C", "[O]=C(C)([A;!#1])", "[O]=[CH][N,O]", "[O]=[CH2]", "[O]=[CX2]=O"}},
            {"O10", {"[O]=[CH]c", "[O]=C([C,c])[a;!#1]", "[O]=C(c)[A;!#1]"}},
            {"O11", {"[O]=C([!#1;!#6])[!#1;!#6]"}},
            {"O", {"[#8]"}},
            {"F", {"[#9-0]"}},
            {"Cl", {"[#17-0]"}},
            {"Br", {"[#35-0]"}},
            {"I", {"[#53-0]"}},
            {"F2", {"[#9-*]"}},
            {"Cl2", {"[#17-*]"}},
            {"Br2", {"[#35-*]"}},
            {"I2", {"[#53-*]", "[#53+*]"}},
            {"P", {"[#15]"}},
            {"S2", {"[S;-,-2,-3,-4,+1,+2,+3,+5,+6]", "[S-0]=[N,O,P,S]"}},
            {"S1", {"[S;A]"}},
            {"S3", {"[s;a]"}},
            {"Me1", {"[#3,#11,#19,#37,#55]", "[#4,#12,#20,#38,#56]", "[#5,#13,#31,#49,#81]", "[#14,#32,#50,#82]", "[#33,#51,#83]", "[#34,#52,#84]"}},
            {"Me2", {"[#21,#22,#23,#24,#25,#26,#27,#28,#29,#30]", "[#39,#40,#41,#42,#43,#44,#45,#46,#47,#48]", "[#72,#73,#74,#75,#76,#77,#78,#79,#80]"}},
            {"Hal", {"[#9,#17,#35,#53;-]", "[#53;+,+2,+3]", "[+;#3,#11,#19,#37,#55]"}},
        };
        // clang-format on
        return queries;
    }

    const unordered_map<string, double>& getLogPContributions()
    {
        // clang-format off
        static unordered_map<string, double> contributions{
            {"C1", 0.1441},
            {"C2", 0.0},
            {"C3", -0.2035},
            {"C4", -0.2051},
            {"C5", -0.2783},
            {"C6", 0.1551},
            {"C7", 0.0017},
            {"C8", 0.08452},
            {"C9", -0.1444},
            {"C10",-0.0516},
            {"C11", 0.1193},
            {"C12", -0.0967},
            {"C13", -0.5443},
            {"C14", 0.0},
            {"C15", 0.245},
            {"C16", 0.198},
            {"C17", 0.0},
            {"C18", 0.1581},
            {"C19", 0.2955},
            {"C20", 0.2713},
            {"C21", 0.136},
            {"C22", 0.4619},
            {"C23", 0.5437},
            {"C24", 0.1893},
            {"C25", -0.8186},
            {"C26", 0.264},
            {"C27", 0.2148},
            {"C", 0.08129},
            {"H1", 0.123},
            {"H2", -0.2677},
            {"H3",  0.2142},
            {"H4",  0.298},
            {"H", 0.1125},
            {"N1", -1.019},
            {"N2", -0.7096},
            {"N3", -1.027},
            {"N4", -0.5188},
            {"N5", 0.08387},
            {"N6", 0.1836},
            {"N7", -0.3187},
            {"N8", -0.4458},
            {"N9", 0.01508},
            {"N10", -1.95},
            {"N11", -0.3239},
            {"N12", -1.119},
            {"N13", -0.3396},
            {"N14", 0.2887},
            {"N", -0.4806},
            {"O1", 0.1552},
            {"O2", -0.2893},
            {"O3", -0.0684},
            {"O4", -0.4195},
            {"O5", 0.0335},
            {"O6", -0.3339},
            {"O7", -1.189},
            {"O8", 0.1788},
            {"O9", -0.1526},
            {"O10", 0.1129},
            {"O11", 0.4833},
            {"O12", -1.326},
            {"O", -0.1188},
            {"F2", -2.996},
            {"F", 0.4202},
            {"Cl2", -2.996},
            {"Cl", 0.6895},
            {"Br2", -2.2996},
            {"Br", 0.8456},
            {"I2", -2.996},
            {"I", 0.8857},
            {"P", 0.8612},
            {"S1", 0.6482},
            {"S2", -0.0024},
            {"S3", 0.6237},
            {"Me1", -0.3808},
            {"Me2", -0.0025},
            {"Hal", -2.996}
        };
        // clang-format on
        return contributions;
    }

    QueryMolecule& loadSmarts(const string& smarts)
    {
        thread_local unordered_map<string, QueryMolecule> smartsHolder;
        if (smartsHolder.count(smarts) == 0)
        {
            BufferScanner scanner(smarts.c_str());
            SmilesLoader loader(scanner);
            loader.loadSMARTS(smartsHolder[smarts]);
        }
        return smartsHolder.at(smarts);
    }

    const unordered_map<string, double>& getMRContributions()
    {
        // clang-format off
        static unordered_map<string, double> contributions{
            {"C1", 2.503},
            {"C2", 2.433},
            {"C3", 2.753},
            {"C4", 2.731},
            {"C5", 5.007},
            {"C6", 3.513},
            {"C7", 3.888},
            {"C8", 2.464},
            {"C9", 2.412},
            {"C10", 2.488},
            {"C11", 2.582},
            {"C12", 2.576},
            {"C13", 4.041},
            {"C14", 3.257},
            {"C15", 3.564},
            {"C16", 3.18},
            {"C17", 3.104},
            {"C18", 3.35},
            {"C19", 4.346},
            {"C20", 3.904},
            {"C21", 3.509},
            {"C22", 4.067},
            {"C23", 3.853},
            {"C24", 2.673},
            {"C25", 3.135},
            {"C26", 4.305},
            {"C27", 2.693},
            {"C", 3.243},
            {"H1", 1.057},
            {"H2", 1.395},
            {"H3", 0.9627},
            {"H4", 1.805},
            {"H", 1.112},
            {"N1", 2.262},
            {"N2", 2.173},
            {"N3", 2.827},
            {"N4", 3.0},
            {"N5", 1.751},
            {"N6", 2.428},
            {"N7", 1.839},
            {"N8", 2.819},
            {"N9", 1.725},
            {"N10", nanf("")},
            {"N11", 2.202},
            {"N12", nanf("")},
            {"N13", 0.2604},
            {"N14", 3.359},
            {"N", 2.134},
            {"O1", 1.08},
            {"O2", 0.8238},
            {"O3", 1.085},
            {"O4", 1.182},
            {"O5", 3.367},
            {"O6", 0.7774},
            {"O7", 0.0},
            {"O8", 3.135},
            {"O9", 0.0},
            {"O10", 0.2215},
            {"O11", 0.389},
            {"O12", nanf("")},
            {"O", 0.6865},
            {"F2", nanf("")},
            {"F", 5.853},
            {"Cl2", nanf("")},
            {"Cl", 5.853},
            {"Br2", nanf("")},
            {"Br", 8.927},
            {"I2", nanf("")},
            {"I", 14.02},
            {"P", 6.92},
            {"S1", 7.591},
            {"S2", 7.365},
            {"S3", 6.691},
            {"Me1", 5.754},
            {"Me2", nanf("")},
            {"Hal", nanf("")}
        };
        // clang-format on
        return contributions;
    }

    unordered_map<string, int> calculateMatches(Molecule& rawMolecule)
    {
        Molecule molecule;
        molecule.clone(rawMolecule);
        if (!molecule.isAromatized())
        {
            molecule.aromatize(AromaticityOptions());
        }
        Array<int> markers;
        molecule.unfoldHydrogens(&markers, -1);
        unordered_set<int> ignoredAtoms;
        ignoredAtoms.reserve(molecule.vertexCount());
        Array<int> mapping;

        unordered_map<string, int> result;
        for (const auto& query : getQueries())
        {
            const auto& queryClass = query.first;
            const auto& querySmartsVector = query.second;
            int matches = 0;
            for (const auto& querySmarts : querySmartsVector)
            {
                auto& queryMolecule = loadSmarts(querySmarts);
                MoleculeSubstructureMatcher matcher(molecule);
                matcher.use_aromaticity_matcher = true;
                matcher.disable_folding_query_h = true;
                matcher.restore_unfolded_h = false;
                matcher.find_unique_embeddings = true;
                matcher.save_for_iteration = true;
                matcher.setQuery(queryMolecule);

                for (bool flag = matcher.find(); flag; flag = matcher.findNext())
                {
                    mapping.clear();
                    mapping.copy(matcher.getQueryMapping(), queryMolecule.vertexEnd());
                    const auto index = mapping[0];
                    if (index > -1 && !ignoredAtoms.count(index))
                    {
                        ignoredAtoms.insert(index);
                        ++matches;
                    }
                }

                if (ignoredAtoms.size() == molecule.vertexCount())
                {
                    break;
                }
            }
            if (matches)
            {
                result[queryClass] = matches;
            }
        }

        return result;
    }

    struct PKANode
    {
        size_t id;
        bool isLeaf;
        double pkaValue;
        shared_ptr<PKANode> yes = nullptr;
        shared_ptr<PKANode> no = nullptr;
        QueryMolecule smarts;
        string smartsString;

        PKANode(const size_t id, const bool isLeaf, const double pkaValue, const string& smartsString)
            : id(id), isLeaf(isLeaf), pkaValue(pkaValue), smartsString(smartsString)
        {
        }
    };

    struct PKACalculator
    {
    public:
        PKACalculator()
        {
            unordered_map<size_t, shared_ptr<PKANode>> nodes;
            for (const auto& rawLine : pkaDecisionTree)
            {
                const auto& line = CSVReader::readCSVRow(rawLine);
                const size_t id = static_cast<size_t>(stoull(line[0]));
                const size_t parentId = static_cast<size_t>(stoull(line[1]));
                const bool isLeaf = !static_cast<bool>(stoi(line[2]));
                const string& smarts = line[4];
                const bool yes = static_cast<bool>(stoi(line[5]));
                const double pkaValue = stod(line[6]);

                auto node = make_shared<PKANode>(id, isLeaf, pkaValue, smarts);
                MoleculeAutoLoader loader(smarts.c_str());
                loader.loadMolecule(node->smarts);

                nodes[id] = node;
                if (parentId > 0)
                {
                    if (yes)
                    {
                        nodes.at(parentId)->yes = node;
                    }
                    else
                    {
                        nodes.at(parentId)->no = node;
                    }
                }
            }
            root = nodes.at(1);
        }

        double calculate(Molecule& target) const
        {
            MoleculeSubstructureMatcher matcher(target);
            return traverse(matcher, root);
        }

    private:
        shared_ptr<PKANode> root = nullptr;

        double traverse(MoleculeSubstructureMatcher& matcher, const shared_ptr<PKANode>& node) const
        {
            if (node->isLeaf)
            {
                return node->pkaValue;
            }
            matcher.setQuery(node->yes->smarts);
            if (matcher.find())
            {
                return traverse(matcher, node->yes);
            }
            return traverse(matcher, node->no);
        }
    };

    const PKACalculator pkaCalculator;
}

namespace indigo
{
    // https://doi.org/10.1021/ci990307l
    double Crippen::logP(Molecule& molecule)
    {
        const auto& matches = calculateMatches(molecule);
        const auto& logPContributions = getLogPContributions();
        double logP = 0.0;
        for (const auto& match : matches)
        {
            logP += logPContributions.at(match.first) * match.second;
        }
        return logP;
    }

    // https://doi.org/10.1021/ci990307l
    double Crippen::molarRefractivity(Molecule& molecule)
    {
        const auto& matches = calculateMatches(molecule);
        const auto& contributions = getMRContributions();
        double mr = 0.0;
        for (const auto& match : matches)
        {
            mr += contributions.at(match.first) * match.second;
        }
        return mr;
    }

    // https://doi.org/10.1021/ci8001815
    double Crippen::pKa(Molecule& molecule)
    {
        Molecule copy;
        copy.clone(molecule);
        copy.aromatize(AromaticityOptions());
        return pkaCalculator.calculate(molecule);
    }
}
