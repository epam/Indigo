#include "molecule/molecule_json_loader.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "layout/molecule_layout.h"
#include "molecule/elements.h"
#include "molecule/ket_commons.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

using namespace rapidjson;
using namespace indigo;
using namespace std;

IMPL_ERROR(MoleculeJsonLoader, "molecule json loader");

MoleculeJsonLoader::MoleculeJsonLoader(Value& mol_nodes, Value& rgroups, rapidjson::Value& simple_objects)
    : _mol_nodes(mol_nodes), _rgroups(rgroups), _simple_objects(simple_objects), _pmol(0), _pqmol(0), _empty_array(kArrayType)
{
}

MoleculeJsonLoader::MoleculeJsonLoader(Value& mol_nodes, Value& rgroups)
    : _empty_array(kArrayType), _mol_nodes(mol_nodes), _rgroups(rgroups), _simple_objects(_empty_array), _pmol(0), _pqmol(0)
{
}

int MoleculeJsonLoader::addBondToMoleculeQuery(int beg, int end, int order, int topology)
{
    std::unique_ptr<QueryMolecule::Bond> bond;
    if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC || order == _BOND_COORDINATION ||
        order == _BOND_HYDROGEN)
        bond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, order);
    else if (order == _BOND_SINGLE_OR_DOUBLE)
        bond.reset(QueryMolecule::Bond::und(QueryMolecule::Bond::nicht(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)),
                                            QueryMolecule::Bond::oder(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE),
                                                                      new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_DOUBLE))));
    else if (order == _BOND_SINGLE_OR_AROMATIC)
        bond.reset(QueryMolecule::Bond::oder(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE),
                                             new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
    else if (order == _BOND_DOUBLE_OR_AROMATIC)
        bond.reset(QueryMolecule::Bond::oder(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_DOUBLE),
                                             new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
    else if (order == _BOND_ANY)
        bond = std::make_unique<QueryMolecule::Bond>();
    else
        throw Error("unknown bond type: %d", order);
    if (topology != 0)
    {
        bond.reset(
            QueryMolecule::Bond::und(bond.release(), new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, topology == 1 ? TOPOLOGY_RING : TOPOLOGY_CHAIN)));
    }
    return _pqmol->addBond(beg, end, bond.release());
}

int MoleculeJsonLoader::addAtomToMoleculeQuery(const char* label, int element, int charge, int valence, int radical, int isotope)
{
    std::unique_ptr<QueryMolecule::Atom> atom = std::make_unique<QueryMolecule::Atom>();
    if (element != -1 && element < ELEM_MAX)
        atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, element);
    else if (element == ELEM_ATOMLIST)
    {
        atom = std::make_unique<QueryMolecule::Atom>(); // ATOM_LIST
    }
    else
    {
        int atom_type = QueryMolecule::getAtomType(label);
        switch (atom_type)
        {
        case _ATOM_PSEUDO:
            atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, label);
            break;
        case _ATOM_A:
            atom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
            break;
        case _ATOM_AH:
            atom->type = QueryMolecule::OP_NONE;
            break;
        case _ATOM_QH:
            atom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
            break;
        case _ATOM_Q:
            atom.reset(QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C))));
            break;
        case _ATOM_XH:
            atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H));
            atom->type = QueryMolecule::OP_OR;
        case _ATOM_X:
            atom->type = QueryMolecule::OP_OR;
            atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
            atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
            atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
            atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
            atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
            break;
        case _ATOM_MH:
            atom->type = QueryMolecule::OP_AND;
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));
            break;
        case _ATOM_M:
            atom->type = QueryMolecule::OP_AND;
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));
            atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
            break;
        case _ATOM_R:
            atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RSITE, 0);
            break;
        }
    }

    if (charge != 0)
        atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, charge)));

    if (valence > 0)
    {
        if (valence == 15)
            valence = 0;
        atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_VALENCE, valence)));
    }

    if (isotope != 0)
        atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_ISOTOPE, isotope)));
    if (radical != 0)
        atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, radical)));

    return _pqmol->addAtom(atom.release());
}

