#include <assert.h>
#include <string.h>
#include <errno.h>

#include "include/zz_priv.h"
#include "include/byteorder.h"
#include "include/zzwrite.h"

#ifdef DEBUG
#define verboseprint(_zz, ...) \
{ \
	if (zzisverbose()) \
	{ \
		char vrstr[MAX_LEN_VR]; \
		fprintf(stdout, "  \033[22m\033[32m(%04x,%04x)\033[0m %s ", \
		        _zz->current.group, _zz->current.element, zzvr2str(zz->current.vr, vrstr)); \
		fprintf(stdout, __VA_ARGS__); \
		fprintf(stdout, "\n"); \
	} \
}
#else
#define verboseprint(...) {}
#endif

static inline bool explicit(struct zzfile *zz) { return zz->ladder[zz->ladderidx].txsyn == ZZ_EXPLICIT; }

static inline void implicit(struct zzio *zi, uint16_t group, uint16_t element, uint32_t length)
{
	ziwrite(zi, &group, 2);
	ziwrite(zi, &element, 2);
	ziwrite(zi, &length, 4);
}

static inline void explicit1(struct zzio *zi, uint16_t group, uint16_t element, const char *vr, uint16_t length)
{
	ziwrite(zi, &group, 2);
	ziwrite(zi, &element, 2);
	ziwrite(zi, vr, 2);
	ziwrite(zi, &length, 2);
}

static inline void explicit2(struct zzio *zi, uint16_t group, uint16_t element, const char *vr, uint32_t length)
{
	uint16_t zero = 0;
	ziwrite(zi, &group, 2);
	ziwrite(zi, &element, 2);
	ziwrite(zi, vr, 2);
	ziwrite(zi, &zero, 2);
	ziwrite(zi, &length, 4);
}

static inline void writetag(struct zzfile *zz, zzKey key, enum VR vr, uint32_t size)
{
	const uint16_t group = ZZ_GROUP(key);
	const uint16_t element = ZZ_ELEMENT(key);
	char dest[MAX_LEN_VR];

	zz->current.element = element;
	zz->current.group = group;
	zz->current.vr = vr;
	zz->current.length = size;
	if (!zz->ladder[zz->ladderidx].txsyn == ZZ_EXPLICIT || group == 0xfffe)
	{
		implicit(zz->zi, group, element, size);
	}
	else
	{
		switch (vr)
		{
		case OB: case OW: case OF: case SQ: case UT: case UN:
			explicit2(zz->zi, group, element, zzvr2str(vr, dest), size); break;
		case AE: case AS: case AT: case CS: case DA: case DS: case DT: case FL: case FD: case IS: case LO:
		case LT: case PN: case SH: case SL: case SS: case ST: case TM: case UI: case UL: case US: case HACK_VR:
			explicit1(zz->zi, group, element, zzvr2str(vr, dest), size); break;
		default: // unknown VR, always 32-bit length
			explicit2(zz->zi, group, element, zzvr2str(vr, dest), size); break;
		}
	}
}

void zzwCopy(struct zzfile *zz, const struct zzfile *orig)
{
	writetag(zz, ZZ_KEY(orig->current.group, orig->current.element), orig->current.vr, orig->current.length);
	if (orig->current.length != UNLIMITED && orig->current.length > 0 && orig->current.vr != SQ && orig->current.group != 0xfffe)
	{
		zisetreadpos(orig->zi, orig->current.pos);	// reposition read marker to beginning of data
		zicopy(zz->zi, orig->zi, orig->current.length);
	}
}

void zzwSQ(struct zzfile *zz, zzKey key, uint32_t size)
{
	writetag(zz, key, SQ, size);
	verboseprint(zz, "\033[22m\033[33m(Sequence)\033[0m");
}

void zzwUN(struct zzfile *zz, zzKey key, uint32_t size)
{
	writetag(zz, key, UN, size);
	if ((long)size == UNLIMITED) verboseprint(zz, "\033[22m\033[33m(Sequence)\033[0m")
	else verboseprint(zz, "\033[22m\033[33m(Unknown)\033[0m");
}

void zzwUN_begin(struct zzfile *zz, zzKey key, long *pos)
{
	zzwUN(zz, key, UNLIMITED);
	if (pos) *pos = ziwritepos(zz->zi) - 4;	// position of length value
	zz->ladderidx++;
	zz->ladder[zz->ladderidx].txsyn = ZZ_IMPLICIT;
	zz->ladder[zz->ladderidx].size = UNLIMITED;
	zz->ladder[zz->ladderidx].group = ZZ_GROUP(key);
	zz->ladder[zz->ladderidx].type = ZZ_SEQUENCE;
}

