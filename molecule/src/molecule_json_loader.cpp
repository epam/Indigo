#include "molecule/molecule_json_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include <string>
#include <unordered_map>

using namespace rapidjson;
using namespace indigo;
using namespace std;


IMPL_ERROR(MoleculeJsonLoader, "molecule json loader");

MoleculeJsonLoader::MoleculeJsonLoader(const Value& molecule) : _molecule( molecule )
{
}

void MoleculeJsonLoader::loadQueryMolecule( QueryMolecule& qmol )
{
}

void MoleculeJsonLoader::loadMolecule( Molecule& mol )
{
    std::string type = _molecule["type"].GetString();
    if( type.compare("molecule") == 0 )
    {
        // parse atoms
        const Value& atoms = _molecule["atoms"];
        for (SizeType i = 0; i < atoms.Size(); i++)
        {
            const Value& a = atoms[i];
            std::string label = a["label"].GetString();
            auto atom_idx = mol.addAtom(Element::fromString(label.c_str()));
            const Value& coords = a["location"];
            if (coords.Size() > 0)
            {
                Vec3f a_pos;
                a_pos.x = coords[0].GetDouble();
                a_pos.y = coords[1].GetDouble();
                a_pos.z = coords[2].GetDouble();
                mol.setAtomXyz(atom_idx, a_pos);
            }
            
            if( a.HasMember("charge"))
            {
                mol.setAtomCharge_Silent( atom_idx, a["charge"].GetInt() );
            }
            
            if( a.HasMember("explicitValence"))
            {
                mol.setExplicitValence(atom_idx, a["explicitValence"].GetInt());
            }
        }
        
        //parse bonds
        if( _molecule.HasMember("bonds") )
        {
            const Value& bonds = _molecule["bonds"];
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
                if( refs.Size() > 1 )
                {
                    int a1 = refs[0].GetInt();
                    int a2 = refs[1].GetInt();
                    int bond_idx = mol.addBond_Silent(a1, a2, order);
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
                } else
                {
                    // TODO:
                }
            }
        }
    }
    else
    {
            throw Error("unknown type: %s", type.c_str());
    }
    printf("explicit: %d\n", mol.getExplicitValence( 7 ));
}