void MoleculeJsonLoader::validateMoleculeBond(int order)
{
    if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC || order == _BOND_COORDINATION ||
        order == _BOND_HYDROGEN)
        return;
    else if (order == _BOND_SINGLE_OR_DOUBLE)
        throw Error("'single or double' bonds are allowed only for queries");
    else if (order == _BOND_SINGLE_OR_AROMATIC)
        throw Error("'single or aromatic' bonds are allowed only for queries");
    else if (order == _BOND_DOUBLE_OR_AROMATIC)
        throw Error("'double or aromatic' bonds are allowed only for queries");
    else if (order == _BOND_ANY)
        throw Error("'any' bonds are allowed only for queries");
    else
        throw Error("unknown bond type: %d", order);
}

void MoleculeJsonLoader::parseAtoms(const rapidjson::Value& atoms, BaseMolecule& mol, std::vector<EnhancedStereoCenter>& stereo_centers)
{
    mol.reaction_atom_mapping.clear_resize(atoms.Size());
    mol.reaction_atom_mapping.zerofill();
    mol.reaction_atom_inversion.clear_resize(atoms.Size());
    mol.reaction_atom_inversion.zerofill();
    mol.reaction_atom_exact_change.clear_resize(atoms.Size());
    mol.reaction_atom_exact_change.zerofill();

    std::vector<int> hcounts;
    hcounts.resize(atoms.Size(), 0);
    for (SizeType i = 0; i < atoms.Size(); i++)
    {
        std::string label;
        int atom_idx = 0, charge = 0, valence = 0, radical = 0, isotope = 0, elem = 0, rsite_idx = 0, mapping = 0, atom_type = 0;
        bool is_not_list = false;
        std::unique_ptr<QueryMolecule::Atom> atomlist;
        const Value& a = atoms[i];
        if (a.HasMember("isotope"))
            isotope = a["isotope"].GetInt();
        if (a.HasMember("attachmentPoints"))
        {
            int val = a["attachmentPoints"].GetInt();
            for (int att_idx = 0; (1 << att_idx) <= val; att_idx++)
                if (val & (1 << att_idx))
                    mol.addAttachmentPoint(att_idx + 1, i);
        }

        if (a.HasMember("type"))
        {
            std::string atom_type = a["type"].GetString();
            if (atom_type == "rg-label" && a.HasMember("$refs") && a["$refs"].Size())
            {
                std::string ref = a["$refs"][0].GetString();
                if (ref.find("rg-") == 0 && ref.erase(0, 3).size())
                {
                    rsite_idx = std::stoi(ref);
                    elem = ELEM_RSITE;
                    label = "R";
                }
                else
                    throw Error("invalid refs: %s", ref.c_str());
            }
            else if (atom_type == "atom-list")
            {
                if (!_pqmol)
                    throw Error("atom-list is allowed only for queries");
                is_not_list = a.HasMember("notList") ? a["notList"].GetBool() : false;
                const Value& elements = a["elements"];
                int pseudo_count = 0;
                elem = ELEM_ATOMLIST;
                for (int j = 0; j < elements.Size(); ++j)
                {
                    auto elem_label = elements[j].GetString();
                    int list_elem = Element::fromString2(elem_label);
                    std::unique_ptr<QueryMolecule::Atom> cur_atom;
                    if (list_elem != -1)
                    {
                        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, list_elem);
                    }
                    else
                    {
                        pseudo_count++;
                        if (pseudo_count > 1)
                            throw Error("%s inside atom list, if present, must be single", elem_label);
                        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, elem_label);
                    }

                    if (atomlist.get() == 0)
                        atomlist.reset(cur_atom.release());
                    else
                        atomlist.reset(QueryMolecule::Atom::oder(atomlist.release(), cur_atom.release()));
                }

                if (is_not_list)
                    atomlist.reset(QueryMolecule::Atom::nicht(atomlist.release()));
            }
            else
                throw Error("invalid atom type: %s", atom_type.c_str());
        }
        else
        {
            label = a["label"].GetString();
            if (label == "D")
            {
                elem = ELEM_H;
                isotope = 2;
            }
            else if (label == "T")
            {
                elem = ELEM_H;
                isotope = 3;
            }
            else
            {
                elem = Element::fromString2(label.c_str());
                if (elem == -1)
                {
                    elem = ELEM_PSEUDO;
                    if (isotope != 0)
                    {
                        throw Error("isotope number not allowed on pseudo-atoms");
                    }
                }
            }
        }

        if (a.HasMember("charge"))
            charge = a["charge"].GetInt();
        if (a.HasMember("explicitValence"))
            valence = a["explicitValence"].GetInt();
        if (a.HasMember("radical"))
            radical = a["radical"].GetInt();

        if (_pmol)
        {
            atom_idx = _pmol->addAtom(elem);
            _pmol->setAtomCharge_Silent(atom_idx, charge);
            _pmol->setAtomRadical(atom_idx, radical);
            _pmol->setAtomIsotope(atom_idx, isotope);
            if (valence > 0 && valence <= 14)
                _pmol->setExplicitValence(atom_idx, valence);
            if (valence == 15)
                _pmol->setExplicitValence(atom_idx, 0);
            if (elem == ELEM_PSEUDO)
            {
                _pmol->setPseudoAtom(atom_idx, label.c_str());
            }
        }
        else
        {
            atom_idx = addAtomToMoleculeQuery(label.c_str(), elem, charge, valence, radical, isotope);

            if (atomlist.get())
                _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), atomlist.release()));
        }

        if (a.HasMember("ringBondCount"))
        {
            if (!_pqmol && !ignore_noncritical_query_features)
                throw Error("ring bond count is allowed only for queries");
            int rbcount = a["ringBondCount"].GetInt();
            if (rbcount == -1) // no ring bonds
                _pqmol->resetAtom(atom_idx,
                                  QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 0)));
            else if (rbcount == -2) // as drawn
            {
                int k, rbonds = 0;
                const Vertex& vertex = _pqmol->getVertex(atom_idx);

                for (k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
                    if (_pqmol->getEdgeTopology(vertex.neiEdge(k)) == TOPOLOGY_RING)
                        rbonds++;

                _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx),
                                                                     new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, rbonds)));
            }
            else if (rbcount > 1)
                _pqmol->resetAtom(atom_idx,
                                  QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx),
                                                           new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, rbcount, (rbcount < 4 ? rbcount : 100))));
            else
                throw Error("ring bond count = %d makes no sense", rbcount);
        }

        if (a.HasMember("substitutionCount"))
        {
            if (!_pqmol)
                throw Error("substitution counts are allowed only for queries");
            int sub_count = a["substitutionCount"].GetInt();

            if (sub_count == -1) // no substitution
                _pqmol->resetAtom(atom_idx,
                                  QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, 0)));
            else if (sub_count == -2)
            {
                _pqmol->resetAtom(atom_idx,
                                  QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN,
                                                                                                                  _pqmol->getVertex(atom_idx).degree())));
            }
            else if (sub_count > 0)
                _pqmol->resetAtom(atom_idx,
                                  QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, sub_count,
                                                                                                                  (sub_count < 6 ? sub_count : 100))));
            else
                throw Error("invalid SUB value: %d", sub_count);
        }

        if (a.HasMember("hCount"))
        {
            if (!_pqmol)
            {
                if (!ignore_noncritical_query_features)
                    throw Error("H count is allowed only for queries");
            }
            hcounts[atom_idx] = a["hCount"].GetInt();
        }

        if (a.HasMember("invRet"))
        {
            mol.reaction_atom_inversion[atom_idx] = a["invRet"].GetInt();
        }

        if (a.HasMember("unsaturatedAtom"))
        {
            if (!_pqmol)
            {
                if (!ignore_noncritical_query_features)
                    throw Error("unsaturation flag is allowed only for queries");
            }
            if (a["unsaturatedAtom"].GetBool())
                _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_UNSATURATION, 0)));
        }

        if (a.HasMember("exactChangeFlag"))
        {
            mol.reaction_atom_exact_change[atom_idx] = a["exactChangeFlag"].GetBool();
        }

        if (a.HasMember("mapping"))
        {
            mol.reaction_atom_mapping[atom_idx] = a["mapping"].GetInt();
        }

        if (rsite_idx)
            mol.allowRGroupOnRSite(atom_idx, rsite_idx);

        if (a.HasMember("stereoLabel"))
        {
            std::string sl = a["stereoLabel"].GetString();
            if (sl.find("abs") != std::string::npos)
            {
                stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_ABS, 1);
            }
            else if (sl.find("or") != std::string::npos)
            {
                int grp = std::stoi(sl.substr(2));
                if (grp)
                    stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_OR, grp);
            }
            else if (sl.find("&") != std::string::npos)
            {
                int grp = std::stoi(sl.substr(1));
                if (grp)
                    stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_AND, grp);
            }
        }

        if (a.HasMember("location"))
        {
            const Value& coords = a["location"];
            if (coords.Size() > 0)
            {
                Vec3f a_pos;
                a_pos.x = coords[0].GetFloat();
                a_pos.y = coords[1].GetFloat();
                a_pos.z = coords[2].GetFloat();
                mol.setAtomXyz(atom_idx, a_pos);
            }
        }

        if (a.HasMember("alias"))
        {
            Array<char> alias;
            alias.readString(a["alias"].GetString(), true);
            int idx = mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT);
            DataSGroup& sgroup = (DataSGroup&)mol.sgroups.getSGroup(idx);
            sgroup.atoms.push(atom_idx);
            sgroup.name.readString("INDIGO_ALIAS", true);
            sgroup.data.copy(alias);
            sgroup.display_pos.x = mol.getAtomXyz(atom_idx).x;
            sgroup.display_pos.y = mol.getAtomXyz(atom_idx).y;
        }
    }

    if (_pqmol)
        for (int k = 0; k < hcounts.size(); k++)
        {
            int expl_h = 0;

            if (hcounts[k] >= 0)
            {
                // count explicit hydrogens
                const Vertex& vertex = mol.getVertex(k);
                int i;

                for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
                {
                    if (mol.getAtomNumber(vertex.neiVertex(i)) == ELEM_H)
                        expl_h++;
                }
            }

            if (hcounts[k] == 1)
            {
                // no hydrogens unless explicitly drawn
                _pqmol->resetAtom(k, QueryMolecule::Atom::und(_pqmol->releaseAtom(k), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, expl_h)));
            }
            else if (hcounts[k] > 1)
            {
                // no hydrogens unless explicitly drawn
                _pqmol->resetAtom(
                    k, QueryMolecule::Atom::und(_pqmol->releaseAtom(k), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, expl_h + hcounts[k] - 1, 100)));
            }
        }
}

