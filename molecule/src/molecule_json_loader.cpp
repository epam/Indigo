#include "molecule/molecule_json_loader.h"
#include "molecule/molecule.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"

#include <string>
#include <unordered_map>

#include "third_party/rapidjson/document.h"

using namespace indigo;
using namespace std;

IMPL_ERROR(MoleculeJsonLoader, "molecule json loader");

MoleculeJsonLoader::MoleculeJsonLoader(Scanner &scanner) : _scanner(scanner) {
}

void MoleculeJsonLoader::loadQueryMolecule(QueryMolecule &qmol) {
}

void MoleculeJsonLoader::loadMolecule(Molecule &mol) {
   using namespace rapidjson;
   /*
    * Read all into a buffer. TODO create a stream reader
    */
   Array<char> buf;
   _scanner.readAll(buf);
   buf.push(0);
   /*
    * Map between ids and vertices 
    */
   unordered_map<string, int> atom_map;

   Document data;

//   auto data = json::parse(buf.ptr());
   if (data.Parse(buf.ptr()).HasParseError())
      throw Error("Error at parsing JSON: %s", buf.ptr());

   /*
    * Everything in a root. TODO: move root as a separate structure
    */
   const Value& root = data["root"];
   
   std::string type = root["type"].GetString();
                      
   if (type.compare("molecule") == 0) {
      const Value &atoms = root["atoms"];
      /*
       * Parse atoms
       */
      for (SizeType i = 0; i < atoms.Size(); i++)
      {
         const Value& a = atoms[i];
         std::string label = a["label"].GetString();
         auto atom_idx = mol.addAtom(Element::fromString(label.c_str()));
         atom_map.insert(make_pair(a["id"].GetString(), atom_idx));

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
      
      /*
       * Parse bonds
       */
      const Value &bonds = root["bonds"];
      for (SizeType i = 0; i < bonds.Size(); i++)
      {
         const Value& b = bonds[i];
         const Value& refs = b["atoms"];
         auto order = b["order"].GetInt();
         if(refs.Size() > 1) {
            const Value& a1 = refs[0]; 
            const Value& a2 = refs[1]; 
            mol.addBond(atom_map.at(a1.GetString()), atom_map.at(a2.GetString()), order);
         } else {
            /*
             * TODO
             */
         }
      }
   } else {
      throw Error("unknown type: %s", type.c_str());
   }
}
