#include "molecule/molecule_json_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "base_cpp/output.h"
#include "layout/molecule_layout.h"
#include <string>
#include <unordered_map>

using namespace rapidjson;
using namespace indigo;
using namespace std;


IMPL_ERROR(MoleculeJsonLoader, "molecule json loader");

MoleculeJsonLoader::MoleculeJsonLoader( Value& mol_nodes, Value& rgroups) : _mol_nodes( mol_nodes ), _rgroups( rgroups), _pmol(0), _pqmol(0)
{
}

int MoleculeJsonLoader::addBondToMoleculeQuery( int beg, int end, int order )
{
    AutoPtr<QueryMolecule::Bond> bond;
    if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC)
        bond.reset(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order));
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
        bond.reset(new QueryMolecule::Bond());
    else
        throw Error("unknown bond type: %d", order);
    // TODO: check what does it mean
/*    if (topology != 0)
    {
        bond.reset(QueryMolecule::Bond::und(bond.release(),
                                            new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, topology == 1 ? TOPOLOGY_RING : TOPOLOGY_CHAIN)));
    }*/
    
    return _pqmol->addBond(beg, end, bond.release());
}

int MoleculeJsonLoader::addAtomToMoleculeQuery( const char* label, int element, int charge, int valence, int radical, int isotope )
{
    AutoPtr<QueryMolecule::Atom> atom;
    atom.reset(new QueryMolecule::Atom());
    if( element != -1 && element != ELEM_RSITE )
        atom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, element));
    else {
        int atom_type = QueryMolecule::getAtomType( label );
        switch (atom_type )
        {
            case _ATOM_PSEUDO:
                atom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_PSEUDO, label));
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
                atom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
            break;
        }
    }
    
    if( charge != 0 )
        atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, charge)));
    
    if( valence > 0 )
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

void MoleculeJsonLoader::validateMoleculeBond( int order )
{
    if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC)
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

void MoleculeJsonLoader::parseAtoms( const rapidjson::Value& atoms, BaseMolecule& mol )
{
    for (SizeType i = 0; i < atoms.Size(); i++)
    {
        std::string label;
        int atom_idx = 0 , charge = 0, valence = 0, radical = 0, isotope = 0, elem = 0, rsite_idx = 0;
        const Value& a = atoms[i];
        if( a.HasMember( "type") )
        {
            std::string atom_type = a["type"].GetString();
            if( atom_type == "rg-label" && a.HasMember("$refs") )
            {
                std::string ref = a["$refs"][0].GetString();
                if( ref.find("rg-") == 0 && ref.erase(0,3).size() )
                {
                    rsite_idx = std::stoi( ref );
                    elem = ELEM_RSITE;
                    label = "R";
                } else
                    throw Error("invalid refs: %s", ref.c_str() );
            } else
                throw Error("invalid atom type: %s", atom_type.c_str() );
        } else
        {
            label = a["label"].GetString();
            if( label == "D" )
            {
                elem = ELEM_H;
                isotope = 2;
            } else
                if( label == "T" )
                {
                    elem = ELEM_H;
                    isotope = 3;
                } else
                {
                    elem = Element::fromString2(label.c_str());
                    if( elem == -1 )
                    {
                        if( _pmol )
                            throw Error("element %s not supported for molecules", label.c_str() );
                    }
                }
        }
        
        if( a.HasMember("charge"))
            charge = a["charge"].GetInt();
        if( a.HasMember("explicitValence"))
            valence = a["explicitValence"].GetInt();
        if( a.HasMember("radical") )
            radical = a["radical"].GetInt();
        if( a.HasMember("isotope") )
            isotope = a["isotope"].GetInt();
        
        if( _pmol )
        {
            atom_idx = _pmol->addAtom( elem );
            if( charge )
                _pmol->setAtomCharge_Silent( atom_idx, charge );
            if( valence )
                _pmol->setExplicitValence( atom_idx, valence );
            if( radical )
                _pmol->setAtomRadical( atom_idx, radical );
            if( isotope )
                _pmol->setAtomIsotope( atom_idx, isotope );
        } else
            atom_idx = addAtomToMoleculeQuery( label.c_str(), elem, charge, valence, radical, isotope );
        
        if( rsite_idx )
            mol.allowRGroupOnRSite( atom_idx, rsite_idx );

        const Value& coords = a["location"];
        if (coords.Size() > 0)
        {
            Vec3f a_pos;
            a_pos.x = coords[0].GetDouble();
            a_pos.y = coords[1].GetDouble();
            a_pos.z = coords[2].GetDouble();
            mol.setAtomXyz(atom_idx, a_pos);
        }
    }
}

