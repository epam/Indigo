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

#ifndef __molecule_rgroups_composition__
#define __molecule_rgroups_composition__

#include "base_cpp/auto_ptr.h"
#include "base_cpp/obj.h"

#include "molecule/molecule_attachments_search.h"

//semi-temporary
#include "base_cpp/red_black.h"

// temporary {
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/smiles_saver.h"
// } temporary

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Molecule;

class MoleculeRGroupsComposition {
public:
   MoleculeRGroupsComposition () {};
   ~MoleculeRGroupsComposition () {};

   static Iterable<Attachment*>*   refine(BaseMolecule &mol);
   static BaseMolecule*            decorate(BaseMolecule &mol, Attachment &at);

   //static Iterable<BaseMolecule*>* combinations(BaseMolecule &mol);

   // temporary {
   static FileOutput file;
   static StandardOutput std;
 
   static const char* printarr(const Array<int> &xs) {
       char *ptr = new char[1024];
       const char *out = ptr;

       ptr += sprintf(ptr, "[");
       const char *prefix = "";
       for (auto i = 0; i < xs.size(); i++) {
           ptr += sprintf(ptr, "%s%d", prefix, xs[i]);
           prefix = ", ";
       }
       ptr += sprintf(ptr, "]");
       return out;
   }

   static void printarr(char *prefix, const Array<int> &arr, char *suffix, Output &out) {
       out.printf("%s", prefix);
       auto n = arr.size();
       for (auto i = 0; i < n; i++) {
           out.printf("%d", arr[i]);
           if (i < n - 1) {
               out.printf(",");
           }
       }
       out.printf("%s", suffix);
   }
   static void printlist(char *prefix, const List<int> &list, char *suffix, Output &out) {
       out.printf("%s", prefix);
       for (auto i = list.begin(); i != list.end(); i = list.next(i)) {
           out.printf("%d,", list[i]);
       }
       out.printf("%s", suffix);
   }
   template <typename F>
   static void printmol(char *prefix, BaseMolecule &mol, char *infix, F f, char *suffix, Output &out) {
       out.printf("%s%s", prefix, infix);

       SmilesSaver saver(out);
       if (mol.isQueryMolecule()) {
           saver.saveQueryMolecule(mol.asQueryMolecule());
       }
       else {
           saver.saveMolecule(mol.asMolecule());
       }

       out.printf("%s", infix);
       f(mol, out);
       out.printf("%s", suffix);
   }
   template <typename F>
   static void printmol(char *prefix, BaseMolecule &mol, F f, Output &out) {
       char *infix = " ";
       char *suffix = "\n";

       if (dynamic_cast<FileOutput*>(&out)) {
           infix = "!";
       }
       printmol(prefix, mol, infix, f, suffix, out);
   }
   static void printmol(char *prefix, BaseMolecule &mol, Output &out) {
       printmol(prefix, mol, [](BaseMolecule &m, Output &o) {}, out);
   }

   static void debug(BaseMolecule &target) {
       auto cb_general = [](Graph &g, Output &out) {
           out.printf("vs=%d.es=%d.cs=%d.cvs=%d.ces=%d",
               g.vertexCount(), g.edgeCount(),
               g.countComponents(), g.countComponentVertices(0), g.countComponentEdges(0));
       };
       auto cb_sssr = [](Graph &g, Output &out) {
           auto n = g.sssrCount();
           out.printf("sssrN=%d.", n);
           for (auto i = 0; i < n; i++) {
               char s[50];
               sprintf(s, "sssr%02d=", i);
               printlist(s, g.sssrVertices(i), "", out);
           }
       };
       auto cb_edges = [](Graph &g, Output &out) {
           for (auto e : g.edges()) {
               char *top_;
               int top = g.getEdgeTopology(e);
               if (top == TOPOLOGY_CHAIN) {
                   top_ = "chain";
               }
               else
               if (top == TOPOLOGY_RING) {
                   top_ = "ring";
               }
               else {
                   top_ = "-";
               }

               out.printf("%d=%d_%d=%s,", e, g.getEdge(e).beg, g.getEdge(e).end, top_);
           }
       };
       auto cb_rsites = [](BaseMolecule &m, Output &out) {
           auto rsN = m.countRSites();
           out.printf("rsitesN=%d.", rsN);

           bool empty = true;
           for (auto v : m.vertices()) {
               if (m.isRSite(v)) {
                   out.printf("%s%d_%d", empty ? "" : ",", v, m.getRSiteAttachmentPointByOrder(v, 0));
                   empty = false;
               }
           }
           if (!empty) {
               out.printf(".");
           }
       };
       auto cb_apoints = [](BaseMolecule &m, Output &out) {
           auto apsN = m.attachmentPointCount();
           out.printf("apointsN=%d.", apsN);

           for (auto v : m.vertices()) {
               Array<int> is;
               m.getAttachmentIndicesForAtom(v, is);
               if (is.size() < 1) {
                   continue;
               }

               char s[50];
               sprintf(s, "is%d=", v);
               printarr(s, is, ".", out);
           }

           for (auto i = 1; i <= apsN; i++) {
               if (m.getAttachmentPoint(i, 0) == -1) {
                   continue;
               }

               int ap;
               bool empty = true;
               out.printf("ap%d=", i);
               for (auto j = 0; (ap = m.getAttachmentPoint(i, j)) != -1; j++) {
                   out.printf("%s%d", empty ? "" : ",", ap);
                   empty = false;
               }
               if (!empty) {
                   out.printf(".");
               }
           }
       };
       auto cb_info = [cb_general, cb_sssr, cb_edges, cb_rsites, cb_apoints]
           (BaseMolecule &m, Output &out) {
           printmol("general", m, cb_general, out);
           printmol("sssr   ", m, cb_sssr, out);
           printmol("edges  ", m, cb_edges, out);
           printmol("rsites ", m, cb_rsites, out);
           printmol("apoints", m, cb_apoints, out);
       };

       printmol("Info ", target, "\n", cb_info, "", std);
       printmol("target", target, file);

       MoleculeRGroups &rs = target.rgroups;
       auto n = rs.getRGroupCount();
       for (auto i = 1; i <= n; i++) {
           printf("rgroup %d / %d:\n", i, n);
           RGroup &r = rs.getRGroup(i);
           printf("\t%d fragments\n", r.fragments.size());
           for (auto j = r.fragments.begin(); j != r.fragments.end(); j = r.fragments.next(j)) {
               printmol("\t\t", *r.fragments[j], "\n", cb_info, "", std);

               char name[50];
               sprintf(name, "rgroup%d.fragment%d", i, j);
               printmol(name, *r.fragments[j], file);
           }

           printarr("\toccurence = [", r.occurrence, "]\n", std);
           printf("\tif_then = %d\n", r.if_then);
           printf("\trest_h = %d\n", r.rest_h);
       }
   }
   // } temporary

   DECL_ERROR;
};

}

#ifdef _WIN32
#endif

#endif