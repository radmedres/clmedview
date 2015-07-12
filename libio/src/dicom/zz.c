#include "include/byteorder.h"

#ifdef POSIX
#define _XOPEN_SOURCE 600
#include <fcntl.h>
#endif

#include "include/zz.h"
#include "include/zz_priv.h"
#include "include/zzio.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <uuid/uuid.h>

const char *versionString =
#include "include/VERSION"
;

static bool verbose = false;

// parse first few tags to grab some vital info
static struct zzfile *earlyparse(struct zzfile *zz, const char *filename)
{
	char dicm[5], endianbuf[6];

	// Check for valid Part 10 header
	zz->part10 = true;	// ever the optimist
	memset(dicm, 0, sizeof(dicm));

	if (!zisetreadpos(zz->zi, 128) || ziread(zz->zi, dicm, 4) != 4 || strncmp(dicm, "DICM", 4) != 0)
	{
		debug("%s does not have a valid part 10 DICOM header", filename);
		zisetreadpos(zz->zi, 0);	// try anyway
		zz->part10 = false;
	}

	if (ziread(zz->zi, &endianbuf, 6) != 6 || !zisetreadpos(zz->zi, zz->part10 ? 128 + 4 : 0))
	{
		return zzclose(zz); // not big enough to be a DICOM file
	}

	// Safety check - are we really reading a part 10 file? First tag MUST be (0x0002, 0x0000)
	// (but also allow (0x0002, 0x0001) since some crazy DICOM implementation uses that)
	if (endianbuf[0] != 2 || endianbuf[1] != 0 || (endianbuf[2] != 0 && endianbuf[2] != 1) || endianbuf[3] != 0)
	{
		zisetreadpos(zz->zi, 0);		// rewind and try over without the part10
		ziread(zz->zi, &endianbuf, 6);
		zisetreadpos(zz->zi, 0);
		zz->part10 = false;
	}

	// Check for big-endian syntax - not supported
	if (endianbuf[0] < endianbuf[1])
	{
		warning("%s appears to be big-endian - this is not supported", filename);
		return zzclose(zz);
	}

	// Naive check for explicit, but unlikely to fail since size of the first tag would have to be very
	// large to have two uppercase letters in it, and it is always very small. Reusing excess data in endianbuf.
	// The reason for this check is that some broken early DICOM implementations wrote the header in implicit.
	if (isupper(endianbuf[4]) && isupper(endianbuf[5]))
	{
		zz->ladder[0].txsyn = ZZ_EXPLICIT;
	}
	else
	{
		zz->ladder[0].txsyn = ZZ_IMPLICIT;
	}

	zz->ladder[0].pos = zireadpos(zz->zi);
	zz->ladder[0].item = -1;
	zz->ladder[0].size = UNLIMITED;
	zz->current.frame = -1;
	zz->frames = -1;
	zz->pxOffsetTable = -1;
	return zz;
}

struct zzfile *zzstdin(struct zzfile *zz)
{
	if (zz)
	{
		memset(zz, 0, sizeof(*zz));
		zz->fileSize = UNLIMITED;
		zz->zi = ziopenstdin();
		//zz->modifiedTime = ?
		return earlyparse(zz, "(stdin)");
	}
	return zz;
}

struct zzfile *zzopen(const char *filename, const char *mode, struct zzfile *infile)
{
	struct zzfile *zz;
	struct stat st;

	if (!infile || !mode || !filename)
	{
		return NULL;
	}
	zz = infile;
	memset(zz, 0, sizeof(*zz));
	if (stat(filename, &st) != 0)
	{
		warning("%s could not be found: %s", filename, strerror(errno));
		return NULL;
	}
	if (!S_ISREG(st.st_mode))
	{
		warning("%s is not a file", filename);
		return NULL;
	}
	zz->zi = ziopenfile(filename, mode);
	if (!zz->zi || !realpath(filename, zz->fullPath))
	{
		warning("%s could not be opened: %s", filename, strerror(errno));
		return NULL;
	}
	zz->fileSize = st.st_size;
	zz->modifiedTime = st.st_mtime;
	ziwillneed(zz->zi, 0, 4096 * 4);	// request 4 pages right away
	return earlyparse(zz, filename);
}

