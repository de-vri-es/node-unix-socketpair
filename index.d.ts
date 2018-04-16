interface FdPair {
	0: number;
	1: number;
}

interface socketpair {
	SOCK_STREAM:    number;
	SOCK_DGRAM:     number;
	SOCK_SEQPACKET: number;

	(type: number): FdPair;
}

declare const unix_socketpair: socketpair;
export = unix_socketpair;
