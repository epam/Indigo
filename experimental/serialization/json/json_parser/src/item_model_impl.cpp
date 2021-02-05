#include "item_model_impl.h"
#include <string>

using namespace indigo2;

GETTER_MODEL_IMPL(ItemImpl, std::string, ID)
GETSETTER_MODEL_IMPL(ItemImpl, std::string, name)
GETSETTER_MODEL_IMPL(ItemImpl, std::string, uri)


