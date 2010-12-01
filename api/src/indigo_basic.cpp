/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#include "indigo_internal.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/auto_ptr.h"
#include "molecule/molecule.h"

CEXPORT int indigoNext (int iter)
{
   INDIGO_BEGIN
   {
      IndigoObject *nextobj = self.getObject(iter).next();

      if (nextobj == 0)
         return 0;
      
      return self.addObject(nextobj);
   }
   INDIGO_END(-1);
}

CEXPORT int indigoHasNext (int iter)
{
   INDIGO_BEGIN
   {
      return self.getObject(iter).hasNext() ? 1 : 0;
   }
   INDIGO_END(-1);
}

CEXPORT const char * indigoName (int handle)
{
   INDIGO_BEGIN
   {
      return self.getObject(handle).getName();
   }
   INDIGO_END(0);
}

CEXPORT int indigoIndex (int handle)
{
   INDIGO_BEGIN
   {
      return self.getObject(handle).getIndex();
   }
   INDIGO_END(-1);
}


CEXPORT int indigoClone (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      return self.addObject(obj.clone());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoSetName (int handle, const char *name)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.isBaseMolecule())
         obj.getBaseMolecule().name.readString(name, true);
      else if (obj.isBaseReaction())
         obj.getBaseReaction().name.readString(name, true);
      else
         throw IndigoError("The object provided is neither a molecule, nor a reaction");
      return 1;
   }
   INDIGO_END(-1);
}


CEXPORT int indigoHasProperty (int handle, const char *prop)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      return props->at2(prop) != 0;
   }
   INDIGO_END(-1)
}

const char * indigoGetProperty (int handle, const char *prop)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      self.tmp_string.copy(props->at(prop));
      self.tmp_string.push(0); // just for safety; a zero byte must be already there
      return self.tmp_string.ptr();
   }
   INDIGO_END(0)
}

int indigoSetProperty (int handle, const char *prop, const char *value)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      if (props->at2(prop) != 0)
         props->at(prop).readString(value, true);
      else
         props->value(props->insert(prop)).readString(value, true);
      return 1;
   }
   INDIGO_END(-1)
}

IndigoPropertiesIter::IndigoPropertiesIter (RedBlackStringObjMap< Array<char> > &props) :
IndigoObject(PROPERTIES_ITER),
_props(props)
{
   _idx = -1;
}

IndigoPropertiesIter::~IndigoPropertiesIter ()
{
}

IndigoProperty::IndigoProperty (RedBlackStringObjMap< Array<char> > &props, int idx) :
IndigoObject(PROPERTY),
_props(props),
_idx(idx)
{
}

IndigoProperty::~IndigoProperty ()
{
}

const char * IndigoProperty::getName ()
{
   return _props.key(_idx);
}

Array<char> & IndigoProperty::getValue ()
{
   return _props.value(_idx);
}

int IndigoProperty::getIndex ()
{
   return _idx;
}

bool IndigoPropertiesIter::hasNext ()
{
   if (_idx == -1)
      return _props.begin() != _props.end();

   return _props.next(_idx) != _props.end();
}

IndigoObject * IndigoPropertiesIter::next ()
{
   if (_idx == -1)
      _idx = _props.begin();
   else if (_idx != _props.end())
      _idx = _props.next(_idx);

   if (_idx == _props.end())
      return 0;

   return new IndigoProperty(_props, _idx);
}

int indigoIterateProperties (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      return self.addObject(new IndigoPropertiesIter(*props));
   }
   INDIGO_END(-1)
}


void indigoIgnoreStereochemistryErrors (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.ignore_stereochemistry_errors = (enabled != 0);
}

void indigoTreatXAsPseudoatom (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.treat_x_as_pseudoatom = (enabled != 0);
}

void indigoDeconvolutionAromatization (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.deconvolution_aromatization = (enabled != 0);
}

void indigoSetMolfileSavingMode (const char *mode)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(mode, "2000") == 0)
      self.molfile_saving_mode = MolfileSaver::MODE_2000;
   else if (strcasecmp(mode, "3000") == 0)
      self.molfile_saving_mode = MolfileSaver::MODE_3000;
   else if (strcasecmp(mode, "auto") == 0)
      self.molfile_saving_mode = MolfileSaver::MODE_AUTO;
   else
      throw IndigoError("unknown value: %s", mode);
}

void indigoSetFilenameEncoding (const char *encoding)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(encoding, "ASCII") == 0)
      self.filename_encoding = ENCODING_ASCII;
   else if (strcasecmp(encoding, "UTF-8") == 0)
      self.filename_encoding = ENCODING_UTF8;
   else
      throw IndigoError("unknown value: %s", encoding);
}

void indigoSetFPOrdQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.ord_qwords = qwords;
}

void indigoSetFPSimQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.sim_qwords = qwords;
}

void indigoSetFPTauQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.tau_qwords = qwords;
}

void indigoSetFPAnyQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.any_qwords = qwords;
}

class _IndigoBasicOptionsHandlersSetter
{
public:
   _IndigoBasicOptionsHandlersSetter ();
};

_IndigoBasicOptionsHandlersSetter::_IndigoBasicOptionsHandlersSetter ()
{
   OptionManager &mgr = indigoGetOptionManager();
   OsLocker locker(mgr.lock);

   mgr.setOptionHandlerBool("ignore-stereochemistry-errors", indigoIgnoreStereochemistryErrors);
   mgr.setOptionHandlerBool("treat-x-as-pseudoatom", indigoTreatXAsPseudoatom);
   mgr.setOptionHandlerBool("deconvolution-aromatization", indigoDeconvolutionAromatization);
   mgr.setOptionHandlerString("molfile-saving-mode", indigoSetMolfileSavingMode);
   mgr.setOptionHandlerString("filename-encoding", indigoSetFilenameEncoding);
   mgr.setOptionHandlerInt("fp-ord-qwords", indigoSetFPOrdQwords);
   mgr.setOptionHandlerInt("fp-sim-qwords", indigoSetFPSimQwords);
   mgr.setOptionHandlerInt("fp-any-qwords", indigoSetFPAnyQwords);
   mgr.setOptionHandlerInt("fp-tau-qwords", indigoSetFPTauQwords);
}

_IndigoBasicOptionsHandlersSetter _indigo_basic_options_handlers_setter;