void MoleculeJsonLoader::parseBonds(const rapidjson::Value& bonds, BaseMolecule& mol)
{
    mol.reaction_bond_reacting_center.clear_resize(bonds.Size());
    mol.reaction_bond_reacting_center.zerofill();

    for (SizeType i = 0; i < bonds.Size(); i++)
    {
        const Value& b = bonds[i];
        const Value& refs = b["atoms"];

        int stereo = 0;
        if (b.HasMember("stereo"))
        {
            stereo = b["stereo"].GetInt();
        }

        int topology = 0;
        if (b.HasMember("topology"))
        {
            topology = b["topology"].GetInt();
            if (topology != 0 && _pmol)
                if (!ignore_noncritical_query_features)
                    throw Error("bond topology is allowed only for queries");
        }

        int rcenter = 0;
        if (b.HasMember("center"))
        {
            rcenter = b["center"].GetInt();
        }

        int order = b["type"].GetInt();
        if (_pmol)
            validateMoleculeBond(order);
        if (refs.Size() > 1)
        {
            int a1 = refs[0].GetInt();
            int a2 = refs[1].GetInt();
            int bond_idx = 0;
            bond_idx = _pmol ? _pmol->addBond_Silent(a1, a2, order) : addBondToMoleculeQuery(a1, a2, order, topology);
            if (stereo)
            {
                switch (stereo)
                {
                case 1:
                    mol.setBondDirection(bond_idx, BOND_UP);
                    break;
                case 3:
                    mol.cis_trans.ignore(bond_idx);
                    break;
                case 4:
                    mol.setBondDirection(bond_idx, BOND_EITHER);
                    break;
                case 6:
                    mol.setBondDirection(bond_idx, BOND_DOWN);
                    break;
                    break;

                default:
                    break;
                }
            }
            if (rcenter)
            {
                mol.reaction_bond_reacting_center[i] = rcenter;
            }
        }
        else
        {
            // TODO:
        }
    }
}

