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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "base_c/defs.h"
#include "indigo.h"
#include "indigo-renderer.h"

void usage (void)
{
   fprintf(stderr,
           "Indigo version: %s\n\n"
           "Usage:\n"
           "  indigo-depict infile.{mol,rxn,cml,smi} outfile.{png,svg,pdf} [parameters]\n"
           "  indigo-depict infile.{sdf,rdf,cml,smi} outfile_%%s.{png,svg,pdf} [parameters]\n"
           "  indigo-depict infile.{sdf,rdf}.gz outfile_%%s.{png,svg,pdf} [parameters]\n"
           "  indigo-depict infile.smi outfile.{mol,rxn,cml} [parameters]\n"
           "  indigo-depict infile.smi outfile.{sdf,rdf,cml} [parameters]\n"
           "  indigo-depict - SMILES outfile.{png,svg,pdf} [parameters]\n"
           "  indigo-depict - SMILES outfile.{mol,rxn,cml} [parameters]\n"
           "  indigo-depict - InChI outfile.{png,svg,pdf} [parameters]\n"
           "  indigo-depict - InChI outfile.{mol,rxn,cml} [parameters]\n"
           "\nParameters:\n"
           "-w <number>\n"
           "   Picture width in pixels\n"
           "-h <number>\n"
           "   Picture height in pixels\n"
           "-bond <number>\n"
           "   Average bond length in pixels\n"
           "-margins <number> <number>\n"
           "   Horizontal and vertical margins, in pixels. No margins by default\n"
           "-thickness <number>\n"
           "   Set relative thickness factor. Default is 1.0\n"
           "-linewidth <number>\n"
           "   Set bond line width factor. Default is 1.0\n"
           "-label <none|hetero|terminal-hetero|all>\n"
           "   Set atom label display mode (default is terminal-hetero)\n"
           "-hydro <on|off>\n"
           "   Show implicit hydrogens (default is on)\n"
           "-[de]arom\n"
           "   Force [de]aromatization\n"
           "-stereo <old|ext|none>\n"
           "   Stereogroups display mode (default is 'old')\n"
           "-cdbwsa\n"
           "   Center double bonds which have an adjacent stereo bond (disabled by default)\n"
           "-query\n"
           "   Treat the input as a query molecule or reaction (disabled by default)\n"
           "-smarts\n"
           "   Treat the input as a SMARTS query molecule or reaction (disabled by default)\n"
           "-id <string>\n"
           "   SDF/RDF field to be put in place of '%%s' in the names of saved files\n"
           "   (default is molecule/reaction number)\n"
           "-catalysts [above|above-and-below]\n"
           "   Reaction catalysts placement w.r.t. the arrow (default is above and below\n"
           "-comment <string>\n"
           "   Text comment to be put above the molecule or reaction. No default value\n"
           "-commentoffset <number> \n"
           "   Vertical space (in pixels) between the comment and the structure\n"
           "-commentfield <string>\n"
           "   Use specified SDF/RDF field as a comment\n"
           "-commentname\n"
           "   Use molecule/reaction name as a comment\n"
           "-commentsize <number>\n"
           "   Text comment font size factor relative to bond thickness (default 6)\n"
           "-commentpos <top|bottom>\n"
           "   Text comment position (bottom by default)\n"
           "-commentalign <0..1>\n"
           "   Text comment alignment, a float value: 0 = left, 0.5 = center, 1 = right\n"
           "-coloring <on|off>\n"
           "   Enable/disable coloring (enabled by default)\n"
           "-hlthick\n"
           "   Enable highlighting with thick lines and bold characters\n"
           "-hlcolor <red> <green> <blue>\n"
           "   Enable highlighting with color. Component values must be in range [0..255]\n"
           "-bgcolor <red> <green> <blue>\n"
           "   Set the background color. Component values must be in range [0..255]\n"
           "-basecolor <red> <green> <blue>\n"
           "   Set the default foreground color. Component values must be in range [0..255]\n"
           "-aamcolor <red> <green> <blue>\n"
           "   Set the color of AAM indices. Component values must be in range [0..255]\n"
           "-dsgcolor <red> <green> <blue>\n"
           "   Set the color of data SGroups. Component values must be in range [0..255]\n"
           "-commentcolor <red> <green> <blue>\n"
           "   Set the color of the comment. Component values must be in range [0..255]\n"
           "-atomnumbers\n"
           "   Show atom numbers (for debugging purposes only)\n"
           "-bondnumbers\n"
           "   Show bond numbers (for debugging purposes only)\n"
           "-onebased\n"
           "   Start atom and bond indices from one (default is from zero)\n"
           "-help\n"
           "   Print this help message\n"
           "\n"
           "Examples:\n"
           "   indigo-depict infile.mol outfile.png -coloring off -arom\n"
           "   indigo-depict database.sdf molecule_%%s.png -id cdbregno -thickness 1.1\n"
           "   indigo-depict database.smi database.sdf\n"
           "   indigo-depict - \"CC.[O-][*-]([O-])=O\" query.png -query\n"
           "   indigo-depict - \"OCO>>CC(C)N\" reaction.rxn\n",
           indigoVersion());
}

