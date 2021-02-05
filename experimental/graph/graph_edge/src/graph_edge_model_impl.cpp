#include "graph_edge_model_impl.h"
#include <string>

using namespace indigo2;

GETSETTER_MODEL_IMPL(GraphEdgeModelImpl, std::shared_ptr<GraphConnectionPoint>, beg);
GETSETTER_MODEL_IMPL(GraphEdgeModelImpl, std::shared_ptr<GraphConnectionPoint>, end);

coord_matrix_t GraphEdgeModelImpl::coord_matrix() const
{
    coord_matrix_t r;
    coord_matrix_t beg_coord = beg();
    coord_matrix_t end_coord = end();

    /* calculate r from beg_coord and end_coord*/

    return r;

}
