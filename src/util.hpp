// Copyright 2017, Maarten de Vries <maarten@de-vri.es>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <node_api.h>

#include <string>

namespace node_unix_socketpair {

template<typename T>
struct maybe_value {
	napi_status status;
	T value;

	maybe_value(napi_status status, T const & value) : status{status}, value{value} {};
	maybe_value(napi_status status) : maybe_value{status, T{}} {};
	maybe_value(T const & value) : maybe_value{napi_ok, value} {};

	explicit operator bool() { return status == napi_ok; }
};

template<>
struct maybe_value<void> {
	napi_status status;

	maybe_value(napi_status status) : status{status} {};

	explicit operator bool() { return status == napi_ok; }
};

using maybe_napi_value     = maybe_value<napi_value>;
using maybe_napi_valuetype = maybe_value<napi_valuetype>;

char const * toString(napi_valuetype type);

maybe_napi_valuetype type_of(napi_env env, maybe_napi_value value);

maybe_napi_value wrapInt(napi_env env, int value);

maybe_napi_value wrapString(napi_env env, std::string const & value);

maybe_napi_value makeFunction(napi_env env, char const * name, napi_callback callback, void * data = nullptr);

maybe_value<int64_t> unwrapInt(napi_env env, maybe_napi_value value);

maybe_napi_value getProperty(napi_env env, maybe_napi_value object, maybe_napi_value key);
maybe_napi_value getProperty(napi_env env, maybe_napi_value object, std::string key);
maybe_value<void> setProperty(napi_env env, maybe_napi_value object, maybe_napi_value key, maybe_napi_value value);
maybe_value<void> setProperty(napi_env env, maybe_napi_value object, std::string key, maybe_napi_value value);

maybe_napi_value getElement(napi_env env, maybe_napi_value array, int index);
maybe_value<void> setElement(napi_env env, maybe_napi_value array, int index, maybe_napi_value value);

napi_value null(napi_env env);
napi_value undefined(napi_env env);

maybe_value<napi_extended_error_info const *> getErrorInfo(napi_env env);
std::string getErrorMessage(napi_env env);

napi_value raise(napi_env env, std::string const & message);
napi_value handleError(napi_env env, napi_status original, std::string const & message);

napi_value raiseTypeError(napi_env env, std::string const & message);
napi_value handleTypeError(napi_env env, std::string const & details, napi_value source, napi_valuetype wanted);

}