#define USAGE() do { usage(); return -1; } while (0)
#define ERROR(str) { fprintf(stderr, str); return -1; }

int parseColor (char *argv[], int i, float *rr, float *gg, float *bb)
{
   int r, g, b;
   
   if (sscanf(argv[i + 1], "%d", &r) != 1 || r < 0 || r > 255)
   {
      fprintf(stderr, "%s is not a valid color index\n", argv[i + 1]);
      return -1;
   }
   if (sscanf(argv[i + 2], "%d", &g) != 1 || g < 0 || g > 255)
   {
      fprintf(stderr, "%s is not a valid color index\n", argv[i + 2]);
      return -1;
   }
   if (sscanf(argv[i + 3], "%d", &b) != 1 || b < 0 || b > 255)
   {
      fprintf(stderr, "%s is not a valid color index\n", argv[i + 3]);
      return -1;
   }

   *rr = r / 255.f;
   *gg = g / 255.f;
   *bb = b / 255.f;
   return 0;
}

int _isMultiline (const char *filename, int *is_reaction)
{
   FILE *f = fopen(filename, "rt");
   int c;

   *is_reaction = 0;
   
   if (f == NULL)
   {
      fprintf(stderr, "Can not open %s for reading\n", filename);
      return -1;
   }

   while ((c = fgetc(f)) != EOF)
   {
      if (c == '>')
         *is_reaction = 1;
      if (c == '\n')
         break;
   }

   if (c == EOF)
      return 0;

   while ((c = fgetc(f)) != EOF)
   {
      if (!isspace(c))
         return 1;
   }

   return 0;
}

int _isReaction (const char *smiles)
{
   return strchr(smiles, '>') != NULL;
}

int _isMultipleCML (const char *filename, int *reaction)
{
   int iter = indigoIterateCMLFile(filename);

   *reaction = 0;
   
   if (indigoHasNext(iter))
   {
      int item = indigoNext(iter);
      if (strstr(indigoRawData(item), "<reaction") != NULL)
         *reaction = 1;
      
      if (indigoHasNext(iter))
      {
         indigoFree(item);
         indigoFree(iter);
         return 1;
      }
      indigoFree(item);
   }

   indigoFree(iter);
   return 0;
}

enum
{
   MODE_SINGLE_MOLECULE,
   MODE_SINGLE_REACTION,
   MODE_MULTILINE_SMILES,
   MODE_SDF,
   MODE_RDF,
   MODE_MULTIPLE_CML
};

enum
{
   ACTION_RENDER,
   ACTION_LAYOUT
};

enum
{
   OEXT_MOL,
   OEXT_RXN,
   OEXT_SDF,
   OEXT_RDF,
   OEXT_CML,
   OEXT_OTHER
};

void onError (const char *message, void *context)
{
   fprintf(stderr, "%s\n", message);
   exit(-1);
}

enum AROMATIZATION {NONE = 0, AROM, DEAROM};

int _prepare (int obj, int arom) {
   if (arom == AROM)
      return indigoAromatize(obj);
   if (arom == DEAROM)
      return indigoDearomatize(obj);
   return 0;
}

void renderToFile (int obj, const char* outfile) {
   int out = indigoWriteFile(outfile);
   indigoRender(obj, out);
   indigoFree(out);
}