bool zztostring(struct zzfile *zz, char *input, int strsize, int charsize)
{
	int i;
	memset(input, 0, strsize);
	if (zz->current.length == 0)
	{
		strncpy(input, "(no value available)", strsize - 1);
		return false;
	}
	switch (zz->current.vr)
	{
	case SQ:
		strncpy(input, "(Sequence)", strsize - 1);
		return false;
	case OB: case OW: case OF: case UT:
		strncpy(input, "...", strsize - 1);
		return false;
	case UN:
		if (zz->current.length == UNLIMITED)
		{
			strncpy(input, "(Sequence)", strsize - 1);
			return false;
		} // else fallthrough
	case AE: case AS: case CS: case DA: case DS: case DT: case IS:
	case LT: case PN: case SH: case ST: case TM: case UI: case LO:
		if (!zzgetstring(zz, input, strsize))
		{
			strncpy(input, "(Error)", strsize - 1);
			break;
		}
		if (zz->current.length > strsize - 1
		    || (zz->utf8 && (int)strlen_utf8(input) > charsize)
		    || (!zz->utf8 && (int)strlen(input) > charsize))
		{
			input[charsize - 1] = '\0';
			input[charsize - 2] = '.';
			input[charsize - 3] = '.';
			input[charsize - 4] = '.';
		}
		for (i = strlen(input) - 1; i >= 0; i--) // strip control chars
		{
			if (zz->current.vr == UN && ((uint8_t)input[i] < 32 || (uint8_t)input[i] >= 127))
			{
				strncpy(input, "(Unknown)", strsize - 1);
				return false; // gave up parsing this UN
			}
			if ((uint8_t)input[i] < 32)
			{
				input[i] = ' '; // make sure we do not print commands
			}
		}
		break;
	case AT:
		snprintf(input, strsize - 1, "(%04x,%04x)", zzgetuint16(zz, 0), zzgetuint16(zz, 1));
		break;
	case UL:
		snprintf(input, strsize - 1, "%u", zzgetuint32(zz, 0));
		break;
	case US:
		snprintf(input, strsize - 1, "%u", zzgetuint16(zz, 0));
		break;
	case SS:
		snprintf(input, strsize - 1, "%d", zzgetint16(zz, 0));
		break;
	case SL:
		snprintf(input, strsize - 1, "%d", zzgetint32(zz, 0));
		break;
	case FL:
		snprintf(input, strsize - 1, "%f", zzgetfloat(zz, 0));
		break;
	case FD:
		snprintf(input, strsize - 1, "%g", zzgetdouble(zz, 0));
		break;
	case OX:
	case NO:
	case HACK_VR:
		return false;
	}
	return true;
}

int zzrDS(struct zzfile *zz, int len, double *input)
{
	char value[MAX_LEN_DS];
	long curr = zz->current.pos;
	int found = 0, ch, strpos = 0;

	zisetreadpos(zz->zi, zz->current.pos);
	memset(value, 0, sizeof(value));
	while (!zieof(zz->zi) && curr < zz->current.pos + zz->current.length && found < len)
	{
		ch = zigetc(zz->zi);
		if (ch == '\\' || ch == EOF)	// found one value
		{
			input[found++] = atof(value);
			memset(value, 0, sizeof(value));
			strpos = 0;
		}
		else if (strpos < MAX_LEN_DS)
		{
			value[strpos++] = ch;
		}
		curr++;
	}
	// last value
	if (found < len)
	{
		input[found++] = atof(value);
	}
	return found;
}

int zzrIS(struct zzfile *zz, int len, long *input)
{
	char value[MAX_LEN_IS];
	long curr = zz->current.pos;
	int found = 0, ch, strpos = 0;

	zisetreadpos(zz->zi, zz->current.pos);
	memset(value, 0, sizeof(value));
	while (!zieof(zz->zi) && curr < zz->current.pos + zz->current.length && found < len)
	{
		ch = zigetc(zz->zi);
		if (ch == '\\' || ch == EOF)	// found one value
		{
			input[found++] = atol(value);
			memset(value, 0, sizeof(value));
			strpos = 0;
		}
		else if (strpos < MAX_LEN_IS)
		{
			value[strpos++] = ch;
		}
		curr++;
	}
	// last value
	if (found < len)
	{
		input[found++] = atol(value);
	}
	return found;
}

