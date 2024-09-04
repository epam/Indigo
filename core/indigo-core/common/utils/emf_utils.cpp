#include <png.h>
#include <sstream>
#include <vector>

#include "emf_utils.h"

using namespace indigo;

// pnglib needs a custom write function for user-defined IO
void png_write_to_stream(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::stringstream* stream = reinterpret_cast<std::stringstream*>(png_get_io_ptr(png_ptr));
    stream->write(reinterpret_cast<char*>(data), length);
}

void png_flush_noop(png_structp)
{
    // No need to flush for a stringstream
}

// Error handling
void png_error_handler(png_structp png_ptr, png_const_charp error_msg)
{
    throw std::runtime_error(error_msg);
}

void png_warning_handler(png_structp png_ptr, png_const_charp warning_msg)
{
    // Warnings can be ignored or logged
}

std::string indigo::dibToPNG(const std::string& dib_data)
{
    if (dib_data.empty())
        return "";

    BITMAPINFOHEADER* pbmi = (BITMAPINFOHEADER*)dib_data.data();
    int width = pbmi->biWidth;
    int height = pbmi->biHeight;
    int bpp = pbmi->biBitCount;

    if (bpp != 24 && bpp != 16)
    {
        return "";
    }

    uint8_t* pixels = (uint8_t*)(dib_data.data() + pbmi->biSize);
    std::vector<uint8_t> png_data;
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
        return "";

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return "";
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return "";
    }

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    std::vector<std::vector<uint8_t>> row_data(height, std::vector<uint8_t>(width * 3));
    std::vector<png_bytep> row_pointers(height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (bpp == 24)
            {
                row_data[height - 1 - i][3 * j + 0] = pixels[(i * width + j) * 3 + 2];
                row_data[height - 1 - i][3 * j + 1] = pixels[(i * width + j) * 3 + 1];
                row_data[height - 1 - i][3 * j + 2] = pixels[(i * width + j) * 3 + 0];
            }
            else if (bpp == 16)
            {
                uint16_t pixel = ((uint16_t*)pixels)[i * width + j];

                uint8_t r = (pixel & 0x7C00) >> 7;
                uint8_t g = (pixel & 0x3E0) >> 2;
                uint8_t b = (pixel & 0x1F) << 3;

                row_data[height - 1 - i][3 * j + 0] = r;
                row_data[height - 1 - i][3 * j + 1] = g;
                row_data[height - 1 - i][3 * j + 2] = b;
            }
        }
        row_pointers[height - 1 - i] = row_data[height - 1 - i].data();
    }

    png_set_rows(png_ptr, info_ptr, row_pointers.data());

    struct PngWriteCallback
    {
        static void callback(png_structp png_ptr, png_bytep data, png_size_t length)
        {
            auto p = reinterpret_cast<std::vector<uint8_t>*>(png_get_io_ptr(png_ptr));
            p->insert(p->end(), data, data + length);
        }
    };

    png_set_write_fn(png_ptr, &png_data, PngWriteCallback::callback, NULL);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return std::string(png_data.begin(), png_data.end());
}