typedef struct tagParams {
   const char *outfile;
   char outfile_ext[4], infile_ext[7];
   int width;
   int height;
   int bond;
   int mode;
   int action;
   const char *id;
   const char *string_to_load;
   const char *file_to_load;
   int hydro_set, query_set, smarts_set;
   int aromatization;
   int out_ext;
   const char *comment;
   const char *comment_field;
   int comment_name;
} Params;

int parseParams (Params* p, int argc, char *argv[]) {
   int i;
   if (strcmp(argv[1], "-") == 0)
   {
      if (_isReaction(argv[2]))
      {
         p->mode = MODE_SINGLE_REACTION;
         p->string_to_load = argv[2];
      }
      else
      {
         p->mode = MODE_SINGLE_MOLECULE;
         p->string_to_load = argv[2];
      }
      if (argc <= 3)
         USAGE();
      i = 3;
   }
   else
   {
      p->infile_ext[0] = 0;
      if (strlen(argv[1]) >= 4 && argv[1][strlen(argv[1]) - 4] == '.')
      {
         p->infile_ext[3] = 0;
         strncpy(p->infile_ext, argv[1] + strlen(argv[1]) - 3, 3);
      }
      else if (strlen(argv[1]) > 7 && argv[1][strlen(argv[1]) - 7] == '.')
      {
         p->infile_ext[6] = 0;
         strncpy(p->infile_ext, argv[1] + strlen(argv[1]) - 6, 6);
      }

      p->file_to_load = argv[1];
      if (strcasecmp(p->infile_ext, "mol") == 0)
         p->mode = MODE_SINGLE_MOLECULE;
      else if (strcasecmp(p->infile_ext, "rxn") == 0)
         p->mode = MODE_SINGLE_REACTION;
      else if (strcasecmp(p->infile_ext, "smi") == 0)
      {
         int reaction;
         
         if (_isMultiline(argv[1], &reaction)) 
            p->mode = MODE_MULTILINE_SMILES;
         else
         {
            if (reaction)
               p->mode = MODE_SINGLE_REACTION;
            else
               p->mode = MODE_SINGLE_MOLECULE;
         }
      }
      else if (strcasecmp(p->infile_ext, "cml") == 0)
      {
         int reaction;
         if (_isMultipleCML(argv[1], &reaction))
            p->mode = MODE_MULTIPLE_CML;
         else
         {
            if (reaction)
               p->mode = MODE_SINGLE_REACTION;
            else
               p->mode = MODE_SINGLE_MOLECULE;
         }
      }
      else if (strcasecmp(p->infile_ext, "sdf") == 0 || strcasecmp(p->infile_ext, "sdf.gz") == 0)
         p->mode = MODE_SDF;
      else if (strcasecmp(p->infile_ext, "rdf") == 0 || strcasecmp(p->infile_ext, "rdf.gz") == 0)
         p->mode = MODE_RDF;
      else
         USAGE();

      i = 2;
   }

   p->outfile = argv[i++];

   if (strlen(p->outfile) < 5 || p->outfile[strlen(p->outfile) - 4] != '.')
      USAGE();

   p->outfile_ext[3] = 0;
   strncpy(p->outfile_ext, p->outfile + strlen(p->outfile) - 3, 3);

   indigoSetOptionBool("treat-x-as-pseudoatom", 1);
   indigoSetOptionBool("render-coloring", 1);
   indigoSetOptionBool("render-highlight-color-enabled", 1);

   for (; i < argc; i++)
   {
      if (strcmp(argv[i], "-w") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting number after -w\n");
            return -1;
         }

         if (sscanf(argv[i], "%d", &p->width) != 1 || p->width <= 0)
         {
            fprintf(stderr, "%s is not a valid width\n", argv[i]);
            return -1;
         }
      }
      else if (strcmp(argv[i], "-h") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting number after -h\n");
            return -1;
         }

         if (sscanf(argv[i], "%d", &p->height) != 1 || p->height <= 0)
         {
            fprintf(stderr, "%s is not a valid height\n", argv[i]);
            return -1;
         }
      }
      else if (strcmp(argv[i], "-margins") == 0)
      {
         int horz, vert;

         if (i + 2 >= argc)
         {
            fprintf(stderr, "expecting two numbers after -margins\n");
            return -1;
         }

         if (sscanf(argv[i + 1], "%d", &horz) != 1 || horz < 0)
         {
            fprintf(stderr, "%s is not a valid horizontal margin\n", argv[i]);
            return -1;
         }
         if (sscanf(argv[i + 2], "%d", &vert) != 1 || vert < 0)
         {
            fprintf(stderr, "%s is not a valid vertical margin\n", argv[i + 1]);
            return -1;
         }

         indigoSetOptionXY("render-margins", horz, vert);
         i += 2;
      }
      else if (strcmp(argv[i], "-thickness") == 0)
      {
         float rt;

         if (++i == argc)
         {
            fprintf(stderr, "expecting number after -thickness\n");
            return -1;
         }

         if (sscanf(argv[i], "%f", &rt) != 1 || rt < 0)
         {
            fprintf(stderr, "%s is not a valid relative thickness\n", argv[i]);
            return -1;
         }
         indigoSetOptionFloat("render-relative-thickness", rt);
      }
      else if (strcmp(argv[i], "-linewidth") == 0)
      {
         float rt;

         if (++i == argc)
         {
            fprintf(stderr, "expecting number after -linewidth\n");
            return -1;
         }

         if (sscanf(argv[i], "%f", &rt) != 1 || rt < 0)
         {
            fprintf(stderr, "%s is not a valid line width value\n", argv[i]);
            return -1;
         }
         indigoSetOptionFloat("render-bond-line-width", rt);
      }
      else if (strcmp(argv[i], "-bond") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting number after -bond\n");
            return -1;
         }

         if (sscanf(argv[i], "%d", &p->bond) != 1 || p->bond <= 0)
         {
            fprintf(stderr, "%s is not a valid bond length\n", argv[i]);
            return -1;
         }
      }
      else if (strcmp(argv[i], "-coloring") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting 'on' or 'off' after -coloring\n");
            return -1;
         }

         if (strcasecmp(argv[i], "on") == 0)
            indigoSetOptionBool("render-coloring", 1);
         else if (strcasecmp(argv[i], "off") == 0)
            indigoSetOptionBool("render-coloring", 0);
         else
         {
            fprintf(stderr, "expecting 'on' or 'off' after -coloring\n");
            return -1;
         }
      }
      else if (strcmp(argv[i], "-hlthick") == 0)
      {
         indigoSetOptionBool("render-highlight-thickness-enabled", 1);
      }
      else if (strcmp(argv[i], "-hlcolor") == 0)
      {
         float r, g, b;

         if (i + 3 >= argc)
         {
            fprintf(stderr, "expecting 3 numbers after -hlcolor\n");
            return -1;
         }

         if (parseColor(argv, i, &r, &g, &b) != 0)
            return -1;

         indigoSetOptionBool("render-highlight-color-enabled", 1);
         indigoSetOptionColor("render-highlight-color", r, g, b);
         i += 3;
      }
      else if (strcmp(argv[i], "-bgcolor") == 0)
      {
         float r, g, b;

         if (i + 3 >= argc)
         {
            fprintf(stderr, "expecting 3 numbers after -bgcolor\n");
            return -1;
         }

         if (parseColor(argv, i, &r, &g, &b) != 0)
            return -1;
         indigoSetOptionColor("render-background-color", r, g, b);
         i += 3;
      }
      else if (strcmp(argv[i], "-basecolor") == 0)
      {
         float r, g, b;

         if (i + 3 >= argc)
         {
            fprintf(stderr, "expecting 3 numbers after -basecolor\n");
            return -1;
         }

         if (parseColor(argv, i, &r, &g, &b) != 0)
            return -1;
         indigoSetOptionColor("render-base-color", r, g, b);
         i += 3;
      }
      else if (strcmp(argv[i], "-aamcolor") == 0)
      {
         float r, g, b;

         if (i + 3 >= argc)
         {
            fprintf(stderr, "expecting 3 numbers after -aamcolor\n");
            return -1;
         }

         if (parseColor(argv, i, &r, &g, &b) != 0)
            return -1;

         indigoSetOptionColor("render-aam-color", r, g, b);
         i += 3;
      }
      else if (strcmp(argv[i], "-dsgcolor") == 0)
      {
         float r, g, b;

         if (i + 3 >= argc)
         {
            fprintf(stderr, "expecting 3 numbers after -aamcolor\n");
            return -1;
         }

         if (parseColor(argv, i, &r, &g, &b) != 0)
            return -1;

         indigoSetOptionColor("render-data-sgroup-color", r, g, b);
         i += 3;
      }
      else if (strcmp(argv[i], "-hydro") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -hydro\n");
            return -1;
         }
         indigoSetOption("render-implicit-hydrogens-visible", argv[i]);
         p->hydro_set = 1;
      }
      else if (strcmp(argv[i], "-label") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -label\n");
            return -1;
         }
         indigoSetOption("render-label-mode", argv[i]);
      }
      else if (strcmp(argv[i], "-arom") == 0)
      {
         p->aromatization = AROM;
      }
      else if (strcmp(argv[i], "-dearom") == 0)
      {
         p->aromatization = DEAROM;
      }
      else if (strcmp(argv[i], "-stereo") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -stereo\n");
            return -1;
         }

         indigoSetOption("render-stereo-style", argv[i]);
      }
      else if (strcmp(argv[i], "-cdbwsa") == 0)
      {
         indigoSetOptionBool("render-center-double-bond-when-stereo-adjacent", 1);
      }
      else if (strcmp(argv[i], "-query") == 0)
      {
         p->query_set = 1;
      }
      else if (strcmp(argv[i], "-smarts") == 0)
      {
         p->smarts_set = 1;
      }
      else if (strcmp(argv[i], "-id") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -id\n");
            return -1;
         }

         p->id = argv[i];
      }
      else if (strcmp(argv[i], "-catalysts") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -catalysts\n");
            return -1;
         }

         indigoSetOption("render-catalysts-placement", argv[i]);
      }
      else if (strcmp(argv[i], "-comment") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -comment\n");
            return -1;
         }

         p->comment = argv[i];
      }
      else if (strcmp(argv[i], "-commentoffset") == 0)
      {
         int offset;

         if (++i >= argc)
         {
            fprintf(stderr, "expecting an integer after -commentoffset\n");
            return -1;
         }

         if (sscanf(argv[i], "%d", &offset) != 1 || offset < 0)
         {
            fprintf(stderr, "%s is not a valid comment offset\n", argv[i]);
            return -1;
         }

         indigoSetOptionInt("render-comment-offset", offset);
      }
      else if (strcmp(argv[i], "-commentfield") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -commentfield\n");
            return -1;
         }

         p->comment_field = argv[i];
      }
      else if (strcmp(argv[i], "-commentname") == 0)
      {
         p->comment_name = 1;
      }
      else if (strcmp(argv[i], "-commentsize") == 0)
      {
         int commentsize;

         if (++i == argc)
         {
            fprintf(stderr, "expecting number after -commentsize\n");
            return -1;
         }

         if (sscanf(argv[i], "%d", &commentsize) != 1 || commentsize <= 0)
         {
            fprintf(stderr, "%s is not a valid font size\n", argv[i]);
            return -1;
         }
         indigoSetOptionFloat("render-comment-font-size", (float)commentsize);
      }
      else if (strcmp(argv[i], "-commentcolor") == 0)
      {
         float r, g, b;

         if (i + 3 >= argc)
         {
            fprintf(stderr, "expecting 3 numbers after -commentcolor\n");
            return -1;
         }

         if (parseColor(argv, i, &r, &g, &b) != 0)
            return -1;

         indigoSetOptionColor("render-comment-color", r, g, b);
         i += 3;
      }
      else if (strcmp(argv[i], "-commentalign") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -commentalign\n");
            return -1;
         }
         indigoSetOption("render-comment-alignment", argv[i]);
      }
      else if (strcmp(argv[i], "-commentpos") == 0)
      {
         if (++i == argc)
         {
            fprintf(stderr, "expecting an identifier after -commentpos\n");
            return -1;
         }
         indigoSetOption("render-comment-position", argv[i]);
      }
      else if (strcmp(argv[i], "-atomnumbers") == 0)
         indigoSetOptionBool("render-atom-ids-visible", 1);
      else if (strcmp(argv[i], "-bondnumbers") == 0)
         indigoSetOptionBool("render-bond-ids-visible", 1);
      else if (strcmp(argv[i], "-onebased") == 0)
         indigoSetOptionBool("render-atom-bond-ids-from-one", 1);
      else if (strcmp(argv[i], "-help") == 0)
      {
         usage();
         return 0;
      }
      else
      {
         fprintf(stderr, "unknown option: %s\n", argv[i]);
         return -1;
      }
   }

   if (p->bond > 0)
      indigoSetOptionFloat("render-bond-length", (float)p->bond);
   
   if (p->width > 0)
      indigoSetOptionInt("render-image-width", p->width);
   if (p->height > 0)
      indigoSetOptionInt("render-image-height", p->height);

   if (p->hydro_set && p->query_set)
   {
      fprintf(stderr, "-hydro conflicts with -query (implicit hydrogens do not exist in queries)\n");
   }
   return 0;
}

