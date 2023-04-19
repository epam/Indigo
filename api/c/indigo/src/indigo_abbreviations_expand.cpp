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

#include "indigo_abbreviations.h"

#include "base_c/bitarray.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/smiles_loader.h"

#include "indigo.h"

#include "indigo_internal.h"
#include "indigo_molecule.h"

#include "layout/molecule_layout.h"

#include <algorithm>

namespace indigo
{
    namespace abbreviations
    {

        enum Direction
        {
            LEFT,
            RIGHT
        };

        // Single abbreviation token
        // Each abbreviation is tokenized first: (C2H5)2N- ->  N (C*2 H*5)*2
        struct Token;
        typedef std::vector<Token> TokenChain;
        struct Token
        {
            enum Type
            {
                Pattern,
                Element,
                Branch
            };
            Type type;

            int index;         // Index in the abbreviations array or element index
            TokenChain branch; // Branch if brackets are present

            int multiplier;
        };

        // Attachment point with order
        struct AttPoint
        {
            explicit AttPoint(int index_, int order_) : index(index_), order(order_)
            {
            }
            int index, order;
        };

        //
        // AbbreviationExpander
        //
        class AbbreviationExpander
        {
        public:
            AbbreviationExpander(const PtrArray<Abbreviation>& abbreviations) : abbreviations(abbreviations)
            {
                ignore_case = false;
                tokenize_level = 0;
                tokenize_direction = expand_direction = RIGHT;
            }

            // Settings
            const PtrArray<Abbreviation>& abbreviations;
            Direction tokenize_direction, expand_direction;
            int tokenize_level;

            bool ignore_case;

            // Results
            int input_index, output_index;

            Array<int> added_atoms;

            bool expandAtomAbbreviation(Molecule& mol, int v);

        private:
            bool expand(const char* label, int input_order, int output_order, Molecule& dest);

            int tokensizeSubExpression(const char* label, TokenChain& tokens);
            int scanSinlgeToken(const char* label, Token& dest);
            bool tokensizeAbbreviation(const char* label, TokenChain& tokens);

            bool expandParsedTokens(TokenChain& tokens, Molecule& m, AttPoint& attach_to);
            bool tryApplyExpansions(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to);
            bool tryCarbonChain(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to);
            bool tryRepetition(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to);
            bool tryExpandToken(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to);
            bool expandParsedTokensWithRev(TokenChain& tokens, Molecule& m, AttPoint& attach_to);
            void attachBond(Molecule& m, AttPoint& attach_to, int index);

            bool compareStrings(const char* s1, const char* s2, size_t len);
        };

        bool AbbreviationExpander::compareStrings(const char* s1, const char* s2, size_t len)
        {
            if (ignore_case)
                return strncasecmp(s1, s2, len) == 0;
            else
                return strncmp(s1, s2, len) == 0;
        }

        int AbbreviationExpander::tokensizeSubExpression(const char* label, TokenChain& tokens)
        {
            size_t size = strlen(label);
            size_t offset = 0;
            while (offset < size)
            {
                Token t;
                int offset2 = scanSinlgeToken(label + offset, t);
                if (offset2 < 0)
                    return -1;
                tokens.push_back(t);
                offset += offset2;
                if (label[offset] == ')')
                    break;
            }
            return offset;
        }

