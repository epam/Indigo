#ifndef __bingo_base_index__
#define __bingo_base_index__

#include "molecule/molecule_fingerprint.h"

#include "indigo_internal.h"

#include "bingo_cf_storage.h"
#include "bingo_exact_storage.h"
#include "bingo_fp_storage.h"
#include "bingo_gross_storage.h"
#include "bingo_object.h"
#include "bingo_properties.h"
#include "bingo_sim_storage.h"
#include "mmf/mmf_mapping.h"

#define BINGO_VERSION "v0.72"

namespace bingo
{
    class Matcher;
    class MatcherQueryData;

    enum class IndexType
    {
        MOLECULE,
        REACTION,
        UNKNOWN
    };

    struct ObjectIndexData
    {
        Array<byte> sub_fp;
        Array<byte> sim_fp;
        Array<char> cf_str;
        Array<char> gross_str;
        dword hash;
    };

    class BaseIndex
    {
    private:
        struct _Header
        {
            MMFAddress properties_offset;
            MMFAddress mapping_offset;
            MMFAddress back_mapping_offset;
            MMFAddress cf_offset;
            MMFAddress sub_offset;
            MMFAddress sim_offset;
            MMFAddress exact_offset;
            MMFAddress gross_offset;
            int object_count;
            int first_free_id;
        };

    public:
        virtual ~BaseIndex();

        virtual std::unique_ptr<Matcher> createMatcher(const char* type, MatcherQueryData* query_data, const char* options) = 0;
        virtual std::unique_ptr<Matcher> createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp) = 0;
        virtual std::unique_ptr<Matcher> createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit) = 0;
        virtual std::unique_ptr<Matcher> createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit,
                                                                    IndigoObject& fp) = 0;

        void create(const char* location, const MoleculeFingerprintParameters& fp_params, const char* options, int index_id);

        void load(const char* location, const char* options, int index_id);

        int add(int obj_id, const ObjectIndexData&);

        void optimize();

        void remove(int id);

        const MoleculeFingerprintParameters& getFingerprintParams() const;

        TranspFpStorage& getSubStorage();

        SimStorage& getSimStorage();

        ExactStorage& getExactStorage();

        GrossStorage& getGrossStorage();

        MMFArray<int>& getIdMapping();

        MMFMapping& getBackIdMapping();

        ByteBufferStorage& getCfStorage();

        int getObjectsCount() const;

        const byte* getObjectCf(int id, int& len);

        const char* getIdPropertyName() const;

        const char* getVersion();

        IndexType getType() const;

        static IndexType determineType(const char* location);

        ObjectIndexData prepareIndexData(IndexObject& obj) const;
        ObjectIndexData prepareIndexDataWithExtFP(IndexObject& obj, IndigoObject& fp) const;

    protected:
        BaseIndex(IndexType type);
        IndexType _type;
        bool _read_only;

    private:
        MMFPtr<_Header> _header;
        MMFPtr<MMFArray<int>> _id_mapping_ptr;
        MMFPtr<MMFMapping> _back_id_mapping_ptr;
        MMFPtr<TranspFpStorage> _sub_fp_storage;
        MMFPtr<SimStorage> _sim_fp_storage;
        MMFPtr<ExactStorage> _exact_storage;
        MMFPtr<GrossStorage> _gross_storage;
        MMFPtr<ByteBufferStorage> _cf_storage;
        MMFPtr<Properties> _properties;

        MoleculeFingerprintParameters _fp_params;
        std::string _location;
        int _lock_fd = -1;

        static void _checkOptions(std::map<std::string, std::string>& option_map, bool is_create);

        static size_t _getMinMMfSize(std::map<std::string, std::string>& option_map);

        static size_t _getMaxMMfSize(std::map<std::string, std::string>& option_map);

        static bool _getAccessType(std::map<std::string, std::string>& option_map);

        void _saveProperties(const MoleculeFingerprintParameters& fp_params, int sub_block_size, int sim_block_size, int cf_block_size,
                             std::map<std::string, std::string>& option_map);

        void _insertIndexData(const ObjectIndexData& obj_data);

        void _mappingCreate();

        void _mappingLoad();

        void _mappingAssign(int obj_id, int base_id);

        void _mappingAdd(int obj_id, int base_id);

        void _mappingRemove(int obj_id);
    };
}

#endif // __bingo_base_index__