std::vector<Bitmap> indigo::ripBitmapsFromEMF(const std::string& emf)
{
    std::vector<Bitmap> bitmaps;
    const uint8_t* data = reinterpret_cast<const uint8_t*>(emf.data());
    const uint8_t* ptr = data;
    const uint8_t* end = data + emf.size();
    // Read EMF records
    while (ptr < end)
    {
        if (end - ptr < sizeof(EMFRecord))
            break;

        const EMFRecord* record = reinterpret_cast<const EMFRecord*>(ptr);
        if (record->nSize < sizeof(EMFRecord) || ptr + record->nSize > end)
            break;

        switch (record->iType)
        {
        case EMR_HEADER:
            // Handle EMR_HEADER if needed
            break;
        case EMR_EOF:
            // Handle EMR_EOF if needed
            break;
        case EMR_STRETCHDIBITS: {
            // Extract bitmap data from EMR_STRETCHDIBITS
            const EMRStretchDIBits* dibits = reinterpret_cast<const EMRStretchDIBits*>(ptr);
            if (ptr + dibits->offBmiSrc + dibits->cbBmiSrc + dibits->cbBits > end)
                break; // Ensure we don't read out of bounds

            Bitmap bitmap;
            // bmi header + bitmap data
            bitmap.dibits = std::string(reinterpret_cast<const char*>(ptr + dibits->offBmiSrc), dibits->cbBmiSrc + dibits->cbBits);
            bitmap.x = dibits->xDest;
            bitmap.y = dibits->yDest;
            bitmap.width = dibits->cxDest;
            bitmap.height = dibits->cyDest;
            bitmap.bitmap_width = dibits->cxSrc;
            bitmap.bitmap_height = dibits->cySrc;
            bitmaps.push_back(bitmap);
            break;
        }

        case EMR_SETDIBITSTODEVICE: {
            // Extract bitmap data from EMR_SETDIBITSTODEVICE
            const EMRSetDIBitsToDevice* dibits = reinterpret_cast<const EMRSetDIBitsToDevice*>(ptr);
            if (ptr + dibits->offBits + dibits->cbBits > end)
                break; // Ensure we don't read out of bounds

            Bitmap bitmap;
            bitmap.dibits = std::string(reinterpret_cast<const char*>(ptr + dibits->offBits), dibits->cbBits);
            bitmap.x = dibits->xDest;
            bitmap.y = dibits->yDest;
            bitmap.width = bitmap.bitmap_width = dibits->cxSrc;
            bitmap.height = bitmap.bitmap_height = dibits->cySrc;
            bitmaps.push_back(bitmap);
            break;
        }

        default:
            // Skip other records
            break;
        }

        ptr += record->nSize;
    }

    return bitmaps;
}

void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    png_voidp a = png_get_io_ptr(pngPtr);
    const char** p = (const char**)a;
    memcpy(data, *p, length);
    *p += length;
}

std::string indigo::decodePNG(const std::string& inputPNGData)
{
    if (inputPNGData.empty())
        return "";

    png_const_charp pngDataPtr = inputPNGData.c_str();
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        return "";

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return "";
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return "";
    }

    png_set_read_fn(png_ptr, &pngDataPtr, pngReadData);
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);

    // Handle different PNG color types for conversion to RGBA
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && png_get_bit_depth(png_ptr, info_ptr) < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    if (!(color_type & PNG_COLOR_MASK_ALPHA))
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    png_read_update_info(png_ptr, info_ptr);

    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 32;   // Always use 32 bits per pixel (RGBA)
    bih.biCompression = 0; // Use BI_RGB (no compression)
    bih.biSizeImage = width * height * 4;
    bih.biXPelsPerMeter = 0; // Not specifying resolution
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    std::vector<unsigned char> pixelData(bih.biSizeImage);
    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; ++y)
    {
        row_pointers[y] = &pixelData[(height - 1 - y) * width * 4]; // Flip row order for correct BMP format
    }

    png_read_image(png_ptr, row_pointers.data());
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    std::string output;
    output.append(reinterpret_cast<const char*>(&bih), sizeof(bih));
    output.append(reinterpret_cast<const char*>(pixelData.data()), pixelData.size());

    return output;
}