void indigo::MoleculeJsonLoader::parseHighlight(const rapidjson::Value& highlight, BaseMolecule& mol)
{
    for (int i = 0; i < highlight.Size(); ++i)
    {
        const rapidjson::Value& val = highlight[i];
        if (val.HasMember("entityType") && val.HasMember("items"))
        {
            const rapidjson::Value& items = val["items"];
            std::string et = val["entityType"].GetString();
            if (et == "atoms")
            {
                for (int j = 0; i < items.Size(); ++i)
                    mol.highlightAtom(items[i].GetInt());
            }
            else if (et == "bonds")
            {
                for (int j = 0; i < items.Size(); ++i)
                    mol.highlightBond(items[i].GetInt());
            }
        }
    }
}

void indigo::MoleculeJsonLoader::parseSelection(const rapidjson::Value& selection, BaseMolecule& mol)
{
    for (int i = 0; i < selection.Size(); ++i)
    {
        const rapidjson::Value& val = selection[i];
        if (val.HasMember("entityType") && val.HasMember("items"))
        {
            const rapidjson::Value& items = val["items"];
            std::string et = val["entityType"].GetString();
            if (et == "atoms")
            {
                for (int j = 0; i < items.Size(); ++i)
                    mol.selectAtom(items[i].GetInt());
            }
            else if (et == "bonds")
            {
                for (int j = 0; i < items.Size(); ++i)
                    mol.selectBond(items[i].GetInt());
            }
        }
    }
}