/// Pass in NULL to use UNLIMITED size
void zzwUN_end(struct zzfile *zz, long *pos)
{
	if (pos)	// set exact size of data written
	{
		ziwriteu32at(zz->zi, ziwritepos(zz->zi) - *pos - 4, *pos);
	}
	else
	{
		implicit(zz->zi, 0xfffe, 0xe0dd, 0);	// add end seq tag
	}
	zz->ladderidx--;
}

void zzwItem_begin(struct zzfile *zz, long *pos)
{
	implicit(zz->zi, 0xfffe, 0xe000, UNLIMITED);
	verboseprint(zz, " ");
	if (pos) *pos = ziwritepos(zz->zi) - 4;	// position of length value
}

void zzwItem_end(struct zzfile *zz, long *pos)
{
	if (pos)	// set exact size of data written
	{
		ziwriteu32at(zz->zi, ziwritepos(zz->zi) - *pos - 4, *pos);
	}
	else
	{
		implicit(zz->zi, 0xfffe, 0xe00d, 0);	// add end item tag
		verboseprint(zz, " ");
	}
}

void zzwSQ_begin(struct zzfile *zz, zzKey key, long *pos)
{
	zzwSQ(zz, key, UNLIMITED);
	if (pos) *pos = ziwritepos(zz->zi) - 4;	// position of length value
	zz->ladderidx++;
	zz->ladder[zz->ladderidx].txsyn = ZZ_EXPLICIT;
	zz->ladder[zz->ladderidx].size = UNLIMITED;
	zz->ladder[zz->ladderidx].group = ZZ_GROUP(key);
	zz->ladder[zz->ladderidx].type = ZZ_SEQUENCE;
}

void zzwSQ_end(struct zzfile *zz, long *pos)
{
	zzwUN_end(zz, pos);
}

void zzwPixelData_begin(struct zzfile *zz, long frames, int bits, long size)
{
	int i;
	uint32_t tmp = 0;
	enum VR vr = OB;

	if ((bits == 16 && zz->ladder[zz->ladderidx].txsyn == ZZ_EXPLICIT) || zz->ladder[zz->ladderidx].txsyn == ZZ_IMPLICIT)
	{
		vr = OW;	// all other cases use OB
	}
	if (zz->ladder[zz->ladderidx].txsyn > ZZ_EXPLICIT)
	{
		writetag(zz, DCM_PixelData, vr, UNLIMITED);	// encapsulated pixel data
		implicit(zz->zi, 0xfffe, 0xe000, sizeof(tmp) * frames);
		zz->pxOffsetTable = ziwritepos(zz->zi);	// position of index table
		for (i = 0; i < frames; i++)
		{
			ziwrite(zz->zi, &tmp, sizeof(tmp));	// write empty index table
		}
	}
	else
	{
		writetag(zz, DCM_PixelData, vr, size);
		zz->pxOffsetTable = -1;			// no index table
	}
	zz->frames = frames;
	verboseprint(zz, " ");
}

void zzwPixelData_frame(struct zzfile *zz, int frame, const void *data, uint32_t size)
{
	long wlen = size;
	if (size % 2 != 0) wlen++;			// padding
	if (zz->ladder[zz->ladderidx].txsyn > ZZ_EXPLICIT)	// write encapsulation
	{
		ziwriteu32at(zz->zi, ziwritepos(zz->zi) - (zz->pxOffsetTable + zz->frames * sizeof(uint32_t)), zz->pxOffsetTable + frame * sizeof(uint32_t));
		implicit(zz->zi, 0xfffe, 0xe000, wlen);
	}
	ziwrite(zz->zi, data, size);
	if (size % 2 != 0) ziwrite(zz->zi, "", 1);	// pad
}

void zzwPixelData_end(struct zzfile *zz)
{
	if (zz->ladder[zz->ladderidx].txsyn > ZZ_EXPLICIT)	// terminate encapsulation
	{
		implicit(zz->zi, 0xfffe, 0xe0dd, 0);
	}
}

void zzwAT(struct zzfile *zz, zzKey key, zzKey key2)
{
	const uint16_t group2 = ZZ_GROUP(key2);
	const uint16_t element2 = ZZ_ELEMENT(key2);

	writetag(zz, key, AT, sizeof(group2) + sizeof(element2));
	ziwrite(zz->zi, &group2, sizeof(group2));
	ziwrite(zz->zi, &element2, sizeof(element2));
	verboseprint(zz, "[%04x,%04x]", group2, element2);
}

void zzwUL(struct zzfile *zz, zzKey key, uint32_t value)
{
	writetag(zz, key, UL, sizeof(value));
	ziwrite(zz->zi, &value, sizeof(value));
	verboseprint(zz, "[%u]", value);
}

void zzwULv(struct zzfile *zz, zzKey key, int len, const uint32_t *value)
{
	const int size = sizeof(*value) * len;
	writetag(zz, key, UL, size);
	ziwrite(zz->zi, value, size);
	verboseprint(zz, "[%u...]", value[0]);
}

