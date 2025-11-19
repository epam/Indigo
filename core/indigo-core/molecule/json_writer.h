/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
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

#include "common/base_cpp/exception.h"
#include "common/math/algebra.h"

namespace indigo
{

    class DLLEXPORT JsonWriter
    {
    public:
        typedef rapidjson::Writer<rapidjson::StringBuffer>::Ch Ch;

        explicit JsonWriter(bool is_pretty = false, bool write_document = false) : pretty_json(is_pretty), _write_document(write_document)
        {
            if (_write_document)
            {
                _document.SetNull();
            }
        }

        void Reset(rapidjson::StringBuffer& os)
        {
            if (pretty_json)
            {
                _pretty_writer.Reset(os);
            }
            else
            {
                _writer.Reset(os);
            }

            // Reset document and context only if write_document is enabled
            if (_write_document)
            {
                _document.SetNull();
                while (!_context_stack.empty())
                {
                    _context_stack.pop();
                }
            }
        }

        bool IsComplete() const
        {
            return pretty_json ? _pretty_writer.IsComplete() : _writer.IsComplete();
        }

        int GetMaxDecimalPlaces() const
        {
            return pretty_json ? _pretty_writer.GetMaxDecimalPlaces() : _writer.GetMaxDecimalPlaces();
        }

        void SetMaxDecimalPlaces(int maxDecimalPlaces)
        {
            if (pretty_json)
                _pretty_writer.SetMaxDecimalPlaces(maxDecimalPlaces);
            else
                _writer.SetMaxDecimalPlaces(maxDecimalPlaces);
        }

        bool Null()
        {
            if (_write_document)
            {
                rapidjson::Value value;
                value.SetNull();
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.Null() : _writer.Null();
        }

        bool Bool(bool b)
        {
            if (_write_document)
            {
                rapidjson::Value value(b);
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.Bool(b) : _writer.Bool(b);
        }

        bool Int(int i)
        {
            if (_write_document)
            {
                rapidjson::Value value(i);
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.Int(i) : _writer.Int(i);
        }

        bool Uint(unsigned u)
        {
            if (_write_document)
            {
                rapidjson::Value value(u);
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.Uint(u) : _writer.Uint(u);
        }

        bool Int64(int64_t i64)
        {
            if (_write_document)
            {
                rapidjson::Value value(i64);
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.Uint64(i64) : _writer.Uint64(i64);
        }

        bool Uint64(uint64_t u64)
        {
            if (_write_document)
            {
                rapidjson::Value value(u64);
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.Uint64(u64) : _writer.Uint64(u64);
        }

        bool Double(double d)
        {
            if (_write_document)
            {
                rapidjson::Value value(d);
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.Double(d) : _writer.Double(d);
        }

        bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false)
        {
            return pretty_json ? _pretty_writer.RawNumber(str, length, copy) : _writer.RawNumber(str, length, copy);
        }

        bool String(const Ch* str, rapidjson::SizeType length, bool copy = false)
        {
            if (_write_document)
            {
                rapidjson::Value value(str, length, _document.GetAllocator());
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.String(str, length, copy) : _writer.String(str, length, copy);
        }

        bool StartObject()
        {
            if (_write_document)
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
            return pretty_json ? _pretty_writer.StartObject() : _writer.StartObject();
        }

        bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false)
        {
            if (_write_document)
            {
                if (!_context_stack.empty())
                {
                    _context_stack.top().pending_key = std::string(str, length);
                }
                return true;
            }
            return pretty_json ? _pretty_writer.Key(str, length, copy) : _writer.Key(str, length, copy);
        }

        bool EndObject(rapidjson::SizeType memberCount = 0)
        {
            if (_write_document)
            {
                if (!_context_stack.empty())
                {
                    _context_stack.pop();
                }
                return true;
            }
            return pretty_json ? _pretty_writer.EndObject(memberCount) : _writer.EndObject(memberCount);
        }

        bool StartArray()
        {
            if (_write_document)
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
            return pretty_json ? _pretty_writer.StartArray() : _writer.StartArray();
        }

        bool EndArray(rapidjson::SizeType elementCount = 0)
        {
            if (_write_document)
            {
                if (!_context_stack.empty())
                {
                    _context_stack.pop();
                }
                return true;
            }
            return pretty_json ? _pretty_writer.EndArray(elementCount) : _writer.EndArray(elementCount);
        }

        bool String(const Ch* const& str)
        {
            if (_write_document)
            {
                rapidjson::Value value(str, _document.GetAllocator());
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.String(str) : _writer.String(str);
        }

        bool String(const std::string& str)
        {
            if (_write_document)
            {
                rapidjson::Value value(str.c_str(), static_cast<rapidjson::SizeType>(str.size()), _document.GetAllocator());
                AddValueToDocument(std::move(value));
                return true;
            }
            return pretty_json ? _pretty_writer.String(str.c_str(), static_cast<rapidjson::SizeType>(str.size()))
                               : _writer.String(str.c_str(), static_cast<rapidjson::SizeType>(str.size()));
        }

        bool Key(const Ch* const& str)
        {
            if (_write_document)
            {
                if (!_context_stack.empty())
                {
                    _context_stack.top().pending_key = str;
                }
                return true;
            }
            return pretty_json ? _pretty_writer.Key(str) : _writer.Key(str);
        }

        bool Key(const std::string& str)
        {
            if (_write_document)
            {
                if (!_context_stack.empty())
                {
                    _context_stack.top().pending_key = str;
                }
                return true;
            }
            return pretty_json ? _pretty_writer.Key(str.c_str()) : _writer.Key(str.c_str());
        }

        bool RawValue(const Ch* json, size_t length, rapidjson::Type type)
        {
            return pretty_json ? _pretty_writer.RawValue(json, length, type) : _writer.RawValue(json, length, type);
        }

        void Flush()
        {
            if (pretty_json)
                _pretty_writer.Flush();
            else
                _writer.Flush();
        }

        void WritePoint(const Vec2f& point)
        {
            if (pretty_json)
            {
                _pretty_writer.StartObject();
                _pretty_writer.Key("x");
                _pretty_writer.Double(point.x);
                _pretty_writer.Key("y");
                _pretty_writer.Double(point.y);
                _pretty_writer.Key("z");
                _pretty_writer.Double(0.0);
                _pretty_writer.EndObject(); // end position
            }
            else
            {
                _writer.StartObject();
                _writer.Key("x");
                _writer.Double(point.x);
                _writer.Key("y");
                _writer.Double(point.y);
                _writer.Key("z");
                _writer.Double(0.0);
                _writer.EndObject(); // end position
            }
        }

        void WriteRect(const Rect2f& rect)
        {
            if (pretty_json)
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
            else
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
        }

        // Get reference to the internal rapidjson::Document
        rapidjson::Document& GetDocument()
        {
            return _document;
        }

        const rapidjson::Document& GetDocument() const
        {
            return _document;
        }

    private:
        struct Context
        {
            rapidjson::Value* value;
            bool is_array;
            std::string pending_key;
        };

        bool pretty_json;
        bool _write_document;
        rapidjson::Writer<rapidjson::StringBuffer> _writer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> _pretty_writer;

        // Document for duplicating all writes
        rapidjson::Document _document;
        std::stack<Context> _context_stack;

        // Helper method to add value to current context
        void AddValueToDocument(rapidjson::Value&& value)
        {
            if (!_write_document)
            {
                return;
            }

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