char *zzgetstring(struct zzfile *zz, char *input, long strsize)
{
	const long desired = MIN(zz->current.length, strsize - 1);
	long result, last;

	zisetreadpos(zz->zi, zz->current.pos);
	last = result = ziread(zz->zi, input, desired);
	input[MIN(result, strsize - 1)] = '\0';	// make sure we zero terminate
	while (last > 0 && input[--last] == ' ')	// remove trailing whitespace
	{
		input[last] = '\0';
	}
	return (result == desired) ? input : NULL;
}

#define CHECK_SEEK_READ(zz, val, idx) \
	zz->current.length >= (long)sizeof(val) * (idx + 1) && zisetreadpos(zz->zi, zz->current.pos + idx * sizeof(val)) && ziread(zz->zi, &val, sizeof(val)) == sizeof(val)

float zzgetfloat(struct zzfile *zz, int idx)
{
	float val;

	if (CHECK_SEEK_READ(zz, val, idx))
	{
		return LE_32(val);
	}
	return 0.0f;
}

double zzgetdouble(struct zzfile *zz, int idx)
{
	double val;

	if (CHECK_SEEK_READ(zz, val, idx))
	{
		return LE_64(val);
	}
	return 0.0;
}

uint32_t zzgetuint32(struct zzfile *zz, int idx)
{
	uint32_t val;

	if (CHECK_SEEK_READ(zz, val, idx))
	{
		return LE_32(val);
	}
	return 0;
}

uint16_t zzgetuint16(struct zzfile *zz, int idx)
{
	uint16_t val;

	if (CHECK_SEEK_READ(zz, val, idx))
	{
		return LE_16(val);
	}
	return 0;
}

int32_t zzgetint32(struct zzfile *zz, int idx)
{
	int32_t val;

	if (CHECK_SEEK_READ(zz, val, idx))
	{
		return LE_32(val);
	}
	return 0;
}

int16_t zzgetint16(struct zzfile *zz, int idx)
{
	int16_t val;

	if (CHECK_SEEK_READ(zz, val, idx))
	{
		return LE_16(val);
	}
	return 0;
}

static void ladder_reduce(struct zzfile *zz, zzKey key)
{
	// Did we leave a group, sequence or item? We can drop out of multiple at the same time.
	while (zz->ladderidx > 0)
	{
		long bytesread = zireadpos(zz->zi) - zz->ladder[zz->ladderidx].pos;
		long size = (zz->ladder[zz->ladderidx].size >= 0) ? zz->ladder[zz->ladderidx].size : INT32_MAX;	// for 32bit systems where UNLIMITED is -1

		if (zz->ladder[zz->ladderidx].type == ZZ_GROUP
		    && ((zz->current.group != zz->ladder[zz->ladderidx].group && zz->current.group != 0xfffe)
		        || bytesread > size
		        || key == DCM_SequenceDelimitationItem
		        || key == DCM_ItemDelimitationItem))
		{
			if ((key == DCM_SequenceDelimitationItem || key == DCM_ItemDelimitationItem || zz->current.group != zz->ladder[zz->ladderidx].group)
			    && bytesread < size && zz->ladder[zz->ladderidx].size != UNLIMITED)
			{
				sprintf(zz->current.warning, "Group %04x has wrong size (%ld, ended at %ld)", zz->ladder[zz->ladderidx].group, size, bytesread);
				zz->current.valid = false;
			}
			zz->ladderidx--;	// end parsing this group now
			continue;
		}
		else if (zz->ladder[zz->ladderidx].type == ZZ_ITEM
		         && (bytesread > size
		             || key == DCM_SequenceDelimitationItem
		             || key == DCM_ItemDelimitationItem))
		{
			if ((key == DCM_SequenceDelimitationItem || key == DCM_ItemDelimitationItem) && bytesread < size && zz->ladder[zz->ladderidx].size != UNLIMITED)
			{
				sprintf(zz->current.warning, "Item has wrong size (%ld, ended at %ld)", size, bytesread);
				zz->current.valid = false;
			}
			if (bytesread > size)
			{
				zz->currNesting--;
			}
			zz->nextNesting--;
			zz->ladderidx--;	// end parsing this item now
			continue;
		}
		else if (zz->ladder[zz->ladderidx].type == ZZ_SEQUENCE
		         && (bytesread > size || key == DCM_SequenceDelimitationItem || (zz->current.group == 0xffff && zz->current.element == 0xffff)))
		{
			// ^^ (0xffff,0xffff) is a Siemens CSA abomination pretending to be a sequence delimination item
			if (key == DCM_SequenceDelimitationItem && bytesread < size && zz->ladder[zz->ladderidx].size != UNLIMITED)
			{
				sprintf(zz->current.warning, "Sequence has wrong size (%ld, ended at %ld)", size, bytesread);
				zz->current.valid = false;
			}
			if (bytesread > size)
			{
				zz->currNesting--;
			}
			if (ZZ_KEY(zz->ladder[zz->ladderidx].group, zz->ladder[zz->ladderidx].element) == DCM_PerFrameFunctionalGroupsSequence)
			{
				zz->current.frame = -1;	// out of frames scope
			}
			zz->nextNesting--;
			// do not react twice on the same sequence delimiter
			if (key == DCM_SequenceDelimitationItem && zz->ladder[zz->ladderidx].size == UNLIMITED) key = 0;
			zz->ladderidx--;	// end parsing this sequence now
			continue;
		}
		break;		// no further cause for regress found
	}
}

