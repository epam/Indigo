
#include <base_cpp/scanner.h>
#include <molecule/molecule.h>
#include <molecule/molecule_fingerprint.h>
#include <molecule/smiles_loader.h>

using namespace indigo;

namespace
{
    constexpr  int fp_item_size = 68;

    MoleculeFingerprintParameters fingerprintParams{false, SimilarityType::ECFP2, 0, 0, 0, 8};

    struct FingerprintData
    {
        const byte fingerprint[fp_item_size - sizeof(int)];
        int position;
    };

    FingerprintData fp(const char* buffer, int begin, int size)
    {
        try
        {
            BufferScanner scanner(&buffer[begin], size);
            SmilesLoader loader(scanner);
            Molecule mol;
            loader.loadMolecule(mol);
            MoleculeFingerprintBuilder fingerprintBuilder(mol, fingerprintParams);
            fingerprintBuilder.process();
            const auto fp = fingerprintBuilder.get();
            FingerprintData result{};
            result.position = begin;
            memcpy((void*)result.fingerprint, fp, fp_item_size - sizeof(int));
            return result;
        }
        catch (Exception& e)
        {
            std::cerr << e.message() << std::endl;
            FingerprintData result{};
            result.position = -1;
            return result;
        }
    }
}
