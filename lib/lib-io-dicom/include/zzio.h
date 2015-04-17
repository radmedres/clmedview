// High-performance buffered IO layer with transparent support for
// packet splitting (and with on-the-fly compression support planned 
// for the future).

// Note: Do not mix write calls to this interface with write calls 
// through other inferfaces to the same files. Do not mix read or 
// write calls to this interface with read calls through other 
// interfaces to the same socket. Always call the flush function 
// after writing before reading if the areas might overlap (taking 
// buffer size into consideration).

#ifndef ZZIO_H
#define ZZIO_H

#define ZZ_DECL_PURE __attribute__((__pure__))
#define ZZ_DECL_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define ZZ_DECL_ALLOC __attribute__((malloc)) __attribute__((warn_unused_result))

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

struct zzio;

#define ZZIO_BUFFERSIZE    8096 // default buffer size

// flags
#define ZZIO_ZLIB         8

/// Header write function. First parameter is number of bytes to be written in next packet.
/// Second parameter is the buffer to write into. Third parameter is user supplied. Returns
/// size of written header.
typedef long headerwritefunc(long, char *, const void*);

/// User supplied fill buffer function. First parameter is buffer to store into. Second 
/// parameter is size of buffer. Third is user supplied. Returns number of bytes in buffer.
/// The function is responsible for memory allocation, and the first time it is called, the
/// first parameter may point to a NULL pointer.
typedef long readbufferfunc(char **, long *, void *);

/// Open a zzio buffer on a socket. It must already be opened and connected.
ZZ_DECL_ALLOC
struct zzio *ziopensocket(int sock, int flags);

/// Open a zzio buffer on standard input.
ZZ_DECL_ALLOC
struct zzio *ziopenstdin(void);

/// Open a zzio buffer on standard output.
ZZ_DECL_ALLOC
struct zzio *ziopenstdout(void);

/// Open a zzio buffer on a file. Mode has the same meaning as for fopen(3).
ZZ_DECL_NONNULL(1, 2) ZZ_DECL_ALLOC
struct zzio *ziopenfile(const char *path, const char *mode);

/// Change buffer size. This may flush the existing buffer.
ZZ_DECL_NONNULL(1)
void zisetbuffersize(struct zzio *zi, long buffersize);

/// Set up packet splitter that turns a stream of data into neat packets with custom headers. Once amount of data
/// equal to size of buffer as told earlier to open function is written, or ziflush() is called, it is preceeded
/// by a header and flushed to disk or network, as appropriate. The writer function is passed an empty buffer
/// of buffersize size to fill with data, and returns the size of the buffer used.
ZZ_DECL_NONNULL(1)
void zisetwriter(struct zzio *zi, headerwritefunc writefunc, long buffersize, void *userdata);

/// Set up a user defined function that reads data into our read buffer. It is passed a pointer to our buffer
/// pointer, so it is allowed to reallocate it. This allows zzio to read very exoticly packeted formats as a
/// unified stream. This function clears the read buffer and resets the file position.
ZZ_DECL_NONNULL(1)
void zisetreader(struct zzio *zi, readbufferfunc readfunc, void *userdata);

/// Return last error in human readable format.
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
const char *zistrerror(const struct zzio *zi);

/// Return non-zero if error flag is set. Will not clear the error flag.
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
int zierror(const struct zzio *zi);

/// Return the read position. Note that this is independent of write position.
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
long zireadpos(const struct zzio *zi);

/// Return the write position. Note that this is independent of read position.
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
long ziwritepos(const struct zzio *zi);

/// Set the read position. Note that this is independent of write position.
ZZ_DECL_NONNULL(1)
bool zisetreadpos(struct zzio *zi, long pos);

/// Set the write position. Note that this is independent of read position.
ZZ_DECL_NONNULL(1)
bool zisetwritepos(struct zzio *zi, long pos);

/// Get a single character from the buffer. This is much faster than ziread()
/// of a single character.
ZZ_DECL_NONNULL(1)
int zigetc(struct zzio *zi);

/// Put a single character. This is much faster than ziwrite() of a single character.
ZZ_DECL_NONNULL(1)
void ziputc(struct zzio *zi, int ch);