        int AbbreviationExpander::scanSinlgeToken(const char* label, Token& dest)
        {
            size_t size = strlen(label);
            int offset = 0;

            if (label[0] == '(')
            {
                dest.type = Token::Branch;
                int offset2 = tokensizeSubExpression(label + 1, dest.branch);
                if (offset2 == -1 || label[offset2 + 1] != ')')
                    return -1;
                offset = offset2 + 2;
            }
            else
            {
                // Trying to apply geedy algorithm
                size_t best_matched_length = 0;
                int best_abbreviation = -1;
                for (int i = 0; i < abbreviations.size(); i++)
                {
                    const Abbreviation* cur = abbreviations[i];

                    const std::vector<std::string>* aliases[2] = {NULL, NULL};
                    if (tokenize_direction == LEFT)
                    {
                        aliases[0] = &cur->left_aliases;
                        if (tokenize_level == 2)
                            aliases[1] = &cur->left_aliases2;
                    }
                    else if (tokenize_direction == RIGHT)
                    {
                        aliases[0] = &cur->right_aliases;
                        if (tokenize_level == 2)
                            aliases[1] = &cur->right_aliases2;
                    }

                    for (int k = 0; k < 2; k++)
                    {
                        if (aliases[k] == NULL)
                            break;
                        for (size_t j = 0; j < aliases[k]->size(); j++)
                        {
                            const std::string& alias = aliases[k]->at(j);

                            if (alias.length() < best_matched_length)
                                continue;

                            if (compareStrings(label, alias.c_str(), alias.length()))
                            {
                                best_matched_length = alias.length();
                                best_abbreviation = i;
                            }
                        }
                    }
                }

                if (best_abbreviation != -1)
                {
                    dest.type = Token::Pattern;
                    dest.index = best_abbreviation;
                    offset += best_matched_length;
                }
                else
                {
                    // Try to match the tail with any known element: H, O, C, N, Sn, Mg
                    // Try to read until hydrogen and digit or other captial letter
                    size_t len = 1;
                    while (len < size)
                    {
                        char ch = label[len];
                        if (ch == 'H' || !isalpha(ch) || (ch >= 'A' && ch <= 'Z'))
                            break;
                        len++;
                    }
                    char symb[4] = {0};
                    strncpy(symb, label, std::min(NELEM(symb) - 1, len));
                    int id = Element::fromString2(symb);
                    if (id < 0)
                        return -1;

                    dest.type = Token::Element;
                    dest.index = id;
                    offset += len;
                }
            }

            // Try to read multiplier
            if (isdigit(label[offset]))
            {
                int shift;
                sscanf(label + offset, "%d%n", &dest.multiplier, &shift);
                offset += shift;
            }
            else
                dest.multiplier = 1;
            return offset;
        }

        bool AbbreviationExpander::tokensizeAbbreviation(const char* label, TokenChain& tokens)
        {
            size_t size = strlen(label);
            size_t offset = 0;
            while (offset < size)
            {
                Token t;
                int offset2 = scanSinlgeToken(label + offset, t);
                if (offset2 < 0)
                    return false;
                tokens.push_back(t);
                offset += offset2;
            }
            return true;
        }

        bool AbbreviationExpander::tryApplyExpansions(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to)
        {
            if (tryCarbonChain(tokens, offset, m, attach_to))
                return true;
            if (tryRepetition(tokens, offset, m, attach_to))
                return true;
            if (tryExpandToken(tokens, offset, m, attach_to))
                return true;
            return false;
        }

        void AbbreviationExpander::attachBond(Molecule& m, AttPoint& attach_to, int idx)
        {
            if (attach_to.index != -1)
                m.addBond(attach_to.index, idx, attach_to.order);
            else
                input_index = idx;
        }

        bool AbbreviationExpander::tryCarbonChain(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to)
        {
            if (attach_to.order != 1)
                return false;

            Token& cur = tokens[offset];
            if (cur.type != Token::Element)
                return false;

            if (cur.multiplier == 1 || cur.index != ELEM_C || offset + 1 == tokens.size())
                return false;

            Token& next = tokens[offset + 1];
            // Check CnH(2n+1), CnH2n pattern
            if (next.multiplier > 1 && next.index == ELEM_H)
            {
                bool tail;
                if (next.multiplier == cur.multiplier * 2)
                {
                    // Intermediate carbon chain
                    tail = false;
                }
                else if (next.multiplier == cur.multiplier * 2 + 1)
                {
                    // Terminator carbon chain
                    tail = true;
                }
                else
                    return false;

                for (int i = 0; i < cur.multiplier; i++)
                {
                    int idx = m.addAtom(ELEM_C);
                    attachBond(m, attach_to, idx);

                    attach_to = AttPoint(idx, 1);
                }
                if (tail)
                    attach_to = AttPoint(-1, 0);
                offset += 2;
                return true;
            }
            return false;
        }