void zzwSL(struct zzfile *zz, zzKey key, int32_t value)
{
	writetag(zz, key, SL, sizeof(value));
	ziwrite(zz->zi, &value, sizeof(value));
	verboseprint(zz, "[%d]", value);
}

void zzwSS(struct zzfile *zz, zzKey key, int16_t value)
{
	writetag(zz, key, SS, sizeof(value));
	ziwrite(zz->zi, &value, sizeof(value));
	verboseprint(zz, "[%d]", value);
}

void zzwUS(struct zzfile *zz, zzKey key, uint16_t value)
{
	writetag(zz, key, US, sizeof(value));
	ziwrite(zz->zi, &value, sizeof(value));
	verboseprint(zz, "[%u]", value);
}

void zzwFL(struct zzfile *zz, zzKey key, float value)
{
	writetag(zz, key, FL, sizeof(value));
	ziwrite(zz->zi, &value, sizeof(value));
	verboseprint(zz, "[%f]", value);
}

void zzwFD(struct zzfile *zz, zzKey key, double value)
{
	writetag(zz, key, FD, sizeof(value));
	ziwrite(zz->zi, &value, sizeof(value));
	verboseprint(zz, "[%f]", value);
}

void zzwOB(struct zzfile *zz, zzKey key, int len, const char *string)
{
	int wlen = len;

	if (len % 2 != 0) wlen++;			// padding
	writetag(zz, key, OB, wlen);
	ziwrite(zz->zi, string, len);
	if (len % 2 != 0) ziwrite(zz->zi, "", 1);	// pad
	verboseprint(zz, "...");
}

void zzwOW(struct zzfile *zz, zzKey key, int len, const uint16_t *string)
{
	const int size = len * 2;
	writetag(zz, key, OW, size);
	ziwrite(zz->zi, string, size);
	verboseprint(zz, "...");
}

void zzwOF(struct zzfile *zz, zzKey key, int len, const float *string)
{
	const int size = len * 4;
	writetag(zz, key, OF, size);
	ziwrite(zz->zi, string, size);
	verboseprint(zz, "[%f...]", string[0]);
}

void zzwUI(struct zzfile *zz, zzKey key, const char *string)
{
	int length = MIN(strlen(string), (size_t)64);
	int wlen = length;

	if (length % 2 != 0) wlen++;			// padding
	writetag(zz, key, UI, wlen);
	ziwrite(zz->zi, string, length);
	if (length % 2 != 0) ziwrite(zz->zi, "", 1);	// pad with null
	verboseprint(zz, "[%s]", string);
}

static inline int countdelims(const char *str, const char delim)
{
	unsigned i, c = 0;
	for (i = 0; i < strlen(str); i++)
	{
		if (str[i] == delim) c++;
	}
	return c;
}

static void wstr(struct zzfile *zz, zzKey key, const char *string, enum VR vr, size_t maxlen)
{
	const size_t multiplen = maxlen * (countdelims(string, '\\') + 1);	// max length times value multiplicity
	const size_t length = MIN(strlen(string), multiplen);
	size_t wlen = length;

	if (length % 2 != 0) wlen++;			// padding
	writetag(zz, key, vr, wlen);
	ziwrite(zz->zi, string, length);
	if (length % 2 != 0)
	{
		// pad with a space to make even length
		ziwrite(zz->zi, " ", 1);
	}
	verboseprint(zz, "[%s]", string);
}
void zzwPN(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, PN, 64); }
void zzwSH(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, SH, 16); }
void zzwAE(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, AE, 16); }
void zzwAS(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, AS, 4); }
void zzwCS(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, CS, 16); }
void zzwLO(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, LO, 64); }
void zzwLT(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, LT, 10240); }
void zzwST(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, ST, 1024); }
void zzwUT(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, UT, UINT32_MAX - 1); }
void zzwDSs(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, DS, 16); }
void zzwDAs(struct zzfile *zz, zzKey key, const char *string) { wstr(zz, key, string, DA, 8); }

void zzwIS(struct zzfile *zz, zzKey key, int value)
{
	char tmp[MAX_LEN_IS];

	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp) - 1, "%d", value);
	wstr(zz, key, tmp, IS, 12);
}

void zzwDSd(struct zzfile *zz, zzKey key, double value)
{
	char tmp[MAX_LEN_DS];

	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp) - 1, "%g", value);
	wstr(zz, key, tmp, DS, 16);
}

