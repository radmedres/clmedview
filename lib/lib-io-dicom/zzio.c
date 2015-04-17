#include <stdlib.h>
#include <stdio.h>

#if defined(__linux__) || defined(__linux)
#define __USE_GNU
#define ZZ_LINUX
#include <sys/sendfile.h>
#endif

#include <assert.h>	// TODO - remove (most) asserts
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

/* Safe MIN and MAX macros that only evaluate their expressions once. */
#undef MAX
#define MAX(a, b) \
	({ typeof (a) _a = (a); \
	   typeof (b) _b = (b); \
	 _a > _b ? _a : _b; })

#undef MIN
#define MIN(a, b) \
	({ typeof (a) _a = (a); \
	   typeof (b) _b = (b); \
	 _a < _b ? _a : _b; })

#include "include/zzio.h"

#define ZZIO_SOCKET	  1
#define ZZIO_READABLE     2
#define ZZIO_WRITABLE     4
#define ZZIO_PIPE         8

struct zzio
{
	int fd;
	int flags;
	long readpos;		// where in the file we start our read buffer
	long writepos;		// where in the file we start our write buffer
	long readbufpos;	// where in our read buffer we are
	long writebufpos;	// where in our write buffer we are
	long readbuflen;	// amount of data in read buffer
	long writebuflen;	// amount of data in write buffer
	char *writebuf;		// write buffer
	char *readbuf;		// read buffer
	long bytesread;		// file size / bytes read on interface	TODO u64bit forced?
	long byteswritten;
	int error;
	long readbufsize;
	long writebufsize;
	long filesize;		// for files only, total file size
	bool eofmarker;		// for streams

	// for header making
	headerwritefunc *writer;
	readbufferfunc *reader;
	void *userdatawriter;
	void *userdatareader;
	char *header;

	// for error reporting
	char errstr[128];

	// for chaining up duplicated writes to other file descriptors
	struct zzio *tee;
	int teeflags;
};

#define warning(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#ifndef NDEBUG
#define debug(...) do { fprintf(stdout, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#else
#define debug(...)
#endif

