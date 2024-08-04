#include <string>
#include <unordered_map>
#include <vector>

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "layout/molecule_layout.h"
#include "molecule/elements.h"
#include "molecule/ket_commons.h"
#include "molecule/ket_document.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_sgroups.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_lib.h"
#include "molecule/monomers_template_library.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

using namespace rapidjson;
using namespace indigo;
using namespace std;

IMPL_ERROR(MoleculeJsonLoader, "molecule json loader");

MoleculeJsonLoader::MoleculeJsonLoader(Document& ket)
    : _mol_array(kArrayType), _mol_nodes(_mol_array), _meta_objects(kArrayType), _templates(kArrayType), _monomer_array(kArrayType),
      _connection_array(kArrayType), _pmol(0), _pqmol(0), ignore_noncritical_query_features(false), components_count(0), _document()
{
    parse_ket(ket);
}

void MoleculeJsonLoader::parse_ket(Document& ket)
{
    Value& root = ket["root"];
    if (root.HasMember("nodes"))
    {
        Value& nodes = root["nodes"];

        // rewind to first molecule node
        for (rapidjson::SizeType i = 0; i < nodes.Size(); ++i)
        {
            if (nodes[i].HasMember("$ref"))
            {
                std::string node_name = nodes[i]["$ref"].GetString();
                Value& node = ket[node_name.c_str()];
                std::string node_type = node["type"].GetString();
                if (node_type.compare("molecule") == 0)
                {
                    _mol_nodes.PushBack(node, ket.GetAllocator());
                }
                else if (node_type.compare("rgroup") == 0 && node_name.size() > 2)
                {
                    std::string rg = "rg";
                    int rg_num = std::atoi(node_name.substr(rg.size()).c_str());
                    _rgroups.emplace_back(rg_num, node);
                }
                else if (node_type.compare("monomer") == 0)
                {
                    _monomer_array.PushBack(node, ket.GetAllocator());
                }
                else
                    throw Error("Unknows node type: %s", node_type.c_str());
            }
            else if (nodes[i].HasMember("type"))
            {
                _meta_objects.PushBack(nodes[i], ket.GetAllocator());
            }
            else
                throw Error("Unsupported node for molecule");
        }
    }

    if (root.HasMember("templates"))
    {
        Value& templates = root["templates"];
        for (rapidjson::SizeType i = 0; i < templates.Size(); ++i)
        {
            std::string template_ref = templates[i]["$ref"].GetString();
            if (ket.HasMember(template_ref.c_str()))
            {
                Value& template_node = ket[template_ref.c_str()];
                std::string id = template_node["id"].GetString();
                _templates.PushBack(template_node, ket.GetAllocator());
                _id_to_template.emplace(id, i);
                _template_ref_to_id.emplace(template_ref, id);
            }
        }
    }

    if (root.HasMember("connections"))
    {
        Value& connections = root["connections"];
        for (rapidjson::SizeType i = 0; i < connections.Size(); ++i)
            _connection_array.PushBack(connections[i], ket.GetAllocator());
    }
}

MoleculeJsonLoader::MoleculeJsonLoader(Scanner& scanner)
    : _mol_array(kArrayType), _mol_nodes(_mol_array), _meta_objects(kArrayType), _templates(kArrayType), _monomer_array(kArrayType),
      _connection_array(kArrayType), _pmol(0), _pqmol(0), ignore_noncritical_query_features(false), components_count(0), _document()
{
    if (scanner.lookNext() == '{')
    {
        Array<char> buf;
        scanner.readAll(buf);
        buf.push(0);
        char* ptr = buf.ptr();
        if (!_document.Parse(ptr).HasParseError())
        {
            if (_document.HasMember("root"))
            {
                parse_ket(_document);
            }
        }
    }
}

MoleculeJsonLoader::MoleculeJsonLoader(Value& mol_nodes)
    : _mol_nodes(mol_nodes), _meta_objects(kArrayType), _templates(kArrayType), _monomer_array(kArrayType), _connection_array(kArrayType), _pmol(0), _pqmol(0),
      ignore_noncritical_query_features(false), ignore_no_chiral_flag(false), skip_3d_chirality(false), treat_x_as_pseudoatom(false), treat_stereo_as(0),
      components_count(0), _document()
{
}

