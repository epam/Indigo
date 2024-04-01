/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License>");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <map>

#include <cppcodec/base64_default_rfc4648.hpp>

#include "base_cpp/output.h"
#include "base_cpp/properties_map.h"
#include "reaction/reaction.h"

#include "indigo_internal.h"

using IndigoObjectTypesMap = std::map<int, const char* const>;
class IndigoObjectTypes : public IndigoObjectTypesMap, public NonCopyable
{
public:
    IndigoObjectTypes();
};

IndigoObjectTypes::IndigoObjectTypes()
{
    emplace(IndigoObject::SCANNER, "<Scanner>");
    emplace(IndigoObject::MOLECULE, "<Molecule>");
    emplace(IndigoObject::QUERY_MOLECULE, "<QueryMolecule>");
    emplace(IndigoObject::REACTION, "<Reaction>");
    emplace(IndigoObject::QUERY_REACTION, "<QueryReaction>");
    emplace(IndigoObject::OUTPUT, "<Output>");
    emplace(IndigoObject::REACTION_ITER, "<ReactionIterator>");
    emplace(IndigoObject::REACTION_MOLECULE, "<ReactionMolecule>");
    emplace(IndigoObject::GROSS_MOLECULE, "<GrossMolecule>");
    emplace(IndigoObject::SDF_LOADER, "<SDFLoader>");
    emplace(IndigoObject::SDF_SAVER, "<SDFSaver>");
    emplace(IndigoObject::RDF_MOLECULE, "<RDFMolecule>");
    emplace(IndigoObject::RDF_REACTION, "<RDFReaction>");
    emplace(IndigoObject::RDF_LOADER, "<RDFLoader>");
    emplace(IndigoObject::SMILES_MOLECULE, "<SmilesMolecule>");
    emplace(IndigoObject::SMILES_REACTION, "<SmilesReaction>");
    emplace(IndigoObject::MULTILINE_SMILES_LOADER, "<MultilineSmilesLoader>");
    emplace(IndigoObject::ATOM, "<Atom>");
    emplace(IndigoObject::ATOMS_ITER, "<AtomsIterator>");
    emplace(IndigoObject::RGROUP, "<RGroup>");
    emplace(IndigoObject::RGROUPS_ITER, "<RGroupsIterator>");
    emplace(IndigoObject::RGROUP_FRAGMENT, "<RGroupFragment>");
    emplace(IndigoObject::RGROUP_FRAGMENTS_ITER, "<RGroupFragmentsIterator>");
    emplace(IndigoObject::ARRAY, "<Array>");
    emplace(IndigoObject::ARRAY_ITER, "<ArrayIterator>");
    emplace(IndigoObject::ARRAY_ELEMENT, "<ArrayElement>");
    emplace(IndigoObject::MOLECULE_SUBSTRUCTURE_MATCH_ITER, "<MoleculeSubstructureMatcherIterator>");
    emplace(IndigoObject::MOLECULE_SUBSTRUCTURE_MATCHER, "<MoleculeSubstructureMatcher>");
    emplace(IndigoObject::REACTION_SUBSTRUCTURE_MATCHER, "<ReactionSubstructureMatcher>");
    emplace(IndigoObject::SCAFFOLD, "<Scaffold>");
    emplace(IndigoObject::DECONVOLUTION, "<Deconvolution>");
    emplace(IndigoObject::DECONVOLUTION_ELEM, "<DeconvolutionElement>");
    emplace(IndigoObject::DECONVOLUTION_ITER, "<DeconvolutionIterator>");
    emplace(IndigoObject::COMPOSITION_ELEM, "<CompositionElement>");
    emplace(IndigoObject::COMPOSITION_ITER, "<CompositionIterator>");
    emplace(IndigoObject::PROPERTIES_ITER, "<PropertiesIterator>");
    emplace(IndigoObject::PROPERTY, "<Property>");
    emplace(IndigoObject::FINGERPRINT, "<Fingerprint>");
    emplace(IndigoObject::BOND, "<Bond>");
    emplace(IndigoObject::BONDS_ITER, "<BondsIterator>");
    emplace(IndigoObject::ATOM_NEIGHBOR, "<AtomNeighbor>");
    emplace(IndigoObject::ATOM_NEIGHBORS_ITER, "<AtomNeighborsIterator>");
    emplace(IndigoObject::SUPERATOM, "<Superatom>");
    emplace(IndigoObject::SUPERATOMS_ITER, "<SuperatomsIterator>");
    emplace(IndigoObject::DATA_SGROUP, "<DataSGroup>");
    emplace(IndigoObject::DATA_SGROUPS_ITER, "<DataSGroupsIterator>");
    emplace(IndigoObject::REPEATING_UNIT, "<RepeatingUnit>");
    emplace(IndigoObject::REPEATING_UNITS_ITER, "<RepeatingUnitsIterator>");
    emplace(IndigoObject::MULTIPLE_GROUP, "<MultipleGroup>");
    emplace(IndigoObject::MULTIPLE_GROUPS_ITER, "<MultipleGroupsIterator>");
    emplace(IndigoObject::GENERIC_SGROUP, "<GenericSGroup>");
    emplace(IndigoObject::GENERIC_SGROUPS_ITER, "<GenericSGroupsIterator>");
    emplace(IndigoObject::SGROUP_ATOMS_ITER, "<SGroupAtomsIterator>");
    emplace(IndigoObject::SGROUP_BONDS_ITER, "<SGroupBondsIterator>");
    emplace(IndigoObject::DECOMPOSITION, "<Decomposition>");
    emplace(IndigoObject::COMPONENT, "<Component>");
    emplace(IndigoObject::COMPONENTS_ITER, "<ComponentsIterator>");
    emplace(IndigoObject::COMPONENT_ATOMS_ITER, "<ComponentAtomsIterator>");
    emplace(IndigoObject::COMPONENT_BONDS_ITER, "<ComponentBondsIterator>");
    emplace(IndigoObject::SUBMOLECULE, "<Submolecule>");
    emplace(IndigoObject::SUBMOLECULE_ATOMS_ITER, "<SubmoleculeAtomsIterator>");
    emplace(IndigoObject::SUBMOLECULE_BONDS_ITER, "<SubmoleculeBondsIterator>");
    emplace(IndigoObject::MAPPING, "<Mapping>");
    emplace(IndigoObject::REACTION_MAPPING, "<ReactionMapping>");
    emplace(IndigoObject::SSSR_ITER, "<SSSRIterator>");
    emplace(IndigoObject::SUBTREES_ITER, "<SubtreesIterator>");
    emplace(IndigoObject::RINGS_ITER, "<RingsIterator>");
    emplace(IndigoObject::EDGE_SUBMOLECULE_ITER, "<EdgeSubmoleculeIterator>");
    emplace(IndigoObject::CML_MOLECULE, "<CMLMolecule>");
    emplace(IndigoObject::CML_REACTION, "<CMLReaction>");
    emplace(IndigoObject::MULTIPLE_CML_LOADER, "<MultipleCMLLoader>");
    emplace(IndigoObject::SAVER, "<Saver>");
    emplace(IndigoObject::ATTACHMENT_POINTS_ITER, "<AttachmentPointsIterator>");
    emplace(IndigoObject::DECOMPOSITION_MATCH, "<DecompositionMatch>");
    emplace(IndigoObject::DECOMPOSITION_MATCH_ITER, "<DecompositionMatchIterator>");
    emplace(IndigoObject::CDX_MOLECULE, "<CDXMolecule>");
    emplace(IndigoObject::CDX_REACTION, "<CDXReaction>");
    emplace(IndigoObject::MULTIPLE_CDX_LOADER, "<MultipleCDXLoader>");
    emplace(IndigoObject::CDX_SAVER, "<CDXSaver>");
    emplace(IndigoObject::SGROUP, "<SGroup>");
    emplace(IndigoObject::SGROUPS_ITER, "<SGroupsIterator>");
    emplace(IndigoObject::TAUTOMER_ITER, "<TautomerIterator>");
    emplace(IndigoObject::TAUTOMER_MOLECULE, "<TautomerMolecule>");
    emplace(IndigoObject::TGROUP, "<TGroup>");
    emplace(IndigoObject::TGROUPS_ITER, "<TGroupsIterator>");
    emplace(IndigoObject::GROSS_REACTION, "<GrossReaction>");
    emplace(IndigoObject::JSON_MOLECULE, "<JsonMolecule>");
    emplace(IndigoObject::JSON_REACTION, "<JsonReaction>");

    if (size() != IndigoObject::INDIGO_OBJECT_LAST_TYPE - 1)
    {
        throw Exception("IndigoObject type name dictionary is inconsistent");
    }
}

