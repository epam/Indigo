#ifndef __item__
#define __item__

#include "../../../item/item.h"
#include "../../../reflection/reflection.h"

#include <mutex>
#include <string>
#include <vector>

class JsonWriter : public virtual FieldsScanner
{
    thread_local ostream& out;

public:
    JsonWriter(ostream& _out) : out(_out)
    {
    }

    to_json(Item& item)
    {
        out << "{";
        item.reflection(*this);
        out << "}";
    }

    template <T> to_json(std::vector<T> item)
    {
        out << "[";
        std::for_each(item.beg(), item.end(), [this](auto e) { to_json(e); out << "," });
        out << "]";
    }

    template <T> to_json(std::unordered_map<K,V> item)
    {
        out << "{";
        std::for_each(item.beg(), item.end(), [this](auto e) {
            out << "\"" << escape(e.first) << "\"=";
            to_json(e.second);
            out << ","
        });
        out << "}";
    }

    template <T> to_json(std::map<K, V> item)
    {
        out << "{";
        std::for_each(item.beg(), item.end(), [this](auto e) {
            out << "\"" << escape(e.first) << "\"=";
            to_json(e.second);
            out << ","
        });
        out << "}";
    }

    to_json(std::string value)
    {
        out << "\"" << escape(value) << "\"";
    }

    template <T> to_json(T value)
    {
        out << value;
    }

    template <T> virtual void process(const std::string& field_name, std::function<const T&()>& getter, std::function<void(const T&)>& setter)
    {
        out << "\"" << escape(field_name) << "\""
            << "=";
        to_json(getter());
        out << ",";
    }
};
} // namespace indigo2

#endif