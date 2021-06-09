#ifndef __bingo_base_index__
#define __bingo_base_index__

#include "bingo_cf_storage.h"
#include "bingo_exact_storage.h"
#include "bingo_fp_storage.h"
#include "bingo_gross_storage.h"
#include "bingo_lock.h"
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

    class Index
    {
    public:
        typedef enum
        {
            MOLECULE,
            REACTION,
            UNKNOWN
        } IndexType;

        virtual Matcher* createMatcher(const char* type, MatcherQueryData* query_data, const char* options) = 0;

        virtual Matcher* createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp) = 0;

        virtual Matcher* createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit) = 0;

        virtual Matcher* createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit, IndigoObject& fp) = 0;

        virtual void create(const char* location, const MoleculeFingerprintParameters& fp_params, const char* options, int index_id) = 0;

        virtual void load(const char* location, const char* options, int index_id) = 0;

        virtual int add(IndexObject& obj, int obj_id, DatabaseLockData& lock_data) = 0;

        virtual int addWithExtFP(IndexObject& obj, int obj_id, DatabaseLockData& lock_data, IndigoObject& fp) = 0;

        virtual void optimize() = 0;

        virtual void remove(int id) = 0;

        virtual const byte* getObjectCf(int id, int& len) = 0;

        virtual const char* getIdPropertyName() = 0;

        virtual const char* getVersion() = 0;

        virtual IndexType getType() const = 0;

        virtual ~Index(){};
    };

    class BaseIndex : public Index
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
        void create(const char* location, const MoleculeFingerprintParameters& fp_params, const char* options, int index_id) override;

        void load(const char* location, const char* options, int index_id) override;

        int add(IndexObject& obj, int obj_id, DatabaseLockData& lock_data) override;

        int addWithExtFP(IndexObject& obj, int obj_id, DatabaseLockData& lock_data, IndigoObject& fp) override;

        void optimize() override;

        void remove(int id) override;

        const MoleculeFingerprintParameters& getFingerprintParams() const;

        TranspFpStorage& getSubStorage();

        SimStorage& getSimStorage();

        ExactStorage& getExactStorage();

        GrossStorage& getGrossStorage();

        BingoArray<int>& getIdMapping();

        BingoMapping& getBackIdMapping();

        ByteBufferStorage& getCfStorage();

        int getObjectsCount() const;

        const byte* getObjectCf(int id, int& len) override;

        const char* getIdPropertyName() override;

        const char* getVersion() override;

        IndexType getType() const override;

        static IndexType determineType(const char* location);

        ~BaseIndex() override;

    protected:
        BaseIndex(IndexType type);
        IndexType _type;
        bool _read_only;

    private:
        struct _ObjectIndexData
        {
            Array<byte> sub_fp;
            Array<byte> sim_fp;
            ArrayChar cf_str;
            ArrayChar gross_str;
            dword hash;
        };

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

        int _index_id;

        static void _checkOptions(std::map<std::string, std::string>& option_map, bool is_create);

        static size_t _getMinMMfSize(std::map<std::string, std::string>& option_map);

        static size_t _getMaxMMfSize(std::map<std::string, std::string>& option_map);

        static bool _getAccessType(std::map<std::string, std::string>& option_map);

        void _saveProperties(const MoleculeFingerprintParameters& fp_params, int sub_block_size, int sim_block_size, int cf_block_size,
                             std::map<std::string, std::string>& option_map);

        bool _prepareIndexData(IndexObject& obj, _ObjectIndexData& obj_data);

        bool _prepareIndexDataWithExtFP(IndexObject& obj, _ObjectIndexData& obj_data, IndigoObject& fp);

        void _insertIndexData(_ObjectIndexData& obj_data);

        void _mappingCreate();

        void _mappingLoad();

        void _mappingAssign(int obj_id, int base_id);

        void _mappingAdd(int obj_id, int base_id);

        void _mappingRemove(int obj_id);
    };
}; // namespace bingo

#endif // __bingo_base_index__
