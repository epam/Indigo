#ifndef __item__
#define __item__

#include <shared_ptr>
#include <string>

#include "../graph_vertex/graph_vertex.h"

namespace indigo2
{
    class Graph : public virtual GraphVertex // interface
    {
    public:
        GETSETTER(const vector<std::shared_ptr<GraphVertex>>&, vertexes) = 0;
        GETSETTER(const vector<std::shared_ptr<GraphEdge>>&, edges) = 0;
    };
} // namespace indigo2

#endif