void MoleculeJsonLoader::handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol)
{
    int start = -1;
    int end = -1;
    int end_bond = -1, start_bond = -1;
    QS_DEF(Array<int>, xbonds);

    for (auto j : bmol.edges())
    {
        if (!bmol.hasEdge(j))
            continue;

        const Edge& edge = bmol.getEdge(j);
        auto itbeg = atoms.find(edge.beg);
        auto itend = atoms.find(edge.end);

        if (itbeg == atoms.end() && itend == atoms.end())
            continue;
        if (itbeg != atoms.end() && itend != atoms.end())
            continue;
        else
        {
            // bond going out of the sgroup
            xbonds.push(j);
            if (start_bond == -1)
            {
                start_bond = j;
                start = itbeg != atoms.end() ? *itbeg : *itend;
            }
            else if (end_bond == -1)
            {
                end_bond = j;
                end = itbeg != atoms.end() ? *itbeg : *itend;
            }
        }
    }

    if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
    {
        QS_DEF(Array<int>, mapping);
        std::unique_ptr<BaseMolecule> rep(bmol.neu());
        rep->makeSubmolecule(bmol, sgroup.atoms, &mapping, 0);

        rep->sgroups.clear(SGroup::SG_TYPE_SRU);
        rep->sgroups.clear(SGroup::SG_TYPE_MUL);

        int rep_start = mapping[start];
        int rep_end = mapping[end];
        MultipleGroup& mg = (MultipleGroup&)sgroup;
        if (mg.multiplier > 1)
        {
            int start_order = start_bond > 0 ? bmol.getBondOrder(start_bond) : -1;
            int end_order = end_bond > 0 ? bmol.getBondOrder(end_bond) : -1;
            for (int j = 0; j < mg.multiplier - 1; j++)
            {
                bmol.mergeWithMolecule(*rep, &mapping, 0);
                int k;
                for (k = rep->vertexBegin(); k != rep->vertexEnd(); k = rep->vertexNext(k))
                    sgroup.atoms.push(mapping[k]);
                if (rep_end >= 0 && end_bond >= 0)
                {
                    int external = bmol.getEdge(end_bond).findOtherEnd(end);
                    bmol.removeBond(end_bond);
                    if (_pmol != 0)
                    {
                        _pmol->addBond(end, mapping[rep_start], start_order);
                        end_bond = _pmol->addBond(mapping[rep_end], external, end_order);
                    }
                    else
                    {
                        _pqmol->addBond(end, mapping[rep_start], new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, start_order));
                        end_bond = _pqmol->addBond(mapping[rep_end], external, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, end_order));
                    }
                    end = mapping[rep_end];
                }
            }
        }
    }
}