        bool AbbreviationExpander::expandParsedTokensWithRev(TokenChain& tokens, Molecule& m, AttPoint& attach_to)
        {
            if (expandParsedTokens(tokens, m, attach_to))
                return true;
            if (expand_direction != LEFT)
                return false;

            std::reverse(tokens.begin(), tokens.end());
            bool ret = expandParsedTokens(tokens, m, attach_to);
            std::reverse(tokens.begin(), tokens.end());
            return ret;
        }

        bool AbbreviationExpander::tryRepetition(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to)
        {
            if (attach_to.order != 1)
                return false;

            Token& cur = tokens[offset];
            if (cur.type != Token::Branch || cur.multiplier == 1)
                return false;

            int atom_bound = m.vertexCount();

            AttPoint cur_attach_to = attach_to;
            int i;
            for (i = 0; i < cur.multiplier; i++)
            {
                if (!expandParsedTokensWithRev(cur.branch, m, cur_attach_to))
                    break;
                if (cur_attach_to.index == -1)
                    break;
            }

            if (i != cur.multiplier)
            {
                // Rollback
                Array<int> new_atoms;
                for (int v = m.vertexBegin(); v != m.vertexEnd(); v = m.vertexNext(v))
                    if (v >= atom_bound)
                        new_atoms.push(v);
                m.removeAtoms(new_atoms);
                return false;
            }

            offset++;
            attach_to = cur_attach_to;
            return true;
        }

        bool AbbreviationExpander::tryExpandToken(TokenChain& tokens, size_t& offset, Molecule& m, AttPoint& attach_to)
        {
            Token& cur = tokens[offset];

            if (cur.multiplier != 1)
                return false;

            Array<int> connection_points;
            if (cur.type == Token::Element)
            {
                if (cur.index == ELEM_H)
                {
                    offset++;
                    attach_to = AttPoint(-1, 0);
                    return true;
                }
                int added = m.addAtom(cur.index);

                // Get the number of bonds to connect
                int valence, hyd;
                int conn = attach_to.order;
                if (offset + 1 < tokens.size())
                {
                    Token& next = tokens[offset + 1];
                    conn += next.multiplier;
                }

                if (!Element::calcValence(cur.index, 0, 0, conn, valence, hyd, false))
                {
                    // Ignore next atom
                    // Appear in the OH3C case when H3 is belong to C
                    conn = attach_to.order;
                    if (!Element::calcValence(cur.index, 0, 0, conn, valence, hyd, false))
                        return false;
                }

                for (int i = 0; i < hyd + conn; i++)
                    connection_points.push(added);
            }
            else if (cur.type == Token::Pattern)
            {
                // Add pattern
                BufferScanner scanner(abbreviations[cur.index]->expansion.c_str());
                SmilesLoader loader(scanner);

                Molecule abbr;
                loader.loadMolecule(abbr);

                Array<int> mapping;
                Array<int> rsites;
                m.mergeWithMolecule(abbr, &mapping);
                for (int v = abbr.vertexBegin(); v != abbr.vertexEnd(); v = abbr.vertexNext(v))
                {
                    int mapped = mapping[v];
                    if (m.isRSite(mapped))
                    {
                        dword bits = m.getRSiteBits(mapped);
                        int id1 = bitGetOneHOIndex(bits);
                        int id2 = bitGetOneHOIndex(bits);
                        if (id1 != id2)
                            throw Exception("Invalid abbreviations specification: %s", abbreviations[cur.index]->expansion.c_str());
                        if (id1 != 0)
                            id1--; // R == R1

                        const Vertex& vertex = m.getVertex(mapped);
                        int nei = vertex.neiBegin();

                        connection_points.expandFill(id1 + 1, -1);
                        connection_points[id1] = vertex.neiVertex(nei); // Point connected to the RSite

                        rsites.push(mapped);
                    }
                }
                m.removeAtoms(rsites);
            }
            else
                return false;

            bool rollback = false;
            int atom_bound = m.vertexCount();
            size_t offset2 = offset + 1;

            attachBond(m, attach_to, connection_points[0]);
            int i = attach_to.order;
            while (i < connection_points.size() - 1 && !rollback)
            {
                if (offset2 >= tokens.size())
                {
                    // If we are at the end then there can be an implicit double bond
                    // Example: -CH2CH=
                    // When we read C H there are no more tokens
                    break;
                }

                Token& next = tokens[offset2];
                for (int j = 0; j < next.multiplier; j++)
                {
                    if (i >= connection_points.size())
                    {
                        rollback = true;
                        break;
                    }

                    if (next.type == Token::Branch)
                    {
                        AttPoint point(connection_points[i], 1);

                        if (!expandParsedTokensWithRev(next.branch, m, point) || point.index != -1)
                        {
                            rollback = true;
                            break;
                        }
                    }
                    else
                    {
                        TokenChain chain;
                        chain.push_back(next);
                        chain[0].multiplier = 1;
                        size_t local_offset = 0;
                        AttPoint point(connection_points[i], 1);
                        if (!tryExpandToken(chain, local_offset, m, point) || point.index != -1)
                        {
                            rollback = true;
                            break;
                        }
                    }
                    i++;
                }
                offset2++;
            }

            if (i > connection_points.size())
                rollback = true;
            if (!rollback)
            {
                if (i == connection_points.size())
                {
                    // This is terminal
                    attach_to = AttPoint(-1, 0);
                }
                else if (i == connection_points.size() - 1)
                    attach_to = AttPoint(connection_points[i], 1); // Last attachment point
                else
                {
                    // Number of tokens are incomlete means that there are double bonds after
                    attach_to = AttPoint(connection_points[i], connection_points.size() - i);
                }
            }

            if (rollback)
            {
                // Rollback
                Array<int> new_atoms;
                for (int v = m.vertexBegin(); v != m.vertexEnd(); v = m.vertexNext(v))
                    if (v >= atom_bound)
                        new_atoms.push(v);
                m.removeAtoms(new_atoms);
                return false;
            }
            offset = offset2;
            return true;
        }

