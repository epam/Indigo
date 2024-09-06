#include <cstdint>

#include "../common/math/algebra.h"

namespace indigo
{

    // Constants for EMF record types
    constexpr uint32_t EMR_HEADER = 1;
    constexpr uint32_t EMR_STRETCHDIBITS = 81;
    constexpr uint32_t EMR_SETDIBITSTODEVICE = 80;
    constexpr uint32_t EMR_EOF = 14;

    struct BITMAPINFOHEADER
    {
        uint32_t biSize;
        int32_t biWidth;
        int32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t biXPelsPerMeter;
        int32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };

    // Bitmap structure
    struct Bitmap
    {
        std::string dibits;
        int x;
        int y;
        int width;
        int height;
        int bitmap_width;
        int bitmap_height;
    };

    // EMF Record structure
    struct EMFRecord
    {
        uint32_t iType; //!< Type of EMR record
        uint32_t nSize; //!< Size of entire record in bytes (multiple of 4).
    };

    struct EMREOF
    {
        EMFRecord emr;
        uint32_t nPalEntries;
        uint32_t offPalEntries;
        uint32_t nSizeLast;
    };

    // EMR_SETDIBITSTODEVICE record structure

    struct EMRSetDIBitsToDevice
    {
        EMFRecord header;
        int32_t boundsLeft;
        int32_t boundsTop;
        int32_t boundsRight;
        int32_t boundsBottom;
        int32_t xDest;
        int32_t yDest;
        int32_t xSrc;
        int32_t ySrc;
        int32_t cxSrc;
        int32_t cySrc;
        uint32_t offBmiSrc;
        uint32_t cbBmiSrc;
        uint32_t offBits;
        uint32_t cbBits;
        uint32_t iUsageSrc;
        uint32_t iStartScan;
        uint32_t cScans;
    };

    // EMR_STRETCHDIBITS record structure
    struct EMRStretchDIBits
    {
        EMFRecord emr;
        int32_t boundsLeft;
        int32_t boundsTop;
        int32_t boundsRight;
        int32_t boundsBottom;
        int32_t xDest;
        int32_t yDest;
        int32_t xSrc;
        int32_t ySrc;
        uint32_t cxSrc;
        uint32_t cySrc;
        uint32_t offBmiSrc;
        uint32_t cbBmiSrc;
        uint32_t offBits;
        uint32_t cbBits;
        uint32_t iUsageSrc;
        uint32_t dwRop;
        uint32_t cxDest;
        uint32_t cyDest;
    };

    struct EMFHeader
    {
        EMFRecord emr;
        int32_t boundsLeft;
        int32_t boundsTop;
        int32_t boundsRight;
        int32_t boundsBottom;
        int32_t frameLeft;
        int32_t frameTop;
        int32_t frameRight;
        int32_t frameBottom;
        uint32_t dSignature;
        uint32_t nVersion;
        uint32_t nBytes;
        uint32_t nRecords;
        uint16_t nHandles;
        uint16_t sReserved;
        uint32_t nDescription;
        uint32_t offDescription;
        uint32_t nPalEntries;
        int32_t deviceCx;
        int32_t deviceCy;
        int32_t millimetersCx;
        int32_t millimetersCy;
        uint32_t cbPixelFormat;
        uint32_t offPixelFormat;
        uint32_t bOpenGL;
        int32_t micrometersCx;
        int32_t micrometersCy;
    };

    std::string dibToPNG(const std::string& dib_data);

    std::vector<Bitmap> ripBitmapsFromEMF(const std::string& emf);

    std::string decodePNG(const std::string& inputPNGData);

    std::string createEMFFromBitmap(const std::string& bmpData);

} // namespace indigo