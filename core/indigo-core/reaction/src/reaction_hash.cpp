#include "reaction/reaction_hash.h"

#include "graph/subgraph_hash.h"
#include "molecule/elements.h"
#include "molecule/molecule_hash.h"
#include "reaction/reaction.h"

namespace indigo
{
    dword ReactionHash::calculate(Reaction& rxn)
    {
        int j;
        dword reactantHash = 0;
        for (j = rxn.reactantBegin(); j != rxn.reactantEnd(); j = rxn.reactantNext(j))
        {
            reactantHash += MoleculeHash::calculate(rxn.getMolecule(j));
        }
        dword productHash = 0;
        for (j = rxn.productBegin(); j != rxn.productEnd(); j = rxn.productNext(j))
        {
            productHash += MoleculeHash::calculate(rxn.getMolecule(j));
        }
        dword catalystHash = 0;
        for (j = rxn.catalystBegin(); j != rxn.catalystEnd(); j = rxn.catalystNext(j))
        {
            catalystHash += MoleculeHash::calculate(rxn.getMolecule(j));
        }
        dword hash = 0;
        hash = (hash + (324723947 + reactantHash)) ^ 93485734985;
        hash = (hash + (324723947 + productHash)) ^ 93485734985;
        hash = (hash + (324723947 + catalystHash)) ^ 93485734985;
        return hash;
    }
}
