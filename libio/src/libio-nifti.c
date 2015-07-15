/*
 * Copyright (C) 2015 Marc Geerlings <m.geerlings@mumc.nl>
 *
 * This file is part of clmedview.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "nifti/include/nifti1.h"
#include "libio-nifti.h"
#include "libcommon-debug.h"

#include <string.h>
#include <unistd.h>
#include <byteswap.h>
#include <math.h>

/*                                                                                                    */
/*                                                                                                    */
/* LOCAL FUNCTIONS                                                                                    */
/*                                                                                                    */
/*                                                                                                    */

MemoryDataType e_NIFTII_ConvertNIFTIIToMemoryDataType (short i16_datatype);
short int i16_NIFTII_ConvertMemoryDataTypeToNIFTII (MemoryDataType te_DataType);
short int i16_NIFTII_GetMemorySizePerElement (short i16_datatype);
short int i16_NIFTII_GetBitPix (short i16_datatype);
short int b_NIFTII_ReadHeaderToMemory (const char* pc_FileName, nifti_1_header* ps_Header);
short int b_NIFTII_ReadVolumeToMemory (const char* pc_FileName, int i32_offset, int i32_MemoryInVolume, void *pv_Data);
short int b_NIFTII_WriteHeaderToFile (const char* pc_FileName, void *pv_Data);
short int b_NIFTII_WriteImageToFile (const char* pc_FileName, int i32_BytesToWrite, int i32_PixelsInVolume, int i32_BLOB_Offset, void *pv_Data);
void v_NIFTII_convert_data_big_to_little_endian(Serie *serie);
void v_NIFTII_swap_4bytes( size_t n , void *ar );
void v_NIFTII_swap_2bytes( size_t n , void *ar );
void v_NIFTII_swap_header( struct nifti_1_header *h /*, int is_nifti*/ );


MemoryDataType
e_NIFTII_ConvertNIFTIIToMemoryDataType (short i16_datatype)
{
  debug_functions ();

  MemoryDataType e_DataType;
  switch (i16_datatype)
  {
    case DT_NONE          : e_DataType = MEMORY_TYPE_ERROR; break;
    case DT_BINARY        : e_DataType = MEMORY_TYPE_ERROR; break;

    case DT_INT8          : e_DataType = MEMORY_TYPE_INT8; break;
    case DT_INT16         : e_DataType = MEMORY_TYPE_INT16; break;
    case DT_INT32         : e_DataType = MEMORY_TYPE_INT32; break;
    case DT_INT64         : e_DataType = MEMORY_TYPE_INT64; break;

    case DT_UINT8         : e_DataType = MEMORY_TYPE_UINT8; break;
    case DT_UINT16        : e_DataType = MEMORY_TYPE_UINT16; break;
    case DT_UINT32        : e_DataType = MEMORY_TYPE_UINT32; break;
    case DT_UINT64        : e_DataType = MEMORY_TYPE_UINT64; break;

    case DT_FLOAT32       : e_DataType = MEMORY_TYPE_FLOAT32; break;
    case DT_FLOAT64       : e_DataType = MEMORY_TYPE_FLOAT64; break;
    case DT_FLOAT128      : e_DataType = MEMORY_TYPE_FLOAT128; break;

    case DT_COMPLEX64     : e_DataType = MEMORY_TYPE_COMPLEX64; break;
    case DT_COMPLEX128    : e_DataType = MEMORY_TYPE_COMPLEX128; break;
    case DT_COMPLEX256    : e_DataType = MEMORY_TYPE_COMPLEX256; break;

    case DT_RGB24         : e_DataType = MEMORY_TYPE_RGB24; break;
    case DT_RGBA32        : e_DataType = MEMORY_TYPE_RGBA32; break;

    case DT_ALL           : e_DataType = MEMORY_TYPE_ERROR; break;
    default               : e_DataType = MEMORY_TYPE_ERROR; break;
  }
  return e_DataType;
}

