#ifndef __bingo_base_index__
#define __bingo_base_index__

#include "bingo_cf_storage.h"
#include "bingo_exact_storage.h"
#include "bingo_fp_storage.h"
#include "bingo_gross_storage.h"
#include "bingo_mapping.h"
#include "bingo_mmf_storage.h"
#include "bingo_object.h"
#include "bingo_properties.h"
#include "bingo_sim_storge.h"
#include "indigo_internal.h"
#include "molecule/molecule_fingerprint.h"

#define BINGO_VERSION "v0.72"

using namespace indigo;

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
            BingoAddr properties_offset;
            BingoAddr mapping_offset;
            BingoAddr back_mapping_offset;
            BingoAddr cf_offset;
            BingoAddr sub_offset;
            BingoAddr sim_offset;
            BingoAddr exact_offset;
            BingoAddr gross_offset;
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

        BingoArray<int>& getIdMapping();

        BingoMapping& getBackIdMapping();

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
        MMFStorage _mmf_storage;
        BingoPtr<_Header> _header;
        BingoPtr<BingoArray<int>> _id_mapping_ptr;
        BingoPtr<BingoMapping> _back_id_mapping_ptr;
        BingoPtr<TranspFpStorage> _sub_fp_storage;
        BingoPtr<SimStorage> _sim_fp_storage;
        BingoPtr<ExactStorage> _exact_storage;
        BingoPtr<GrossStorage> _gross_storage;
        BingoPtr<ByteBufferStorage> _cf_storage;
        BingoPtr<Properties> _properties;

        MoleculeFingerprintParameters _fp_params;
        std::string _location;

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
}; // namespace bingo

#endif // __bingo_base_index__
