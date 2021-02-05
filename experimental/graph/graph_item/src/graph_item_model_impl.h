#ifndef __graph_item_model_impl__
#define __graph_item_model_impl__

#include <weak_ptr>
#include <string>

#include "../graph_item.h"

namespace indigo2
{
    class Graph;
    class GraphItemModelImpl : public virtual GraphItem, public virtual ItemModelImpl
    {
    public:
        GETTER(std::weak_ptr<Graph>, graph);
        GETSETTER(std::string, label);
        GETSETTER(std::string, style);
        GETSETTER(bool, selected);
        GETSETTER(bool, highlighted);
        GETSETTER(coord_matrix_t, coord_matrix);
        
    private:
        std::shared_ptr<Graph> graph;
        std::string label;
        std::string style;
        bool selected;
        bool highlighted;
        coord_matrix_t coord_matrix;
    };
} // namespace indigo2

#endif