void zzwDSdv(struct zzfile *zz, zzKey key, int len, const double *value)
{
	size_t wlen = 0, lenval;
	long pos = ziwritepos(zz->zi);
	int i;
	char tmp[MAX_LEN_DS + 1];

	// Set to position of size field, depending on transfer syntax
	pos += explicit(zz) ? 6 : 4;
	writetag(zz, key, DS, 0);

	// Write out values
	for (i = 0; i < len; i++)
	{
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp) - 2, "%g", value[i]);
		lenval = strlen(tmp);
		if (i + 1 < len)
		{
			strcat(tmp, "\\");	// value delimiter
			lenval++;
		}
		ziwrite(zz->zi, tmp, lenval);
		wlen += lenval;
	}
	if (wlen % 2 != 0)
	{
		wlen++;				// padding
		ziwrite(zz->zi, " ", 1);	// pad with a space to make even length
	}

	// Now set the size of the tag
	if (explicit(zz)) ziwriteu16at(zz->zi, wlen, pos);
	else ziwriteu32at(zz->zi, wlen, pos);
	verboseprint(zz, "[%f...]", value[0]);
}

void zzwDA(struct zzfile *zz, zzKey key, time_t datestamp)
{
	char tmp[MAX_LEN_DA];
	struct tm stamp;

	localtime_r(&datestamp, &stamp);
	strftime(tmp, MAX_LEN_DA, "%Y%m%d", &stamp);
	wstr(zz, key, tmp, DA, 8);
}

void zzwTM(struct zzfile *zz, zzKey key, struct timeval datetimestamp)
{
	char tmp[MAX_LEN_TM], frac[8];
	struct tm stamp;
	time_t datestamp = datetimestamp.tv_sec;

	localtime_r(&datestamp, &stamp);
	strftime(tmp, MAX_LEN_DT, "%H%M%S", &stamp);
	if (datetimestamp.tv_usec > 0)
	{
		memset(frac, 0, sizeof(frac));
		snprintf(frac, 7, ".%06u", (unsigned)datetimestamp.tv_usec);
		strcat(tmp, frac);
	}
	wstr(zz, key, tmp, TM, 16);
}

void zzwDT(struct zzfile *zz, zzKey key, struct timeval datetimestamp)
{
	char tmp[MAX_LEN_DT], frac[8];
	struct tm stamp;
	time_t datestamp = datetimestamp.tv_sec;

	localtime_r(&datestamp, &stamp);
	strftime(tmp, MAX_LEN_DT, "%Y%m%d%H%M%S", &stamp);
	if (datetimestamp.tv_usec > 0)
	{
		memset(frac, 0, sizeof(frac));
		snprintf(frac, 7, ".%06u", (unsigned)datetimestamp.tv_usec);
		strcat(tmp, frac);
	}
	wstr(zz, key, tmp, DT, 26);
}

struct zzfile *zzcreate(const char *filename, struct zzfile *zz, const char *sopclass, const char *sopinstanceuid, const char *transfer)
{
	memset(zz, 0, sizeof(*zz));
	zz->acrNema = false;
	zz->ladder[0].txsyn = ZZ_EXPLICIT;
	zz->zi = ziopenfile(filename, "w");
	if (zz->zi) zzwHeader(zz, sopclass, sopinstanceuid, transfer);
	if (strcmp(transfer, UID_LittleEndianImplicitTransferSyntax) == 0)
	{
		zz->ladder[0].txsyn = ZZ_IMPLICIT;
	}
	return zz;
}

void zzwHeader(struct zzfile *zz, const char *sopclass, const char *sopinstanceuid, const char *transfer)
{
	char version[3];
	char zeroes[128];			// FIXME, i hate this inefficiency
	const uint32_t startpos = 128 + 4 + 8;

	version[0] = 0;
	version[1] = 1;
	version[2] = 0;
	memset(zeroes, 0, sizeof(zeroes));

	ziwrite(zz->zi, zeroes, 128);
	ziwrite(zz->zi, "DICM", 4);

	zzwUL(zz, DCM_FileMetaInformationGroupLength, 0);	// length zero, fixing it below
	zzwOB(zz, DCM_FileMetaInformationVersion, 2, version);
	zzwUI(zz, DCM_MediaStorageSOPClassUID, sopclass);
	zzwUI(zz, DCM_MediaStorageSOPInstanceUID, sopinstanceuid);
	zzwUI(zz, DCM_TransferSyntaxUID, transfer);
	zzwUI(zz, DCM_ImplementationClassUID, "1.2.3.4.8.2");
	zzwSH(zz, DCM_ImplementationVersionName, "zzdicom");
	zzwAE(zz, DCM_SourceApplicationEntityTitle, "ZZ_NONE");

	// write group size
	ziwriteu32at(zz->zi, ziwritepos(zz->zi) - (startpos + 4), startpos);
}

void zzwEmpty(struct zzfile *zz, zzKey key, enum VR vr)
{
	writetag(zz, key, vr, 0);
	verboseprint(zz, "[]");
}