void _setComment (int obj, Params *p) {
   if (p->comment_name) {
      const char *name = indigoName(obj);
      if (name != NULL && *name != 0) {
         indigoSetOption("render-comment", name);
         return;
      }
   }
   if (p->comment_field != NULL) {
      if (indigoHasProperty(obj, p->comment_field)) {
         const char *prop = indigoGetProperty(obj, p->comment_field);
         if (prop != NULL && *prop != 0) {
            indigoSetOption("render-comment", prop);
            return;
         }
      }
   }
   if (p->comment != NULL && *p->comment != 0) {
      indigoSetOption("render-comment", p->comment);
      return;
   }
}

int main (int argc, char *argv[])
{
   Params p;
   int obj = -1, reader = -1, writer = -1; 
   int i = 0;
   char number[100];
   char outfilename[4096];
   const char *id;

   p.width = 
      p.height = 
      p.bond = 
      p.mode = -1;
   p.id =
      p.string_to_load = 
      p.file_to_load = NULL;
   p.hydro_set = 
      p.query_set = 
     p.smarts_set = 0;
   p.aromatization = NONE;
   p.comment_field = NULL;
   p.comment = NULL;
   p.comment_name = 0;

   if (argc <= 2)
      USAGE();

   indigoSetErrorHandler(onError, 0);

   indigoSetOption("ignore-stereochemistry-errors", "on");

   if (parseParams(&p, argc, argv) < 0)
      return -1;

   p.out_ext = OEXT_OTHER;
   if (strcmp(p.outfile_ext, "mol") == 0)
      p.out_ext = OEXT_MOL;
   else if (strcmp(p.outfile_ext, "sdf") == 0)
      p.out_ext = OEXT_SDF;
   else if (strcmp(p.outfile_ext, "rxn") == 0)
      p.out_ext = OEXT_RXN;
   else if (strcmp(p.outfile_ext, "rdf") == 0)
      p.out_ext = OEXT_RDF;
   else if (strcmp(p.outfile_ext, "cml") == 0)
      p.out_ext = OEXT_CML;

   // guess whether to layout or render by extension
   p.action = ACTION_LAYOUT;
   if (p.out_ext == OEXT_OTHER) {
      indigoSetOption("render-output-format", p.outfile_ext);
      p.action = ACTION_RENDER;
   }

   // read in the input
   reader = (p.file_to_load != NULL) ? indigoReadFile(p.file_to_load) : indigoReadString(p.string_to_load);

   if (p.mode == MODE_SINGLE_MOLECULE) {

      if (p.id != NULL)
         ERROR("on single input, setting '-id' is not allowed\n");

      if (p.out_ext == OEXT_RXN)
         ERROR("reaction output specified for molecule input\n");

      if (p.smarts_set)
         obj = indigoLoadSmarts(reader);
      else if (p.query_set)
         obj = indigoLoadQueryMolecule(reader);
      else
         obj = indigoLoadMolecule(reader);

      _prepare(obj, p.aromatization);
      if (p.action == ACTION_LAYOUT) {
         indigoLayout(obj);
         if (p.out_ext == OEXT_MOL)
            indigoSaveMolfileToFile(obj, p.outfile);
         else
            indigoSaveCmlToFile(obj, p.outfile);
      } else {
         _setComment(obj, &p);
         renderToFile(obj, p.outfile);
      }
   } else if (p.mode == MODE_SINGLE_REACTION) {
      if (p.id != NULL)
         ERROR("on single input, setting '-id' is not allowed\n"); 

      if (p.out_ext == OEXT_MOL)
         ERROR("molecule output specified for reaction input\n"); 

      if (p.smarts_set)
         obj = indigoLoadReactionSmarts(reader);
      else if (p.query_set)
         obj = indigoLoadQueryReaction(reader);
      else
         obj = indigoLoadReaction(reader);
      _prepare(obj, p.aromatization);
      if (p.action == ACTION_LAYOUT) {
         indigoLayout(obj);
         if (p.out_ext == OEXT_CML)
            indigoSaveCmlToFile(obj, p.outfile);
         else
            indigoSaveRxnfileToFile(obj, p.outfile);
      } else {
         _setComment(obj, &p);
         renderToFile(obj, p.outfile);
      }
   } else  {
      int item;
      int have_percent_s = (strstr(p.outfile, "%s") != NULL);

      if (p.mode == MODE_MULTILINE_SMILES)
         obj = indigoIterateSmiles(reader);
      else if (p.mode == MODE_SDF)
         obj = indigoIterateSDF(reader);
      else if (p.mode == MODE_MULTIPLE_CML)
         obj = indigoIterateCML(reader);
      else if (p.mode == MODE_RDF)
         obj = indigoIterateRDF(reader);
      else {
         fprintf(stderr, "internal error: wrong branch\n");
         return -1;
      }

      if ((p.out_ext == OEXT_MOL || p.out_ext == OEXT_RXN || p.out_ext == OEXT_OTHER) && !have_percent_s)
         ERROR("on multiple output, output file name must have '%%s'\n");

      if (p.out_ext == OEXT_SDF || p.out_ext == OEXT_RDF ||
         (p.out_ext == OEXT_CML && !have_percent_s))
      {
         writer = indigoWriteFile(p.outfile);
         if (p.out_ext == OEXT_RDF)
            indigoRdfHeader(writer);
         if (p.out_ext == OEXT_CML)
            indigoCmlHeader(writer);
      }

      i = -1;

      while ((item = indigoNext(obj))) {
         int rc;
         ++i;

         if (writer > 0)
            printf("saving item #%d... ", i);
         else
         {
            if (p.id) {
               if (!indigoHasProperty(item, p.id))  {
                  fprintf(stderr, "item #%d does not have %s, skipping\n", i, p.id);
                  continue;
               }
               id = indigoGetProperty(item, p.id);

               snprintf(outfilename, sizeof(outfilename), p.outfile, id);
            } else {
               snprintf(number, sizeof(number), "%d", i);
               snprintf(outfilename, sizeof(outfilename), p.outfile, number);
            }
            printf("saving %s... ", outfilename);
         }

         indigoSetErrorHandler(0, 0);

         if (_prepare(item, p.aromatization) < 0)
         {
            printf("%s\n", indigoGetLastError());
            indigoSetErrorHandler(onError, 0);
            continue;
         }

         if (p.action == ACTION_LAYOUT)
         {
            if (indigoLayout(item) < 0)
            {
               printf("%s\n", indigoGetLastError());
               indigoSetErrorHandler(onError, 0);
               continue;
            }
         }

         if (writer > 0) {
            if (p.out_ext == OEXT_SDF)
               rc = indigoSdfAppend(writer, item);
            else if (p.out_ext == OEXT_RDF)
               rc = indigoRdfAppend(writer, item);
            else
               rc = indigoCmlAppend(writer, item);
         } else {
            if (p.action == ACTION_LAYOUT) {
               if (p.out_ext == OEXT_MOL)
                  rc = indigoSaveMolfileToFile(item, outfilename);
               else if (p.out_ext == OEXT_RXN)
                  rc = indigoSaveRxnfileToFile(item, outfilename);
               else
                  ERROR("extension unexpected");
            } else {
               _setComment(item, &p);
               rc = indigoRenderToFile(item, outfilename);
            }
         }

         if (rc < 0)
         {
            printf("%s\n", indigoGetLastError());
            indigoSetErrorHandler(onError, 0);
            continue;
         }

         indigoFree(item);
         indigoSetErrorHandler(onError, 0);
         printf("\n");
      }

      if (writer > 0)
      {
         if (p.out_ext == OEXT_CML)
            indigoCmlFooter(writer);
         indigoFree(writer);
      }
   }

   indigoFree(reader);
   indigoFree(obj);

   return 0;
}
