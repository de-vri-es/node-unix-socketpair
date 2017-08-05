#!/bin/env node
'use strict';

function requireBuiltModule(name) {
	try {
		return require('../build/Debug/' + name);
	} catch (e) {
		return require('../build/Release/' + name);
	}
}

const socketpair = requireBuiltModule('unix-socketpair');
const net        = require('net');

const timeout      = 100;
const test_message = Buffer.from('test');

let buffer = Buffer.alloc(0);

const fds = socketpair();
console.error('file descriptors: ' + fds);

const sock = [
	new net.Socket({fd: fds[0], readable: true, writable: true}),
	new net.Socket({fd: fds[1], readable: true, writable: true}),
]

function printBuffer() {
	console.error('expected buffer contents: ' + test_message);
	console.error('  actual buffer contents: ' + buffer);
}

sock[0].once('error', err => {
	console.error('unexpected error on sock[0]: ' + err);
	process.exit(1);
});

sock[1].once('error', err => {
	console.error('unexpected error on sock[1]: ' + err);
	process.exit(1);
});

sock[1].on('data', data => {
	buffer = Buffer.concat([buffer, data]);
});

const timer = setTimeout(() => {
	const str = buffer.toString();
	console.error('expected connection to be closed with test message in buffer within ' + timeout + 'ms');
	printBuffer();
	console.error('TEST FAILED');
	process.exit(1);
}, timeout);

sock[1].once('end', () => {
	clearTimeout(timer);
	printBuffer();
	if (buffer.compare(test_message) != 0) {
		console.error('TEST FAILED');
		process.exit(1);
	}
	console.error('TEST SUCCEEDED');
});

sock[0].end(test_message);
