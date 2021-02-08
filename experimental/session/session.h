#ifndef __session__
#define __session__

#include "../item/item.h"

namespace indigo2
{

    funcRetObj(
        int thisId, enum {GET_EXCEPTION, CLOSE_CALL}  clear, params)
    {
        static std::unordered_map<id_t, struct {std::unique_ptr<void> ret_val, std::exception_ptr exception}> rets
        auto thisObj = Session.getObject<Molecule>(thisId);
        try
        {
            r  = exportObject(thisObj->getAtom());
            if string
                rets[thisObj->id()] = {new char(string.length() + 1), null_ptr};

        }
        catch (ex)
        {
            rets[thisObj->id()] = {null, ex};
        }
        return r;
    }
    string clear(id_t id)
    {
        if (rets.find(id))
        {
            if (rets[id].exception)
                return rets[id].exception.mesage;
            if (!exception || CLOSE_CALL)
                rets.remove(id);
        }
        return nullptr;
    }

    typedef id_t int;

    class Exported
    {
        std::shared_ptr<Item> item;
        std::shared_ptr<Item> parent;
        thread_local std::unique_ptr<char> ret_str;
        thread_local std::unique_ptr<char> exception_str;
    }

    class Session : public virtual Item // interface
    {
        Session default_session = SessionDefaultImpl();
    public:
        exportObject(std::shared_ptr<Item> item, std::shared_ptr<Item> parent)
        {
            exported[item->id()] = std::pair<std::shared_ptr<Item>, std::shared_ptr<Item>>(item, parent);
            return item->id();
        }

        exportObject(std::vector<Item>& item, std::shared_ptr<Item> parent);
        exportObject(std::vector<T>& item, std::shared_ptr<Item> parent);
        exportObject(std::map<T>& item, std::shared_ptr<Item> parent);
        exportObject(std::set<T>& item, std::shared_ptr<Item> parent);

        template<T> T getObject(id_t id)
        {
            return exported[id];
        }

        void dropObject(id_t id)
        {
            exported.remove(id);
        }

        void close()
        {
            exported.clear();
        }




        std::shared_ptr<Atom> atom()
        {
            return std::shared_ptr<Atom>(new Atom());
        }


    protected:
        REFLECTION
        REFLECTION_FIELD(ID)
        REFLECTION_FIELD(name)
        REFLECTION_FIELD(uri)
    }
    private:
        Item()
        {
        }
    }
};
} // namespace indigo2

#endif