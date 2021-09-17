#include "IndigoMolecule.h"

#include <indigo.h>

using namespace indigo_cpp;

IndigoMolecule::IndigoMolecule(const int id, const IndigoSession& indigo) : IndigoChemicalEntity(id, indigo)
{
}

std::string IndigoMolecule::molfile() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoMolfile(id));
}

std::string IndigoMolecule::ctfile() const
{
    return molfile();
}