        bool AbbreviationExpander::expandParsedTokens(TokenChain& tokens, Molecule& m, AttPoint& attach_to)
        {
            AttPoint attach_to_saved = attach_to;
            size_t index = 0;
            bool try_swapped = false;

            int atom_bound = m.vertexCount();
            bool first = true;

            while (index < tokens.size())
            {
                if (!first && attach_to.index == -1)
                {
                    attach_to = attach_to_saved;
                    // Rollback
                    Array<int> new_atoms;
                    for (int v = m.vertexBegin(); v != m.vertexEnd(); v = m.vertexNext(v))
                        if (v >= atom_bound)
                            new_atoms.push(v);
                    m.removeAtoms(new_atoms);
                    return false;
                }

                if (tryApplyExpansions(tokens, index, m, attach_to))
                {
                    first = false;
                    try_swapped = false;
                    continue;
                }

                if (try_swapped)
                    return false;

                if (expand_direction == LEFT && index + 1 < tokens.size())
                {
                    // Try to swap two tokens if H is the first
                    // Happens when left labels are written with right hydrogens: CHO-, C2H5-, etc
                    Token& cur = tokens[index];
                    Token& next = tokens[index + 1];
                    if (cur.type == Token::Element && next.type == Token::Element)
                        std::swap(tokens[index], tokens[index + 1]);

                    try_swapped = true;
                    continue;
                }

                return false;
            }
            return true;
        }

        bool AbbreviationExpander::expand(const char* label, int input_order, int output_order, Molecule& m)
        {
            TokenChain tokens;
            if (!tokensizeAbbreviation(label, tokens))
                return false;

            if (expand_direction == LEFT)
                std::reverse(tokens.begin(), tokens.end());

            m.clear();

            AttPoint begin_att_point(-1, input_order);
            AttPoint end_att_point = begin_att_point;

            if (!expandParsedTokens(tokens, m, end_att_point))
                return false;

            if (end_att_point.order != output_order)
                return false;

            output_index = end_att_point.index;

            return true;
        }

