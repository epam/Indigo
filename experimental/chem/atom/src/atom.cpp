#include "../atom.h"

/* methods defined here must access data via interface: chem_element = chem_element()*/
bool Atom::isQueryAtom()
{
    bool r = false;
    r = r || chem_element().size() != 1; // there is alternative: [N,O,P]
    /*etc. checks*/
    return r;
}