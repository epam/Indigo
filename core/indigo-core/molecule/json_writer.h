/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an \"AS IS\" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __json_writer_h__
#define __json_writer_h__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stack>
#include <string>

#include "common/base_cpp/exception.h"
#include "common/math/algebra.h"

namespace indigo
{
    // Forward declarations
    class CompactJsonWriter;
    class PrettyJsonWriter;
    class DocumentJsonWriter;

    // Interface for JSON writers
    class DLLEXPORT JsonWriter
    {
    public:
        using Ch = char;

        static std::unique_ptr<JsonWriter> createJsonWriter(bool pretty);
        static std::unique_ptr<JsonWriter> createJsonDocumentWriter();

        virtual ~JsonWriter() = default;

        // Rule of five - prevent copying but allow moving
        JsonWriter(const JsonWriter&) = delete;
        JsonWriter& operator=(const JsonWriter&) = delete;
        JsonWriter(JsonWriter&&) = default;
        JsonWriter& operator=(JsonWriter&&) = default;

        virtual void Reset(rapidjson::StringBuffer& buffer) = 0;
        virtual bool IsComplete() const = 0;
        virtual int GetMaxDecimalPlaces() const = 0;
        virtual void SetMaxDecimalPlaces(int maxDecimalPlaces) = 0;

        virtual bool Null() = 0;
        virtual bool Bool(bool value) = 0;
        virtual bool Int(int value) = 0;
        virtual bool Uint(unsigned value) = 0;
        virtual bool Int64(int64_t value) = 0;
        virtual bool Uint64(uint64_t value) = 0;
        virtual bool Double(double value) = 0;
        virtual bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false) = 0;
        virtual bool String(const Ch* str, rapidjson::SizeType length, bool copy = false) = 0;
        virtual bool String(const Ch* const& str) = 0;
        virtual bool String(const std::string& str) = 0;

