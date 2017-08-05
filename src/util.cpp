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

#include "util.hpp"

#include <string>

namespace node_unix_socketpair {

char const * toString(napi_valuetype type) {
	switch (type) {
		case napi_undefined:  return "undefined";
		case napi_null:       return "null";
		case napi_boolean:    return "boolean";
		case napi_number:     return "number";
		case napi_string:     return "string";
		case napi_symbol:     return "symbol";
		case napi_object:     return "object";
		case napi_function:   return "function";
		case napi_external:   return "external";
	};
	return "unknown";
}

maybe_napi_valuetype type_of(napi_env env, maybe_napi_value value) {
	if (!value) return value.status;
	napi_valuetype result;
	napi_status status = napi_typeof(env, value.value, &result);
	return {status, result};
}

maybe_napi_value wrapInt(napi_env env, int value) {
	napi_value result;
	napi_status status = napi_create_number(env, value, &result);
	return {status, result};
}

maybe_napi_value wrapString(napi_env env, std::string const & value) {
	napi_value result;
	napi_status status = napi_create_string_utf8(env, value.data(), value.size(), &result);
	return {status, result};
}

maybe_napi_value makeFunction(napi_env env, char const * name, napi_callback callback, void * data) {
	napi_value result;
	napi_status status = napi_create_function(env, name, callback, data, &result);
	return {status, result};
}

maybe_value<int64_t> unwrapInt(napi_env env, maybe_napi_value value) {
	if (!value) return value.status;
	int64_t result;
	napi_status status = napi_get_value_int64(env, value.value, &result);
	return {status, result};
}

maybe_napi_value getProperty(napi_env env, maybe_napi_value object, maybe_napi_value key) {
	if (!object) return object.status;
	if (!key)    return key.status;
	napi_value result;
	napi_status status = napi_get_property(env, object.value, key.value, &result);
	return {status, result};
}

maybe_napi_value getProperty(napi_env env, maybe_napi_value object, int key) {
	return getProperty(env, object, wrapInt(env, key));
}

maybe_napi_value getProperty(napi_env env, maybe_napi_value object, std::string key) {
	return getProperty(env, object, wrapString(env, key));
}

maybe_value<void> setProperty(napi_env env, maybe_napi_value object, maybe_napi_value key, maybe_napi_value value) {
	if (!object) return object.status;
	if (!key)    return key.status;
	if (!value)  return value.status;
	return napi_set_property(env, object.value, key.value, value.value);
}

maybe_value<void> setProperty(napi_env env, maybe_napi_value object, int key, maybe_napi_value value) {
	return setProperty(env, object, wrapInt(env, key), value);
}

maybe_value<void> setProperty(napi_env env, maybe_napi_value object, std::string key, maybe_napi_value value) {
	return setProperty(env, object, wrapString(env, key), value);
}

maybe_napi_value getElement(napi_env env, maybe_napi_value array, int index) {
	if (!array) return array.status;
	napi_value result;
	napi_status status = napi_get_element(env, array.value, index, &result);
	return {status, result};
}

maybe_value<void> setElement(napi_env env, maybe_napi_value array, int index, maybe_napi_value value) {
	if (!array) return array.status;
	if (!value) return value.status;
	napi_status status = napi_set_element(env, array.value, index, value.value);
	return {status};
}

napi_value null(napi_env env) {
	napi_value result;
	napi_status status = napi_get_null(env, &result);
	if (status != napi_ok) napi_fatal_error(nullptr, "failed to get null instance");
	return result;
}

napi_value undefined(napi_env env) {
	napi_value result;
	napi_status status = napi_get_undefined(env, &result);
	if (status != napi_ok) napi_fatal_error(nullptr, "failed to get undefined instance");
	return result;
}

maybe_value<napi_extended_error_info const *> getErrorInfo(napi_env env) {
	napi_extended_error_info const * info;
	napi_status status = napi_get_last_error_info(env, &info);
	return {status, info};
}

std::string getErrorMessage(napi_env env) {
	maybe_value<napi_extended_error_info const *> info = getErrorInfo(env);
	if (!info) return "";
	return info.value->error_message;
}

napi_value raise(napi_env env, std::string const & message) {
	napi_throw_error(env, message.c_str());
	return undefined(env);
}

napi_value handleError(napi_env env, napi_status original, std::string const & details) {
	std::string message;
	if (details.size()) message += details + ": ";
	message += "napi call failed with status " + std::to_string(int(original));

	std::string error_details = getErrorMessage(env);
	if (error_details.size()) message += std::string{": "} + error_details;

	return raise(env, message.c_str());
}

napi_value raiseTypeError(napi_env env, std::string const & message) {
	napi_throw_type_error(env, message.c_str());
	return undefined(env);
}

napi_value handleTypeError(napi_env env, std::string const & details, napi_value source, napi_valuetype wanted) {
	std::string message = "invalid type";
	if (details.size()) message += " for " + details;
	message += std::string(", expected ") + toString(wanted);

	{
		maybe_napi_valuetype result = type_of(env, source);
		if (result) message += std::string(", got ") + toString(result.value);
	}

	return raiseTypeError(env, message.c_str());
}

}