bool zzread(struct zzfile *zz, uint16_t *group, uint16_t *element, long *len)
{
	char transferSyntaxUid[MAX_LEN_UI];
	// Represent the three different variants of tag headers in one union
	struct
	{
		uint16_t group;
		uint16_t element;
		union { uint32_t len; struct { char vr[2]; uint16_t len; } evr; } buffer;
	} header;
	zzKey key;

	zz->currNesting = zz->nextNesting;
	zz->current.fake = false;

	// Did we leave a group, sequence or item? We can drop out of multiple at the same time.
	// Considering only ladder size and position here, so that we can generate fake delimiters.
	while (zz->ladderidx > 0)
	{
		long bytesread = zireadpos(zz->zi) - zz->ladder[zz->ladderidx].pos + 1;
		long size = (zz->ladder[zz->ladderidx].size >= 0) ? zz->ladder[zz->ladderidx].size : INT32_MAX;	// for 32bit systems where UNLIMITED is -1

		if (zz->ladder[zz->ladderidx].type == ZZ_GROUP && bytesread > size)
		{
			zz->ladderidx--;	// end parsing this group now
			continue;
		}
		else if (zz->ladder[zz->ladderidx].type == ZZ_ITEM && bytesread > size)
		{
			zz->nextNesting--;
			zz->ladderidx--;	// end parsing this item now
			zz->current.group = *group = ZZ_GROUP(DCM_ItemDelimitationItem);
			zz->current.element = *element = ZZ_ELEMENT(DCM_ItemDelimitationItem);
			zz->current.fake = true;
			return true;
		}
		else if (zz->ladder[zz->ladderidx].type == ZZ_SEQUENCE && bytesread > size)
		{
			if (ZZ_KEY(zz->ladder[zz->ladderidx].group, zz->ladder[zz->ladderidx].element) == DCM_PerFrameFunctionalGroupsSequence)
			{
				zz->current.frame = -1;	// out of frames scope
			}
			zz->nextNesting--;
			zz->ladderidx--;	// end parsing this sequence now
			zz->current.group = *group = ZZ_GROUP(DCM_SequenceDelimitationItem);
			zz->current.element = *element = ZZ_ELEMENT(DCM_SequenceDelimitationItem);
			zz->current.fake = true;
			return true;
		}
		break;		// no further cause for regress found
	}

	if (ziread(zz->zi, &header, 8) != 8)		// group+element then either VR + 0, VR+VL, or just VL
	{
		return false;
	}
	zz->current.valid = true;
	zz->current.warning[0] = '\0';
	zz->previous.group = zz->current.group;
	zz->previous.element = zz->current.element;
	zz->previous.ladderidx = zz->ladderidx;
	zz->current.group = *group = header.group;
	zz->current.element = *element = header.element;
	key = ZZ_KEY(header.group, header.element);

	// Check if we should reduce ladder depth
	ladder_reduce(zz, key);

	// Having possibly reduced the ladder stack, now reduce private group stack to fit, if necessary.
	while (zz->privmax >= 0
	       && (zz->privgroup[zz->privmax].ladderidx > zz->ladderidx
	           || (zz->privgroup[zz->privmax].ladderidx == zz->ladderidx && zz->privgroup[zz->privmax].group != zz->current.group && zz->current.group != 0xfffe)))
	{
		zz->privmax--;
		zz->prividx = MIN(zz->privmax, zz->prividx);
	}

	// Is it a private group tag? If so, a new one? If so, do we have a creator for it?
	if (zz->privmax >= 0 && zz->privmax > zz->prividx && zz->current.group % 2 > 0
	    && zz->current.element >= 0x1000
	    && (zz->prividx == -1 || (zz->current.element >> 8) << 8 != zz->privgroup[zz->prividx].offset
	        || zz->privgroup[zz->prividx].ladderidx != zz->ladderidx))
	{
		int i;
		for (i = 0; i <= zz->privmax; i++)
		{
			if ((zz->current.element >> 8) << 8 == zz->privgroup[i].offset && zz->privgroup[i].ladderidx == zz->ladderidx)
			{
				zz->prividx = i;
				break;
			}
		}
	}
	// Not a private group?
	else if (zz->current.group % 2 == 0)
	{
		zz->prividx = -1;
	}

	// Try explicit VR
	if (zz->ladder[zz->ladderidx].txsyn != ZZ_IMPLICIT && key != DCM_Item && key != DCM_ItemDelimitationItem
	    && key != DCM_SequenceDelimitationItem && header.group != 0x0000)
	{
		uint32_t rlen;
		zz->current.vr = ZZ_VR(header.buffer.evr.vr[0], header.buffer.evr.vr[1]);

		switch (zz->current.vr)
		{
		case AE: case AS: case AT: case CS: case DA: case DS: case DT: case FL: case FD: case IS: case LO:
		case LT: case PN: case SH: case SL: case SS: case ST: case TM: case UI: case UL: case US: case HACK_VR:
			*len = LE_16(header.buffer.evr.len);	// the insane 16 bit size variant
			break;
		case SQ: case UN: case OB: case OW: case OF: case UT:
		default:	// all future VRs are 4 byte size, see PS3.5 6.2
			if (ziread(zz->zi, &rlen, 4) != 4)
			{
				return false;		// the 32 bit variant
			}
			*len = LE_32(rlen);
			break;
		}
		if (zz->current.vr == SQ || (*len == UNLIMITED && zz->current.vr == UN))
		{
			zz->nextNesting++;
		}
	}
	else	// the sad legacy implicit variant
	{
		zz->current.vr = NO;	// no info
		*len = LE_32(header.buffer.len);
		if (*len == UNLIMITED && key != DCM_PixelData && key != DCM_Item && key != DCM_SequenceDelimitationItem && key != DCM_ItemDelimitationItem)
		{
			zz->nextNesting++;
		}
	}
	zz->current.length = *len;
	zz->current.pos = zireadpos(zz->zi);	// anything we read after this, we roll back the position for

	switch (key)
	{
	case DCM_SpecificCharacterSet:
		zzgetstring(zz, zz->characterSet, sizeof(zz->characterSet) - 1);
		zz->utf8 = strcmp(zz->characterSet, "ISO_IR 192") == 0;
		break;
	case DCM_NumberOfFrames:
		zzrIS(zz, 1, &zz->frames);
		break;
	case DCM_Item:
		zz->nextNesting++;
		if (zz->current.pxstate == ZZ_PIXELDATA)
		{
			zz->current.pxstate = ZZ_OFFSET_TABLE;
			if (zz->current.length != 0)
			{
				zz->pxOffsetTable = zz->current.pos;
			}
		}
		else if (zz->current.pxstate == ZZ_OFFSET_TABLE)
		{
			zz->current.pxstate = ZZ_PIXELITEM;
			zz->current.frame = 0;
		}
		else if (zz->current.pxstate == ZZ_PIXELITEM)
		{
			zz->current.frame++;
		}
		break;
	case DCM_PixelData:
		if (*len == UNLIMITED)
		{
			// Start special ugly handling of the encapsulated pixel data attribute
			zz->current.pxstate = ZZ_PIXELDATA;
			zz->nextNesting++;
		}
		break;
	case DCM_SequenceDelimitationItem:
		if (zz->current.pxstate != ZZ_NOT_PIXEL)
		{
			zz->current.pxstate = ZZ_NOT_PIXEL;
			zz->nextNesting--;
		}
		break;
	case DCM_MediaStorageSOPClassUID:
		zzgetstring(zz, zz->sopClassUid, sizeof(zz->sopClassUid) - 1);
		break;
	case DCM_MediaStorageSOPInstanceUID:
		zzgetstring(zz, zz->sopInstanceUid, sizeof(zz->sopInstanceUid) - 1);
		break;
	case DCM_TransferSyntaxUID:
		zzgetstring(zz, transferSyntaxUid, sizeof(transferSyntaxUid) - 1);
		if (strcmp(transferSyntaxUid, UID_LittleEndianImplicitTransferSyntax) == 0)
		{
			// once over the header, start parsing implicit
			zz->ladder[0].txsyn = ZZ_IMPLICIT;
		}
		else if (strcmp(transferSyntaxUid, UID_JPEGLSLosslessTransferSyntax) == 0)
		{
			zz->ladder[0].txsyn = ZZ_EXPLICIT_JPEGLS;
		}
		else if (strcmp(transferSyntaxUid, UID_DeflatedExplicitVRLittleEndianTransferSyntax) == 0)
		{
			// once over the header, start inflating
			zz->ladder[0].txsyn = ZZ_EXPLICIT_COMPRESSED;
			warning("Deflate transfer syntax found - not supported yet");
			return false;
		}
		else if (strcmp(transferSyntaxUid, UID_BigEndianExplicitTransferSyntax) == 0)
		{
			warning("Big endian transfer syntax found - not supported");
			return false;
		}
		// else continue to believe it is explicit little-endian, which really is the only sane thing to use
		break;
	case DCM_ACR_NEMA_RecognitionCode:
		zz->acrNema = true;
		break;
	case DCM_ItemDelimitationItem:
	default:
		break;
	}

	if (key == DCM_FileMetaInformationGroupLength)
	{
		zz->ladderidx = 1;
		zz->ladder[1].pos = zz->current.pos;
		zz->ladder[1].size = zzgetuint32(zz, 0);
		zz->ladder[1].txsyn = zz->ladder[0].txsyn;
		zz->ladder[1].group = header.group;
		zz->ladder[1].element = header.element;
		zz->ladder[1].type = ZZ_GROUP;
		zz->ladder[1].item = -1;
	}
	else if (zz->ladder[zz->ladderidx].csahack)
	{
		// do nothing
	}
	else if (header.element == 0x0000 || (key != DCM_PixelData && *len == UNLIMITED) || zz->current.vr == SQ || key == DCM_Item)
	{
		// Entered into a group or sequence, copy parameters
		if (zz->ladderidx + 1 >= MAX_LADDER)
		{
			warning("Too deep group/sequence nesting - giving up");
			return false;	// stop parsing and give up!
		}
		zz->ladderidx++;
		memset(&zz->ladder[zz->ladderidx], 0, sizeof(zz->ladder[zz->ladderidx]));
		zz->ladder[zz->ladderidx].group = header.group;
		zz->ladder[zz->ladderidx].element = header.element;
		if (header.element == 0x0000)
		{
			zz->ladder[zz->ladderidx].size = zzgetuint32(zz, 0);
			zz->ladder[zz->ladderidx].type = ZZ_GROUP;
			zz->ladder[zz->ladderidx].item = -1;
		}
		else
		{
			zz->ladder[zz->ladderidx].size = *len;
			if (key == DCM_Item)
			{
				int i;

				zz->ladder[zz->ladderidx].type = ZZ_ITEM;
				zz->ladder[zz->ladderidx].item++;
				for (i = zz->ladderidx - 1; i >= 0; i--)
				{
					if (ZZ_KEY(zz->ladder[i].group, zz->ladder[i].element) == DCM_PerFrameFunctionalGroupsSequence)
					{
						zz->current.frame++;	// if parent is advanced to the next frame
						break;
					}
					else if (zz->ladder[i].type == ZZ_SEQUENCE)
					{
						break;			// not direct parent, so stop checking here
					}
				}
			}
			else
			{
				zz->ladder[zz->ladderidx].type = ZZ_SEQUENCE;
				zz->ladder[zz->ladderidx + 0].item = -1;
				zz->ladder[zz->ladderidx + 1].item = -1;	// first item brings it to zero
			}
		}
		zz->ladder[zz->ladderidx].pos = zz->current.pos;
		if (zz->current.vr != UN)
		{
			zz->ladder[zz->ladderidx].txsyn = zz->ladder[zz->ladderidx - 1].txsyn;	// inherit transfer syntax
		}
		else
		{
			zz->ladder[zz->ladderidx].txsyn = ZZ_IMPLICIT;	// UN is always implicit
		}
	}

	// Is it a private group creator?
	if (zz->privmax < MAX_LADDER && zz->current.group % 2 > 0 && zz->current.element < 0x1000 && zz->current.vr == LO
	    && !zz->ladder[zz->ladderidx].csahack)
	{
		zz->privmax++;
		zz->privgroup[zz->privmax].offset = zz->current.element << 8;
		memset(zz->privgroup[zz->privmax].creator, 0, sizeof(zz->privgroup[zz->privmax].creator));
		if (!zzgetstring(zz, zz->privgroup[zz->privmax].creator, sizeof(zz->privgroup[zz->privmax].creator) - 1))
		{
			strncpy(zz->privgroup[zz->privmax].creator, "(Error)", MAX_LEN_LO - 1);
		}
		zz->privgroup[zz->privmax].ladderidx = zz->ladderidx;
		zz->privgroup[zz->privmax].group = zz->current.group;
	}

	return true;
}

