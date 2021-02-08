namespace indigo2
{
    class Serializer // interface
    {
    public:
        template <T>
        virtual void process(const std::string& field_name, std::function<const T&()>& getter,
                             std::function<void(const T&)>& setter) = 0 void process(Item& item) = 0;
        template <T> virtual void process(std::vector<T> item) = 0;
        template <T> virtual void process(std::unordered_map<K, V> item) = 0;
        template <T> virtual void process(std::map<K, V> item) = 0;
        template <T> virtual void process(std::unordered_set<T> item) = 0;
        template <T> virtual void process(std::set<T> item) = 0;
        template <T> virtual void process(std::iterator<T> beg, std::iterator<T> end) = 0;
        virtual void process(std::string value) = 0;
        template <T> virtual void process(T value) = 0;
    };
} // namespace indigo2