short int
i16_NIFTII_ConvertMemoryDataTypeToNIFTII (MemoryDataType te_DataType)
{
  short int i16_datatype = DT_NONE;

  switch (te_DataType)
  {
    case MEMORY_TYPE_ERROR      : i16_datatype = DT_NONE; break;

    case MEMORY_TYPE_INT8       : i16_datatype = DT_NONE; break;
    case MEMORY_TYPE_INT16      : i16_datatype = DT_INT16; break;
    case MEMORY_TYPE_INT32      : i16_datatype = DT_INT32; break;
    case MEMORY_TYPE_INT64      : i16_datatype = DT_INT64; break;

    case MEMORY_TYPE_UINT8      : i16_datatype = DT_UINT8; break;
    case MEMORY_TYPE_UINT16     : i16_datatype = DT_UINT16; break;
    case MEMORY_TYPE_UINT32     : i16_datatype = DT_UINT32; break;
    case MEMORY_TYPE_UINT64     : i16_datatype = DT_UINT64; break;

    case MEMORY_TYPE_FLOAT32    : i16_datatype = DT_FLOAT32; break;
    case MEMORY_TYPE_FLOAT64    : i16_datatype = DT_FLOAT64; break;
    case MEMORY_TYPE_FLOAT128   : i16_datatype = DT_FLOAT128; break;

    case MEMORY_TYPE_COMPLEX64  : i16_datatype = DT_COMPLEX64; break;
    case MEMORY_TYPE_COMPLEX128 : i16_datatype = DT_COMPLEX128; break;
    case MEMORY_TYPE_COMPLEX256 : i16_datatype = DT_COMPLEX256; break;

    case MEMORY_TYPE_RGB24      : i16_datatype = DT_RGB24; break;
    case MEMORY_TYPE_RGBA32     : i16_datatype = DT_RGBA32; break;
    default                     : i16_datatype = DT_NONE; break;
  }
  return i16_datatype;
}

short int
i16_NIFTII_GetMemorySizePerElement (short i16_datatype)
{
  debug_functions ();

  short int ui16_numberOfBytes=0;

  switch (i16_datatype)
  {
    case DT_NONE        : ui16_numberOfBytes = 0; break;
    //case DT_UNKNOWN       : ui8_numberOfBytes = 0; break;

    case DT_BINARY        : ui16_numberOfBytes = 1; break;
    case DT_UNSIGNED_CHAR : ui16_numberOfBytes = 1; break;
//    case DT_UINT8         : ui8_numberOfBytes = 1; break;
    case DT_INT8          : ui16_numberOfBytes = 1; break;

    case DT_SIGNED_SHORT  : ui16_numberOfBytes = 2; break;
//    case DT_INT16         : ui8_numberOfBytes = 2; break;
    case DT_UINT16        : ui16_numberOfBytes = 2; break;

    case DT_SIGNED_INT    : ui16_numberOfBytes = 4; break;
    case DT_UINT32        : ui16_numberOfBytes = 4; break;
//    case DT_INT32         : ui8_numberOfBytes = 4; break;
    case DT_INT64         : ui16_numberOfBytes = 8; break;

    case DT_FLOAT         : ui16_numberOfBytes = 4; break;
//    case DT_FLOAT32       : ui8_numberOfBytes = 4; break;
    case DT_FLOAT64       : ui16_numberOfBytes = 8; break;
    case DT_FLOAT128      : ui16_numberOfBytes =16; break;

    case DT_COMPLEX       : ui16_numberOfBytes = 8; break;
//    case DT_COMPLEX64     : ui8_numberOfBytes = 8; break;
    case DT_COMPLEX128    : ui16_numberOfBytes =16; break;
    case DT_COMPLEX256    : ui16_numberOfBytes =32; break;

//    case DT_DOUBLE        : ui8_numberOfBytes = 8; break;

    case DT_RGB           : ui16_numberOfBytes = 3; break;
//    case DT_RGB24         : ui8_numberOfBytes = 3; break;
    case DT_RGBA32        : ui16_numberOfBytes = 4; break;

    case DT_ALL           : ui16_numberOfBytes = 0; break;
    default               : ui16_numberOfBytes = 0; break;
  }
  return ui16_numberOfBytes;
}

