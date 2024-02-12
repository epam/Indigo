/****************************************************************************
 * Copyright (C) from 2024 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "base_cpp/scanner.h"
#include <fstream>
#include <iostream>
#include <molecule/CDXCommons.h>
#include <molecule/molecule_json_saver.h>
#include <string>
#include <vector>

using namespace indigo;

template <typename T>
std::string toHex(T val)
{
    std::ostringstream oss;
    oss << "0x" << std::setw(sizeof(T) * 2) << std::setfill('0') << std::hex << val;
    return oss.str();
}
template std::string toHex<>(uint16_t);
template std::string toHex<>(uint32_t);

std::string toHex(Array<byte>& arr, int count)
{
    std::ostringstream oss;
    for (int i = 0; i < count; i++)
    {
        if (i > 0)
            oss << ' ';
        oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned short>(arr[i]);
    }
    return oss.str().c_str();
}

template <typename T>
void save_hex(indigo::JsonWriter& json, T val)
{
    std::string hex = toHex(val);
    json.RawNumber(hex.c_str(), static_cast<rapidjson::SizeType>(hex.size()));
}

std::string coordToStr(uint16_t lo, uint16_t hi)
{
    double dlo = lo / 65536.0;
    double dhi = hi;
    return std::to_string(dhi + dlo);
}

template <typename T>
static T read(byte*& ptr, uint32_t& size)
{
    T res = *reinterpret_cast<T*>(ptr);
    unsigned int shift = sizeof(T);
    ptr += shift;
    size -= shift;
    return res;
}
template uint16_t read(byte*&, uint32_t&);

static void saveProperty(uint16_t tag, uint32_t len, Array<byte>& buf, indigo::JsonWriter& json)
{
    json.StartObject();
    json.Key("tag");
    save_hex(json, tag);
    json.Key("len");
    json.Uint(len);
    json.Key("hex");
    json.String(toHex(buf, len).c_str());
    auto it = KCDXPropToName.find(tag);
    if (it != KCDXPropToName.end() || tag == 0x1500 || tag == 0x1501)
    {
        ECDXType type = ECDXType::CDXString;
        if (it != KCDXPropToName.end())
        {
            json.Key("prop_name");
            json.String(it->second.first.c_str());
            type = it->second.second;
        }

        switch (type)
        {
        case ECDXType::CDXCoordinate: {
            uint16_t x_lo = (buf[1] << 8) | buf[0];
            uint16_t x_hi = (buf[3] << 8) | buf[2];
            json.Key("coord");
            json.String(coordToStr(x_lo, x_hi).c_str());
            break;
        }
        case ECDXType::CDXPoint2D: {
            uint16_t y_lo = (buf[1] << 8) | buf[0];
            uint16_t y_hi = (buf[3] << 8) | buf[2];
            uint16_t x_lo = (buf[5] << 8) | buf[4];
            uint16_t x_hi = (buf[7] << 8) | buf[6];
            json.Key("x");
            json.String(coordToStr(x_lo, x_hi).c_str());
            json.Key("y");
            json.String(coordToStr(y_lo, y_hi).c_str());
            break;
        }
        case ECDXType::CDXPoint3D: {
            uint16_t z_lo = (buf[1] << 8) | buf[0];
            uint16_t z_hi = (buf[3] << 8) | buf[2];
            uint16_t y_lo = (buf[5] << 8) | buf[4];
            uint16_t y_hi = (buf[7] << 8) | buf[6];
            uint16_t x_lo = (buf[9] << 8) | buf[8];
            uint16_t x_hi = (buf[11] << 8) | buf[10];
            json.Key("x");
            json.String(coordToStr(x_lo, x_hi).c_str());
            json.Key("y");
            json.String(coordToStr(y_lo, y_hi).c_str());
            json.Key("z");
            json.String(coordToStr(z_lo, z_hi).c_str());
            break;
        }
        case ECDXType::CDXRectangle: {
            uint16_t top_lo = (buf[1] << 8) | buf[0];
            uint16_t top_hi = (buf[3] << 8) | buf[2];
            uint16_t left_lo = (buf[5] << 8) | buf[4];
            uint16_t left_hi = (buf[7] << 8) | buf[6];
            uint16_t bottom_lo = (buf[9] << 8) | buf[8];
            uint16_t bottom_hi = (buf[11] << 8) | buf[10];
            uint16_t right_lo = (buf[13] << 8) | buf[12];
            uint16_t right_hi = (buf[15] << 8) | buf[14];
            json.Key("top");
            json.String(coordToStr(top_lo, top_hi).c_str());
            json.Key("left");
            json.String(coordToStr(left_lo, left_hi).c_str());
            json.Key("bottom");
            json.String(coordToStr(bottom_lo, bottom_hi).c_str());
            json.Key("right");
            json.String(coordToStr(right_lo, right_hi).c_str());
            break;
        }
        case ECDXType::CDXUINT8:
            json.Key("val");
            json.Uint(buf[0]);
            break;
        case ECDXType::CDXUINT16:
            json.Key("val");
            json.Uint(*reinterpret_cast<uint16_t*>(buf.ptr()));
            break;
        case ECDXType::CDXINT8:
            json.Key("val");
            json.Int(*reinterpret_cast<int8_t*>(buf.ptr()));
            break;
        case ECDXType::CDXINT16:
            json.Key("val");
            json.Int(*reinterpret_cast<int16_t*>(buf.ptr()));
            break;
        case ECDXType::CDXString:
            byte* ptr = buf.ptr();
            auto style_runs = read<uint16_t>(ptr, len);
            if (style_runs > 0)
            {
                json.Key("style_runs");
                json.StartArray();
                for (int i = 0; i < style_runs; i++)
                {
                    json.StartObject();
                    json.Key("start");
                    json.Uint(read<uint16_t>(ptr, len));
                    json.Key("font");
                    json.Uint(read<uint16_t>(ptr, len));
                    json.Key("style");
                    save_hex(json, len);
                    json.Key("size");
                    json.Double(read<uint16_t>(ptr, len) / 20.0);
                    json.Key("color");
                    json.Uint(read<uint16_t>(ptr, len));
                    json.EndObject();
                }
                json.EndArray();
            }
            json.Key("str");
            json.String(reinterpret_cast<char*>(ptr), len);
            break;
        }
    }
    json.EndObject();
}

void readProperty(uint16_t tag, Scanner& scan, indigo::JsonWriter& json)
{
    int len = 0;
    uint16_t size;
    scan.read(2, &size);
    len = size;
    Array<byte> buf;
    if (len > 0)
    {
        if (0xFFFF == len)
        {
            scan.read(4, &len);
        }
        buf.expandFill(len, 0);
        scan.read(len, buf.ptr());
    }
    saveProperty(tag, len, buf, json);
}

static void readObject(uint16_t tag, Scanner& scan, indigo::JsonWriter& json);

static void readObjOrProp(uint16_t tag, Scanner& scan, indigo::JsonWriter& json)
{
    if (tag >= kCDXTag_Object)
    {
        readObject(tag, scan, json);
    }
    else
    {
        readProperty(tag, scan, json);
    }
}

static void readObject(uint16_t tag, Scanner& scan, indigo::JsonWriter& json)
{
    json.StartObject();
    json.Key("tag");
    save_hex(json, tag);
    auto it = KCDXObjToName.find(tag);
    if (it != KCDXObjToName.end())
    {
        json.Key("obj_name");
        json.String(it->second.c_str());
    }
    uint32_t id;
    scan.read(4, &id);
    json.Key("id");
    save_hex(json, id);
    json.Key("content");
    json.StartArray();
    while (1)
    {
        scan.read(2, &tag);
        if (tag == 0)
            break;
        readObjOrProp(tag, scan, json);
    }
    json.EndArray();
    json.EndObject();
}

void print_usage()
{
    printf("Usage: cdx-dump [-p] file.cdx\ncdx-dump [-r] file.json file.cdx\n-p for pretty json\n-r for reverse mode - from json to cdx");
}

void parse_cdx(const char* filename, bool pretty_json)
{
    FileScanner sc(filename);
    // Skip header
    sc.seek(22, SEEK_CUR); // VcjD0100 + 0x01020304 + 10 zero bytes

    rapidjson::StringBuffer s;
    indigo::JsonWriter json(pretty_json);
    json.Reset(s);
    uint16_t tag;
    json.StartArray();
    while (1)
    {
        sc.read(2, &tag);
        if (tag == 0) // End of file
            break;
        readObjOrProp(tag, sc, json);
    }
    json.EndArray();
    printf("\n%s\n", s.GetString());
}

template <typename T>
void write(std::ofstream& ofs, T val)
{
    ofs.write(reinterpret_cast<char*>(&val), sizeof T);
}

void save_nodes(std::ofstream& cdx, rapidjson::Value& nodes)
{
    for (rapidjson::SizeType node_idx = 0; node_idx < nodes.Size(); ++node_idx)
    {
        auto& node = nodes[node_idx];
        if (node.HasMember("tag"))
        {
            uint16_t tag = 0xFFFF & std::stol(node["tag"].GetString(), nullptr, 0);
            write(cdx, tag);
            if (tag >= kCDXTag_Object)
            {
                uint32_t id = 0xFFFFFFFF & std::stol(node["id"].GetString(), nullptr, 0);
                write(cdx, id);
                if (node.HasMember("content"))
                {
                    save_nodes(cdx, node["content"]);
                }
                write<uint16_t>(cdx, 0);
            }
            else // save property
            {
                uint32_t llen = node["len"].GetUint();
                uint16_t slen = 0xFFFF;
                if (llen >= slen)
                {
                    write(cdx, slen);
                    write(cdx, llen);
                }
                else
                {
                    slen = slen & llen;
                    write(cdx, slen);
                }
                std::istringstream hex(node["hex"].GetString());
                std::string sbyte;
                while (getline(hex, sbyte, ' '))
                {
                    uint8_t b = 0xFF & std::stoi(sbyte, nullptr, 16);
                    write(cdx, b);
                }
            }
        }
    }
}

void json_to_cdx(const char* json_file_name, const char* cdx_filename)
{
    std::ifstream json_file(json_file_name);
    std::stringstream json;
    json << json_file.rdbuf();
    rapidjson::Document data;
    if (!data.Parse(json.str().c_str()).HasParseError())
    {
        std::ofstream cdx(cdx_filename, std::ios::binary);
        cdx << "VjCD0100";
        write(cdx, kCDXMagicNumber);
        cdx.write(kCDXReserved, sizeof(kCDXReserved));
        save_nodes(cdx, data);
        write<uint16_t>(cdx, 0);
        cdx.close();
    }
    else
    {
        printf("Parse error %d at offset %zu.", data.GetParseError(), data.GetErrorOffset());
    }
}

int main(int argc, char* argv[])
{
    std::vector<std::string_view> input_files;
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    bool pretty_json = false;
    bool reverse = false;
    for (const auto& arg : args)
    {
        if (arg == "-p")
        {
            pretty_json = true;
            continue;
        }
        if (arg == "-r")
        {
            reverse = true;
            continue;
        }
        input_files.push_back(arg);
    }
    if (input_files.empty())
    {
        print_usage();
        return 0;
    }
    if (pretty_json && reverse)
    {
        printf("-p has no sense in reverse mode.\n");
        print_usage();
        return 0;
    }
    if (reverse)
    {
        json_to_cdx(input_files[0].data(), input_files[1].data());
    }
    else
    {
        parse_cdx(input_files[0].data(), pretty_json);
    }
}
