AUTOMAKE_OPTIONS              = subdir-objects
AM_CFLAGS                     = -Iinclude/                                     \
                                -I../libcommon/include/                        \
                                -I../libmemory/include/                        \
                                -Isrc/nifti/include/                           \
                                -Isrc/dicom/include/

lib_LTLIBRARIES               = libio-dicom.la libio-nifti.la

libio_dicom_la_LDFLAGS        = -module -no-undefined -avoid-version
libio_dicom_la_SOURCES        = src/libio-dicom.c src/dicom/znzlib.c           \
                                src/dicom/zzio.c src/dicom/zz.c

libio_nifti_la_LDFLAGS        = -module -no-undefined -avoid-version
libio_nifti_la_SOURCES        = src/libio-nifti.c src/nifti/nifti1_io.c