short int
i16_NIFTII_GetBitPix (short i16_datatype)
{
  debug_functions ();

  short int ui8_bitpix=0;

  switch (i16_datatype)
  {
    case DT_NONE          : ui8_bitpix =  0; break;
    case DT_BINARY        : ui8_bitpix =  1; break;
    case DT_UNSIGNED_CHAR : ui8_bitpix =  1; break;
    case DT_INT8          : ui8_bitpix =  8; break;

    case DT_SIGNED_SHORT  : ui8_bitpix = 16; break;
    case DT_UINT16        : ui8_bitpix = 16; break;

    case DT_SIGNED_INT    : ui8_bitpix =  8; break;
    case DT_UINT32        : ui8_bitpix = 32; break;
    case DT_INT64         : ui8_bitpix = 64; break;

    case DT_FLOAT         : ui8_bitpix = 32; break;
    case DT_FLOAT64       : ui8_bitpix = 64; break;
    case DT_FLOAT128      : ui8_bitpix =128; break;

    case DT_COMPLEX       : ui8_bitpix = 64; break;
    case DT_COMPLEX128    : ui8_bitpix =128; break;
    case DT_COMPLEX256    : ui8_bitpix =256; break;

    case DT_RGB           : ui8_bitpix = 24; break;

    case DT_RGBA32        : ui8_bitpix = 32; break;

    case DT_ALL           : ui8_bitpix =  0; break;
    default               : ui8_bitpix =  0; break;
  }
  return ui8_bitpix;
}

short int
b_NIFTII_ReadHeaderToMemory(const char* pc_FileName, nifti_1_header* ps_Header)
{
  debug_functions ();

  FILE *pf_InputFile;
  unsigned int ui32_FileSize;
  unsigned char uc_Extention[4]={255,0,0,0};

  pf_InputFile = fopen (pc_FileName, "rb");
  if( pf_InputFile==NULL)
  {
    debug_error ("Could not open the file '%s'.", pc_FileName);
    return 0;
  }

  fseek(pf_InputFile, 0, SEEK_END);
  ui32_FileSize = ftell (pf_InputFile);
  fseek (pf_InputFile, 0, SEEK_SET);

  unsigned int ui32_BytesRead = fread (ps_Header, 1, MIN_HEADER_SIZE, pf_InputFile);
  if (ui32_BytesRead != MIN_HEADER_SIZE)
  {
    debug_error ("Error while reading the file '%s'.", pc_FileName);
    return 0;
  }

  if (ui32_FileSize != MIN_HEADER_SIZE)
  {
    fread (&uc_Extention[0], 4, 1, pf_InputFile);
    if (uc_Extention[0] != 0)
    {
      // No support for a strange extention file, so exit application
      debug_error ("There is currently no support for extended datasets.");
      return 0;
    }
  }

  fclose(pf_InputFile);
  return 1;
}

short int
b_NIFTII_ReadVolumeToMemory (const char* pc_FileName, int i32_offset, int i32_MemoryInVolume, void *pv_Data)
{
  debug_functions ();

  FILE *pf_InputFile;

  pf_InputFile = fopen (pc_FileName, "rb");
  if( pf_InputFile == NULL)
  {
    debug_error ("Could not open the file '%s'.", pc_FileName);
    return 0;
  }

  fseek (pf_InputFile, i32_offset, SEEK_SET);
  fread (pv_Data, 1, i32_MemoryInVolume, pf_InputFile);

  fclose (pf_InputFile);

  return 1;
}

short int
b_NIFTII_WriteHeaderToFile(const char* pc_FileName, void *pv_Data)
{
  debug_functions ();

  FILE *pf_OutputFile;

  pf_OutputFile = fopen (pc_FileName, "wb");
  if( pf_OutputFile==NULL)
  {
    debug_error ("Could not open the file '%s'.", pc_FileName);
    return 0;
  }
  fwrite (pv_Data, 1, NII_HEADER_SIZE, pf_OutputFile);

  fclose(pf_OutputFile);
  return 1;
}

