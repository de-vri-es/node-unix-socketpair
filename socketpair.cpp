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

#include <node.h>
#include <node_api.h>

#include <cerrno>
#include <cstring>
#include <cstdint>
#include <string>
#include <utility>

#include <sys/socket.h>

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

using maybe_napi_value = maybe_value<napi_value>;

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

maybe_napi_value makeFunction(napi_env env, char const * name, napi_callback callback, void * data = nullptr) {
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

napi_value raise(napi_env env, std::string const & message) {
	napi_throw_error(env, message.c_str());
	return undefined(env);
}

napi_value handleError(napi_env env, napi_status original) {
	maybe_value<napi_extended_error_info const *> info = getErrorInfo(env);
	if (!info) {
		std::string message = "napi call failed with status " + std::to_string(int(original));
		return raise(env, message.c_str());
	}
	std::string message = "napi call failed with status " + std::to_string(int(original)) + ": " + info.value->error_message;
	return raise(env, message.c_str());
}

napi_value socketpair(napi_env env, napi_callback_info info) {
	std::size_t argc;
	napi_value argv;
	napi_value self;
	void * data;

	{
		napi_status status = napi_get_cb_info(env, info, &argc, &argv, &self, &data);
		if (status != napi_ok) return handleError(env, status);
	}

	if (argc != 1) {
		return raise(env, "expected exactly 1 argument: type (SOCK_STREAM or SOCK_DGRAM)");
	}

	maybe_value<int64_t> type = unwrapInt(env, getProperty(env, argv, 0));
	if (!type) return handleError(env, type.status);

	int fds[2];
	{
		int result = ::socketpair(AF_LOCAL, type.value | SOCK_CLOEXEC | SOCK_NONBLOCK, 0, fds);
		if (result != 0) {
			int error = errno;
			return raise(env, "call to socketpair() failed with error " + std::to_string(error) + ": " + std::strerror(error));
		}
	}

	napi_value array;
	{
		maybe_value<void> result = napi_create_array_with_length(env, 2, &array);
		if (!result) return handleError(env, result.status);
	}
	{
		maybe_value<void> result = setProperty(env, array, 0, wrapInt(env, fds[0]));
		if (!result) return handleError(env, result.status);
	}
	{
		maybe_value<void> result = setProperty(env, array, 0, wrapInt(env, fds[0]));
		if (!result) return handleError(env, result.status);
	}

	return undefined(env);
}

void init(napi_env env, napi_value exports, napi_value module, void *) {
	(void) exports;
	maybe_napi_value function = makeFunction(env, "socketpair", socketpair);
	if (!function) handleError(env, function.status);

	maybe_value<void> result{napi_ok};

	result = setProperty(env, function, "SOCK_STREAM", wrapInt(env, SOCK_STREAM));
	if (!result) handleError(env, result.status);

	result = setProperty(env, function, "SOCK_DGRAM",  wrapInt(env, SOCK_DGRAM));
	if (!result) handleError(env, result.status);

	result = setProperty(env, module, "exports", function);
	if (!result) handleError(env, result.status);
}

NAPI_MODULE(unix_socketpair, init)

}
