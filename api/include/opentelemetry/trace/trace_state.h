// Copyright 2020, OpenTelemetry Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <exception>

#include "opentelemetry/nostd/span.h"
#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/nostd/unique_ptr.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace trace
{

/**
 * TraceState carries tracing-system specific context in a list of key-value pairs. TraceState
 * allows different vendors to propagate additional information and inter-operate with their legacy
 * id formats.
 *
 * For more information, see the W3C Trace Context specification:
 * https://www.w3.org/TR/trace-context
 */
class TraceState
{
public:
  static constexpr int kKeyMaxSize       = 256;
  static constexpr int kValueMaxSize     = 256;
  static constexpr int kMaxKeyValuePairs = 32;

  // Class to store key-value pairs.
  class Entry
  {
  public:
    Entry() : key_(nullptr), value_(nullptr){};

    // Copy constructor
    Entry(Entry &copy)
    {
      key_   = CopyStringToPointer(copy.key_.get());
      value_ = CopyStringToPointer(copy.value_.get());
    }

    // Copy assignment operator
    Entry &operator=(Entry &other)
    {
      key_   = CopyStringToPointer(other.key_.get());
      value_ = CopyStringToPointer(other.value_.get());
      return *this;
    }

    // Move constructor and assignment operator
    Entry(Entry &&other) = default;
    Entry(const Entry &other) {
      key_   = CopyStringToPointer(other.key_.get());
      value_ = CopyStringToPointer(other.value_.get());
    }
    Entry &operator=(Entry &&other) = default;

    // Creates an Entry for a given key-value pair.
    Entry(nostd::string_view key, nostd::string_view value) noexcept
    {
      key_   = CopyStringToPointer(key);
      value_ = CopyStringToPointer(value);
    }

    // Gets the key associated with this entry.
    nostd::string_view GetKey() const { return key_.get(); }

    // Gets the value associated with this entry.
    nostd::string_view GetValue() const { return value_.get(); }

    // Sets the value for this entry. This overrides the previous value.
    void SetValue(nostd::string_view value) { value_ = CopyStringToPointer(value); }

  private:
    // Store key and value as raw char pointers to avoid using std::string.
    nostd::unique_ptr<const char[]> key_;
    nostd::unique_ptr<const char[]> value_;

    // Copies string into a buffer and returns a unique_ptr to the buffer.
    // This is a workaround for the fact that strcpy doesn't accept a const char* destination.
    nostd::unique_ptr<const char[]> CopyStringToPointer(nostd::string_view str)
    {
      try {
        std::cout<<"A"<<std::endl;
        std::cout<<str<<std::endl;
        nostd::unique_ptr<char[]> temp(new char[str.size() + 1]);
        std::cout<<"B"<<std::endl;
        strcpy(temp.get(), str.data());
        std::cout<<"C"<<std::endl;
        return nostd::unique_ptr<const char[]>(temp.release());
      } catch (std::exception& e) {
        std::cout << e.what() << '\n';
        throw;
      }
    }
  };

  // An empty TraceState.
  TraceState() noexcept : entries_(new Entry[kMaxKeyValuePairs]), num_entries_(0) {}

  // movable and copiable
  TraceState(TraceState &&trace_state)
  {
    std::cout<<"moving"<<std::endl;
    entries_.reset(new Entry[kMaxKeyValuePairs]);
    std::cout<<"moving1"<<std::endl;
    num_entries_ = 0;
    for (const auto &entry : trace_state.Entries())
    {
      std::cout<<"moving2"<<std::endl;
      Entry copy = entry;
      (entries_.get())[num_entries_] = Entry(copy);
      num_entries_++;
    }
    std::cout<<"moving3"<<std::endl;
  }

  TraceState(const TraceState &trace_state)
  {
    std::cout<<"copying"<<std::endl;
    entries_.reset(new Entry[kMaxKeyValuePairs]);
    num_entries_ = 0;
    for (const auto &entry : trace_state.Entries())
    {
      Entry copy = entry;
      (entries_.get())[num_entries_] = Entry(copy);
      num_entries_++;
    }
    std::cout<<"copying2"<<std::endl;
  }

  bool operator==(const TraceState &that) const noexcept
  {
    if (num_entries_ != that.num_entries_)
      return false;
    nostd::string_view value;
    for (const auto &entry : that.Entries())
    {
      Get(entry.GetKey(), value);
      if (value != entry.GetValue())
        return false;
    }
    return true;
  }

  // Returns false if no such key, otherwise returns true and populates the value parameter with the
  // associated value.
  bool Get(nostd::string_view key, nostd::string_view &value) const noexcept
  {
    for (const auto &entry : Entries())
    {
      if (key == entry.GetKey())
      {
        value = entry.GetValue();
        return true;
      }
    }
    return false;
  }

  // Creates an Entry for the key-value pair and adds it to entries. Returns true if pair was added
  // succesfully, false otherwise. If value is null or entries_ is full, this function is a no-op.
  bool Set(nostd::string_view key, nostd::string_view value) noexcept
  {
    std::cout<<"setting: "<<key<<" "<<value<<std::endl;
    if (value.empty() || num_entries_ >= kMaxKeyValuePairs)
    {
      return false;
    }
    Entry entry(key, value);
    (entries_.get())[num_entries_] = entry;

    std::cout<<"set "<<(entries_.get())[num_entries_].GetKey().data()<<std::endl;
    num_entries_++;
    std::cout<<"setting complete"<<std::endl;
    return true;
  }

  // Returns true if there are no keys, false otherwise.
  bool Empty() const noexcept { return num_entries_ == 0; }

  // Returns a span of all the entries. The TraceState object must outlive the span.
  nostd::span<Entry> Entries() const noexcept
  {
    return nostd::span<Entry>(entries_.get(), num_entries_);
  }

  // Returns whether key is a valid key. See https://www.w3.org/TR/trace-context/#key
  static bool IsValidKey(nostd::string_view key)
  {
    if (key.empty() || key.size() > kKeyMaxSize || !IsLowerCaseAlphaOrDigit(key[0]))
    {
      return false;
    }

    int ats = 0;

    for (const char c : key)
    {
      if (!IsLowerCaseAlphaOrDigit(c) && c != '_' && c != '-' && c != '@' && c != '*' && c != '/')
      {
        return false;
      }
      if ((c == '@') && (++ats > 1))
      {
        return false;
      }
    }
    return true;
  }

  // Returns whether value is a valid value. See https://www.w3.org/TR/trace-context/#value
  static bool IsValidValue(nostd::string_view value)
  {
    if (value.empty() || value.size() > kValueMaxSize)
    {
      return false;
    }

    for (const char c : value)
    {
      if (c < ' ' || c > '~' || c == ',' || c == '=')
      {
        return false;
      }
    }
    return true;
  }

private:
  // Store entries in a C-style array to avoid using std::array or std::vector.
  nostd::unique_ptr<Entry[]> entries_;

  // Maintain the number of entries in entries_. Must be in the range [0, kMaxKeyValuePairs].
  int num_entries_;

  static bool IsLowerCaseAlphaOrDigit(char c) { return isdigit(c) || islower(c); }
};

}  // namespace trace
OPENTELEMETRY_END_NAMESPACE