int zzutil(int argc, char **argv, int minArgs, const char *usage, const char *help, struct zzopts *opts)
{
	FILE *out = stderr;
	int i, ignparams = 1;	// always tell caller to ignore argv[0]
	int extraparams = 0;	// params required by other params
	bool usageReq = false;
	struct zzopts *iter;

	iter = opts;
	while (iter && iter->opt)
	{
		iter->found = false;
		iter++;
	}
	for (i = 1; i < argc; i++)
	{
		if (extraparams)
		{
			// eat this parameter
			extraparams--;
		}
		else if (strcmp(argv[i], "--") == 0)
		{
			ignparams++;
			break;	// stop parsing command line parameters
		}
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
		{
			fprintf(stdout, "%s\n", help);
			fprintf(stdout, "  %-28s Verbose output\n", "-v");
			fprintf(stdout, "  %-28s This help\n", "-h|--help");
			fprintf(stdout, "  %-28s Version of zzdicom package\n", "--version");
			fprintf(stdout, "  %-28s Short help\n", "--usage");
			iter = opts;
			while (iter && iter->opt)	// null-entry terminated
			{
				fprintf(stdout, "  %-28s %s\n", iter->opt, iter->description);
				iter++;
			}
			usageReq = true;
			out = stdout;
		}
		else if (strcmp(argv[i], "--usage") == 0)
		{
			usageReq = true;
			out = stdout;
		}
		else if (strcmp(argv[i], "--version") == 0)
		{
			fprintf(stdout, "%s is part of zzdicom version %s\n", argv[0], versionString);
			exit(0);
		}
		else if (strcmp(argv[i], "-v") == 0)	// verbose
		{
			ignparams++;
			verbose = true;
		}
		else
		{
			iter = opts;
			while (iter && iter->opt)
			{
				int lim = strlen(iter->opt);
				char *space = strchr(iter->opt, ' ');
				if (space)
				{
					lim = space - iter->opt;
				}
				if (strncmp(argv[i], iter->opt, lim) == 0)
				{
					iter->found = true;
					iter->argstart = i;
					ignparams += 1 - (int)iter->counts + iter->args;
					extraparams += iter->args;
				}
				iter++;
			}
		}
	}
	if (extraparams || usageReq || argc - ignparams < minArgs)
	{
		fprintf(out, "Usage: %s [-v|--version|-h|--usage", argv[0]);
		iter = opts;
		while (iter && iter->opt)
		{
			fprintf(out, "|%s", iter->opt);
			iter++;
		}
		fprintf(out, "] %s\n", usage);
		exit(!usageReq);
	}
	return ignparams;
}

