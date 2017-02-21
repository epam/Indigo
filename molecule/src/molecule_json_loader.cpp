#include "molecule/molecule_json_loader.h"
#include "molecule/molecule.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include <unordered_map>

using namespace indigo;
using namespace std;
#include "third_party/json/json.hpp"

IMPL_ERROR(MoleculeJsonLoader, "molecule json loader");

MoleculeJsonLoader::MoleculeJsonLoader(Scanner &scanner) : _scanner(scanner) {
}

void MoleculeJsonLoader::loadMolecule(Molecule &mol) {
   using json = nlohmann::json;
   Array<char> buf;
   _scanner.readAll(buf);
   buf.push(0);
   
   unordered_map<string, int> atom_map;

   auto data = json::parse(buf.ptr());
   auto& root = data["root"];
   
   auto type = root["type"].get<std::string>();
   
   
   if (type.compare("molecule") == 0) {
      auto& atoms = root["atoms"];
      for (auto& a : atoms) {
//         std::cout << a.dump(4) << std::endl;
         auto label = a["label"].get<std::string>();
         auto atom_idx = mol.addAtom(Element::fromString(label.c_str()));
         atom_map.insert(make_pair(a["id"].get<std::string>(), atom_idx));
      }
      
      auto& bonds = root["bonds"];
      for (auto& b : bonds) {
         auto refs = b["refs"].get<vector<string> >();
         auto order = b["order"].get<int>();
         if(refs.size() > 1) {
            mol.addBond(atom_map.at(refs[0]), atom_map.at(refs[1]), order);
         }
      }
      
   } else {
      throw Error("unknown type: %s", type);
   }
   
//   std::cout << data.dump(4) << std::endl;
//   std::cout << type << std::endl;
   
   
   
   //   auto x = 5;
}