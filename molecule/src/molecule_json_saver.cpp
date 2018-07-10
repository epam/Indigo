/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "base_cpp/output.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molecule.h"
#include "molecule/elements.h"
#include "base_cpp/locale_guard.h"

#include "third_party/rapidjson/writer.h"
#include "third_party/rapidjson/stringbuffer.h"
#include <sstream>

using namespace indigo;

IMPL_ERROR(MoleculeJsonSaver, "molecule json saver");

MoleculeJsonSaver::MoleculeJsonSaver (Output &output) : _output(output)
{
}
void MoleculeJsonSaver::saveQueryMolecule (QueryMolecule &qmol)
{
}

void MoleculeJsonSaver::saveMolecule (Molecule &mol)
{
   using namespace rapidjson;
//   throw Error("saveMolecule is not implemented for JSON saver");
   LocaleGuard locale_guard;
//
   StringBuffer s;
   Writer<StringBuffer> writer(s);
   std::stringstream result;
   QS_DEF(Array<char>, buf);
   ArrayOutput out(buf);

   _mol = &mol;

   writer.StartObject(); 

   writer.Key("root"); 
   writer.StartObject(); 

   writer.Key("id"); 
   writer.String(""); 

   writer.Key("type"); 
   writer.String("molecule"); 

   if (_mol->name.ptr() != 0)
   {
   }

   bool have_hyz = _mol->have_xyz;
   bool have_z = BaseMolecule::hasZCoord(*_mol);

   if (_mol->vertexCount() > 0)
   {
      writer.Key("atoms"); 
      writer.StartArray();
      for (auto i : _mol->vertices())
      {
         writer.StartObject(); 

         buf.clear();
         out.printf("a%d", i + 1);
         buf.push(0);

         writer.Key("id"); 
         writer.String(buf.ptr()); 

         _mol->getAtomDescription(i, buf);
         writer.Key("label"); 
         writer.String(buf.ptr()); 
         if (have_hyz) 
         {
            const Vec3f &coord = _mol->getAtomXyz(i);
            writer.Key("location"); 
            writer.StartArray();
            writer.Double(coord.x); 
            writer.Double(coord.y); 
            writer.Double(coord.z); 
            writer.EndArray();
         }
         writer.EndObject();
      }
      writer.EndArray();
   }

   if (_mol->edgeCount() > 0)
   {
      writer.Key("bonds"); 
      writer.StartArray();
      for (auto i : _mol->edges())
      {
         writer.StartObject(); 

         const Edge &e1 = _mol->getEdge(i);
         writer.Key("atoms"); 
         writer.StartArray();

         buf.clear();
         out.printf("a%d", e1.beg + 1);
         buf.push(0);
         writer.String(buf.ptr()); 

         buf.clear();
         out.printf("a%d", e1.end + 1);
         buf.push(0);
         writer.String(buf.ptr()); 
         writer.EndArray();

         writer.Key("order"); 
         writer.Uint(_mol->getBondOrder(i)); 
         writer.EndObject();
      }
      writer.EndArray();
   }

   writer.EndObject();   // root
   writer.EndObject();

   result << s.GetString();
   _output.printf("%s", result.str().c_str());
}