int MoleculeJsonLoader::addBondToMoleculeQuery(int beg, int end, int order, int topology, int direction)
{
    return _pqmol->addBond(beg, end, QueryMolecule::createQueryMoleculeBond(order, topology, direction));
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
        atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_BOND_ORDER, valence)));

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
        int atom_idx = 0, charge = 0, valence = 0, radical = 0, isotope = 0, elem = 0, rsite_idx = 0;
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
                    if (a.HasMember("attachmentOrder"))
                    {
                        const auto& rattachments = a["attachmentOrder"];
                        for (SizeType j = 0; j < rattachments.Size(); ++j)
                        {
                            const auto& rattachment = rattachments[j];
                            if (rattachment.HasMember("attachmentAtom"))
                            {
                                auto attindex = rattachment["attachmentAtom"].GetInt();
                                int attid = -1;
                                if (rattachment.HasMember("attachmentId"))
                                {
                                    attid = rattachment["attachmentId"].GetInt();
                                }
                                mol.setRSiteAttachmentOrder(i, attindex, attid);
                            }
                        }
                    }
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
                for (rapidjson::SizeType j = 0; j < elements.Size(); ++j)
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
                isotope = DEUTERIUM;
            }
            else if (label == "T")
            {
                elem = ELEM_H;
                isotope = TRITIUM;
            }
            else
            {
                elem = Element::fromString2(label.c_str());
                if (elem == -1)
                {
                    if (!_pqmol && QueryMolecule::getAtomType(label.c_str()) != _ATOM_PSEUDO)
                        throw Error("'%s' label is allowed only for queries", label.c_str());
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

        if (a.HasMember("selected") && a["selected"].GetBool())
        {
            if (_pmol)
                _pmol->selectAtom(atom_idx);
            else
                _pqmol->selectAtom(atom_idx);
        }

        if (a.HasMember("ringBondCount"))
        {
            if (_pqmol)
            {
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
                    _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx),
                                                                         new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, rbcount, rbcount)));
                else
                    throw Error("ring bond count = %d makes no sense", rbcount);
            }
            else if (!ignore_noncritical_query_features)
                throw Error("ring bond count is allowed only for queries");
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

        if (a.HasMember("implicitHCount"))
        {
            if (_pmol)
                _pmol->setImplicitH(atom_idx, a["implicitHCount"].GetInt());
            else
            {
                int count = a["implicitHCount"].GetInt();
                if (count < 0)
                    throw Error("Wrong value for implicitHCount: %d", count);
                _pqmol->resetAtom(atom_idx,
                                  QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_IMPLICIT_H, count)));
            }
        }

        if (a.HasMember("invRet"))
        {
            mol.reaction_atom_inversion[atom_idx] = a["invRet"].GetInt();
        }

        if (a.HasMember("unsaturatedAtom"))
        {
            if (_pqmol)
            {
                if (a["unsaturatedAtom"].GetBool())
                    _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_UNSATURATION, 0)));
            }
            else if (!ignore_noncritical_query_features)
                throw Error("unsaturation flag is allowed only for queries");
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
            mol.setAlias(atom_idx, a["alias"].GetString());
        }

        if (a.HasMember("cip"))
        {
            std::string cip_str = a["cip"].GetString();
            auto cip = stringToCIP(cip_str);
            if (cip != CIPDesc::NONE)
                mol.setAtomCIP(atom_idx, cip);
        }

        if (a.HasMember("queryProperties"))
        {
            if (_pqmol)
            {
                auto qProps = a["queryProperties"].GetObject();
                if (qProps.HasMember("customQuery"))
                {
                    std::string customQuery = qProps["customQuery"].GetString();
                    std::unique_ptr<QueryMolecule::Atom> atom = make_unique<QueryMolecule::Atom>();
                    SmilesLoader::readSmartsAtomStr(customQuery, atom);
                    _pqmol->resetAtom(atom_idx, atom.release());
                }
                else
                {
                    if (qProps.HasMember("aromaticity"))
                    {
                        std::string arom = qProps["aromaticity"].GetString();
                        int aromatic;
                        if (arom == ATOM_AROMATIC_STR)
                            aromatic = ATOM_AROMATIC;
                        else if (arom == ATOM_ALIPHATIC_STR)
                            aromatic = ATOM_ALIPHATIC;
                        else
                            throw Error("Wrong value for aromaticity.");
                        _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx),
                                                                             new QueryMolecule::Atom(QueryMolecule::ATOM_AROMATICITY, aromatic)));
                    }
                    if (qProps.HasMember("ringMembership"))
                    {
                        int rmem = qProps["ringMembership"].GetInt();
                        _pqmol->resetAtom(
                            atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_SSSR_RINGS, rmem)));
                    }
                    if (qProps.HasMember("ringSize"))
                    {
                        int rsize = qProps["ringSize"].GetInt();
                        _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx),
                                                                             new QueryMolecule::Atom(QueryMolecule::ATOM_SMALLEST_RING_SIZE, rsize)));
                    }
                    if (qProps.HasMember("connectivity"))
                    {
                        int conn = qProps["connectivity"].GetInt();
                        _pqmol->resetAtom(
                            atom_idx, QueryMolecule::Atom::und(_pqmol->releaseAtom(atom_idx), new QueryMolecule::Atom(QueryMolecule::ATOM_CONNECTIVITY, conn)));
                    }
                    if (qProps.HasMember("chirality"))
                    {
                        std::string chirality_value = qProps["chirality"].GetString();
                        int chirality;
                        if (chirality_value == "clockwise")
                            chirality = QueryMolecule::CHIRALITY_CLOCKWISE;
                        else if (chirality_value == "anticlockwise")
                            chirality = QueryMolecule::CHIRALITY_ANTICLOCKWISE;
                        else
                            throw Error("Wrong value for chirality.");
                        _pqmol->resetAtom(atom_idx, QueryMolecule::Atom::und(
                                                        _pqmol->releaseAtom(atom_idx),
                                                        new QueryMolecule::Atom(QueryMolecule::ATOM_CHIRALITY, QueryMolecule::CHIRALITY_GENERAL, chirality)));
                    }
                }
            }
            else
                throw Error("queryProperties is allowed only for queries");
        }
    }

    if (_pqmol)
        for (int k = 0; k < static_cast<int>(hcounts.size()); k++)
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
        int bond_idx = 0;
        if (b.HasMember("customQuery"))
        {
            if (!_pqmol)
                throw Error("customQuery is allowed only for queries");
            std::string customQuery = b["customQuery"].GetString();
            std::unique_ptr<QueryMolecule::Bond> bond = make_unique<QueryMolecule::Bond>();
            SmilesLoader::readSmartsBondStr(customQuery, bond);
            if (refs.Size() == 2)
            {
                int begin = refs[0].GetInt();
                int end = refs[1].GetInt();
                bond_idx = _pqmol->addBond(begin, end, bond.release());
            }
            else
            {
                throw Error("Wrong bond atoms count");
            }
        }
        else
        {
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
                int direction = BOND_ZERO;
                if (_pqmol && stereo && order == BOND_SINGLE)
                {
                    if (stereo == BIOVIA_STEREO_UP)
                        direction = BOND_UP;
                    else if (stereo == BIOVIA_STEREO_DOWN)
                        direction = BOND_DOWN;
                }
                bond_idx = _pmol ? _pmol->addBond_Silent(a1, a2, order) : addBondToMoleculeQuery(a1, a2, order, topology, direction);
                if (stereo)
                {
                    switch (stereo)
                    {
                    case BIOVIA_STEREO_UP:
                        mol.setBondDirection(bond_idx, BOND_UP);
                        break;
                    case BIOVIA_STEREO_DOUBLE_CISTRANS:
                        mol.cis_trans.ignore(bond_idx);
                        mol.setBondDirection(bond_idx, BOND_EITHER);
                        break;
                    case BIOVIA_STEREO_ETHER:
                        mol.setBondDirection(bond_idx, BOND_EITHER);
                        break;
                    case BIOVIA_STEREO_DOWN:
                        mol.setBondDirection(bond_idx, BOND_DOWN);
                        break;
                    default:
                        break;
                    }
                }

                if (b.HasMember("cip"))
                {
                    std::string cip_str = b["cip"].GetString();
                    auto cip = stringToCIP(cip_str);
                    if (cip != CIPDesc::NONE)
                        mol.setBondCIP(bond_idx, cip);
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
        if (b.HasMember("selected"))
        {
            if (b["selected"].GetBool())
            {
                if (_pmol)
                    _pmol->selectBond(bond_idx);
                else
                    _pqmol->selectBond(bond_idx);
            }
        }
    }
}

