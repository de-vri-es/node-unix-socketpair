# unix-socketpair
This add-on exposes the real POSIX socketpair() to Node.js applications.
`socketpair()` is unrivaled in simplicity when you want to create connected sockets.
It allows very easy creation of a bi-directional data channel for a child process implemented in some other language than Node.js.

The sockets are created with the `SOCK_CLOEXEC` and `SOCK_NONBLOCK` flags, so they should work with Node.js without a problem.
Note that the socket is `dup()'ed` into a child process if you pass it to the `stdio` argument of `require('child_process').spawn` or friends,
so `SOCK_CLOEXEC` will not cause problems in that scenario.

### Note
The add-on is implemented with `napi`, which is still experimental.
You will have to run `node` with the `--napi-modules` flags for now if you want to use this module.

# API

## function socketpair(type)

```
const socketpair = require('unix-socketpair');
let fds = socketpair(type)
```

Create a pair of connected sockets.
* `type`: `socketpair.SOCK_STREAM` (default) or `socketpair.SOCK_DGRAM`
* returns: an array holding the two file descriptors for the created sockets.
* throws: if the underlying call to `socketpair()` fails

The created sockets are already connected, so if you wrap them in a `net.Socket`,
they will not generate a `open` or `connected` event.

For more information, see `man 2 socketpair`.

### Warning
Node has no knowledge of raw file descriptors.
One of the consequences is that they will not keep the `node` process running,
but there could be other consequences as well.

You should generally wrap one or both of the file descriptors in an object like `net.Socket`.
If you don't wrap a file descriptor, for example to pass it to a child process, then you should close it
(in the parent process) as soon as possible (right after the spawn) with `fs.close()` or `fs.closeSync()`.

# Examples

This section shows some examples of how you can use `unix-socketpair` .

# Printing the file descriptors
The simplest thing you could do with the file descriptors is print them.
The example below shows how you might do that.


```
const socketpair = require('unix-socketpair');

const   stream_fds = socketpair(socketpair.SOCK_STREAM); // Make a pair of connected unix streaming sockets.
const datagram_fds = socketpair(socketpair.SOCK_DGRAM);  // Make a pair of connected unix datagram sockets.

console.log('streaming descriptors:', stream_fds);
console.log('datagram descriptors: ', datagram_fds);

```

Could output the following (the numbers may vary):
```
streaming descriptors: [3, 4]
datagram descriptors:  [5, 6]
```

# Wrapping in `net.Socket`
You can create `net.Socket` instances to use one or both of the created sockets as you normally would in Node.js.
At the moment of writing `dgram.Socket` does not support the creation from file descriptors,
and `net.Socket` only supports streaming sockets.

The example below wraps both sockets in a `net.Socket`, but you could also wrap one of the two
and send the other to a child process.

```
const socketpair = require('unix-socketpair');
const net        = require('net');

const fds = socketpair(socketpair.SOCK_STREAM);

const sock0 = net.Socket({fd: fds[0], readable: true, writable: true});
const sock1 = net.Socket({fd: fds[1], readable: true, writable: true});

let buffer = Buffer.alloc(0);

sock1.on('data', function (data) {
	buffer = Buffer.concat([buffer, data])
});
sock1.on('close', function() {
	console.log(buffer.toString())
});

sock0.write('Hello World!');
sock0.end();
```

Should output the following:
```
Hello World!
```
