#ifndef ZZ_WRITE_H
#define ZZ_WRITE_H

#include "zz_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

void zzwAE(struct zzfile *zz, zzKey key, const char *string);
void zzwAS(struct zzfile *zz, zzKey key, const char *string);
void zzwAT(struct zzfile *zz, zzKey key, zzKey key2);
void zzwCS(struct zzfile *zz, zzKey key, const char *string);
void zzwDA(struct zzfile *zz, zzKey key, time_t datestamp);
void zzwDAs(struct zzfile *zz, zzKey key, const char *string);
void zzwDT(struct zzfile *zz, zzKey key, struct timeval datetimestamp);
void zzwDSs(struct zzfile *zz, zzKey key, const char *string);
void zzwDSd(struct zzfile *zz, zzKey key, double value);
void zzwDSdv(struct zzfile *zz, zzKey key, int len, const double *value);
void zzwFL(struct zzfile *zz, zzKey key, float value);
void zzwFD(struct zzfile *zz, zzKey key, double value);
void zzwIS(struct zzfile *zz, zzKey key, int value);
void zzwLO(struct zzfile *zz, zzKey key, const char *string);
void zzwLT(struct zzfile *zz, zzKey key, const char *string);
void zzwOB(struct zzfile *zz, zzKey key, int len, const char *string);
void zzwOF(struct zzfile *zz, zzKey key, int len, const float *string);
void zzwOW(struct zzfile *zz, zzKey key, int len, const uint16_t *string);
void zzwPN(struct zzfile *zz, zzKey key, const char *string);
void zzwSH(struct zzfile *zz, zzKey key, const char *string);
void zzwSL(struct zzfile *zz, zzKey key, int32_t value);
void zzwSS(struct zzfile *zz, zzKey key, int16_t value);
void zzwST(struct zzfile *zz, zzKey key, const char *string);
void zzwTM(struct zzfile *zz, zzKey key, struct timeval datetimestamp);
void zzwUI(struct zzfile *zz, zzKey key, const char *string);
void zzwUL(struct zzfile *zz, zzKey key, uint32_t value);
void zzwULv(struct zzfile *zz, zzKey key, int len, const uint32_t *value);
void zzwUS(struct zzfile *zz, zzKey key, uint16_t value);
void zzwUT(struct zzfile *zz, zzKey key, const char *string);

void zzwHeader(struct zzfile *zz, const char *sopclass, const char *sopinstanceuid, const char *transfer);
void zzwEmpty(struct zzfile *zz, zzKey key, enum VR vr);
struct zzfile *zzcreate(const char *filename, struct zzfile *zz, const char *sopclass, const char *sopinstanceuid, const char *transfer);

/// Pass in NULL below to use unlimited size
void zzwUN_begin(struct zzfile *zz, zzKey key, long *pos);

/// Pass in NULL below to use unlimited size. Must be NULL here if NULL in call above.
void zzwUN_end(struct zzfile *zz, long *pos);

/// Pass in NULL below to use unlimited size. pos will contain start position of SQ.
void zzwSQ_begin(struct zzfile *zz, zzKey key, long *pos);

/// Pass in NULL below to use unlimited size. Must be NULL here if NULL in call above. pos will contain size of sequence.
void zzwSQ_end(struct zzfile *zz, long *pos);

void zzwItem_begin(struct zzfile *zz, long *pos);
void zzwItem_end(struct zzfile *zz, long *pos);

void zzwPixelData_begin(struct zzfile *zz, long frames, int bits, long size);
void zzwPixelData_frame(struct zzfile *zz, int frame, const void *data, uint32_t size);
void zzwPixelData_end(struct zzfile *zz);

/// Copy tag from one file to another, converting to explicit if necessary
void zzwCopy(struct zzfile *src, const struct zzfile *dst);

#ifdef __cplusplus
}
#endif

#endif