void indigo::MoleculeJsonLoader::parseHighlight(const rapidjson::Value& highlight, BaseMolecule& mol)
{
    for (rapidjson::SizeType i = 0; i < highlight.Size(); ++i)
    {
        const rapidjson::Value& val = highlight[i];
        if (val.HasMember("entityType") && val.HasMember("items"))
        {
            const rapidjson::Value& items = val["items"];
            std::string et = val["entityType"].GetString();
            if (et == "atoms")
            {
                for (rapidjson::SizeType j = 0; j < items.Size(); ++j)
                    mol.highlightAtom(items[j].GetInt());
            }
            else if (et == "bonds")
            {
                for (rapidjson::SizeType j = 0; j < items.Size(); ++j)
                    mol.highlightBond(items[j].GetInt());
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

        int rep_start{-1};
        if (start != -1)
            rep_start = mapping[start];

        int rep_end{-1};
        if (end != -1)
            rep_end = mapping[end];

        MultipleGroup& mg = (MultipleGroup&)sgroup;
        if (mg.multiplier > 1)
        {
            int start_order = start_bond >= 0 ? bmol.getBondOrder(start_bond) : -1;
            int end_order = end_bond >= 0 ? bmol.getBondOrder(end_bond) : -1;
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
        const Value& atoms = s["atoms"];
        std::string sg_type_str = s["type"].GetString(); // GEN, MUL, SRU, SUP
        if (sg_type_str == "queryComponent")
        {
            if (_pqmol)
            {
                _pqmol->components.expandFill(_pqmol->components.size() + atoms.Size(), 0);
                components_count++;
                for (rapidjson::SizeType j = 0; j < atoms.Size(); ++j)
                {
                    int atom_idx = atoms[j].GetInt();
                    _pqmol->components[atom_idx] = components_count;
                }
            }
            else
                throw Error("queryProperties is allowed only for queries");
            continue;
        }
        int sg_type = SGroup::getType(sg_type_str.c_str());
        int grp_idx = mol.sgroups.addSGroup(sg_type);
        SGroup& sgroup = mol.sgroups.getSGroup(grp_idx);
        // add atoms
        std::unordered_set<int> sgroup_atoms;
        for (rapidjson::SizeType j = 0; j < atoms.Size(); ++j)
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
                sg.contracted = s["expanded"].GetBool() ? DisplayOption::Expanded : DisplayOption::Contracted;
            if (s.HasMember("attachmentPoints"))
            {
                const Value& attachmentPoints = s["attachmentPoints"];
                assert(attachmentPoints.IsArray());
                int attachmentAtom{-1};
                int leavingAtom{-1};
                std::string attachmentId{""};

                for (SizeType j = 0; j < attachmentPoints.Size(); ++j)
                {
                    if (!attachmentPoints[j].HasMember("attachmentAtom"))
                        throw Error("Attachment atom in a superatom is mandatory. Check input ket-file");
                    attachmentAtom = attachmentPoints[j]["attachmentAtom"].GetInt();
                    if (attachmentPoints[j].HasMember("leavingAtom"))
                    {
                        leavingAtom = attachmentPoints[j]["leavingAtom"].GetInt();
                    }
                    if (attachmentPoints[j].HasMember("attachmentId"))
                    {
                        attachmentId = attachmentPoints[j]["attachmentId"].GetString();
                    }
                    int ap_idx = sg.attachment_points.add();
                    Superatom::_AttachmentPoint& ap = sg.attachment_points.at(ap_idx);
                    ap.aidx = attachmentAtom;
                    ap.lvidx = leavingAtom;
                    ap.apid.readString(attachmentId.c_str(), true);
                }
            }
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
            for (rapidjson::SizeType j = 0; j < bonds.Size(); ++j)
            {
                sgroup.bonds.push(bonds[j].GetInt());
            }
        }
    }
}

void MoleculeJsonLoader::parseProperties(const rapidjson::Value& props, BaseMolecule& mol)
{
    auto& properties = mol.properties().insert(0);
    for (SizeType i = 0; i < props.Size(); i++)
    {
        const Value& prop = props[i];
        if (prop.HasMember("key") && prop.HasMember("value"))
            properties.insert(prop["key"].GetString(), prop["value"].GetString());
    }
}

void MoleculeJsonLoader::fillXBondsAndBrackets(Superatom& sa, BaseMolecule& mol)
{
    std::unordered_set<int> atoms;
    std::vector<Vec2f> brackets;
    for (auto val : sa.atoms)
        atoms.insert(val);

    // fill crossing bonds

    for (auto src_atom : sa.atoms)
    {
        const auto& vx = mol.getVertex(src_atom);
        const auto& src_pos = mol.getAtomXyz(src_atom);
        for (int i = vx.neiBegin(); i != vx.neiEnd(); i = vx.neiNext(i))
        {
            auto target_atom = vx.neiVertex(i);
            if (atoms.find(target_atom) == atoms.end())
            {
                const auto& target_pos = mol.getAtomXyz(target_atom);
                sa.bonds.push(vx.neiEdge(i));
                brackets.emplace_back((target_pos.x - src_pos.x) / 2, (target_pos.y - src_pos.y) / 2);
            }
        }
    }

    // fill brackets

    for (size_t i = 0; i < brackets.size(); i += 2)
    {
        Vec2f* brk_pos = sa.brackets.push();
        brk_pos[0].copy(brackets[i]);
        if (i + 1 == brackets.size())
            brk_pos[1].set(0, 0);
        else
            brk_pos[1].copy(brackets[i + 1]);
    }
}

static void parseIdtAlias(const rapidjson::Value& parent, std::string& idt_alias_base, bool& idt_has_modifications, std::string& idt_five_prime_end,
                          std::string& idt_internal, std::string& idt_three_prime_end)
{
    auto& idt_alias_node = parent["idtAliases"];
    if (idt_alias_node.HasMember("base"))
        idt_alias_base = idt_alias_node["base"].GetString();
    if (idt_alias_node.HasMember("modifications"))
    {
        idt_has_modifications = true;
        auto& idt_modifications_node = idt_alias_node["modifications"];
        if (idt_modifications_node.HasMember("endpoint5"))
            idt_five_prime_end = idt_modifications_node["endpoint5"].GetString();
        if (idt_modifications_node.HasMember("internal"))
            idt_internal = idt_modifications_node["internal"].GetString();
        if (idt_modifications_node.HasMember("endpoint3"))
            idt_three_prime_end = idt_modifications_node["endpoint3"].GetString();
    }
}

static IdtAlias parseIdtAlias(const rapidjson::Value& parent)
{
    std::string template_class, idt_alias_base, idt_five_prime_end, idt_internal, idt_three_prime_end;
    bool idt_has_modifications = false;
    parseIdtAlias(parent, idt_alias_base, idt_has_modifications, idt_five_prime_end, idt_internal, idt_three_prime_end);
    if (idt_has_modifications)
        return IdtAlias(idt_alias_base, idt_five_prime_end, idt_internal, idt_three_prime_end);
    else
        return IdtAlias(idt_alias_base);
}

void MoleculeJsonLoader::addMonomerTemplate(const rapidjson::Value& mt_json, MonomerTemplateLibrary* library, KetDocument* document)
{
    if (!mt_json.HasMember("id"))
        throw Error("Monomer template without id");

    std::string id = mt_json["id"].GetString();

    if (!mt_json.HasMember("class"))
        throw Error("Monomer template without class");
    std::string monomer_class = mt_json["class"].GetString();

    bool unresolved = false;
    if (mt_json.HasMember("unresolved"))
        unresolved = mt_json["unresolved"].GetBool();

    IdtAlias idt_alias;
    if (mt_json.HasMember("idtAliases"))
    {
        idt_alias = parseIdtAlias(mt_json);
        if (idt_alias.getBase().size() == 0)
            throw Error("Monomer template %s contains IDT alias without base.", id.c_str());
    }
    else if (unresolved)
    {
        throw Error("Unresoved monomer '%s' without IDT alias.", id.c_str());
    }

    auto& mon_template = library->addMonomerTemplate(id, monomer_class, idt_alias, unresolved);
    mon_template.parseOptsFromKet(mt_json);

    // parse atoms
    mon_template.parseAtoms(mt_json["atoms"]);

    // parse bonds
    if (mt_json.HasMember("bonds"))
    {
        mon_template.parseBonds(mt_json["bonds"]);
    }

    if (mt_json.HasMember("attachmentPoints"))
    {
        auto& att_points = mt_json["attachmentPoints"];
        for (SizeType i = 0; i < att_points.Size(); i++)
        {
            auto& ap = att_points[i];
            std::string ap_type, label;
            int attachment_atom;
            if (ap.HasMember("label"))
                label = ap["label"].GetString();
            attachment_atom = ap["attachmentAtom"].GetInt();
            auto& att_point = mon_template.AddAttachmentPoint(label, attachment_atom);
            att_point.parseOptsFromKet(ap);
            if (ap.HasMember("leavingGroup"))
            {
                auto& lv = ap["leavingGroup"];
                if (lv.HasMember("atoms"))
                {
                    std::vector<int> leaving_group;
                    auto& atoms = lv["atoms"];
                    for (SizeType i = 0; i < atoms.Size(); i++)
                    {
                        leaving_group.emplace_back(atoms[i].GetInt());
                    }
                    att_point.setLeavingGroup(leaving_group);
                }
            }
        }
    }
}

void MoleculeJsonLoader::addToLibMonomerGroupTemplate(MonomerTemplateLibrary& library, const rapidjson::Value& monomer_group_template)
{
    if (monomer_group_template.HasMember("id"))
    {
        std::string id = monomer_group_template["id"].GetString();
        std::string name, template_class;

        if (monomer_group_template.HasMember("name"))
            name = monomer_group_template["name"].GetString();
        if (monomer_group_template.HasMember("class"))
            template_class = monomer_group_template["class"].GetString();

        std::string idt_alias_base, idt_five_prime_end, idt_internal, idt_three_prime_end;
        bool idt_has_modifications = false;
        if (monomer_group_template.HasMember("idtAliases"))
        {
            parseIdtAlias(monomer_group_template, idt_alias_base, idt_has_modifications, idt_five_prime_end, idt_internal, idt_three_prime_end);
            if (idt_alias_base.size() == 0)
                throw Error("Group monomer template %s(id=%s) contains IDT alias without base.", name.c_str(), id.c_str());
        }

        if (idt_has_modifications)
            library.addMonomerGroupTemplate(
                MonomerGroupTemplate(id, name, template_class, IdtAlias(idt_alias_base, idt_five_prime_end, idt_internal, idt_three_prime_end)));
        else
            library.addMonomerGroupTemplate(MonomerGroupTemplate(id, name, template_class, idt_alias_base));

        if (monomer_group_template.HasMember("templates"))
        {
            MonomerGroupTemplate& mgt = library.getMonomerGroupTemplateById(id);
            auto& templates = monomer_group_template["templates"];
            for (SizeType i = 0; i < templates.Size(); i++)
            {
                std::string template_ref = templates[i]["$ref"].GetString();
                mgt.addTemplate(library, _template_ref_to_id[template_ref]);
            }
        }
    }
}

void MoleculeJsonLoader::loadMonomerLibrary(MonomerTemplateLibrary& library)
{
    Molecule mol;
    // Add monomer teplates
    for (SizeType i = 0; i < _templates.Size(); i++)
    {
        auto& mt = _templates[i];
        if (mt.HasMember("type") && mt["type"].GetString() == std::string("monomerTemplate"))
        {
            addMonomerTemplate(mt, &library, nullptr);
        }
    }

    // Monomer group templates add after adding all monomers
    for (SizeType i = 0; i < _templates.Size(); i++)
    {
        auto& mt = _templates[i];
        // int tp = mt.GetType();
        // Parse library
        if (mt.HasMember("type") && mt["type"].GetString() == std::string("monomerGroupTemplate"))
            addToLibMonomerGroupTemplate(library, mt);
    }
}

std::string MoleculeJsonLoader::monomerTemplateClass(const rapidjson::Value& monomer_template)
{
    std::string result;
    if (monomer_template.HasMember("class"))
        result = monomer_template["class"].GetString();
    else if (monomer_template.HasMember("classHELM"))
    {
        result = monomer_template["classHELM"].GetString();
        if (result == kMonomerClassPEPTIDE)
            result = kMonomerClassAA;
    }
    else
        result = kMonomerClassCHEM;
    return result;
}

int MoleculeJsonLoader::parseMonomerTemplate(const rapidjson::Value& monomer_template, BaseMolecule& mol, StereocentersOptions stereochemistry_options)
{
    auto tg_idx = mol.tgroups.addTGroup();
    TGroup& tg = mol.tgroups.getTGroup(tg_idx);
    tg.tgroup_id = tg_idx + 1;
    Value one_tgroup(kArrayType);
    Document data;
    rapidjson::Value monomer_template_cp;
    monomer_template_cp.CopyFrom(monomer_template, data.GetAllocator());
    one_tgroup.PushBack(monomer_template_cp, data.GetAllocator());
    MoleculeJsonLoader loader(one_tgroup);
    loader.stereochemistry_options = stereochemistry_options;
    tg.fragment.reset(mol.neu());
    loader.loadMolecule(*tg.fragment);
    auto& monomer_mol = *tg.fragment;
    std::string mclass, alias, natreplace;
    std::unordered_set<int> leaving_atoms;
    if (monomer_template.HasMember("id"))
    {
        std::string id = monomer_template["id"].GetString();
        tg.tgroup_text_id.appendString(id.c_str(), true);

        mclass = monomerTemplateClass(monomer_template);

        if (mclass.size())
        {
            if (mclass == kMonomerClassAminoAcid)
                mclass = kMonomerClassAA;
            else if (mclass == kMonomerClassDAminoAcid)
                mclass = kMonomerClassdAA;
            else
                std::transform(mclass.begin(), mclass.end(), mclass.begin(), ::toupper);

            std::string analog_alias;
            if (monomer_template.HasMember("naturalAnalog"))
                analog_alias = monomerAliasByName(mclass, monomer_template["naturalAnalog"].GetString());
            else if (monomer_template.HasMember("naturalAnalogShort"))
                analog_alias = monomer_template["naturalAnalogShort"].GetString();

            if (analog_alias.size())
            {
                std::string natrep_class = mclass == kMonomerClassdAA ? kMonomerClassAA : mclass;
                natreplace = natrep_class + "/" + analog_alias;
                tg.tgroup_natreplace.appendString(natreplace.c_str(), true);
            }

            tg.tgroup_class.appendString(mclass.c_str(), true);
            if (monomer_template.HasMember("alias"))
            {
                alias = monomer_template["alias"].GetString();
                if (mclass == kMonomerClassdAA && alias.find(kPrefix_d) == 0)
                {
                    alias.erase(0, 1);
                    mclass = kMonomerClassAA;
                }
                tg.tgroup_alias.readString(alias.c_str(), true);
                tg.tgroup_name.readString(monomerNameByAlias(mclass, alias).c_str(), true);
            }

            if (monomer_template.HasMember("name"))
            {
                std::string tg_name = monomer_template["name"].GetString();
                if (mclass == kMonomerClassdAA && tg_name.find(kPrefix_d) == 0)
                {
                    tg_name.erase(0, 1);
                    mclass = kMonomerClassAA;
                }
                tg.tgroup_name.readString(tg_name.c_str(), true);
            }
            bool unresolved = false;
            if (monomer_template.HasMember("unresolved"))
                tg.unresolved = monomer_template["unresolved"].GetBool();
        }

        if (monomer_template.HasMember("fullName"))
        {
            std::string tg_full_name = monomer_template["fullName"].GetString();
            tg.tgroup_full_name.readString(tg_full_name.c_str(), true);
        }

        if (monomer_template.HasMember("idtAliases"))
        {

            tg.idt_alias.readString(parseIdtAlias(monomer_template).getBase().c_str(), true);
        }

        if (monomer_template.HasMember("attachmentPoints"))
        {
            auto& att_points = monomer_template["attachmentPoints"];
            std::vector<MonomerAttachmentPoint> attachment_descs;
            int att_index = 0;
            for (SizeType i = 0; i < att_points.Size(); i++)
            {
                auto& ap = att_points[i];
                std::string att_label(1, 'A' + static_cast<char>(att_index));
                MonomerAttachmentPoint att_desc = {-1, -1, att_label + (att_index > 0 ? (att_index > 1 ? 'x' : 'r') : 'l')};
                if (ap.HasMember("leavingGroup"))
                {
                    int grp_idx = monomer_mol.sgroups.addSGroup(SGroup::SG_TYPE_SUP);
                    auto& sa = (Superatom&)monomer_mol.sgroups.getSGroup(grp_idx);
                    auto& atoms = ap["leavingGroup"]["atoms"];
                    std::string group_name;
                    for (SizeType j = 0; j < atoms.Size(); j++)
                    {
                        auto la = atoms[j].GetInt();
                        sa.atoms.push(la);
                        leaving_atoms.insert(la);
                        int total_h = 0;
                        if (!monomer_mol.isRSite(la) && !monomer_mol.isPseudoAtom(la) && !monomer_mol.isTemplateAtom(la))
                            total_h = monomer_mol.getAtomTotalH(la);
                        Array<char> label;
                        monomer_mol.getAtomSymbol(la, label);
                        if (label.size())
                        {
                            group_name += label.ptr();
                            if (total_h)
                            {
                                group_name += 'H';
                                if (total_h > 1)
                                    group_name += std::to_string(total_h);
                            }
                        }
                    }
                    sa.sa_class.appendString("LGRP", true);
                    sa.subscript.appendString(group_name.c_str(), true);

                    att_desc.leaving_group = grp_idx;
                    fillXBondsAndBrackets(sa, monomer_mol);
                }

                if (ap.HasMember("attachmentAtom"))
                    att_desc.attachment_atom = ap["attachmentAtom"].GetInt();

                if (ap.HasMember("label"))
                {
                    std::string label = ap["label"].GetString();
                    if (label.size() > 1)
                        att_desc.id = convertAPFromHELM(label);
                }

                if (ap.HasMember("type"))
                {
                    std::string label_type = ap["type"].GetString();
                    if (label_type == "left")
                        att_desc.id = kLeftAttachmentPoint;
                    else if (label_type == "right")
                    {
                        att_index = 1;
                        att_desc.id = kRightAttachmentPoint;
                    }
                    else if (att_index < 2)
                        att_index = 2;
                }

                attachment_descs.push_back(att_desc);
                att_index++;
            }

            int grp_idx = monomer_mol.sgroups.addSGroup(SGroup::SG_TYPE_SUP);
            auto& sa = (Superatom&)monomer_mol.sgroups.getSGroup(grp_idx);

            if (mclass.size())
                sa.sa_class.appendString(mclass.c_str(), true);

            if (alias.size())
                sa.subscript.appendString(alias.c_str(), true);

            if (natreplace.size())
                sa.sa_natreplace.appendString(natreplace.c_str(), true);

            for (const auto& att_desc : attachment_descs)
            {
                int atp_index = sa.attachment_points.add();
                auto& atp = sa.attachment_points[atp_index];
                atp.aidx = att_desc.attachment_atom;
                if (att_desc.leaving_group >= 0)
                {
                    auto& lgrp = (Superatom&)monomer_mol.sgroups.getSGroup(att_desc.leaving_group);
                    if (lgrp.atoms.size())
                        atp.lvidx = lgrp.atoms[0];
                }
                atp.apid.appendString(att_desc.id.c_str(), true);
            }

            for (int i = monomer_mol.vertexBegin(); i != monomer_mol.vertexEnd(); i = monomer_mol.edgeNext(i))
            {
                if (leaving_atoms.find(i) == leaving_atoms.end())
                    sa.atoms.push(i);
            }

            fillXBondsAndBrackets(sa, monomer_mol);
        }
    }
    return tg_idx;
}

std::string MoleculeJsonLoader::monomerMolClass(const std::string& class_name)
{
    auto mclass = class_name;
    if (class_name == kMonomerClassAminoAcid)
        return kMonomerClassAA;

    if (mclass == kMonomerClassDAminoAcid)
        return kMonomerClassdAA;

    if (mclass == kMonomerClassRNA || mclass == kMonomerClassDNA || mclass.find(kMonomerClassMOD) == 0 || mclass.find(kMonomerClassXLINK) == 0)
        return mclass;

    std::transform(mclass.begin(), mclass.end(), mclass.begin(), ::toupper);
    return mclass;
}

void MoleculeJsonLoader::loadMolecule(BaseMolecule& mol, bool load_arrows)
{
    ObjArray<Array<int>> mol_mappings;
    for (rapidjson::SizeType node_idx = 0; node_idx < _mol_nodes.Size(); ++node_idx)
    {
        std::vector<EnhancedStereoCenter> stereo_centers;
        std::unique_ptr<BaseMolecule> pmol(mol.neu());
        _pmol = nullptr;
        _pqmol = nullptr;
        if (pmol->isQueryMolecule())
        {
            _pqmol = &pmol->asQueryMolecule();
        }
        else
        {
            _pmol = &pmol->asMolecule();
        }
        mol.original_format = BaseMolecule::KET;

        auto& mol_node = _mol_nodes[node_idx];

        if (mol_node.HasMember("atoms"))
        {
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

            // parse SGroups
            if (mol_node.HasMember("sgroups"))
            {
                parseSGroups(mol_node["sgroups"], *pmol);
            }

            if (mol_node.HasMember("stereoFlagPosition"))
            {
                setStereoFlagPosition(mol_node["stereoFlagPosition"], node_idx, mol);
            }

            if (mol_node.HasMember("properties"))
            {
                parseProperties(mol_node["properties"], *pmol);
            }
        }
        else
        {
            throw Error("Expected atoms block not found");
        }

        Array<int> mapping;
        mol.mergeWithMolecule(*pmol, &mapping, 0);
        mol_mappings.push().copy(mapping);

        for (auto& sc : stereo_centers)
        {
            sc._atom_idx = mapping[sc._atom_idx];
            _stereo_centers.push_back(sc);
        }
    }

    MoleculeRGroups& rgroups = mol.rgroups;
    Document data;
    for (auto& rgrp : _rgroups)
    {
        RGroup& rgroup = rgroups.getRGroup(rgrp.first);
        Value one_rnode(kArrayType);
        Value& rnode = rgrp.second;
        if (rnode.HasMember("fragments"))
        {
            auto& rfragments = rnode["fragments"];
            for (SizeType i = 0; i < rfragments.Size(); i++)
            {
                std::unique_ptr<BaseMolecule> fragment(mol.neu());
                one_rnode.PushBack(rfragments[i], data.GetAllocator());
                MoleculeJsonLoader loader(one_rnode);
                loader.stereochemistry_options = stereochemistry_options;
                loader.loadMolecule(*fragment.get());
                rgroup.fragments.add(fragment.release());
                one_rnode.Clear();
            }
        }
        else
        {
            std::unique_ptr<BaseMolecule> fragment(mol.neu());
            one_rnode.PushBack(rnode, data.GetAllocator());
            MoleculeJsonLoader loader(one_rnode);
            loader.stereochemistry_options = stereochemistry_options;
            loader.loadMolecule(*fragment.get());
            rgroup.fragments.add(fragment.release());
        }
    }

    // Add monomer teplates
    for (SizeType i = 0; i < _templates.Size(); i++)
    {
        auto& mt = _templates[i];
        // int tp = mt.GetType();
        if (mt.HasMember("type") && mt["type"].GetString() == std::string("monomerTemplate"))
            int tgroup_id = parseMonomerTemplate(mt, mol, stereochemistry_options);
    }

    std::unordered_map<int, int> monomer_id_mapping;
    for (SizeType i = 0; i < _monomer_array.Size(); i++)
    {
        auto& ma = _monomer_array[i];
        int idx = mol.asMolecule().addAtom(-1);
        int monomer_id = std::stoi(std::string(ma["id"].GetString()));
        monomer_id_mapping.emplace(monomer_id, idx);

        if (ma.HasMember("alias"))
            mol.asMolecule().setTemplateAtom(idx, ma["alias"].GetString());

        if (ma.HasMember("seqid"))
            mol.asMolecule().setTemplateAtomSeqid(idx, ma["seqid"].GetInt());

        if (ma.HasMember("position"))
        {
            auto& pos_val = ma["position"];
            mol.setAtomXyz(idx, static_cast<float>(pos_val["x"].GetDouble()), static_cast<float>(pos_val["y"].GetDouble()), 0);
        }

        std::string template_id = ma["templateId"].GetString();
        auto temp_it = _id_to_template.find(template_id);
        if (temp_it != _id_to_template.end())
        {
            mol.asMolecule().setTemplateAtomClass(idx, monomerMolClass(monomerTemplateClass(_templates[temp_it->second])).c_str());
            mol.asMolecule().setTemplateAtomTemplateIndex(idx, temp_it->second);
        }
    }

    std::vector<int> ignore_cistrans(mol.edgeCount());
    std::vector<int> sensible_bond_directions(mol.edgeCount());
    for (int i = 0; i < mol.edgeCount(); i++)
        if (mol.getBondDirection(i) == BOND_EITHER)
        {
            if (mol.cis_trans.isIgnored(i))
            {
                ignore_cistrans[i] = 1;
                sensible_bond_directions[i] = 1;
            }
            else if (MoleculeCisTrans::isGeomStereoBond(mol, i, 0, true))
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

    if (!mol.getChiralFlag())
        for (int i : mol.vertices())
        {
            int type = mol.stereocenters.getType(i);
            if (type == MoleculeStereocenters::ATOM_ABS)
                mol.stereocenters.setType(i, MoleculeStereocenters::ATOM_AND, 1);
        }

    mol.buildCisTrans(ignore_cistrans.data());
    mol.have_xyz = true;
    if (mol.stereocenters.size() == 0)
    {
        mol.buildFrom3dCoordinatesStereocenters(stereochemistry_options);
    }

    for (const auto& sc : _stereo_centers)
    {
        if (mol.stereocenters.getType(sc._atom_idx) == 0)
        {
            if (stereochemistry_options.ignore_errors)
                mol.addStereocentersIgnoreBad(sc._atom_idx, sc._type, sc._group, false); // add non-valid stereocenters
            else if (!_pqmol)
                throw Error("stereo type specified for atom #%d, but the bond "
                            "directions does not say that it is a stereocenter",
                            sc._atom_idx);
        }
        else
            mol.stereocenters.setType(sc._atom_idx, sc._type, sc._group);
    }

    for (int i : mol.edges())
    {
        if (mol.getBondDirection(i) > 0 && !sensible_bond_directions[i])
        {
            if (!stereochemistry_options.ignore_errors && !_pqmol)
                throw Error("direction of bond #%d makes no sense", i);
        }
    }

    // handle monomer's connections after all
    for (rapidjson::SizeType i = 0; i < _connection_array.Size(); ++i)
    {
        auto& connection = _connection_array[i];
        int order = _BOND_ANY;
        if (connection.HasMember("connectionType"))
        {
            std::string conn_type = connection["connectionType"].GetString();
            if (conn_type == "single")
                order = BOND_SINGLE;
            else if (conn_type == "hydrogen")
                order = _BOND_HYDROGEN;
        }
        auto& ep1 = connection["endpoint1"];
        auto& ep2 = connection["endpoint2"];

        int id1 = -1;
        int id2 = -1;
        std::string atp1, atp2;

        if (ep1.HasMember("monomerId"))
        {
            id1 = monomer_id_mapping.at(extract_id(ep1["monomerId"].GetString(), "monomer"));
            atp1 = convertAPFromHELM(ep1["attachmentPointId"].GetString());
        }
        else if (ep1.HasMember("moleculeId") && ep1.HasMember("atomId"))
        {
            int mol_id = extract_id(ep1["moleculeId"].GetString(), "mol");
            id1 = mol_mappings[mol_id][atoi(ep1["atomId"].GetString())];
        }
        else
            throw Error("Invalid endpoint");

        if (ep2.HasMember("monomerId"))
        {
            id2 = monomer_id_mapping.at(extract_id(ep2["monomerId"].GetString(), "monomer"));
            atp2 = convertAPFromHELM(ep2["attachmentPointId"].GetString());
        }
        else if (ep2.HasMember("moleculeId") && ep2.HasMember("atomId"))
        {
            int mol_id = extract_id(ep2["moleculeId"].GetString(), "mol");
            id2 = mol_mappings[mol_id][atoi(ep2["atomId"].GetString())];
        }
        else
            throw Error("Invalid endpoint");

        if (atp1.size())
            mol.setTemplateAtomAttachmentOrder(id1, id2, atp1.c_str());

        if (atp2.size())
            mol.setTemplateAtomAttachmentOrder(id2, id1, atp2.c_str());

        mol.asMolecule().addBond_Silent(id1, id2, order);
    }

    MoleculeLayout ml(mol, false);
    ml.layout_orientation = UNCPECIFIED;
    ml.updateSGroups();
    loadMetaObjects(_meta_objects, mol.meta());
    int arrows_count = mol.meta().getMetaCount(KETReactionArrow::CID);
    if (arrows_count && !load_arrows)
        throw Error("Not a molecule. Found %d arrows.", arrows_count);
}

void MoleculeJsonLoader::loadMetaObjects(rapidjson::Value& meta_objects, MetaDataStorage& meta_interface)
{
    static const std::unordered_map<std::string, int> arrow_string2type = {
        {"open-angle", ReactionComponent::ARROW_BASIC},
        {"filled-triangle", ReactionComponent::ARROW_FILLED_TRIANGLE},
        {"filled-bow", ReactionComponent::ARROW_FILLED_BOW},
        {"dashed-open-angle", ReactionComponent::ARROW_DASHED},
        {"failed", ReactionComponent::ARROW_FAILED},
        {"both-ends-filled-triangle", ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE},
        {"equilibrium-filled-half-bow", ReactionComponent::ARROW_EQUILIBRIUM_FILLED_HALF_BOW},
        {"equilibrium-filled-triangle", ReactionComponent::ARROW_EQUILIBRIUM_FILLED_TRIANGLE},
        {"equilibrium-open-angle", ReactionComponent::ARROW_EQUILIBRIUM_OPEN_ANGLE},
        {"unbalanced-equilibrium-filled-half-bow", ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_BOW},
        {"unbalanced-equilibrium-large-filled-half-bow", ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_LARGE_FILLED_HALF_BOW},
        {"unbalanced-equilibrium-open-half-angle", ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_OPEN_HALF_ANGLE},
        {"unbalanced-equilibrium-filled-half-triangle", ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_TRIANGLE},
        {"elliptical-arc-arrow-filled-bow", ReactionComponent::ARROW_ELLIPTICAL_ARC_FILLED_BOW},
        {"elliptical-arc-arrow-filled-triangle", ReactionComponent::ARROW_ELLIPTICAL_ARC_FILLED_TRIANGLE},
        {"elliptical-arc-arrow-open-angle", ReactionComponent::ARROW_ELLIPTICAL_ARC_OPEN_ANGLE},
        {"elliptical-arc-arrow-open-half-angle", ReactionComponent::ARROW_ELLIPTICAL_ARC_OPEN_HALF_ANGLE},
        {"retrosynthetic", ReactionComponent::ARROW_RETROSYNTHETIC}};

    if (meta_objects.IsArray())
    {
        for (rapidjson::SizeType obj_idx = 0; obj_idx < meta_objects.Size(); ++obj_idx)
        {
            std::string node_type = meta_objects[obj_idx]["type"].GetString();
            auto& mobj = meta_objects[obj_idx];
            if (node_type == "simpleObject" || node_type == "text")
            {
                if (mobj.HasMember("data"))
                {
                    auto& sobj = mobj["data"];
                    if (sobj.HasMember("mode")) // ellipse or rectangle or line
                    {
                        int mode = 0;
                        Vec2f p1, p2;
                        std::string obj_mode = sobj["mode"].GetString();
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
                        if (sobj.HasMember("pos"))
                        {
                            auto pos = sobj["pos"].GetArray();
                            if (pos.Size() == 2)
                            {
                                p1.x = pos[0]["x"].GetFloat();
                                p1.y = pos[0]["y"].GetFloat();
                                p2.x = pos[1]["x"].GetFloat();
                                p2.y = pos[1]["y"].GetFloat();
                            }
                            else
                                throw Error("Bad pos array size %d. Most be equal to 2.", pos.Size());
                        }
                        meta_interface.addMetaObject(new KETSimpleObject(mode, std::make_pair(p1, p2)));
                    }
                    else if (sobj.HasMember("content") && sobj.HasMember("position"))
                    {
                        std::string content = sobj["content"].GetString();
                        Vec3f text_origin;
                        text_origin.x = sobj["position"]["x"].GetFloat();
                        text_origin.y = sobj["position"]["y"].GetFloat();
                        text_origin.z = sobj["position"]["z"].GetFloat();
                        meta_interface.addMetaObject(new KETTextObject(text_origin, content));
                    }
                }
            }
            else if (node_type == "arrow")
            {
                auto& mobj_data = mobj["data"];
                const rapidjson::Value& arrow_begin = mobj_data["pos"][0];
                const rapidjson::Value& arrow_end = mobj_data["pos"][1];
                std::string mode = mobj_data["mode"].GetString();
                int arrow_type = ReactionComponent::ARROW_BASIC;
                auto arrow_type_it = arrow_string2type.find(mode);
                if (arrow_type_it != arrow_string2type.end())
                    arrow_type = arrow_type_it->second;

                Vec2f arr_begin(arrow_begin["x"].GetFloat(), arrow_begin["y"].GetFloat());
                Vec2f arr_end(arrow_end["x"].GetFloat(), arrow_end["y"].GetFloat());
                if (mobj_data.HasMember("height"))
                {
                    auto height = mobj_data["height"].GetFloat();
                    meta_interface.addMetaObject(new KETReactionArrow(arrow_type, arr_begin, arr_end, height));
                }
                else
                    meta_interface.addMetaObject(new KETReactionArrow(arrow_type, arr_begin, arr_end));
            }
            else if (node_type == "plus")
            {
                const rapidjson::Value& plus_location = mobj["location"];
                Vec2f plus_pos(plus_location[0].GetFloat(), plus_location[1].GetFloat());
                meta_interface.addMetaObject(new KETReactionPlus(plus_pos));
            }
            else if (node_type == "image")
            {
                const rapidjson::Value& bbox_val = mobj["boundingBox"];
                Vec2f lt(bbox_val["x"].GetFloat(), bbox_val["y"].GetFloat());
                Vec2f rb(lt);
                rb.add(Vec2f(bbox_val["width"].GetFloat(), -bbox_val["height"].GetFloat()));
                std::string image_format_str = mobj["format"].GetString();
                KETImage::ImageFormat image_format;
                if (image_format_str == KImagePNG)
                    image_format = KETImage::EKETPNG;
                else if (image_format_str == KImageSVG)
                    image_format = KETImage::EKETSVG;
                else
                    throw Exception("Unsupported image format: %s", image_format_str.c_str());

                meta_interface.addMetaObject(new KETImage(Rect2f(lt, rb), image_format, mobj["data"].GetString()));
            }
        }
    }
}
