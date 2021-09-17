#include "layout/layout_pattern_holder.h"

using namespace indigo;

IMPL_ERROR(LayoutPatternHolder, "layout_pattern_holder");

LayoutPatternHolder::LayoutPatternHolder()
{
    _initPatterns();
}

ObjArray<PatternLayout>& LayoutPatternHolder::getPatterns()
{
    return _patterns;
}

int LayoutPatternHolder::_pattern_cmp(PatternLayout& p1, PatternLayout& p2, void* context)
{
    long diff = p2.morganCode() - p1.morganCode();

    if (diff != 0)
        return diff;

    diff = p2.vertexCount() + p2.edgeCount() - p1.vertexCount() - p1.edgeCount();

    if (diff != 0)
        return diff;

    diff = p2.vertexCount() - p1.vertexCount();

    if (diff != 0)
        return diff;

    return p2.edgeCount() - p1.edgeCount();
}

void LayoutPatternHolder::_initPatterns()
{
    {
        struct LayoutPattenItem
        {
            enum
            {
                _ADD_ATOM,
                _ADD_BOND,
                _OUTLINE_POINT
            };
            int type;
            int idx_or_type;
            int v1, v2;
            float x, y;
        };

#define BEGIN_PATTERN(name)                                                                                                                                    \
    {                                                                                                                                                          \
        PatternLayout& p = _patterns.push();                                                                                                                   \
        p.setName(name);                                                                                                                                       \
        static LayoutPattenItem _items[] = {

#define ADD_ATOM(idx, x, y) {LayoutPattenItem::_ADD_ATOM, idx, -1, -1, x, y},
#define ADD_BOND(idx1, idx2, type) {LayoutPattenItem::_ADD_BOND, type, idx1, idx2, -1.f, -1.f},
#define OUTLINE_POINT(idx, x, y) {LayoutPattenItem::_OUTLINE_POINT, idx, -1, -1, x, y},
        //#define FIX_PATTERN

#define END_PATTERN()                                                                                                                                          \
    }                                                                                                                                                          \
    ;                                                                                                                                                          \
    for (int i = 0; i < NELEM(_items); i++)                                                                                                                    \
    {                                                                                                                                                          \
        LayoutPattenItem& item = _items[i];                                                                                                                    \
        if (item.type == LayoutPattenItem::_ADD_ATOM)                                                                                                          \
            if (p.addAtom(item.x, item.y) != item.idx_or_type)                                                                                                 \
                throw Error("incorrect atom order in the pattern '%s'", p.getName());                                                                          \
        if (item.type == LayoutPattenItem::_ADD_BOND)                                                                                                          \
            p.addBond(item.v1, item.v2, item.idx_or_type);                                                                                                     \
        if (item.type == LayoutPattenItem::_OUTLINE_POINT)                                                                                                     \
            if (p.addOutlinePoint(item.x, item.y) != item.idx_or_type)                                                                                         \
                throw Error("incorrect outline order in the pattern '%s'", p.getName());                                                                       \
    }                                                                                                                                                          \
    }

#include "layout_patterns.inc"

#undef BEGIN_PATTERN
//#undef FIX_PATTERN
#undef ADD_ATOM
#undef ADD_BOND
#undef OUTLINE_POINT
#undef END_PATTERN

        for (int i = 0; i < _patterns.size(); i++)
            _patterns[i].calcMorganCode();
        _patterns.qsort(_pattern_cmp, 0);
    }
}
