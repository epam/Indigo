#include "molecule/molecule_json_loader.h"
#include "molecule/molecule.h"
#include "base_cpp/scanner.h"

using namespace indigo;
#include "third_party/json/json.hpp"

MoleculeJsonLoader::MoleculeJsonLoader(Scanner &scanner) : _scanner(scanner) {
}

void MoleculeJsonLoader::loadMolecule(Molecule &mol) {
   using json = nlohmann::json;
   json j;

   // add a number that is stored as double (note the implicit conversion of j to an object)
   j["pi"] = 3.141;
   std::cout << j.dump(4) << std::endl;
   
   //   auto x = 5;
}