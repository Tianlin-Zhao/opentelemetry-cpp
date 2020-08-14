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

#include <stdio.h>
#include <iterator>
#include <string>
#include <vector>
#include "curl/curl.h"

#pragma once

namespace HTTP_SERVER_NS
{

// Because libcurl global init is not thread safe, at most one HttpClients object
// should be standing at the same time only.
class HttpClients
{
public:
  // Intialize the environment for all clients
  HttpClients() { curl_global_init(CURL_GLOBAL_ALL); }

  // Stop libcurl
  ~HttpClients() { curl_global_cleanup(); }

  class HttpClient
  {
  public:
    HttpClient()
    {
      res  = CURLcode();
      curl = curl_easy_init();
    }

    // clean up the curl
    ~HttpClient()
    {
      curl_slist_free_all(list);
      curl_easy_cleanup(curl);
    }

    // Sending the request stored and return true if communication successful,
    // and false otherwise.
    bool SendRequest(std::string url)
    {
      if (curl)
      {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1);
      }
      res = curl_easy_perform(curl);
      if (res != CURLE_OK)
        return false;
      else
        return true;
    }

    // Set the headers of the post request to be transmitted.
    void SetHeaders(std::vector<std::string> headers)
    {
      for (std::vector<std::string>::iterator it = headers.begin(); it != headers.end(); it++)
      {
        list = curl_slist_append(list, (*it).c_str());
      }
    }

    // Set the POST fields of the post request to be transmitted, need to percent-encode '=' sign
    // in trace state to "%3D"
    void AddPostField(std::string post_field_name, std::string post_field_value)
    {
      char *name  = curl_easy_escape(curl, post_field_name.c_str(), 0);
      char *value = curl_easy_escape(curl, post_field_value.c_str(), 0);
      fields += "&" + std::string(name) + "=" + std::string(value);
    }

  private:
    CURL *curl;
    CURLcode res;
    struct curl_slist *list = NULL;
    std::string fields;
  };

  HttpClient StartNewClient() { return HttpClient(); }
};
}  // namespace HTTP_SERVER_NS
