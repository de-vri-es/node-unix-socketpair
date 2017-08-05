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

#include <node_api.h>

#include <cerrno>
#include <cstring>
#include <cstdint>
#include <string>
#include <utility>

#include <sys/socket.h>

#include <iostream>

namespace node_unix_socketpair {

napi_value socketpair(napi_env env, napi_callback_info info) {
	std::size_t argc = 1;
	napi_value argv[1];
	napi_value self;
	void * data;

	{
		maybe_value<void> result = napi_get_cb_info(env, info, &argc, argv, &self, &data);
		if (!result) return handleError(env, result.status, "napi_get_cb_info");
	}

	maybe_value<int64_t> type = unwrapInt(env, argv[0]);
	if (!type) return handleTypeError(env, "first argument", argv[0], napi_number);

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
		if (!result) return handleError(env, result.status, "create result array");
	}
	{
		maybe_value<void> result = setElement(env, array, 0, wrapInt(env, fds[0]));
		if (!result) return handleError(env, result.status, "set result[0] to int");
	}
	{
		maybe_value<void> result = setElement(env, array, 0, wrapInt(env, fds[0]));
		if (!result) return handleError(env, result.status, "set result[1] to int");
	}

	return array;
}

void init(napi_env env, napi_value exports, napi_value module, void *) {
	(void) exports;
	maybe_napi_value function = makeFunction(env, "socketpair", socketpair);
	if (!function) handleError(env, function.status, "make socketpair() function object");

	maybe_value<void> result{napi_ok};

	result = setProperty(env, function, "SOCK_STREAM", wrapInt(env, SOCK_STREAM));
	if (!result) handleError(env, result.status, "set socketpair.SOCK_STREAM");

	result = setProperty(env, function, "SOCK_DGRAM",  wrapInt(env, SOCK_DGRAM));
	if (!result) handleError(env, result.status, "set socketpair.SOCK_DGRAM");

	result = setProperty(env, module, "exports", function);
	if (!result) handleError(env, result.status, "set export to socketpair function");
}

NAPI_MODULE(unix_socketpair, init)

}