/// General purpose read function.
ZZ_DECL_NONNULL(1, 2)
long ziread(struct zzio *zi, void *buf, long count);

/// General purpose write function.
ZZ_DECL_NONNULL(1, 2)
long ziwrite(struct zzio *zi, const void *buf, long count);

/// Inform the zzio code about how much data you intend to read. zzio may use
/// this information to speed up the read.
ZZ_DECL_NONNULL(1)
void ziwillneed(struct zzio *zi, long offset, long length);

/// Close a buffer. Will also close the socket or file it operates on. Always returns NULL.
ZZ_DECL_NONNULL(1)
struct zzio *ziclose(struct zzio *zi);

/// Returns true if read position is at end of file or stream.
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
bool zieof(const struct zzio *zi);

/// Flushes the write buffer to file. Note that this does not force the OS to flush its write
/// buffer to disk or to send it on the network. Also, you must flush the write buffer before
/// you read in the same area, otherwise you might get old data.
ZZ_DECL_NONNULL(1)
void ziflush(struct zzio *zi);

/// Flush and force OS to commit file data to storage. Does nothing for sockets. File metadata
/// (modification and creation time) may remain uncommitted.
ZZ_DECL_NONNULL(1)
void zicommit(struct zzio *zi);

/// Write two bytes at a certain position in the file without changing write position.
/// If splitter is active, or operating on a socket, the position must be within the
/// current buffer. Returns true on success.
ZZ_DECL_NONNULL(1)
bool ziwriteu16at(struct zzio *zi, uint16_t value, long pos);

/// Write four bytes at a certain position in the file without changing write position.
/// If splitter is active, or operating on a socket, the position must be within the
/// current buffer. Returns true on success.
ZZ_DECL_NONNULL(1)
bool ziwriteu32at(struct zzio *zi, uint32_t value, long pos);

/// Highly optimized buffer read function (implemented with memory mapping where possible) 
/// meant for sequential access patterns. 
ZZ_DECL_NONNULL(1)
void *zireadbuf(struct zzio *zi, long size);

/// Deallocate memory buffer created with zireadbuf
ZZ_DECL_NONNULL(1, 2)
void zifreebuf(struct zzio *zi, void *buf, long size);

/// Fast copy between the read position of one to write position of another zzio handle.
ZZ_DECL_NONNULL(1, 2)
long zicopy(struct zzio *dst, struct zzio *src, long length);

/// Number of bytes written to file
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
long zibyteswritten(struct zzio *zi);

/// Number of bytes read from file
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
long zibytesread(struct zzio *zi);

/// Set eof flag. This useful for network streaming protocols, and can be called from within a reader function to indicate
/// that we are at the end of the stream. The eof flag must be cleared by zicleareof().
ZZ_DECL_NONNULL(1)
void ziseteof(struct zzio *zi);

/// Clear the eof flag. See ziseteof().
ZZ_DECL_NONNULL(1)
void zicleareof(struct zzio *zi);

// Return current file descriptor
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
int zifd(struct zzio *zi);

/// Duplicate read buffer refills to another zzio file descriptor.
#define ZZIO_TEE_READ   1
/// Duplicate write buffer flushes to another zzio file descriptor.
#define ZZIO_TEE_WRITE  2

/// Duplicate all reads and/or writes to another zzio file descriptor.
/// You can combine read and write redirections. Attempts to seek outside a 
/// single read/write buffer will break this functionality.
ZZ_DECL_NONNULL(1, 2)
void zitee(struct zzio *zi, struct zzio *tee, int flags);

/// Return true if we can rewind the given zzio file descriptor. Will
/// return false for pipes and sockets.
ZZ_DECL_NONNULL(1) ZZ_DECL_PURE
bool zirewindable(struct zzio *zi);

/// Clear write buffer history
ZZ_DECL_NONNULL(1)
void ziresetwritebuffer(struct zzio *zi);

/* --------------------------------------------------------------------- */
// Private functions below - only for unit testing

struct zzio *ziopenread(const char *path, int bufsize, int flags);
struct zzio *ziopenwrite(const char *path, int bufsize, int flags);
struct zzio *ziopenmodify(const char *path, int bufsize, int flags);

#endif
