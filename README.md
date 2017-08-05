# unix-socketpair
This add-on exposes the real POSIX socketpair() to NodeJS applications.
`socketpair()` is unrivaled in simplicity when you want to create connected sockets.
It allows very easy creation of a bi-directional data channel for a child process implemented in some other language than NodeJS.

The sockets are created with the `SOCK_CLOEXEC` and `SOCK_NONBLOCK` flags, so they should work with NodeJS without a problem.
Note that the socket is `dup()'ed` into a child process if you pass it to the `stdio` argument of `require('child_process').spawn` or friends,
so `SOCK_CLOEXEC` will not cause problems in that scenario.

# API

## function socketpair(type)

```
const socketpair = require('unix-socketpair');
let fds = socketpair(type)
```

Create a pair of connected sockets.
* type: `socketpair.SOCK_STREAM` or `socketpair.SOCK_DGRAM`
* return value: an array holding two file descriptors
* throws: if the underlying call to `socketpair()` fails

For more information, see `man 2 socketpair`.


# Example

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