short int
b_NIFTII_WriteImageToFile (const char* pc_FileName, int i32_BytesToWrite, int i32_PixelsInVolume, int i32_BLOB_Offset, void *pv_Data)
{
  debug_functions ();

  FILE *pf_OutputFile;

  pf_OutputFile = fopen (pc_FileName, "ab");

  if( pf_OutputFile==NULL)
  {
    debug_error ("Could not open the file '%s'.", pc_FileName);
    return 0;
  }

  fseek(pf_OutputFile, i32_BLOB_Offset, SEEK_SET);
  fwrite(pv_Data, i32_BytesToWrite, i32_PixelsInVolume, pf_OutputFile);

  fclose(pf_OutputFile);
  return 1;
}

void
v_NIFTII_convert_data_big_to_little_endian (Serie *serie)
{
  short int num_bytes = memory_serie_get_memory_space(serie);

  unsigned int i32_memory_size = serie->matrix.i16_x * serie->matrix.i16_y * serie->matrix.i16_z * serie->num_time_series;
  unsigned int i32_blobCnt;

  void *pv_Data=serie->data;
  switch (serie->data_type)
  {
    case MEMORY_TYPE_INT8    :
    case MEMORY_TYPE_UINT8   :
      break;
    case MEMORY_TYPE_INT16   :
    case MEMORY_TYPE_UINT16  :
      {
        unsigned short int x16_Value;

        for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
        {
          x16_Value = (unsigned short int)(*(unsigned short int *)(pv_Data));
          (*(unsigned short int *)(pv_Data)) = __bswap_16(x16_Value);

          pv_Data+=num_bytes;
        }
      }
      break;
    case MEMORY_TYPE_INT32   :
    case MEMORY_TYPE_UINT32  :
    case MEMORY_TYPE_FLOAT32 :
      {
        unsigned int x32_Value;

        for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
        {
          x32_Value = (unsigned int)(*(unsigned int *)(pv_Data));
          (*(unsigned int *)(pv_Data)) = __bswap_32(x32_Value);

          pv_Data+=num_bytes;
        }
      }
      break;
    case MEMORY_TYPE_INT64   :
    case MEMORY_TYPE_UINT64  :
    case MEMORY_TYPE_FLOAT64 :
      {
        unsigned long long x64_Value;

        for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
        {
          x64_Value = (unsigned long long)(*(unsigned long long *)(pv_Data));
          (*(unsigned long long *)(pv_Data)) = __bswap_64(x64_Value);

          pv_Data+=num_bytes;
        }
      }
      break;
    default : break;
  }
}

void v_NIFTII_swap_2bytes( size_t n , void *ar )
{
  register size_t ii ;
  unsigned char * cp1 = (unsigned char *)ar, * cp2 ;
  unsigned char   tval;

  for( ii=0 ; ii < n ; ii++ )
  {
    cp2 = cp1 + 1;
    tval = *cp1;
    *cp1 = *cp2;
    *cp2 = tval;
    cp1 += 2;
  }
  return;
}

void v_NIFTII_swap_4bytes( size_t n , void *ar )
{
  register size_t ii ;
  unsigned char * cp0 = (unsigned char *)ar, * cp1, * cp2 ;
  register unsigned char tval ;

  for( ii=0 ; ii < n ; ii++ )
  {
    cp1 = cp0;
    cp2 = cp0+3;
    tval = *cp1;
    *cp1 = *cp2;
    *cp2 = tval;
    cp1++;
    cp2--;
    tval = *cp1;
    *cp1 = *cp2;
    *cp2 = tval;
    cp0 += 4;
  }
  return ;
}

