#include "IndigoBaseMolecule.h"

#include <indigo.h>

using namespace indigo_cpp;

IndigoBaseMolecule::IndigoBaseMolecule(const int id, const IndigoSession& indigo) : IndigoChemicalEntity(id, indigo)
{
}

std::string IndigoBaseMolecule::molfile() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoMolfile(id));
}

std::string IndigoBaseMolecule::ctfile() const
{
    return molfile();
}