const IndigoObjectTypes& getObjectTypesMap()
{
    static IndigoObjectTypes indigoObjectTypes;
    return indigoObjectTypes;
}

IndigoObject::IndigoObject(int type_)
{
    type = type_;
}

IndigoObject::~IndigoObject()
{
}

const char* IndigoObject::getTypeName() const
{
    try
    {
        const IndigoObjectTypes& types = getObjectTypesMap();
        return types.at(type);
    }
    catch (std::out_of_range&)
    {
        return "NAME_UNKNOWN";
    }
}

const char* IndigoObject::debugInfo() const
{
    return getTypeName();
}

void IndigoObject::toBase64String(Array<char>& str)
{
    Array<char> temp;
    toString(temp);
    auto encoded = base64::encode(temp.ptr(), temp.size());
    str.readString(encoded.c_str(), true);
}

void IndigoObject::toString(Array<char>& str)
{
    throw IndigoError("can not convert %s to string", debugInfo());
}

void IndigoObject::toBuffer(Array<char>& buf)
{
    return toString(buf);
}

Molecule& IndigoObject::getMolecule()
{
    throw IndigoError("%s is not a molecule", debugInfo());
}

const Molecule& IndigoObject::getMolecule() const
{
    throw IndigoError("%s is not a molecule", debugInfo());
}