void v_NIFTII_swap_header( struct nifti_1_header *h /*, int is_nifti*/ )
{

   /* if ANALYZE, swap as such and return */
//   if( ! is_nifti ) {
//      v_NIFTII_swap_as_analyze((nifti_analyze75 *)h);
//      return;
//   }

   /* otherwise, swap all NIFTI fields */

   v_NIFTII_swap_4bytes(1, &h->sizeof_hdr);
   v_NIFTII_swap_4bytes(1, &h->extents);
   v_NIFTII_swap_2bytes(1, &h->session_error);

   v_NIFTII_swap_2bytes(8, h->dim);
   v_NIFTII_swap_4bytes(1, &h->intent_p1);
   v_NIFTII_swap_4bytes(1, &h->intent_p2);
   v_NIFTII_swap_4bytes(1, &h->intent_p3);

   v_NIFTII_swap_2bytes(1, &h->intent_code);
   v_NIFTII_swap_2bytes(1, &h->datatype);
   v_NIFTII_swap_2bytes(1, &h->bitpix);
   v_NIFTII_swap_2bytes(1, &h->slice_start);

   v_NIFTII_swap_4bytes(8, h->pixdim);

   v_NIFTII_swap_4bytes(1, &h->vox_offset);
   v_NIFTII_swap_4bytes(1, &h->scl_slope);
   v_NIFTII_swap_4bytes(1, &h->scl_inter);
   v_NIFTII_swap_2bytes(1, &h->slice_end);

   v_NIFTII_swap_4bytes(1, &h->cal_max);
   v_NIFTII_swap_4bytes(1, &h->cal_min);
   v_NIFTII_swap_4bytes(1, &h->slice_duration);
   v_NIFTII_swap_4bytes(1, &h->toffset);
   v_NIFTII_swap_4bytes(1, &h->glmax);
   v_NIFTII_swap_4bytes(1, &h->glmin);

   v_NIFTII_swap_2bytes(1, &h->qform_code);
   v_NIFTII_swap_2bytes(1, &h->sform_code);

   v_NIFTII_swap_4bytes(1, &h->quatern_b);
   v_NIFTII_swap_4bytes(1, &h->quatern_c);
   v_NIFTII_swap_4bytes(1, &h->quatern_d);
   v_NIFTII_swap_4bytes(1, &h->qoffset_x);
   v_NIFTII_swap_4bytes(1, &h->qoffset_y);
   v_NIFTII_swap_4bytes(1, &h->qoffset_z);

   v_NIFTII_swap_4bytes(4, h->srow_x);
   v_NIFTII_swap_4bytes(4, h->srow_y);
   v_NIFTII_swap_4bytes(4, h->srow_z);

   return ;
}


/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */


te_ImageIOFiletype
memory_io_niftii_file_type (const char *pc_File)
{
  debug_functions ();

  //Check if file exists
  if (access (pc_File, F_OK) != 0) return 1;

  nifti_1_header *ps_Header;
  char ac_SwapHeaderImage[512];
  te_ImageIOFiletype e_FileType;

  ps_Header = (nifti_1_header *)(calloc(1, sizeof (nifti_1_header)));
  e_FileType = MUMC_FILETYPE_NOT_KNOWN;

  // find extention in string
  char *ac_Extention = strrchr (pc_File, '.');
  if (ac_Extention == NULL)
  {
    return MUMC_FILETYPE_NOT_KNOWN;
  }

  if ((!strcasecmp (ac_Extention, ".nii")) || (!strcasecmp (ac_Extention, ".hdr")))
  {
    //check if this file contains a valid header
    if(b_NIFTII_ReadHeaderToMemory((char *)pc_File, ps_Header))
    {
      e_FileType= MUMC_FILETYPE_NOT_KNOWN;
    }

    if (strncmp(ps_Header->magic, "n+", 2)==0)
    {
      e_FileType=MUMC_FILETYPE_NIFTII_SF;
    }
    else if (strncmp(ps_Header->magic, "ni", 2)==0)
    {
      e_FileType=MUMC_FILETYPE_NIFTII_TF;
    }
    else if ((ps_Header->sizeof_hdr == MIN_HEADER_SIZE) && (strcasecmp(ac_Extention, ".hdr")==0))
    {
      //proberly an Analyze header
      e_FileType=MUMC_FILETYPE_ANALYZE75;
    }
    else
    {
      e_FileType=MUMC_FILETYPE_NOT_KNOWN;
    }
  }
  else if (strcasecmp(ac_Extention, ".img")==0)
  {
    strncpy(ac_SwapHeaderImage, pc_File, sizeof(ac_SwapHeaderImage));
    e_FileType = memory_io_niftii_file_type (ac_SwapHeaderImage);
  }
  else if (strcasecmp(ac_Extention, ".dcm")==0)
  {
    // BUILD HERE A DICOM CHECK

  }
  else
  {
    e_FileType=MUMC_FILETYPE_NOT_KNOWN;
  }

  free(ps_Header);

  return e_FileType;
}