void MoleculeJsonLoader::setStereoFlagPosition(const rapidjson::Value& pos, int fragment_index, BaseMolecule& mol)
{
    Vec3f s_pos;
    if (pos.HasMember("x"))
        s_pos.x = pos["x"].GetFloat();

    if (pos.HasMember("y"))
        s_pos.y = pos["y"].GetFloat();

    if (pos.HasMember("z"))
        s_pos.z = pos["z"].GetFloat();

    mol.setStereoFlagPosition(fragment_index, s_pos);
}

void MoleculeJsonLoader::parseSGroups(const rapidjson::Value& sgroups, BaseMolecule& mol)
{
    for (SizeType i = 0; i < sgroups.Size(); i++)
    {
        const Value& s = sgroups[i];
        std::string sg_type_str = s["type"].GetString(); // GEN, MUL, SRU, SUP
        int sg_type = SGroup::getType(sg_type_str.c_str());
        int grp_idx = mol.sgroups.addSGroup(sg_type);
        SGroup& sgroup = mol.sgroups.getSGroup(grp_idx);
        const Value& atoms = s["atoms"];
        // add atoms
        std::unordered_set<int> sgroup_atoms;
        for (int j = 0; j < atoms.Size(); ++j)
        {
            int atom_idx = atoms[j].GetInt();
            sgroup.atoms.push(atom_idx);
            sgroup_atoms.insert(atom_idx);
            if (sg_type == SGroup::SG_TYPE_MUL)
            {
                MultipleGroup& mg = (MultipleGroup&)sgroup;
                if (mg.multiplier)
                    mg.parent_atoms.push(atom_idx);
            }
        }

        // add brackets
        Vec2f* p = sgroup.brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);
        p = sgroup.brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);

        // add specific parameters
        switch (sg_type)
        {
        case SGroup::SG_TYPE_GEN:
            // no special parameters
            break;
        case SGroup::SG_TYPE_MUL: {
            MultipleGroup& mg = (MultipleGroup&)sgroup;
            if (s.HasMember("mul"))
            {
                int mult = s["mul"].GetInt();
                mg.multiplier = mult;
            }
        }
        break;
        case SGroup::SG_TYPE_SRU: {
            RepeatingUnit& ru = (RepeatingUnit&)sgroup;
            if (s.HasMember("subscript"))
            {
                ru.subscript.readString(s["subscript"].GetString(), true);
            }

            if (s.HasMember("connectivity"))
            {
                std::string conn = s["connectivity"].GetString();
                if (conn == "HT")
                    ru.connectivity = RepeatingUnit::HEAD_TO_TAIL;
                else if (conn == "HH")
                    ru.connectivity = RepeatingUnit::HEAD_TO_HEAD;
                else if (conn == "EU")
                    ru.connectivity = RepeatingUnit::EITHER;
            }
        }
        break;
        case SGroup::SG_TYPE_SUP: {
            Superatom& sg = (Superatom&)sgroup;
            if (s.HasMember("name"))
                sg.subscript.readString(s["name"].GetString(), true);
            if (s.HasMember("expanded"))
                sg.contracted = s["expanded"].GetBool() ? 0 : 1;
        }
        break;
        case SGroup::SG_TYPE_DAT: {
            DataSGroup& dsg = (DataSGroup&)sgroup;
            if (s.HasMember("fieldName"))
                dsg.name.readString(s["fieldName"].GetString(), true);

            if (s.HasMember("fieldData"))
                dsg.data.readString(s["fieldData"].GetString(), true);

            if (s.HasMember("fieldType"))
                dsg.description.readString(s["fieldType"].GetString(), true);

            if (s.HasMember("queryType"))
                dsg.querycode.readString(s["queryType"].GetString(), true);

            if (s.HasMember("queryOp"))
                dsg.queryoper.readString(s["queryOp"].GetString(), true);

            if (s.HasMember("x"))
                dsg.display_pos.x = s["x"].GetFloat();

            if (s.HasMember("y"))
                dsg.display_pos.y = s["y"].GetFloat();

            if (s.HasMember("dataDetached"))
                dsg.detached = s["dataDetached"].GetBool();
            else
                dsg.detached = true;
            // TODO: placement = relative, display = display_units
            if (s.HasMember("placement"))
                dsg.relative = s["placement"].GetBool();

            if (s.HasMember("display"))
                dsg.display_units = s["display"].GetBool();

            if (s.HasMember("tag"))
            {
                auto tag = s["tag"].GetString();
                if (strlen(tag))
                    dsg.tag = *tag;
            }

            if (s.HasMember("displayedChars"))
                dsg.num_chars = s["displayedChars"].GetInt();
        }
        break;
        default:
            throw Error("Invalid sgroup type %s", sg_type_str.c_str());
        }

        if (sg_type != SGroup::SG_TYPE_DAT)
            handleSGroup(sgroup, sgroup_atoms, mol);

        if (s.HasMember("bonds"))
        {
            const Value& bonds = s["bonds"];
            for (int j = 0; j < bonds.Size(); ++j)
            {
                sgroup.bonds.push(bonds[j].GetInt());
            }
        }
    }
}

