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

#include "indigo_internal.h"

#include "molecule/molecule_attachments_search.h"
#include "molecule/molecule_rgroups_composition.h"

class DLLEXPORT IndigoAttachment : public IndigoObject {
public:
    explicit IndigoAttachment(Attachment& at) : IndigoObject(GROSS), str(print(at)) {}
    ~IndigoAttachment() {}

    virtual void toString(Array<char> &out) {
        out.appendString(str, true);
    }

protected:
    const char * str;
};

static std::function<IndigoObject*(Attachment*)> wrap(Indigo& indigo) {
    return[&indigo](Attachment* at) {
        IndigoObject* obj = new IndigoAttachment(*at);
        indigo.addObject(obj);
        return obj;
    };
}

class DLLEXPORT IndigoCompositionIter : public IndigoObject {
public:
    IndigoCompositionIter(Indigo& indigo, BaseMolecule& mol) : IndigoObject(COMPOSITION_ITER),
    iterable(MoleculeRGroupsComposition::refine(mol)), 
    it(map(wrap(indigo), iterable->iterator(), true)) {}
    virtual ~IndigoCompositionIter() { delete iterable; }

    virtual IndigoObject* next() { return it->next(); }
    virtual bool hasNext() { return it->hasNext(); }

protected:
    Iterable<Attachment*>* iterable;
    Iterator<IndigoObject*>* it;
};

CEXPORT int indigoRGroupComposition(int molecule, const char* options)
{
   INDIGO_BEGIN
   {
      BaseMolecule& target = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoCompositionIter(self, target));
   }
   INDIGO_END(-1);
}

template<>
class DLLEXPORT Cleaner<IndigoObject*> : public Iterator<IndigoObject*>{
public:
    explicit Cleaner(bool clean) {}
protected:
    IndigoObject* remember(IndigoObject* t) { return t; }
    void          release() {}
private:
    bool clean;
};