void MoleculeJsonLoader::parseBonds( const rapidjson::Value& bonds, BaseMolecule& mol, int atom_base_idx )
{
    for (SizeType i = 0; i < bonds.Size(); i++)
    {
        const Value& b = bonds[i];
        const Value& refs = b["atoms"];
        
        int stereo = 0;
        if( b.HasMember("stereo") )
        {
            stereo = b["stereo"].GetInt();
        }
        
        int order = b["type"].GetInt();
        if( _pmol )
            validateMoleculeBond( order );
        if( refs.Size() > 1 )
        {
            int a1 = refs[0].GetInt() + atom_base_idx;
            int a2 = refs[1].GetInt() + atom_base_idx;
            
            int bond_idx = 0;
            bond_idx = _pmol ? _pmol->addBond_Silent( a1, a2, order ) : addBondToMoleculeQuery( a1, a2, order );
            if( stereo )
            {
                switch ( stereo ) {
                    case 1:
                        mol.setBondDirection( bond_idx, BOND_UP );
                        break;
                    case 3:
                        mol.cis_trans.ignore( bond_idx );
                        break;
                    case 4:
                        mol.setBondDirection( bond_idx, BOND_EITHER);
                        break;
                    case 6:
                        mol.setBondDirection( bond_idx, BOND_DOWN );
                        break;
                    default:
                        break;
                }
            }
            // mol.reaction_bond_reacting_center[bond_idx] = rcenter;
        } else
        {
            // TODO:
        }
    }
}

void MoleculeJsonLoader::parseSGroups( const rapidjson::Value& sgroups, BaseMolecule& mol, int sg_parent )
{
    for( SizeType i = 0; i < sgroups.Size(); i++ )
    {
        const Value& s = sgroups[i];
        std::string sg_type = s["type"].GetString(); //GEN, MUL, SRU, SUP
        int idx = mol.sgroups.addSGroup( sg_type.c_str() );
        SGroup* sgroup = &mol.sgroups.getSGroup(idx);
        sgroup->original_group = i + 1;
        if ( sg_parent > 0 )
            sgroup->parent_group = sg_parent;

        const Value& atoms = s["atoms"];
        for( int j = 0; j < atoms.Size(); ++j )
        {
            int atom_idx = atoms[j].GetInt();
            sgroup->atoms.push( atom_idx );
            for (auto k : mol.edges())
            {
                const Edge& edge = mol.getEdge(k);
                if (((sgroup->atoms.find(edge.beg) != -1) && (sgroup->atoms.find(edge.end) == -1)) ||
                    ((sgroup->atoms.find(edge.end) != -1) && (sgroup->atoms.find(edge.beg) == -1)))
                    sgroup->bonds.push(k);
            }
        }

        if( sg_type == "MUL" )
        {
            // MUL
            if( s.HasMember("mul") )
            {
                int mult = s["mul"].GetInt();
                MultipleGroup* mg = (MultipleGroup*)sgroup;
                mg->multiplier = mult;
            }
        } else if( sg_type == "SRU" )
        {
            // for SRU
            RepeatingUnit* ru = (RepeatingUnit*)sgroup;
            if( s.HasMember("subscript") )
            {
                ru->subscript.readString( s["subscript"].GetString(), true );
            }
            
            if( s.HasMember("connectivity"))
            {
                std::string conn = s["connectivity"].GetString();
                if(  conn == "HT" )
                  ru->connectivity = RepeatingUnit::HEAD_TO_TAIL;
                else if( conn == "HH" )
                  ru->connectivity = RepeatingUnit::HEAD_TO_HEAD;
                else if( conn == "EU" )
                  ru->connectivity = RepeatingUnit::EITHER;
            }
        } else if ( sg_type == "SUP" )
        {
            // for SUP
            Superatom* sg = (Superatom*) sgroup;
            if( s.HasMember("name"))
            {
                sg->subscript.readString( s["name"].GetString(), true );
            }
        } else if ( sg_type == "DAT" )
        {
          //
        }
        Vec2f* p = sgroup->brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);
        p = sgroup->brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);
    }
}