        struct Options
        {
            Options(Direction expand_direction, Direction tokenize_direction, int tokenize_level = 1, bool ignore_case = false)
                : expand_direction(expand_direction), tokenize_direction(tokenize_direction), tokenize_level(tokenize_level)
            {
            }

            Direction expand_direction, tokenize_direction;
            int tokenize_level;
            bool ignore_case;

            void set(AbbreviationExpander& expander)
            {
                expander.expand_direction = expand_direction;
                expander.tokenize_direction = tokenize_direction;
                expander.tokenize_level = tokenize_level;
                expander.ignore_case = ignore_case;
            }
        };

        bool AbbreviationExpander::expandAtomAbbreviation(Molecule& mol, int v)
        {
            // Determine bonds configuration: number of bonds on the left and on the right
            Vec3f& pos = mol.getAtomXyz(v);
            const Vertex& vertex = mol.getVertex(v);
            int count_left = 0, count_right = 0, count_middle = 0;
            Array<int> left_atoms, right_atoms;
            for (int nei = vertex.neiBegin(); nei != vertex.neiEnd(); nei = vertex.neiNext(nei))
            {
                int nei_atom = vertex.neiVertex(nei);
                Vec3f& nei_pos = mol.getAtomXyz(nei_atom);
                int order = mol.getBondOrder(vertex.neiEdge(nei));
                if (nei_pos.x < pos.x)
                {
                    count_left += order;
                    left_atoms.push(nei_atom);
                }
                else
                {
                    count_right += order;
                    right_atoms.push(nei_atom);
                }

                Vec3f diff = nei_pos;
                diff.sub(pos);
                if (fabs(diff.y) > fabs(diff.x))
                    count_middle += order;
            }

            int input_order, output_order;

            bool on_left = false, on_right = false, is_single = false;
            if (vertex.degree() == 1)
            {
                if (count_middle)
                    on_left = on_right = true;
                if (count_left)
                    on_right = true;
                if (count_right)
                    on_left = true;
                input_order = std::max(count_left, count_right);
                output_order = 0;
                is_single = true;
            }
            else
            {
                on_right = true;
                input_order = count_left;
                output_order = count_right;
            }

            // Try to expand according to the directions
            Array<Options> options;

            if (on_right && !on_left)
            {
                options.push(Options(RIGHT, RIGHT));
                options.push(Options(LEFT, LEFT));
            }
            else
            {
                if (on_right)
                    options.push(Options(RIGHT, RIGHT));
                if (on_left)
                {
                    options.push(Options(LEFT, LEFT));
                    options.push(Options(LEFT, RIGHT));
                    options.push(Options(RIGHT, RIGHT));
                    options.push(Options(LEFT, LEFT, 2));
                }
            }

            // Duplicate all the options with ignore case flag
            int opt_cnt = options.size();
            for (int i = 0; i < opt_cnt; i++)
            {
                Options opt = options[i];
                opt.ignore_case = true;
                options.push(opt);
            }

            bool found = false;
            Molecule expanded;
            for (int i = 0; i < options.size(); i++)
            {
                options[i].set(*this);

                if (expand(mol.getPseudoAtom(v), input_order, output_order, expanded))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                return false;

            // Merge expanded abbreviation with the source molecule and connect bonds
            Array<int> mapping;
            mol.mergeWithMolecule(expanded, &mapping);

            // if (right_atoms.size() == 0)
            //   right_atoms.swap(left_atoms);

            for (int i = 0; i < left_atoms.size(); i++)
                mol.flipBond(left_atoms[i], v, mapping[input_index]);

            int target_end = output_index;
            if (is_single == 1)
                target_end = input_index;

            for (int i = 0; i < right_atoms.size(); i++)
                mol.flipBond(right_atoms[i], v, mapping[target_end]);

            // Collect added atoms and set their coordinate to the initial atom
            // Layout procedure will find correct atom coordinates, but neighbours
            // should know correct relative position
            for (int ve = expanded.vertexBegin(); ve != expanded.vertexEnd(); ve = expanded.vertexNext(ve))
            {
                int idx = mapping[ve];
                mol.setAtomXyz(idx, mol.getAtomXyz(v));
                added_atoms.push(mapping[ve]);
            }

            int sid = mol.sgroups.addSGroup(SGroup::SG_TYPE_SUP);
            Superatom& super = (Superatom&)mol.sgroups.getSGroup(sid);
            super.subscript.readString(mol.getPseudoAtom(v), true);
            for (int ve = expanded.vertexBegin(); ve != expanded.vertexEnd(); ve = expanded.vertexNext(ve))
                super.atoms.push(mapping[ve]);

            mol.removeAtom(v);

            return true;
        }

        //
        // Interface functions
        //
        CEXPORT int indigoExpandAbbreviations(int molecule)
        {
            INDIGO_BEGIN
            {
                // TODO 
                AbbreviationExpander expander(indigoGetAbbreviationsInstance().abbreviations);

                IndigoObject& obj = self.getObject(molecule);
                Molecule& mol = obj.getMolecule();

                float avg_bond_length = 0;
                if (Molecule::hasCoord(mol))
                {
                    // Detect average bond length
                    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
                    {
                        const Edge& edge = mol.getEdge(e);
                        Vec3f p1 = mol.getAtomXyz(edge.beg);
                        Vec3f p2 = mol.getAtomXyz(edge.end);
                        Vec3f diff;
                        diff.diff(p1, p2);
                        avg_bond_length += diff.length();
                    }
                    avg_bond_length /= mol.edgeCount();
                }

                // Collect pseudoatoms first because abbreviation can change atom ordering because
                // it deletes atoms and adds new ones
                Array<int> pseudoatoms;
                for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
                {
                    if (mol.isPseudoAtom(v))
                        pseudoatoms.push(v);
                }
                int count = 0;
                for (int i = 0; i < pseudoatoms.size(); i++)
                {
                    // Try to expand this pseudoatom
                    count += expander.expandAtomAbbreviation(mol, pseudoatoms[i]);
                }

                if (count > 0 && Molecule::hasCoord(mol))
                {
                    // Layout expanded parts
                    MoleculeLayout ml(mol, self.smart_layout);
                    ml.max_iterations = self.layout_max_iterations;
                    ml.layout_orientation = (layout_orientation_value)self.layout_orientation;
                    ml.respect_existing_layout = true;
                    // Check if there are not bonds at all or if the coordinates are invalid
                    if (fabs(avg_bond_length) > 0.001)
                        ml.bond_length = avg_bond_length;

                    Filter f;
                    f.initNone(mol.vertexEnd());
                    for (int i = 0; i < expander.added_atoms.size(); i++)
                        f.unhide(expander.added_atoms[i]);
                    ml.filter = &f;

                    ml.make();
                    RedBlackSet<int> atoms_with_nei;

                    for (int i = 0; i < expander.added_atoms.size(); i++)
                    {
                        const Vertex& vertex = mol.getVertex(expander.added_atoms[i]);
                        for (int nei = vertex.neiBegin(); nei != vertex.neiEnd(); nei = vertex.neiNext(nei))
                        {
                            int nei_atom = vertex.neiVertex(nei);
                            atoms_with_nei.find_or_insert(nei_atom);
                        }
                        atoms_with_nei.find_or_insert(expander.added_atoms[i]);
                    }
                    for (int i = atoms_with_nei.begin(); i != atoms_with_nei.end(); i = atoms_with_nei.next(i))
                        mol.markBondStereocenters(atoms_with_nei.key(i));
                }

                return count;
            }
            INDIGO_END(-1);
        }

    } // namespace abbreviations
} // namespace indigo
