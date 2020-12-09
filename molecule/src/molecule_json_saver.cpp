/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "molecule/molecule_json_saver.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

#include <vector>
#include <set>

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(MoleculeJsonSaver, "molecule json saver");

MoleculeJsonSaver::MoleculeJsonSaver(Output& output) : _output(output)
{
}

void MoleculeJsonSaver::saveBonds( BaseMolecule* mol_base, rapidjson::Writer<rapidjson::StringBuffer>& writer )
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);

    if (mol_base->edgeCount() > 0)
    {
        for (auto i : mol_base->edges())
        {
            writer.StartObject();
            writer.Key("type");
            writer.Uint(mol_base->getBondOrder(i));
            
            const Edge& e1 = mol_base->getEdge(i);
            writer.Key("atoms");
            writer.StartArray();
            writer.Int(e1.beg);
            writer.Int(e1.end);
            writer.EndArray();

            int stereo = mol_base->getBondDirection(i);
            switch( stereo )
            {
                case BOND_UP:
                    stereo = 1;
                    break;
                case BOND_EITHER:
                    stereo = 4;
                    break;
                case BOND_DOWN:
                    stereo = 6;
                    break;
                default:
                    break;
            }
            
            if( mol_base->cis_trans.isIgnored(i) )
                stereo = 3;
            
            if( stereo )
            {
                writer.Key("stereo");
                writer.Uint( stereo );
            }
            writer.EndObject();
        }
    }
}

void MoleculeJsonSaver::saveAtoms( BaseMolecule* mol_base, Writer<StringBuffer>& writer  )
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    if (mol_base->vertexCount() > 0)
    {
        for (auto i : mol_base->vertices())
        {
            writer.StartObject();
            QS_DEF(Array<int>, rg_list);
            int radical = 0;
            if( mol_base->isRSite(i) )
            {
                mol_base->getAllowedRGroups( i, rg_list );
                writer.Key("type");
                writer.String("rg-label");
                writer.Key("$refs");
                writer.StartArray();
                for( int j = 0; j < rg_list.size(); ++j )
                {
                    buf.clear();
                    out.printf("rg-%d", rg_list[j] );
                    buf.push(0);
                    writer.String(buf.ptr());
                }
                writer.EndArray();
            } else
            {
                radical = mol_base->getAtomRadical(i);
                mol_base->getAtomSymbol(i, buf);
                writer.Key("label");
                writer.String(buf.ptr());
            }
            if (BaseMolecule::hasCoord(*mol_base) )
            {
                const Vec3f& coord = mol_base->getAtomXyz(i);
                writer.Key("location");
                writer.StartArray();
                writer.Double(coord.x);
                writer.Double(coord.y);
                writer.Double(coord.z);
                writer.EndArray();
            }
            int charge = mol_base->getAtomCharge(i);
            int evalence = mol_base->getExplicitValence(i);
            int isotope = mol_base->getAtomIsotope(i);
            if( charge )
            {
                writer.Key("charge");
                writer.Int( charge );
            }
            if( evalence > 0 )
            {
                writer.Key("explicitValence");
                writer.Int( evalence );
            }
            if( radical )
            {
                writer.Key("radical");
                writer.Int( radical );
            }
            
            if( isotope )
            {
                writer.Key("isotope");
                writer.Int( isotope );
            }
            writer.EndObject();
        }
    }
}

void MoleculeJsonSaver::saveRGroup( PtrPool<BaseMolecule>& fragments, int rgnum, rapidjson::Writer<rapidjson::StringBuffer>& writer )
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);

    buf.clear();
    out.printf( "rg%d", rgnum );
    buf.push(0);

    writer.Key(buf.ptr());
    writer.StartObject();
    writer.Key("rlogic");
    writer.StartObject();
    writer.Key("number");
    writer.Int( rgnum );
    writer.EndObject(); //rlogic
    writer.Key("type");
    writer.String("rgroup");

    writer.Key("atoms");
    writer.StartArray();
    for (int j = fragments.begin(); j != fragments.end(); j = fragments.next(j))
        saveAtoms( fragments[j], writer );
    writer.EndArray();

    writer.Key("bonds");
    writer.StartArray();
    for (int j = fragments.begin(); j != fragments.end(); j = fragments.next(j))
        saveBonds( fragments[j], writer );
    writer.EndArray();
    writer.EndObject();
}


void MoleculeJsonSaver::saveMolecule( BaseMolecule& mol )
{
    // bool have_z = BaseMolecule::hasZCoord(*_mol);
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    LocaleGuard locale_guard;
    //
    StringBuffer s;
    std::set<int> rgrp_full_list;
    Writer<StringBuffer> writer(s);
    std::stringstream result;

    _pmol = dynamic_cast<Molecule*>(&mol);
    _pqmol = dynamic_cast<QueryMolecule*>(&mol);

    writer.StartObject();
    
    writer.Key("root");
    writer.StartObject();
    writer.Key("nodes");
    writer.StartArray();

    writer.StartObject();
    writer.Key("$ref");
    writer.String( "mol0" );
    writer.EndObject();

    int n_rgroups = mol.rgroups.getRGroupCount();
    for ( int i = 1; i <= n_rgroups; ++i )
    {
        buf.clear();
        out.printf( "rg%d", i );
        buf.push(0);
        writer.StartObject();
        writer.Key("$ref");
        writer.String( buf.ptr() );
        writer.EndObject();
    }
    
    writer.EndArray(); // nodes
    writer.EndObject(); // root
    
    writer.Key("mol0");
    writer.StartObject();
    writer.Key("type");
    writer.String("molecule");

    writer.Key("atoms");
    writer.StartArray();
    saveAtoms( &mol, writer );
    writer.EndArray();

    writer.Key("bonds");
    writer.StartArray();
    saveBonds( &mol, writer );
    writer.EndArray();
    
    writer.EndObject(); // mol0

    for ( int i = 1; i <= n_rgroups; i++ )
    {
        auto& rgrp = mol.rgroups.getRGroup(i);
        if( rgrp.fragments.size() )
            saveRGroup( rgrp.fragments, i, writer );
    }
   
    writer.EndObject();
    result << s.GetString();
    _output.printf("%s", result.str().c_str());
}