// this function may be useful in the future. Just implement missing calculations for TBC fields.
std::string indigo::createEMFFromBitmap(const std::string& bmpData)
{
    const BITMAPINFOHEADER* bmpHeader = reinterpret_cast<const BITMAPINFOHEADER*>(bmpData.data());

    std::string emfData;

    EMFHeader emfHeader = {};
    emfHeader.emr.iType = 1; // EMR_HEADER
    emfHeader.emr.nSize = sizeof(EMFHeader);
    emfHeader.boundsLeft = 0;
    emfHeader.boundsTop = 0;
    emfHeader.boundsRight = static_cast<int32_t>(bmpHeader->biWidth - 1);
    emfHeader.boundsBottom = static_cast<int32_t>(bmpHeader->biHeight - 1);
    emfHeader.frameLeft = 0;
    emfHeader.frameTop = 0;
    const float dpi = 96.0f;
    emfHeader.frameRight = 0;  // TBC: 13894
    emfHeader.frameBottom = 0; // TBC: 13894

    emfHeader.dSignature = 0x464D4520;
    emfHeader.nVersion = 0x00010000;
    emfHeader.nBytes = 0;   // Will be updated later
    emfHeader.nRecords = 0; // Will be updated later
    emfHeader.nHandles = 1; // Not used
    emfHeader.sReserved = 0;
    emfHeader.nDescription = 0;
    emfHeader.offDescription = 0;
    emfHeader.nPalEntries = 0;     // No palette
    emfHeader.deviceCx = 1920;     // TBC:
    emfHeader.deviceCy = 1080;     // TBC:
    emfHeader.millimetersCx = 521; // TBC:
    emfHeader.millimetersCy = 293; // TBC:
    emfHeader.micrometersCx = emfHeader.millimetersCx * 1000;
    emfHeader.micrometersCy = emfHeader.millimetersCy * 1000;
    emfHeader.cbPixelFormat = 0;
    emfHeader.offPixelFormat = 0;
    emfHeader.bOpenGL = 0;

    emfData.append(reinterpret_cast<const char*>(&emfHeader), sizeof(emfHeader));
    emfHeader.nRecords++;

    uint32_t currentOffset = static_cast<uint32_t>(emfData.size());

    EMRStretchDIBits emrStretchDIBits = {};
    emrStretchDIBits.emr.iType = 81; // EMR_STRETCHDIBITS
    emrStretchDIBits.emr.nSize = sizeof(EMRStretchDIBits) + bmpHeader->biSize + bmpHeader->biSizeImage;
    emrStretchDIBits.boundsLeft = 0;
    emrStretchDIBits.boundsTop = 0;
    emrStretchDIBits.boundsRight = static_cast<int32_t>(bmpHeader->biWidth - 1);
    emrStretchDIBits.boundsBottom = static_cast<int32_t>(bmpHeader->biHeight - 1);
    emrStretchDIBits.xDest = 0;
    emrStretchDIBits.yDest = 0;
    emrStretchDIBits.xSrc = 0;
    emrStretchDIBits.ySrc = 0;
    emrStretchDIBits.cxSrc = static_cast<uint32_t>(bmpHeader->biWidth);
    emrStretchDIBits.cySrc = static_cast<uint32_t>(bmpHeader->biHeight);
    emrStretchDIBits.offBmiSrc = static_cast<uint32_t>(sizeof(EMRStretchDIBits));
    emrStretchDIBits.cbBmiSrc = bmpHeader->biSize;
    emrStretchDIBits.offBits = static_cast<uint32_t>(emrStretchDIBits.offBmiSrc + bmpHeader->biSize);
    emrStretchDIBits.cbBits = bmpHeader->biSizeImage;
    emrStretchDIBits.iUsageSrc = 0;
    emrStretchDIBits.dwRop = 0x00CC0020; // SRCCOPY
    emrStretchDIBits.cxDest = static_cast<uint32_t>(bmpHeader->biWidth);
    emrStretchDIBits.cyDest = static_cast<uint32_t>(bmpHeader->biHeight);

    emfData.append(reinterpret_cast<const char*>(&emrStretchDIBits), sizeof(emrStretchDIBits));
    emfData.append(reinterpret_cast<const char*>(bmpHeader), bmpHeader->biSize);
    emfData.append(bmpData.data() + sizeof(BITMAPINFOHEADER), bmpHeader->biSizeImage);
    emfHeader.nRecords++;

    EMREOF emrEOF = {};
    emrEOF.emr.iType = 14; // EMR_EOF
    emrEOF.emr.nSize = sizeof(EMREOF);
    emfData.append(reinterpret_cast<const char*>(&emrEOF), sizeof(emrEOF));
    emfHeader.nRecords++;
    emfHeader.nBytes = static_cast<uint32_t>(emfData.size());
    emfData.replace(0, sizeof(emfHeader), reinterpret_cast<const char*>(&emfHeader), sizeof(emfHeader));
    return emfData;
}
