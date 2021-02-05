#ifndef __graph_edge_model_impl__
#define __graph_edge_model_impl__

#include <weak_ptr>
#include <string>

#include "../graph_edge.h"

namespace indigo2
{
    class GraphVertex;

    class GraphEdgeModelImpl : public virtual GraphEdge, virtual GtaphItemModelImpl 
    {
    public:
        GETSETTER(std::weak_ptr<GraphConnectionPoint>, beg);
        GETSETTER(std::weak_ptr<GraphConnectionPoint>, end);
        GETTER(coord_matrix_t, coord_matrix); 
    };
} // namespace indigo2

#endif