short int
memory_io_niftii_load (Serie *serie, const char *pc_Filename, const char *pc_Image)
{
  debug_functions ();

  if (serie == NULL) return 0;

  short int i16_BytesToRead;
  short int i16_wasSwapped=0;
  int i32_PixelsInSlice, i32_MemoryPerSlice, i32_MemoryVolume;


  nifti_1_header *ps_Header;
  ps_Header = (nifti_1_header *)(calloc (1, sizeof (nifti_1_header)));
  if (ps_Header == NULL) return 0;

  b_NIFTII_ReadHeaderToMemory ((char *)pc_Filename, ps_Header);
  // check endiane

  if (ps_Header->sizeof_hdr == 1543569408)
  {
    v_NIFTII_swap_header(ps_Header);
    i16_wasSwapped=1;
  }

  if ((ps_Header->sizeof_hdr == MIN_HEADER_SIZE) && (ps_Header->intent_code == NIFTI_INTENT_NONE))
  {
//    serie->pv_Header = ps_Header;
    serie->e_SerieType = SERIE_ORIGINAL;
    serie->matrix.i16_x = ps_Header->dim[1];
    serie->matrix.i16_y = ps_Header->dim[2];
    serie->matrix.i16_z = ps_Header->dim[3];




    serie->num_time_series = ps_Header->dim[4];


    if ((ps_Header->pixdim[0] == -1) || (ps_Header->pixdim[0] == 1))
    {
      serie->d_Qfac=ps_Header->pixdim[0];
    }
    else
    {
      serie->d_Qfac=1;
    }



    serie->pixel_dimension.x = (ps_Header->pixdim[1]>0)?ps_Header->pixdim[1]:1;
    serie->pixel_dimension.y = (ps_Header->pixdim[2]>0)?ps_Header->pixdim[2]:1;
    serie->pixel_dimension.z = (ps_Header->pixdim[3]>0)?ps_Header->pixdim[3]:1;


    serie->slope = ps_Header->scl_slope;
    serie->offset = ps_Header->scl_inter;


    serie->u8_AxisUnits = ps_Header->xyzt_units;

    serie->raw_data_type = ps_Header->datatype;
    serie->data_type = e_NIFTII_ConvertNIFTIIToMemoryDataType (ps_Header->datatype);
    serie->input_type=memory_io_niftii_file_type((char *)pc_Filename);

    // check if niftii is true niftii or analyze file
    if (memcmp(ps_Header->magic, "n+1\0", 4) == 0)
    {
      //File is in a niftii1 format read quaternions en standard space matrix

      serie->i16_QuaternionCode = ps_Header->qform_code;

      serie->ps_Quaternion = calloc (1, sizeof (ts_Quaternion));
      serie->ps_QuaternationOffset = calloc (1, sizeof (ts_Quaternion));

      serie->ps_Quaternion->I = ps_Header->quatern_b;
      serie->ps_Quaternion->J = ps_Header->quatern_c;
      serie->ps_Quaternion->K = ps_Header->quatern_d;

      serie->ps_QuaternationOffset->I = ps_Header->qoffset_x;
      serie->ps_QuaternationOffset->J = ps_Header->qoffset_y;
      serie->ps_QuaternationOffset->K = ps_Header->qoffset_z;



      serie->i16_StandardSpaceCode = ps_Header->sform_code;

      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][0]=ps_Header->srow_x[0];// / t_Maximum.x;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][0]=ps_Header->srow_x[1];// / t_Maximum.x;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][0]=ps_Header->srow_x[2];// / t_Maximum.x;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][0]=ps_Header->srow_x[3];

      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][1]=ps_Header->srow_y[0];// / t_Maximum.y;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][1]=ps_Header->srow_y[1];// / t_Maximum.y;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][1]=ps_Header->srow_y[2];// / t_Maximum.y;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][1]=ps_Header->srow_y[3];

      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][2]=ps_Header->srow_z[0];// / t_Maximum.z;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][2]=ps_Header->srow_z[1];// / t_Maximum.z;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][2]=ps_Header->srow_z[2];// / t_Maximum.z;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][2]=ps_Header->srow_z[3];

      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][3]=0;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][3]=0;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][3]=0;
      serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][3]=1;

      if((serie->i16_StandardSpaceCode == NIFTI_XFORM_UNKNOWN) &&
         (serie->i16_QuaternionCode == NIFTI_XFORM_UNKNOWN))
      {
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][0]=1;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][0]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][0]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][0]=0;

        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][1]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][1]=1;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][1]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][1]=0;

        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][2]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][2]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][2]=1;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][2]=0;

        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][3]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][3]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][3]=0;
        serie->t_StandardSpaceIJKtoXYZ.af_Matrix[3][3]=1;

        serie->t_StandardSpaceXYZtoIJK = tda_algebra_matrix_4x4_inverse(&serie->t_StandardSpaceIJKtoXYZ);

        serie->pt_RotationMatrix = &serie->t_StandardSpaceIJKtoXYZ;
        serie->pt_InverseMatrix = &serie->t_StandardSpaceXYZtoIJK;
      }
      else if((serie->i16_StandardSpaceCode == NIFTI_XFORM_ALIGNED_ANAT) ||
              (serie->i16_StandardSpaceCode == NIFTI_XFORM_TALAIRACH) ||
              (serie->i16_StandardSpaceCode == NIFTI_XFORM_MNI_152))
      {
        serie->t_StandardSpaceXYZtoIJK = tda_algebra_matrix_4x4_inverse(&serie->t_StandardSpaceIJKtoXYZ);
        serie->pt_RotationMatrix = &serie->t_StandardSpaceIJKtoXYZ;
        serie->pt_InverseMatrix = &serie->t_StandardSpaceXYZtoIJK;
      }
      else if(serie->i16_QuaternionCode == NIFTI_XFORM_SCANNER_ANAT)
      {
        v_memory_io_handleSpace (serie);

        serie->pt_RotationMatrix = &serie->t_ScannerSpaceIJKtoXYZ;
        serie->pt_InverseMatrix = &serie->t_ScannerSpaceXYZtoIJK;
      }
    }

    i16_BytesToRead = i16_NIFTII_GetMemorySizePerElement (ps_Header->datatype);

    i32_PixelsInSlice = serie->matrix.i16_x * serie->matrix.i16_y;
    i32_MemoryPerSlice = i16_BytesToRead * i32_PixelsInSlice;
    i32_MemoryVolume = i32_MemoryPerSlice * serie->matrix.i16_z * serie->num_time_series;


    serie->data = calloc (1, i32_MemoryVolume);
    serie->pv_OutOfBlobValue = calloc (1, i16_BytesToRead);

    if (serie->data != NULL)
    {
      int i32_Offset;
      if (pc_Image==NULL)
      {
        // Image and header file are the same;
        i32_Offset=352;
        b_NIFTII_ReadVolumeToMemory ((char *)pc_Filename, i32_Offset, i32_MemoryVolume, serie->data);

        if (i16_wasSwapped)
        {
          v_NIFTII_convert_data_big_to_little_endian(serie);
        }
      }
    }
    memory_serie_set_upper_and_lower_borders_from_data(serie);
    return 1;
  }

  else
  {
    #ifdef ENABLE_DEBUGGING
    printf("Big endian files are not supported");
    #endif
  }

  return 0;
}