void zziterinit(struct zzfile *zz)
{
	if (zz)
	{
		if (zirewindable(zz->zi))
		{
			zisetreadpos(zz->zi, zz->ladder[0].pos);
		}
		else
		{
			zz->fileSize = UNLIMITED;
		}
		zz->currNesting = 0;
		zz->nextNesting = 0;
		zz->ladderidx = 0;
		zz->prividx = -1;
		zz->privmax = -1;
		zz->current.pos = -1;
		zz->current.group = 0;
		zz->current.element = 0;
		zz->current.length = 0;
		zz->current.pxstate = ZZ_NOT_PIXEL;
	}
}

bool zziternext(struct zzfile *zz, uint16_t *group, uint16_t *element, long *len)
{
	// Check if we should read the next tag -- try to iterate over as many tags as possible, even if data is totally fubar
	if (zz && !zieof(zz->zi) && !zierror(zz->zi)
	    && (zz->current.length == UNLIMITED || (zz->current.pos + zz->current.length < zz->fileSize)
	        || zz->current.vr == SQ || zz->current.group == 0xfffe || zz->fileSize == UNLIMITED))
	{
		bool doread = true;

		if (zz->current.pos > 0 && zz->current.length > 0 && zz->current.length != UNLIMITED
		    && !(zz->current.group == 0xfffe && zz->current.element == 0xe000 && zz->current.pxstate == ZZ_NOT_PIXEL)
		    && zz->current.vr != SQ)
		{
			// go to start of next tag
			if (!zisetreadpos(zz->zi, zz->current.pos + zz->current.length))
			{
				doread = false;
			}
			// note if this conditional is not entered, we will try to parse the contents of the tag
		}
		if (doread && !zieof(zz->zi) && zzread(zz, group, element, len))
		{
			return true;
		}
	}
	// Final check for ladder depth reductions
	if (zz)
	{
		zz->previous.group = zz->current.group;
		zz->previous.element = zz->current.element;
		zz->current.group = 0xffff;
		zz->current.element = 0xffff;
		ladder_reduce(zz, ZZ_KEY(0xffff, 0xffff));
	}
	*len = 0;
	return false;	// do NOT use any other returned data in this case!
}