BaseMolecule& IndigoObject::getBaseMolecule()
{
    throw IndigoError("%s is not a base molecule", debugInfo());
}

QueryMolecule& IndigoObject::getQueryMolecule()
{
    throw IndigoError("%s is not a query molecule", debugInfo());
}

void IndigoObject::copyProperties(PropertiesMap& other)
{
    auto& props = getProperties();
    props.copy(other);
}

PropertiesMap& IndigoObject::getProperties()
{
    throw IndigoError("%s can not have properties", debugInfo());
}

MonomersProperties& IndigoObject::getMonomersProperties()
{
    throw IndigoError("%s can not have monomers properties", debugInfo());
}

Reaction& IndigoObject::getReaction()
{
    throw IndigoError("%s is not a reaction", debugInfo());
}

BaseReaction& IndigoObject::getBaseReaction()
{
    throw IndigoError("%s is not a base reaction", debugInfo());
}

QueryReaction& IndigoObject::getQueryReaction()
{
    throw IndigoError("%s is not a query reaction", debugInfo());
}

IndigoObject* IndigoObject::next()
{
    throw IndigoError("%s is not iterable", debugInfo());
}

bool IndigoObject::hasNext()
{
    throw IndigoError("%s is not iterable", debugInfo());
}

void IndigoObject::remove()
{
    throw IndigoError("%s is not removeable", debugInfo());
}

const char* IndigoObject::getName()
{
    throw IndigoError("%s does not have a name", debugInfo());
}

int IndigoObject::getIndex()
{
    throw IndigoError("%s does not have an index", debugInfo());
}

IndigoObject* IndigoObject::clone()
{
    throw IndigoError("%s is not cloneable", debugInfo());
}