void MoleculeJsonLoader::loadMolecule( BaseMolecule& mol )
{
    _pmol = dynamic_cast<Molecule*>(&mol);
    _pqmol = dynamic_cast<QueryMolecule*>(&mol);
    if( _pmol == NULL && _pqmol == NULL ) throw Error("unknown molecule type: %s", typeid(mol).name());
    BaseMolecule* pbm;
    if( _pmol )
        pbm = static_cast<BaseMolecule*>( _pmol );
    if( _pqmol )
        pbm = static_cast<BaseMolecule*>( _pqmol );
    int atoms_count = 0;
    for( int node_idx = 0; node_idx < _mol_nodes.Size(); ++node_idx )
    {
        auto& mol_node = _mol_nodes[node_idx];
        std::string type = mol_node["type"].GetString();
        if( type.compare("molecule") == 0 || type.compare("rgroup") == 0 )
        {
            // parse atoms
            auto& atoms = mol_node["atoms"];
            parseAtoms( atoms , mol );
            //parse bonds
            if( mol_node.HasMember("bonds") )
            {
                parseBonds( mol_node["bonds"], mol, atoms_count );
            }
            atoms_count += atoms.Size();
            // parse SGroups
            if( mol_node.HasMember("sgroups") )
            {
                parseSGroups( mol_node["sgroups"], mol) ;
            }
        }
        else
        {
            throw Error("unknown type: %s", type.c_str());
        }
    }
    
    MoleculeRGroups& rgroups = pbm->rgroups;
    if( _rgroups.Size() )
    {
        Document data;
        for( int rsite_idx = 0; rsite_idx < _rgroups.Size(); ++rsite_idx)
        {
            RGroup& rgroup = rgroups.getRGroup(rsite_idx + 1);
            AutoPtr<BaseMolecule> fragment(pbm->neu());
            Value one_rnode(kArrayType);
            Value& rnode = _rgroups[ rsite_idx];
            one_rnode.PushBack( rnode , data.GetAllocator() );
            auto empty_val = Value(kArrayType);
            MoleculeJsonLoader loader( one_rnode, empty_val );
            loader.loadMolecule( *fragment.get() );
            rgroup.fragments.add(fragment.release());
        }
    }

    //if (mol.stereocenters.size() == 0)
    //    mol.stereocenters.buildFrom3dCoordinates( stereochemistry_options );

    
    if (mol.stereocenters.size() == 0 && BaseMolecule::hasCoord(mol))
    {
        QS_DEF(Array<int>, sensible_bond_orientations);
        
        sensible_bond_orientations.clear_resize(mol.vertexEnd());
        mol.stereocenters.buildFromBonds(stereochemistry_options, sensible_bond_orientations.ptr());
        
        if (!stereochemistry_options.ignore_errors)
            for (int i = 0; i < mol.vertexCount(); i++)
                if (mol.getBondDirection(i) > 0 && !sensible_bond_orientations[i])
                    throw Error("direction of bond #%d makes no sense", i);
    }
    
    MoleculeLayout ml(mol, false);
    ml.layout_orientation = UNCPECIFIED;
    ml.make();
}