struct zzfile *zzclose(struct zzfile *zz)
{
	if (zz && zz->zi)
	{
		ziclose(zz->zi);
	}
	return NULL;
}

// Convert 128bit value to string
// Adapted from http://stackoverflow.com/questions/4361441/c-print-a-biginteger-in-base-10
static char *u128tostr(uint64_t hi, uint64_t lo, char *str)
{
	char *ptr = str;
	int d[39], i, j;

	memset(d, 0, sizeof(d));
	for (i = 63; i > -1; i--)
	{
		if ((hi >> i) & 1) d[0]++;
		for (j = 0; j < 39; j++) d[j] *= 2;
		for (j = 0; j < 38; j++)
		{
			d[j+1] += d[j]/10;
			d[j] %= 10;
		}
	}
	for (i = 63; i > -1; i--)
	{
		if ((lo >> i) & 1) d[0]++;
		if (i > 0) for (j = 0; j < 39; j++) d[j] *= 2;
		for (j = 0; j < 38; j++)
		{
			d[j+1] += d[j]/10;
			d[j] %= 10;
		}
	}
	for (i = 38; i > 0; i--) if (d[i] > 0) break; // skip initial zeroes
	for (; i > -1; i--) *ptr++ = '0' + d[i]; // copy & reverse string
	return str;
}

// input must be at least max UI length
void uuid_unparse_dicom(uuid_t uuid, char *str)
{
	uint64_t hi;
	uint64_t lo;
	const uint8_t *ptr = uuid;
	uint64_t tmp;
	char buf[40];

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	hi = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	lo = tmp;

	memset(buf, 0, sizeof(buf));
	sprintf(str, "2.25.%s", u128tostr(hi, lo, buf));
}

// See http://www.itu.int/rec/T-REC-X.667-200808-I/en
// TODO, when uuid_generate_time_safe() support gets widespread, consider
// that instead.
char *zzmakeuid(char *input, int size)
{
	uuid_t uuid;
	memset(input, 0, size);
	strcpy(input, "2.25.");
	uuid_generate_time(uuid); // based on MAC address
	uuid_unparse_dicom(uuid, input + strlen(input));
	return input;
}

char *zzanonuid(char *input, int size)
{
	uuid_t uuid;
	memset(input, 0, size);
	strcpy(input, "2.25.");
	uuid_generate_random(uuid); // completely random
	uuid_unparse_dicom(uuid, input + strlen(input));
	return input;
}

bool zzisverbose()
{
	return verbose;
}