        virtual bool StartObject() = 0;
        virtual bool EndObject(rapidjson::SizeType memberCount = 0) = 0;
        virtual bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false) = 0;
        virtual bool Key(const Ch* const& str) = 0;
        virtual bool Key(const std::string& str) = 0;

        virtual bool StartArray() = 0;
        virtual bool EndArray(rapidjson::SizeType elementCount = 0) = 0;

        virtual bool RawValue(const Ch* json, size_t length, rapidjson::Type type) = 0;
        virtual void Flush() = 0;

        virtual void WritePoint(const Vec2f& point) = 0;
        virtual void WriteRect(const Rect2f& rect) = 0;

        // For DocumentJsonWriter - returns reference to internal document
        virtual rapidjson::Document& GetDocument()
        {
            throw Exception("GetDocument() not supported by this writer");
        }

        virtual const rapidjson::Document& GetDocument() const
        {
            throw Exception("GetDocument() not supported by this writer");
        }

    protected:
        JsonWriter() = default;
    };

    // Compact JSON writer - writes to string buffer without pretty printing
    class DLLEXPORT CompactJsonWriter : public JsonWriter
    {
    public:
        void Reset(rapidjson::StringBuffer& buffer) override
        {
            _writer.Reset(buffer);
        }

        bool IsComplete() const override
        {
            return _writer.IsComplete();
        }

        int GetMaxDecimalPlaces() const override
        {
            return _writer.GetMaxDecimalPlaces();
        }

        void SetMaxDecimalPlaces(int maxDecimalPlaces) override
        {
            _writer.SetMaxDecimalPlaces(maxDecimalPlaces);
        }

        bool Null() override
        {
            return _writer.Null();
        }

        bool Bool(bool value) override
        {
            return _writer.Bool(value);
        }

        bool Int(int value) override
        {
            return _writer.Int(value);
        }

        bool Uint(unsigned value) override
        {
            return _writer.Uint(value);
        }

        bool Int64(int64_t value) override
        {
            return _writer.Int64(value);
        }

        bool Uint64(uint64_t value) override
        {
            return _writer.Uint64(value);
        }

        bool Double(double value) override
        {
            return _writer.Double(value);
        }

        bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false) override
        {
            return _writer.RawNumber(str, length, copy);
        }

        bool String(const Ch* str, rapidjson::SizeType length, bool copy = false) override
        {
            return _writer.String(str, length, copy);
        }

        bool String(const Ch* const& str) override
        {
            return _writer.String(str);
        }

        bool String(const std::string& str) override
        {
            return _writer.String(str.c_str(), static_cast<rapidjson::SizeType>(str.size()));
        }

        bool StartObject() override
        {
            return _writer.StartObject();
        }

        bool EndObject(rapidjson::SizeType memberCount = 0) override
        {
            return _writer.EndObject(memberCount);
        }

        bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false) override
        {
            return _writer.Key(str, length, copy);
        }

        bool Key(const Ch* const& str) override
        {
            return _writer.Key(str);
        }

        bool Key(const std::string& str) override
        {
            return _writer.Key(str.c_str());
        }

        bool StartArray() override
        {
            return _writer.StartArray();
        }

        bool EndArray(rapidjson::SizeType elementCount = 0) override
        {
            return _writer.EndArray(elementCount);
        }

        bool RawValue(const Ch* json, size_t length, rapidjson::Type type) override
        {
            return _writer.RawValue(json, length, type);
        }

        void Flush() override
        {
            _writer.Flush();
        }

        void WritePoint(const Vec2f& point) override
        {
            _writer.StartObject();
            _writer.Key("x");
            _writer.Double(point.x);
            _writer.Key("y");
            _writer.Double(point.y);
            _writer.Key("z");
            _writer.Double(0.0);
            _writer.EndObject();
        }

        void WriteRect(const Rect2f& rect) override
        {
            _writer.StartObject();
            _writer.Key("x");
            _writer.Double(rect.left());
            _writer.Key("y");
            _writer.Double(rect.top());
            _writer.Key("width");
            _writer.Double(rect.width());
            _writer.Key("height");
            _writer.Double(rect.height());
            _writer.EndObject();
        }

    private:
        friend std::unique_ptr<JsonWriter> JsonWriter::createJsonWriter(bool pretty);
        CompactJsonWriter() = default;

        rapidjson::Writer<rapidjson::StringBuffer> _writer;
    };

    // Pretty JSON writer - writes to string buffer with pretty printing
    class DLLEXPORT PrettyJsonWriter : public JsonWriter
    {
    public:
        void Reset(rapidjson::StringBuffer& buffer) override
        {
            _pretty_writer.Reset(buffer);
        }

        bool IsComplete() const override
        {
            return _pretty_writer.IsComplete();
        }

        int GetMaxDecimalPlaces() const override
        {
            return _pretty_writer.GetMaxDecimalPlaces();
        }

        void SetMaxDecimalPlaces(int maxDecimalPlaces) override
        {
            _pretty_writer.SetMaxDecimalPlaces(maxDecimalPlaces);
        }

        bool Null() override
        {
            return _pretty_writer.Null();
        }

        bool Bool(bool value) override
        {
            return _pretty_writer.Bool(value);
        }

        bool Int(int value) override
        {
            return _pretty_writer.Int(value);
        }

        bool Uint(unsigned value) override
        {
            return _pretty_writer.Uint(value);
        }

        bool Int64(int64_t value) override
        {
            return _pretty_writer.Int64(value);
        }

        bool Uint64(uint64_t value) override
        {
            return _pretty_writer.Uint64(value);
        }

        bool Double(double value) override
        {
            return _pretty_writer.Double(value);
        }

        bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false) override
        {
            return _pretty_writer.RawNumber(str, length, copy);
        }

        bool String(const Ch* str, rapidjson::SizeType length, bool copy = false) override
        {
            return _pretty_writer.String(str, length, copy);
        }

        bool String(const Ch* const& str) override
        {
            return _pretty_writer.String(str);
        }

        bool String(const std::string& str) override
        {
            return _pretty_writer.String(str.c_str(), static_cast<rapidjson::SizeType>(str.size()));
        }

        bool StartObject() override
        {
            return _pretty_writer.StartObject();
        }

        bool EndObject(rapidjson::SizeType memberCount = 0) override
        {
            return _pretty_writer.EndObject(memberCount);
        }

        bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false) override
        {
            return _pretty_writer.Key(str, length, copy);
        }

        bool Key(const Ch* const& str) override
        {
            return _pretty_writer.Key(str);
        }

        bool Key(const std::string& str) override
        {
            return _pretty_writer.Key(str.c_str());
        }

        bool StartArray() override
        {
            return _pretty_writer.StartArray();
        }

        bool EndArray(rapidjson::SizeType elementCount = 0) override
        {
            return _pretty_writer.EndArray(elementCount);
        }

        bool RawValue(const Ch* json, size_t length, rapidjson::Type type) override
        {
            return _pretty_writer.RawValue(json, length, type);
        }

        void Flush() override
        {
            _pretty_writer.Flush();
        }

        void WritePoint(const Vec2f& point) override
        {
            _pretty_writer.StartObject();
            _pretty_writer.Key("x");
            _pretty_writer.Double(point.x);
            _pretty_writer.Key("y");
            _pretty_writer.Double(point.y);
            _pretty_writer.Key("z");
            _pretty_writer.Double(0.0);
            _pretty_writer.EndObject();
        }

        void WriteRect(const Rect2f& rect) override
        {
            _pretty_writer.StartObject();
            _pretty_writer.Key("x");
            _pretty_writer.Double(rect.left());
            _pretty_writer.Key("y");
            _pretty_writer.Double(rect.top());
            _pretty_writer.Key("width");
            _pretty_writer.Double(rect.width());
            _pretty_writer.Key("height");
            _pretty_writer.Double(rect.height());
            _pretty_writer.EndObject();
        }

    private:
        friend std::unique_ptr<JsonWriter> JsonWriter::createJsonWriter(bool pretty);
        PrettyJsonWriter() = default;

        rapidjson::PrettyWriter<rapidjson::StringBuffer> _pretty_writer;
    };

    // Document JSON writer - writes to rapidjson::Document
    class DLLEXPORT DocumentJsonWriter : public JsonWriter
    {
    public:
        void Reset(rapidjson::StringBuffer& /*buffer*/) override
        {
            _document.SetNull();
            while (!_context_stack.empty())
            {
                _context_stack.pop();
            }
        }

        bool IsComplete() const override
        {
            return _context_stack.empty();
        }

        int GetMaxDecimalPlaces() const override
        {
            return -1; // Not applicable for document writer
        }

        void SetMaxDecimalPlaces(int /*maxDecimalPlaces*/) override
        {
            // Not applicable for document writer
        }

        bool Null() override
        {
            rapidjson::Value value;
            value.SetNull();
            AddValueToDocument(std::move(value));
            return true;
        }

        bool Bool(bool value) override
        {
            rapidjson::Value json_value(value);
            AddValueToDocument(std::move(json_value));
            return true;
        }

        bool Int(int value) override
        {
            rapidjson::Value json_value(value);
            AddValueToDocument(std::move(json_value));
            return true;
        }

        bool Uint(unsigned value) override
        {
            rapidjson::Value json_value(value);
            AddValueToDocument(std::move(json_value));
            return true;
        }

        bool Int64(int64_t value) override
        {
            rapidjson::Value json_value(value);
            AddValueToDocument(std::move(json_value));
            return true;
        }

        bool Uint64(uint64_t value) override
        {
            rapidjson::Value json_value(value);
            AddValueToDocument(std::move(json_value));
            return true;
        }

        bool Double(double value) override
        {
            rapidjson::Value json_value(value);
            AddValueToDocument(std::move(json_value));
            return true;
        }

        bool RawNumber(const Ch* str, rapidjson::SizeType length, bool /*copy*/) override
        {
            // For document writer, we parse the number and store it
            rapidjson::Value value;
            value.SetString(str, length, _document.GetAllocator());
            AddValueToDocument(std::move(value));
            return true;
        }

        bool String(const Ch* str, rapidjson::SizeType length, bool /*copy*/) override
        {
            rapidjson::Value value(str, length, _document.GetAllocator());
            AddValueToDocument(std::move(value));
            return true;
        }

        bool String(const Ch* const& str) override
        {
            rapidjson::Value value(str, _document.GetAllocator());
            AddValueToDocument(std::move(value));
            return true;
        }

        bool String(const std::string& str) override
        {
            rapidjson::Value value(str.c_str(), static_cast<rapidjson::SizeType>(str.size()), _document.GetAllocator());
            AddValueToDocument(std::move(value));
            return true;
        }

        bool StartObject() override
        {
            rapidjson::Value* obj_ptr = nullptr;
            if (_context_stack.empty())
            {
                _document.SetObject();
                obj_ptr = &_document;
            }
            else
            {
                rapidjson::Value obj(rapidjson::kObjectType);
                AddValueToDocument(std::move(obj));

                // Get pointer to the just-added object
                const Context& ctx = _context_stack.top();
                if (ctx.is_array)
                {
                    obj_ptr = &(*ctx.value)[ctx.value->Size() - 1];
                }
                else
                {
                    // For objects, we need to find the member we just added
                    auto member_it = ctx.value->MemberBegin() + (ctx.value->MemberCount() - 1);
                    obj_ptr = &member_it->value;
                }
            }

            _context_stack.push({obj_ptr, false, ""});
            return true;
        }

        bool EndObject(rapidjson::SizeType /*memberCount*/) override
        {
            if (!_context_stack.empty())
            {
                _context_stack.pop();
            }
            return true;
        }

        bool Key(const Ch* str, rapidjson::SizeType length, bool /*copy*/) override
        {
            if (!_context_stack.empty())
            {
                _context_stack.top().pending_key = std::string(str, length);
            }
            return true;
        }

        bool Key(const Ch* const& str) override
        {
            if (!_context_stack.empty())
            {
                _context_stack.top().pending_key = str;
            }
            return true;
        }

        bool Key(const std::string& str) override
        {
            if (!_context_stack.empty())
            {
                _context_stack.top().pending_key = str;
            }
            return true;
        }

        bool StartArray() override
        {
            rapidjson::Value* arr_ptr = nullptr;
            if (_context_stack.empty())
            {
                _document.SetArray();
                arr_ptr = &_document;
            }
            else
            {
                rapidjson::Value arr(rapidjson::kArrayType);
                AddValueToDocument(std::move(arr));

                // Get pointer to the just-added array
                const Context& ctx = _context_stack.top();
                if (ctx.is_array)
                {
                    arr_ptr = &(*ctx.value)[ctx.value->Size() - 1];
                }
                else
                {
                    // For objects, we need to find the member we just added
                    auto member_it = ctx.value->MemberBegin() + (ctx.value->MemberCount() - 1);
                    arr_ptr = &member_it->value;
                }
            }

            _context_stack.push({arr_ptr, true, ""});
            return true;
        }

        bool EndArray(rapidjson::SizeType /*elementCount*/) override
        {
            if (!_context_stack.empty())
            {
                _context_stack.pop();
            }
            return true;
        }

        bool RawValue(const Ch* json, size_t length, rapidjson::Type /*type*/) override
        {
            // Parse the raw JSON and add it to the document
            rapidjson::Document temp_doc;
            temp_doc.Parse(json, length);
            if (!temp_doc.HasParseError())
            {
                rapidjson::Value value;
                value.CopyFrom(temp_doc, _document.GetAllocator());
                AddValueToDocument(std::move(value));
                return true;
            }
            return false;
        }

        void Flush() override
        {
            // Nothing to flush for document writer
        }

        void WritePoint(const Vec2f& point) override
        {
            StartObject();
            Key("x");
            Double(point.x);
            Key("y");
            Double(point.y);
            Key("z");
            Double(0.0);
            EndObject(0);
        }

        void WriteRect(const Rect2f& rect) override
        {
            StartObject();
            Key("x");
            Double(rect.left());
            Key("y");
            Double(rect.top());
            Key("width");
            Double(rect.width());
            Key("height");
            Double(rect.height());
            EndObject(0);
        }

        rapidjson::Document& GetDocument() override
        {
            return _document;
        }

        const rapidjson::Document& GetDocument() const override
        {
            return _document;
        }

    private:
        friend std::unique_ptr<JsonWriter> JsonWriter::createJsonDocumentWriter();

        DocumentJsonWriter()
        {
            _document.SetNull();
        }

        struct Context
        {
            rapidjson::Value* value;
            bool is_array;
            std::string pending_key;
        };

        rapidjson::Document _document;
        std::stack<Context> _context_stack;

        void AddValueToDocument(rapidjson::Value&& value)
        {
            if (_context_stack.empty())
            {
                // Root level - swap with document
                _document.Swap(value);
            }
            else
            {
                Context& ctx = _context_stack.top();
                if (ctx.is_array)
                {
                    // Add to array
                    ctx.value->PushBack(std::move(value), _document.GetAllocator());
                }
                else
                {
                    // Add to object with pending key
                    if (!ctx.pending_key.empty())
                    {
                        rapidjson::Value key(ctx.pending_key.c_str(), _document.GetAllocator());
                        ctx.value->AddMember(std::move(key), std::move(value), _document.GetAllocator());
                        ctx.pending_key.clear();
                    }
                }
            }
        }
    };
} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
