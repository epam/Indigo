#ifndef __graph_edge__
#define __graph_edge__

#include <weak_ptr>
#include <string>

#include "../graph_item/graph_item.h"

namespace indigo2
{
    class GraphVertex;

    class GraphEdge : public virtual GraphItem // interface
    {
    public:
        GETSETTER(std::weak_ptr<GraphConnectionPoint>, beg) = 0;
        GETSETTER(std::weak_ptr<GraphConnectionPoint>, end) = 0;
        GETTER(coord_matrix_t, coord_matrix) = 0; // should calculate values from beg's and end's coordinates
        SETTER(coord_matrix_t, coord_matrix)      // disable setter
        {
        }

    protected:
        REFLECTION
        REFLECTION_FIELD(beg)
        REFLECTION_FIELD(end)
        REFLECTION_END(GraphItem)
    };
} // namespace indigo2

#endif