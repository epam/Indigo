#include "IndigoChemicalEntity.h"

#include <indigo-inchi.h>

using namespace indigo_cpp;

IndigoChemicalEntity::IndigoChemicalEntity(const int id, const IndigoSession& indigo) : IndigoObject(id, indigo)
{
}

void IndigoChemicalEntity::aromatize() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoAromatize(id));
}

void IndigoChemicalEntity::dearomatize() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoDearomatize(id));
}

void IndigoChemicalEntity::layout() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoLayout(id));
}

void IndigoChemicalEntity::clean2d() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoClean2d(id));
}

std::string IndigoChemicalEntity::smiles() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoSmiles(id));
}

std::string IndigoChemicalEntity::cml() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoCml(id));
}

std::string IndigoChemicalEntity::inchi() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoInchiGetInchi(id));
}
