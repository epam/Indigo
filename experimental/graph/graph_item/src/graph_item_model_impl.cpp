#include "graph_item_model_impl.h"
#include <string>

using namespace indigo2;

GETTER_MODEL_IMPL(std::shared_ptr<Graph>, graph);
GETSETTER_MODEL_IMPL(std::string, label);
GETSETTER_MODEL_IMPL(std::string, style);
GETSETTER_MODEL_IMPL(bool, selected);
GETSETTER_MODEL_IMPL(bool, highlighted);
GETSETTER_MODEL_IMPL(coord_matrix_t, coord_matrix);