short int
memory_io_niftii_save (Serie *serie, const char *pc_File, const char *pc_ImageFile)
{
  debug_functions ();

  short int i16_BytesToWrite;
  int i32_PixelsInSlice, i32_MemoryPerSlice, i32_MemoryVolume, i32_MemoryInBlob;

  nifti_1_header *ps_Header;
  ps_Header = calloc (1, NII_HEADER_SIZE);
  if (ps_Header == NULL) return 1;

  ps_Header->sizeof_hdr = MIN_HEADER_SIZE;

  ps_Header->dim[0] = (serie->num_time_series>1) ? 4 : 3;
  ps_Header->dim[1] = serie->matrix.i16_x;
  ps_Header->dim[2] = serie->matrix.i16_y;
  ps_Header->dim[3] = serie->matrix.i16_z;
  ps_Header->dim[4] = serie->num_time_series;

  ps_Header->pixdim[1] = serie->pixel_dimension.x;
  ps_Header->pixdim[2] = serie->pixel_dimension.y;
  ps_Header->pixdim[3] = serie->pixel_dimension.z;

  ps_Header->scl_slope = serie->slope;
  ps_Header->scl_inter = serie->offset;

  ps_Header->qform_code = serie->i16_QuaternionCode;
  ps_Header->quatern_b = serie->ps_Quaternion->I;
  ps_Header->quatern_c = serie->ps_Quaternion->J;
  ps_Header->quatern_d = serie->ps_Quaternion->K;

  ps_Header->qoffset_x = serie->ps_QuaternationOffset->I;
  ps_Header->qoffset_y = serie->ps_QuaternationOffset->J;
  ps_Header->qoffset_z = serie->ps_QuaternationOffset->K;

  ps_Header->sform_code = serie->i16_StandardSpaceCode;

  ps_Header->srow_x[0] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][0];
  ps_Header->srow_x[1] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][1];
  ps_Header->srow_x[2] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][2];
  ps_Header->srow_x[3] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[0][3];

  ps_Header->srow_y[0] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][0];
  ps_Header->srow_y[1] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][1];
  ps_Header->srow_y[2] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][2];
  ps_Header->srow_y[3] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[1][3];

  ps_Header->srow_z[0] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][0];
  ps_Header->srow_z[1] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][1];
  ps_Header->srow_z[2] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][2];
  ps_Header->srow_z[3] = serie->t_StandardSpaceIJKtoXYZ.af_Matrix[2][3];


  ps_Header->datatype = i16_NIFTII_ConvertMemoryDataTypeToNIFTII(serie->data_type);
  ps_Header->bitpix = i16_NIFTII_GetBitPix (serie->raw_data_type);

  i16_BytesToWrite = i16_NIFTII_GetMemorySizePerElement (serie->raw_data_type);
  i32_PixelsInSlice = serie->matrix.i16_x * serie->matrix.i16_y;
  i32_MemoryPerSlice = i16_BytesToWrite * i32_PixelsInSlice;
  i32_MemoryVolume = i32_MemoryPerSlice * serie->matrix.i16_z;
  i32_MemoryInBlob = i32_MemoryVolume * serie->num_time_series;

  // If the File should be saved as two files
  if (pc_ImageFile == NULL)
  {
    ps_Header->vox_offset = NII_HEADER_SIZE;
    memccpy (ps_Header->magic, "n+1", 1, 3);

    b_NIFTII_WriteHeaderToFile (pc_File, ps_Header);
    b_NIFTII_WriteImageToFile (pc_File, 1, i32_MemoryInBlob, NII_HEADER_SIZE, serie->data);
  }
  else
  {
    ps_Header->vox_offset = 0;
    memccpy (ps_Header->magic, "ni1", 1, 3);

    b_NIFTII_WriteHeaderToFile (pc_File, ps_Header);
    b_NIFTII_WriteImageToFile (pc_ImageFile, 1, i32_MemoryInBlob, 0, serie->data);
  }

  free (ps_Header);
  ps_Header = NULL;

  return 0;
}

