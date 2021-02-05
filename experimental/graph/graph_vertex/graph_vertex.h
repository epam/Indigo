#ifndef __graph_vertex__
#define __graph_vertex__

#include <weak_ptr>
#include <string>

#include "../graph_item/graph_item.h"

namespace indigo2
{
    class GraphVertex : public virtual GraphItem // interface
    {
    public:
        GETSETTER(vector<std::weak_ptr<GraphVertex>>, connection_points) = 0; 
    };
} // namespace indigo2

#endif