void MoleculeJsonLoader::loadMolecule(BaseMolecule& mol)
{
    for (int node_idx = 0; node_idx < _mol_nodes.Size(); ++node_idx)
    {
        std::vector<EnhancedStereoCenter> stereo_centers;
        std::unique_ptr<BaseMolecule> pmol(mol.neu());
        _pmol = NULL;
        _pqmol = NULL;
        if (pmol->isQueryMolecule())
        {
            _pqmol = &pmol->asQueryMolecule();
        }
        else
        {
            _pmol = &pmol->asMolecule();
        }

        auto& mol_node = _mol_nodes[node_idx];
        std::string type = mol_node["type"].GetString();
        if (type.compare("molecule") == 0 || type.compare("rgroup") == 0)
        {
            /* int chiral = 0;
            if (mol_node.HasMember("chiral"))
                chiral = mol_node["chiral"].GetInt();
            pmol->setChiralFlag(chiral); */
            // parse atoms
            auto& atoms = mol_node["atoms"];
            parseAtoms(atoms, *pmol, stereo_centers);
            // parse bonds
            if (mol_node.HasMember("bonds"))
            {
                parseBonds(mol_node["bonds"], *pmol);
            }
            pmol->unhighlightAll();
            if (mol_node.HasMember("highlight"))
            {
                parseHighlight(mol_node["highlight"], *pmol);
            }

            if (mol_node.HasMember("selection"))
            {
                parseSelection(mol_node["selection"], *pmol);
            }

            // parse SGroups
            if (mol_node.HasMember("sgroups"))
            {
                parseSGroups(mol_node["sgroups"], *pmol);
            }

            if (mol_node.HasMember("stereoFlagPosition"))
            {
                setStereoFlagPosition(mol_node["stereoFlagPosition"], node_idx, mol);
            }
        }
        else
        {
            throw Error("unknown type: %s", type.c_str());
        }

        Array<int> mapping;
        mol.mergeWithMolecule(*pmol, &mapping, COPY_BOND_DIRECTIONS);

        for (auto& sc : stereo_centers)
        {
            sc._atom_idx = mapping[sc._atom_idx];
            _stereo_centers.push_back(sc);
        }
    }

    MoleculeRGroups& rgroups = mol.rgroups;
    if (_rgroups.Size())
    {
        Document data;
        for (int rsite_idx = 0; rsite_idx < _rgroups.Size(); ++rsite_idx)
        {
            RGroup& rgroup = rgroups.getRGroup(rsite_idx + 1);
            std::unique_ptr<BaseMolecule> fragment(mol.neu());
            Value one_rnode(kArrayType);
            Value& rnode = _rgroups[rsite_idx];
            one_rnode.PushBack(rnode, data.GetAllocator());
            auto empty_val = Value(kArrayType);
            MoleculeJsonLoader loader(one_rnode, empty_val);
            loader.loadMolecule(*fragment.get());
            rgroup.fragments.add(fragment.release());
        }
    }

    std::vector<int> ignore_cistrans(mol.edgeCount());
    std::vector<int> sensible_bond_directions(mol.edgeCount());
    for (int i = 0; i < mol.edgeCount(); i++)
        if (mol.getBondDirection(i) == BOND_EITHER)
        {
            if (MoleculeCisTrans::isGeomStereoBond(mol, i, 0, true))
            {
                ignore_cistrans[i] = 1;
                sensible_bond_directions[i] = 1;
            }
            else
            {
                int k;
                const Vertex& v = mol.getVertex(mol.getEdge(i).beg);

                for (k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                {
                    if (MoleculeCisTrans::isGeomStereoBond(mol, v.neiEdge(k), 0, true))
                    {
                        ignore_cistrans[v.neiEdge(k)] = 1;
                        sensible_bond_directions[i] = 1;
                        break;
                    }
                }
            }
        }

    mol.buildFromBondsStereocenters(stereochemistry_options, sensible_bond_directions.data());
    mol.buildFromBondsAlleneStereo(stereochemistry_options.ignore_errors, sensible_bond_directions.data());

    // int num_atoms = mol.vertices();
    // printf("%d", num_atoms);
    if (!mol.getChiralFlag())
        for (int i : mol.vertices())
        {
            int type = mol.stereocenters.getType(i);
            if (type == MoleculeStereocenters::ATOM_ABS)
                mol.stereocenters.setType(i, MoleculeStereocenters::ATOM_AND, 1);
        }

    mol.buildCisTrans(ignore_cistrans.data());
    if (mol.stereocenters.size() == 0)
    {
        mol.buildFrom3dCoordinatesStereocenters(stereochemistry_options);
    }

    for (const auto& sc : _stereo_centers)
    {
        // const int undefined_pyramid[] = {-1, -1, -1, -1};
        if (mol.stereocenters.getType(sc._atom_idx) == 0)
        {
            if (!stereochemistry_options.ignore_errors)
                throw Error("stereo type specified for atom #%d, but the bond "
                            "directions does not say that it is a stereocenter",
                            sc._atom_idx);
            mol.addStereocentersIgnoreBad(sc._atom_idx, sc._type, sc._group, false); // add non-valid stereocenters
        }
        else
            mol.stereocenters.setType(sc._atom_idx, sc._type, sc._group);
    }

    MoleculeLayout ml(mol, false);
    ml.layout_orientation = UNCPECIFIED;
    ml.updateSGroups();
    if (_simple_objects.IsArray())
    {
        for (int obj_idx = 0; obj_idx < _simple_objects.Size(); ++obj_idx)
        {
            auto& simple_object = _simple_objects[obj_idx];
            if (simple_object.HasMember("mode")) // ellipse or rectangle or line
            {
                int mode = 0;
                Vec2f p1, p2;
                std::string obj_mode = simple_object["mode"].GetString();
                if (obj_mode.compare("ellipse") == 0)
                {
                    mode = KETSimpleObject::EKETEllipse;
                }
                else if (obj_mode.compare("rectangle") == 0)
                {
                    mode = KETSimpleObject::EKETRectangle;
                }
                else if (obj_mode.compare("line") == 0)
                {
                    mode = KETSimpleObject::EKETLine;
                }
                else
                    throw Error("Unknown simple object mode:%s", obj_mode.c_str());
                if (simple_object.HasMember("pos"))
                {
                    auto pos = simple_object["pos"].GetArray();
                    if (pos.Size() == 2)
                    {
                        p1.x = pos[0]["x"].GetFloat();
                        p1.y = pos[0]["y"].GetFloat();
                        p2.x = pos[1]["x"].GetFloat();
                        p2.y = pos[1]["y"].GetFloat();
                    }
                    else
                        throw("Bad pos array size %d. Most be equal to 2.", pos.Size());
                }
                mol.addMetaObject(new KETSimpleObject(mode, Rect2f(p1, p2)));
            }
            else if (simple_object.HasMember("content") && simple_object.HasMember("position"))
            {
                std::string content = simple_object["content"].GetString();
                Vec3f text_origin;
                text_origin.x = simple_object["position"]["x"].GetFloat();
                text_origin.y = simple_object["position"]["y"].GetFloat();
                text_origin.z = simple_object["position"]["z"].GetFloat();
                mol.addMetaObject(new KETTextObject(text_origin, content));
            }
        }
    }
}