// Convenience functions for setting error. Currently no way to clear them. We also do not check if flag is set on function entry.
// FIXME: Wrap printf behind an #ifdef DEBUG conditional
#define ASSERT_OR_RETURN(zi, retval, expr, ...) \
	do { if (!(expr)) { if (zi) { snprintf(zi->errstr, sizeof(zi->errstr) - 1,  __VA_ARGS__); fprintf(stderr, "%s\n", zi->errstr); } assert(!#expr); return retval; } } while(0)
#define ASSERT(zi, expr, ...) \
	do { if (!(expr)) { if (zi) { snprintf(zi->errstr, sizeof(zi->errstr) - 1,  __VA_ARGS__); fprintf(stderr, "%s\n", zi->errstr); } assert(!#expr); } } while(0)

static inline long zi_write_raw(struct zzio *zi, void *buf, long len);

/// Base read function that interfaces with the kernel. The aim is to call this function as few times as possible
/// to reduce context switches. reqlen may be -1 if there is no required size. TODO add zlib support
// TODO -- split len into req_len and desired_len...
// zi->readpos must be updated before calling
// FIXME - MSG_DONTWAIT does not work on *BSD! need to set socket to non-blocking...
static inline long zi_read_raw(struct zzio *zi, void *buf, long len, long reqlen)
{
	long result = 0, sum;
	if (zi->flags & ZZIO_SOCKET)
	{
		errno = 0;
		sum = recv(zi->fd, buf, len, MSG_DONTWAIT);
		if (sum < reqlen) // we did not get the amount required, so block until we do
		{
			sum = 0;
			do // we MUST get at least reqlen, so block until we have this amount
			{
				errno = 0;
				result = recv(zi->fd, buf, reqlen - sum, MSG_WAITALL);
				sum += MAX(0, result);
				// it is now possible that we exited recv above without
				// getting all the required data, even if we blocked
				// and even if there is no actual error; in  this case,
				// errno is set to EAGAIN or EWOULDBLOCK
			} while (sum < reqlen && (errno == EAGAIN || errno == EWOULDBLOCK));
			//ASSERT_OR_RETURN(zi, result, result >= 0, "Read error: %s", strerror(errno));
		}
	}
	else if (zi->flags & ZZIO_PIPE)
	{
		errno = 0;
		sum = read(zi->fd, buf, len);
		ASSERT_OR_RETURN(zi, sum, sum >= 0, "Read error: %s", strerror(errno));
		if (sum < reqlen)
		{
			sum = 0;
			do
			{
				errno = 0;
				result = read(zi->fd, buf, reqlen - sum);
				sum += MAX(0, result);
			} while (sum < reqlen && (errno == EAGAIN || errno == EWOULDBLOCK));
		}
		ASSERT_OR_RETURN(zi, result, result >= 0, "Read error: %s", strerror(errno));
	}
	else
	{
		sum = pread(zi->fd, buf, len, zi->readpos);
		ASSERT_OR_RETURN(zi, sum, sum >= 0, "Read error: %s", strerror(errno));
	}
	if (zi->tee && zi->teeflags & ZZIO_TEE_READ) // duplicate the read
	{
		long ret;
		// a flush is necessary here to correctly order writes from the tee's buffer with our own writes
		ziflush(zi->tee);
		ret = zi_write_raw(zi->tee, buf, sum);
		zi->tee->byteswritten += ret;
		ASSERT(zi, ret == sum, "Tee read write error (%ld/%ld): %s", ret, sum, strerror(errno));
	}
	zi->bytesread += sum;
	return sum;
}

/// Base write function that interfaces with the kernel. See zi_read_raw().
static inline long zi_write_raw(struct zzio *zi, void *buf, long len)	// zi->writepos must be updated
{
	long result;
	if (zi->flags & ZZIO_SOCKET || zi->flags & ZZIO_PIPE)
	{
		result = write(zi->fd, buf, len);
	}
	else
	{
		result = pwrite(zi->fd, buf, len, zi->writepos);
	}
	//ASSERT_OR_RETURN(zi, result, result != -1, "Write error: %s", strerror(errno));
	if (zi->tee && zi->teeflags & ZZIO_TEE_WRITE) // duplicate the write
	{
		long ret;
		// a flush is necessary here to correctly order writes from the tee's buffer with our own writes
		ziflush(zi->tee);
		ret = zi_write_raw(zi->tee, buf, len);
		ASSERT(zi, ret == result, "Tee write error: %s", strerror(errno));
	}
	zi->writepos += result;
	return result;
}

struct zzio *ziopenstdin()
{
	const int bufsize = ZZIO_BUFFERSIZE;
	struct zzio *zi = NULL;

	zi = calloc(1, sizeof(*zi));
	zi->fd = STDIN_FILENO;
	zi->readbuf = calloc(1, bufsize);
	zi->readbufsize = bufsize;
	zi->flags = ZZIO_READABLE | ZZIO_PIPE;
	return zi;
}

struct zzio *ziopenstdout()
{
	const int bufsize = ZZIO_BUFFERSIZE;
	struct zzio *zi = NULL;

	zi = calloc(1, sizeof(*zi));
	zi->fd = STDOUT_FILENO;
	zi->writebuf = calloc(1, bufsize);
	zi->writebufsize = bufsize;
	zi->flags = ZZIO_WRITABLE | ZZIO_PIPE;
	return zi;
}

struct zzio *ziopenread(const char *path, int bufsize, int flags)
{
	struct stat st;
	struct zzio *zi = NULL;
	int fd;

#ifdef ZZ_LINUX
	fd = open(path, O_RDONLY | O_NOATIME);
#else
	fd = open(path, O_RDONLY);
#endif
	if (fd == -1)
	{
		return NULL;
	}
	zi = calloc(1, sizeof(*zi));
	zi->readbuf = calloc(1, bufsize);
	zi->flags = flags | ZZIO_READABLE;
	zi->fd = fd;
	zi->readbufsize = bufsize;
	fstat(zi->fd, &st);
	zi->filesize = st.st_size;
	return zi;
}

struct zzio *ziopenwrite(const char *path, int bufsize, int flags)
{
	struct zzio *zi = NULL;
	int fd;

	fd = creat(path, S_IRUSR | S_IWUSR | S_IRGRP);
	if (fd == -1)
	{
		return NULL;
	}
	zi = calloc(1, sizeof(*zi));
	zi->fd = fd;
	zi->writebuf = calloc(1, bufsize);
	zi->flags = flags | ZZIO_WRITABLE;
	zi->writebufsize = bufsize;
	return zi;
}

struct zzio *ziopenmodify(const char *path, int bufsize, int flags)
{
	struct stat st;
	struct zzio *zi = NULL;
	int fd;

	fd = open(path, O_RDWR);
	if (fd == -1)
	{
		return NULL;
	}
	zi = calloc(1, sizeof(*zi));
	zi->readbuf = calloc(1, bufsize);
	zi->writebuf = calloc(1, bufsize);
	zi->fd = fd;
	zi->flags = flags | ZZIO_WRITABLE | ZZIO_READABLE;
	zi->readbufsize = bufsize;
	zi->writebufsize = bufsize;
	fstat(zi->fd, &st);
	zi->filesize = st.st_size;
	return zi;
}

struct zzio *ziopenfile(const char *path, const char *mode)
{
	const char *p = mode;
	bool doread = false, dowrite = false;
	const int bufsize = 8192;

	while (*p)
	{
		switch (*p)
		{
		case 'r' : doread = true; break;
		case 'w' : dowrite = true; break;
		case '+' : dowrite = true; doread = true; break;
		default : return NULL;
		}
		p++;
	}
	if (doread && dowrite) return ziopenmodify(path, bufsize, 0);
	else if (doread) return ziopenread(path, bufsize, 0);
	else return ziopenwrite(path, bufsize, 0);
}

struct zzio *ziopensocket(int sock, int flags)
{
	const int bufsize = ZZIO_BUFFERSIZE;
	struct zzio *zi = NULL;

	ASSERT_OR_RETURN(zi, NULL, sock >=0, "Invalid socket to open");
	zi = calloc(1, sizeof(*zi));
	zi->fd = sock;
	zi->readbuf = calloc(1, bufsize);
	zi->writebuf = calloc(1, bufsize);
	zi->flags = flags | ZZIO_WRITABLE | ZZIO_READABLE | ZZIO_SOCKET;
	zi->readbufsize = bufsize;
	zi->writebufsize = bufsize;
	return zi;
}

void zisetbuffersize(struct zzio *zi, long buffersize)
{
	ziflush(zi);
	free(zi->readbuf);
	free(zi->writebuf);
	zi->readbuf = calloc(1, buffersize);
	zi->writebuf = calloc(1, buffersize);
	zi->readbufsize = buffersize;
	zi->writebufsize = buffersize;
}

const char *zistrerror(const struct zzio *zi)
{
	return zi->errstr;
}

void zisetwriter(struct zzio *zi, headerwritefunc writefunc, long buffersize, void *userdata)
{
	ASSERT_OR_RETURN(zi, , !(zi->flags & ZZIO_WRITABLE && zi->flags & ZZIO_READABLE && !(zi->flags & ZZIO_SOCKET)), "Cannot use splitter on file modification");
	zi->writer = writefunc;
	zi->userdatawriter = userdata;
	free(zi->header);
	zi->header = calloc(1, buffersize);
}

void zisetreader(struct zzio *zi, readbufferfunc readfunc, void *userdata)
{
	ASSERT_OR_RETURN(zi, , !(zi->flags & ZZIO_WRITABLE && zi->flags & ZZIO_READABLE && !(zi->flags & ZZIO_SOCKET)), "Cannot use splitter on file modification");
	zi->reader = readfunc;
	zi->userdatareader = userdata;
	zi->readbuflen = 0;
	zi->readpos = 0;
}

long zireadpos(const struct zzio *zi)
{
	return zi->readpos + zi->readbufpos;
}

long ziwritepos(const struct zzio *zi)
{
	return zi->writepos + zi->writebufpos;
}

long zibyteswritten(struct zzio *zi)	// not including packet headers?
{
	return zi->byteswritten + zi->writebuflen;
}

long zibytesread(struct zzio *zi)	// not including packet headers?
{
	return zi->bytesread;
}

static inline void writeheader(struct zzio *zi, long length)
{
	long size = zi->writer(length, zi->header, zi->userdatawriter);
	long chunk = zi_write_raw(zi, zi->header, size);
	ASSERT(zi, chunk == size, "Header write failure");
}

void zicommit(struct zzio *zi)
{
	ziflush(zi);	// flush our buffers
	if (!(zi->flags & ZZIO_SOCKET || zi->flags & ZZIO_PIPE))
	{
#if _POSIX_SYNCHRONIZED_IO > 0
		fdatasync(zi->fd);
#else
		fsync(zi->fd);
#endif
	}
}

void ziflush(struct zzio *zi)
{
	long chunk;

	// invalidate read buffer if (partially) within write buffer
	if ((zi->readbufpos > zi->writebufpos && zi->readbufpos < zi->writebufpos + zi->writebuflen)
	    || (zi->readbufpos + zi->readbuflen > zi->writebufpos && zi->readbufpos + zi->readbuflen < zi->writebufpos + zi->writebuflen)
	    || (zi->readbufpos < zi->writebufpos && zi->readbufpos + zi->readbuflen > zi->writebufpos + zi->writebuflen))
	{
		if (!(zi->flags & ZZIO_SOCKET || zi->flags & ZZIO_PIPE))
		{
			zi->readbuflen = 0;
			zi->readbufpos = 0;
		}
	}

	// commit write buffer
	assert((zi->flags & ZZIO_WRITABLE) || zi->writebuflen == 0);
	if (zi->writebuflen > 0 && zi->writer) writeheader(zi, zi->writebufpos);
	zi->writebufpos = 0;
	while (zi->writebuflen - zi->writebufpos > 0)
	{
		chunk = zi_write_raw(zi, zi->writebuf + zi->writebufpos, zi->writebuflen - zi->writebufpos);
		if (chunk > 0)
		{
			if (!(zi->flags & ZZIO_SOCKET || zi->flags & ZZIO_PIPE))
			{
				zi->filesize += MAX(0, zi->writepos + chunk - zi->filesize);
			}
			zi->writebufpos += chunk;
			zi->byteswritten += chunk;
		}
		assert(zi->writebufpos <= zi->writebufsize && zi->writebufpos >= 0);
		assert(zi->writebuflen <= zi->writebufsize);
	}
	assert(zi->writebuflen == zi->writebufpos);
	zi->writebufpos = 0;
	zi->writebuflen = 0;
}

// TODO - Optimize me
bool ziwriteu16at(struct zzio *zi, uint16_t value, long pos)
{
	long curr = ziwritepos(zi);
	bool result = zisetwritepos(zi, pos);
	ASSERT_OR_RETURN(zi, false, result, "Out of buffer bounds");
	ziwrite(zi, &value, 2);
	return zisetwritepos(zi, curr);
}

// TODO - Optimize me
bool ziwriteu32at(struct zzio *zi, uint32_t value, long pos)
{
	long curr = ziwritepos(zi);
	bool result = zisetwritepos(zi, pos);
	ASSERT_OR_RETURN(zi, false, result, "Out of buffer bounds");
	ziwrite(zi, &value, 4);
	return zisetwritepos(zi, curr);
}

void ziwillneed(struct zzio *zi, long offset, long length)
{
#ifdef ZZ_LINUX
	posix_fadvise(zi->fd, offset, length, POSIX_FADV_WILLNEED);
#endif
}

void ziputc(struct zzio *zi, int ch)
{
	if (zi->writebufsize <= zi->writebufpos + 1)
	{
		ziflush(zi);
	}
	zi->writebuf[zi->writebufpos] = ch;
	zi->writebufpos++;
	zi->writebuflen++;
}

// Flush for the read buffer
static inline bool zi_reposition_read(struct zzio *zi, long pos, long reqlen)
{
	zi->readpos = pos;
	zi->readbufpos = 0;
	if (zi->reader)	// packetizer
	{
		zi->readbuflen = zi->reader(&zi->readbuf, &zi->readbufsize, zi->userdatareader);
		zi->readpos += zi->readbuflen;
	}
	else
	{
		zi->readbuflen = zi_read_raw(zi, zi->readbuf, zi->readbufsize, reqlen);	// Read next buffer
	}
	return zi->readbuflen != 0; // TODO also return false if pos outsize bounds
}

int zigetc(struct zzio *zi)
{
	if (zi->readbuflen > zi->readbufpos)
	{
		return (unsigned char)zi->readbuf[zi->readbufpos++];
	}
	else
	{
		zi_reposition_read(zi, zi->readpos + zi->readbufpos, 1);
		return (unsigned char)zi->readbuf[zi->readbufpos++];
	}
}

// TODO allow skipping forward, also in sockets and packetized input?
bool zisetreadpos(struct zzio *zi, long pos)
{
	if (pos > zi->readpos + zi->readbuflen && (zi->flags & ZZIO_SOCKET || zi->flags & ZZIO_PIPE))
	{
		// Seeking behind buffer in a pipe or a socket, so skip ahead
		long result = 0, piece, skipped = 0, remaining = pos - zi->readpos + zi->readbuflen;

		while (remaining)
		{
			piece = MIN(zi->readbufsize, remaining);
			zi->readpos += zi->readbuflen;
			skipped += zi->readbuflen;
			result = zi_read_raw(zi, zi->readbuf, zi->readbufsize, piece);
			if (result >= 0)
			{
				zi->readbuflen = result;
				zi->readbufpos = MIN(zi->readbufsize - 1, labs(pos - skipped));
				remaining = MAX(remaining - result, 0);
			}
			ASSERT(zi, result >= 0, "Failed skipping ahead");
		}
		ASSERT(zi, zi->readpos + zi->readbufpos == pos, "Failed to find correct position (%ld + %ld != %ld)", zi->readpos, zi->readbufpos, pos);
		return true;
	}
	else if (pos > zi->readpos + zi->readbuflen || pos < zi->readpos)
	{
		// Seeking outside of buffer
		ASSERT(zi, !(zi->flags & ZZIO_SOCKET || zi->flags & ZZIO_PIPE), "Cannot seek before buffer with sockets or pipes");
		ASSERT(zi, !zi->reader, "Cannot seek outside of buffer range with reader set");
		if (zi->reader) { return false; }	// TODO make error message
		return zi_reposition_read(zi, pos, -1);
	}
	else
	{
		zi->readbufpos = pos - zi->readpos;
		return true;
	}
}

bool zisetwritepos(struct zzio *zi, long pos)
{
	if (pos > zi->writepos + zi->writebuflen || pos <  zi->writepos - (zi->writebuflen - zi->writebufpos))
	{
		// Seeking outside of buffer
		if (zi->flags & ZZIO_SOCKET) return false;
		ziflush(zi);
		zi->writepos = pos;
	}
	else
	{
		zi->writebufpos = pos - zi->writepos;
	}
	return true;
}

long ziread(struct zzio *zi, void *buf, long count)
{
	long len, remaining = count;

	assert(zi->readbuflen >= zi->readbufpos);
	assert(zi->readbuflen >= 0);
	do
	{
		// Read as much as we can from buffer
		assert(count >= remaining);
		len = MIN(remaining, zi->readbuflen - zi->readbufpos);
		assert(len >= 0);
		memcpy(buf + (count - remaining), zi->readbuf + zi->readbufpos, len);
		zi->readbufpos += len;
		// Is buffer empty now and we need more?
		remaining -= len;
		assert(remaining >= 0);
		if (remaining > 0)	// yes, read in more
		{
			zi_reposition_read(zi, zi->readpos + zi->readbufpos, MIN(zi->readbufsize, remaining));
			// TODO - optimize if no packetizer, by reading remainder raw? see ziwrite
			if (zi->readbuflen <= 0) return 0;
		}
	} while (remaining > 0);
	return count;
}

long ziwrite(struct zzio *zi, const void *buf, long count)
{
	long len, remaining = count;

	do
	{
		// Write as much as we can into buffer
		len = MIN(remaining, zi->writebufsize - zi->writebufpos);
		memcpy(zi->writebuf + zi->writebufpos, buf + (count - remaining), len);
		zi->writebuflen += MAX(0, zi->writebufpos + len - zi->writebuflen); // extend length of buffer depending on where we are in it
		zi->writebufpos += len;

		// Is buffer full now and we need more?
		remaining -= len;
		assert(remaining >= 0);
		if (remaining > 0)
		{
			ziflush(zi); // buffer blown, so flush it
		}
	} while (remaining > 0);
	return count;
}

struct zzio *ziclose(struct zzio *zi)
{
	ziflush(zi);
	close(zi->fd);
	free(zi->readbuf);
	free(zi->writebuf);
	free(zi->header);
	free(zi);
	return NULL;
}

bool zieof(const struct zzio *zi)
{
	return ((zi->flags & ZZIO_READABLE) && zi->eofmarker && zi->readbufpos >= zi->readbuflen)
	        || ((zi->flags & ZZIO_READABLE) 
	            && !(zi->flags & ZZIO_SOCKET)
	            && !(zi->flags & ZZIO_PIPE)
	            && zi->readpos + zi->readbufpos >= zi->filesize);
}

int zierror(const struct zzio *zi)
{
	return (zi->errstr[0] != '\0');
}

void *zireadbuf(struct zzio *zi, long size)
{
	void *addr, *bytes;

	if ((zi->flags & ZZIO_SOCKET) || zi->reader)
	{
		bytes = malloc(size);
		if (zi->readbuflen - zi->readbufpos > 0) // save whatever lies unconsumed in the buffer already
		{
			memcpy(bytes, zi->readbuf, zi->readbuflen - zi->readbufpos);
		}
		ziread(zi, bytes, size - zi->readbufpos);
	}
	else
	{
		int result;
		long pos = zi->readpos + zi->readbufpos;
		long offset = pos & ~(sysconf(_SC_PAGE_SIZE) - 1);	// start at page aligned offset
		addr = mmap(NULL, size + pos - offset, PROT_READ, MAP_SHARED, zi->fd, offset);
		ASSERT_OR_RETURN(zi, NULL, addr != MAP_FAILED, "Memory map failed: %s", strerror(errno));
		bytes = addr + pos - offset;	// increment by page alignment shift
		result = madvise(addr, size + pos - offset, MADV_SEQUENTIAL);
		ASSERT(zi, result == 0, "madvise failed: %s", strerror(errno));
	}
	zi->readpos += size;
	zi->readbuflen = 0;
	zi->readbufpos = 0;
	zi->bytesread += size;
	return bytes;
}

void zifreebuf(struct zzio *zi, void *buf, long size)
{
	if ((zi->flags & ZZIO_SOCKET) || zi->reader)
	{
		free(buf);
		return;
	}
	// else unmap memory
	void *addr = (void *)((intptr_t)buf & ~(sysconf(_SC_PAGE_SIZE) - 1));
	long realsize = size + buf - addr;
	int result = munmap(addr, realsize);
	ASSERT(zi, result == 0, "munmap failed: %s", strerror(errno));
}

#ifdef ZZ_LINUX
// duplicate data from one pipe to another, then to a destination; will often not actually copy anything
static inline void dupefd(int pipefd, int *pipetee, long bytes_in_pipe, int flags, int target)
{
	long result, sum = bytes_in_pipe;
	while (sum > 0)
	{
		result = tee(pipefd, pipetee[1], sum, flags);
		if (result < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue; // interrupted, try again
			}
			warning("Failed to tee from pipe: %s", strerror(errno));
			break;
		}
		sum -= result;
	}
	sum = bytes_in_pipe;
	while (sum > 0)
	{
		result = splice(pipetee[0], NULL, target, NULL, sum, flags);
		if (result < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue; // interrupted, try again
			}
			warning("Failed to read from pipe: %s", strerror(errno));
			assert(false);
			break;
		}
		sum -= result;
	}
}
#endif

long zicopy(struct zzio *dst, struct zzio *src, long length)
{
	long result;
	if (length <= dst->writebufsize)  // fast track solution, read directly into write buffer
	{
		if (length > dst->writebufsize - dst->writebufpos)
		{
			ziflush(dst);	// flush write buffer to make space
		}
		result = ziread(src, dst->writebuf + dst->writebufpos, length);
		if (result > 0)
		{
			dst->writebuflen += result;
			dst->writebufpos += result;
		}
		return result;
	}
	else // lots of data, break out the big tools
	{
#ifdef ZZ_LINUX
		// This implements kernel side "zero-copy" of data between file descriptors,
		// allowing higher theoretical throughput, and quite real reduced CPU usage.
		int pipefd[2], pipeteein[2], pipeteeout[2];
		long total_sent = 0, bytes_in_pipe, remainder;
		unsigned baseflags = SPLICE_F_NONBLOCK, flags = 0;

		if (src->writebuflen > 0)
		{
			ziflush(src); // commit writes before reading (although caller should have done this)
		}
		if (dst->writebuflen > 0)
		{
			ziflush(dst); // commit writes before writing, when bypassing buffer
		}
		lseek(src->fd, src->readpos + src->readbufpos, SEEK_SET);
		lseek(dst->fd, dst->writepos, SEEK_SET);
		if (!(dst->flags & ZZIO_SOCKET || dst->flags & ZZIO_PIPE))
		{
			posix_fallocate(dst->fd, dst->writepos, length);
		}
		if (src->flags & ZZIO_SOCKET)
		{
			baseflags |= SPLICE_F_MOVE;
		}
		if (pipe(pipefd) || pipe(pipeteein) || pipe(pipeteeout))
		{
			warning("Failed to make pipe: %s", strerror(errno));
			return -1;
		}
		while (total_sent < length)
		{
			remainder = length - total_sent;
			result = splice(src->fd, NULL, pipefd[1], NULL, remainder, baseflags);
			if (result < 0)
			{
				if (errno == EINTR || errno == EAGAIN)
				{
					continue; // interrupted, try again
				}
				warning("Failed to write to pipe: %s", strerror(errno));
				close(pipefd[0]);
				close(pipefd[1]);
				return -1;
			}
			bytes_in_pipe = result;
			total_sent += result;
			flags = baseflags;
			if (total_sent < length)
			{
				flags |= SPLICE_F_MORE; // tell kernel that more is coming
			}
			if (src->tee && (src->teeflags & ZZIO_TEE_READ))
			{
				dupefd(pipefd[0], pipeteein, bytes_in_pipe, flags, zifd(src->tee));
			}
			if (dst->tee && (dst->teeflags & ZZIO_TEE_WRITE))
			{
				dupefd(pipefd[0], pipeteeout, bytes_in_pipe, flags, zifd(dst->tee));
			}
			while (bytes_in_pipe > 0)
			{
				result = splice(pipefd[0], NULL, dst->fd, NULL, bytes_in_pipe, flags);
				if (result < 0)
				{
					if (errno == EINTR || errno == EAGAIN)
					{
						continue; // interrupted, try again
					}
					warning("Failed to read from pipe: %s", strerror(errno));
					close(pipefd[0]);
					close(pipefd[1]);
					return -1;
				}
				bytes_in_pipe -= result;
			}
		}
		close(pipefd[0]);
		close(pipefd[1]);
		close(pipeteein[0]);
		close(pipeteein[1]);
		close(pipeteeout[0]);
		close(pipeteeout[1]);
		if (src->tee)
		{
			src->tee->writepos += total_sent;
			src->tee->byteswritten += total_sent;
		}
		if (dst->tee)
		{
			dst->tee->writepos += total_sent;
			dst->tee->byteswritten += total_sent;
		}
		src->readpos += total_sent;
		src->readbuflen = 0;
		src->readbufpos = 0;
		src->bytesread += total_sent;
		dst->writepos += total_sent;
		dst->byteswritten += total_sent;
		return total_sent;
#else
		// SLOW test version -- to use as benchmark, and for non-linux versions for now
		char *buffer = malloc(length);
		memset(buffer, 0, length);
		ziread(src, buffer, length);
		ziwrite(dst, buffer, length);
		free(buffer);
		return length;
#endif
	}
}

void ziseteof(struct zzio *zi)
{
	zi->eofmarker = true;
}

void zicleareof(struct zzio *zi)
{
	zi->eofmarker = false;
}

int zifd(struct zzio *zi)
{
	return zi->fd;
}

void zitee(struct zzio *zi, struct zzio *target, int flags)
{
	zi->tee = target;
	zi->teeflags = flags;
}

bool zirewindable(struct zzio *zi)
{
	return !(zi->flags & ZZIO_SOCKET || zi->flags & ZZIO_PIPE);
}

void ziresetwritebuffer(struct zzio *zi)
{
	// This makes relative addresses to start of a series of write
	// operations work as expected.
	zi->writepos = 0;

	zi->writebufpos = 0;
	zi->writebuflen = 0;
}
