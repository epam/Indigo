#include "../item.h"

#ifndef __item_model_impl__
#define __item_model_impl__

namespace indigo2
{

    class ItemModelImpl : public virtual Item
    {
    public:
        GETTER(id_t, ID); // id to use as ref when serialized

        GETSETTER(std::string, name);
        GETSETTER(std::string, uri);

    private:
        int ID;
        std::string name;
        std::string uri;
        std::mutex lock;
    };

} // namespace indigo2
#endif