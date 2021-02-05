#ifndef __graph_item__
#define __graph_item__

#include <shared_ptr>
#include <string>

#include "../item/item.h"
#include <vector>

namespace indigo2
{
    typedef coord_matrix_t float[9]; // [x, y, z /*center (diagonals intersection point) of circumscribed parallepiped*/,
                                     // size_x, size_y, size_z /*dimensions of circumscribed parallepiped*/,
                                     // rot_x, rot_y, rot_z /*rotation of circumscribed parallepiped*/,]

    class Graph;
    class GraphItem : public Item // interface
    {

    public:
        GETTER(std::shared_ptr<Graph>, graph) = 0;
        GETSETTER(std::string, label) = 0;
        GETSETTER(std::unordered_map<std::string, std::string>, style) = 0;
        GETSETTER(bool, selected) = 0;
        GETSETTER(bool, highlighted) = 0;
        GETSETTER(coord_matrix_t, coord_matrix) = 0;

    protected:
        REFLECTION
        REFLECTION_FIELD(graph)
        REFLECTION_FIELD(label)
        REFLECTION_FIELD(style)
        REFLECTION_FIELD(selected)
        REFLECTION_FIELD(highlighted)
        REFLECTION_END(Item)
    };
} // namespace indigo2

#endif