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

#include "include/lib-io-niftii.h"
#include "lib-common-debug.h"

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

te_ImageIOFiletype
//e_NIFTII_FileType (char *pc_File)
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

  pf_OutputFile = fopen (pc_FileName, "w");
  if( pf_OutputFile==NULL)
  {
    debug_error ("Could not open the file '%s'.", pc_FileName);
    return 0;
  }
  fwrite (pv_Data, 1, sizeof (nifti_1_header), pf_OutputFile);

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






/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */

short int
memory_io_niftii_load (Serie *serie, const char *pc_Filename, const char *pc_Image)
{
  debug_functions ();

  if (serie == NULL) return 0;

  short int i16_BytesToRead;
  int i32_PixelsInSlice, i32_MemoryPerSlice, i32_MemoryVolume;

  nifti_1_header *ps_Header;
  ps_Header = (nifti_1_header *)(calloc (1, sizeof (nifti_1_header)));
  if (ps_Header == NULL) return 0;

  b_NIFTII_ReadHeaderToMemory ((char *)pc_Filename, ps_Header);
  // check endiane
  if (ps_Header->sizeof_hdr == MIN_HEADER_SIZE)
  {
    serie->pv_Header = ps_Header;

    serie->matrix.x = ps_Header->dim[1];
    serie->matrix.y = ps_Header->dim[2];
    serie->matrix.z = ps_Header->dim[3];

    serie->num_time_series = ps_Header->dim[4];

    serie->pixel_dimension.x = ps_Header->pixdim[1];
    serie->pixel_dimension.y = ps_Header->pixdim[2];
    serie->pixel_dimension.z = ps_Header->pixdim[3];

    serie->slope = ps_Header->scl_slope;
    serie->offset = ps_Header->scl_inter;

    serie->raw_data_type = ps_Header->datatype;
    serie->data_type = e_NIFTII_ConvertNIFTIIToMemoryDataType (ps_Header->datatype);
    serie->input_type=memory_io_niftii_file_type((char *)pc_Filename);

    i16_BytesToRead = i16_NIFTII_GetMemorySizePerElement (ps_Header->datatype);

    i32_PixelsInSlice = serie->matrix.x * serie->matrix.y;
    i32_MemoryPerSlice = i16_BytesToRead * i32_PixelsInSlice;
    i32_MemoryVolume = i32_MemoryPerSlice * serie->matrix.z * serie->num_time_series;


    serie->i16_QuaternionCode = ps_Header->qform_code;
    serie->ps_Quaternion = memory_quaternion_new ();
    serie->ps_QuternationOffset = memory_quaternion_new ();

    serie->ps_Quaternion->I = ps_Header->quatern_b;
    serie->ps_Quaternion->J = ps_Header->quatern_c;
    serie->ps_Quaternion->K = ps_Header->quatern_d;

    serie->ps_Quaternion->W = sqrt(1.0 - (serie->ps_Quaternion->I * serie->ps_Quaternion->I +
                                          serie->ps_Quaternion->J * serie->ps_Quaternion->J +
                                          serie->ps_Quaternion->K * serie->ps_Quaternion->K));


    serie->ps_QuternationOffset->I = ps_Header->qoffset_x;
    serie->ps_QuternationOffset->J = ps_Header->qoffset_y;
    serie->ps_QuternationOffset->K = ps_Header->qoffset_z;
/*
    printf("Quaternion\n");
    printf(" W         : %f\n",serie->ps_Quaternion->W);
    printf(" [I, J, K] : [%.3f,%.3f,%.3f]\n",serie->ps_Quaternion->I,serie->ps_Quaternion->J,serie->ps_Quaternion->K);

    printf("Offset\n");
    printf(" [x, y, z] : [%.3f,%.3f,%.3f]\n",serie->ps_QuternationOffset->I,serie->ps_QuternationOffset->J,serie->ps_QuternationOffset->K);

*/
    serie->data = calloc (1, i32_MemoryVolume);
    serie->pv_OutOfBlobValue = calloc (1, i16_BytesToRead);

    if (serie->data != NULL)
    {
      int i32_Offset;
      if (pc_Image==NULL)
      {
        // Image and header file are the same;
        i32_Offset=sizeof(nifti_1_header);
        b_NIFTII_ReadVolumeToMemory ((char *)pc_Filename, i32_Offset, i32_MemoryVolume, serie->data);
      }
    }
    memory_serie_set_upper_and_lower_borders_from_data(serie);
//    free (ps_Header), ps_Header = NULL;
    return 1;
  }
  else if (ps_Header->sizeof_hdr == 1543569408) // big endian file
  {
    serie->pv_Header = ps_Header;

    serie->matrix.x = __bswap_constant_16(ps_Header->dim[1]);
    serie->matrix.y = __bswap_constant_16(ps_Header->dim[2]);
    serie->matrix.z = __bswap_constant_16(ps_Header->dim[3]);

    serie->num_time_series = __bswap_constant_16(ps_Header->dim[4]);

    serie->pixel_dimension.x = __bswap_32(ps_Header->pixdim[1]);
    serie->pixel_dimension.y = __bswap_32(ps_Header->pixdim[2]);
    serie->pixel_dimension.z = __bswap_32(ps_Header->pixdim[3]);

    serie->pixel_dimension.x = (serie->pixel_dimension.x==0) ? 1 : serie->pixel_dimension.x;
    serie->pixel_dimension.y = (serie->pixel_dimension.y==0) ? 1 : serie->pixel_dimension.y;
    serie->pixel_dimension.z = (serie->pixel_dimension.z==0) ? 1 : serie->pixel_dimension.z;


    serie->slope = __bswap_32(ps_Header->scl_slope);
    serie->offset = __bswap_32(ps_Header->scl_inter);

    serie->raw_data_type = __bswap_16(ps_Header->datatype);
    serie->data_type = e_NIFTII_ConvertNIFTIIToMemoryDataType (__bswap_16(ps_Header->datatype));
    serie->input_type=memory_io_niftii_file_type((char *)pc_Filename);

    i16_BytesToRead = i16_NIFTII_GetMemorySizePerElement (__bswap_16(ps_Header->datatype));

    i32_PixelsInSlice = serie->matrix.x * serie->matrix.y;
    i32_MemoryPerSlice = i16_BytesToRead * i32_PixelsInSlice;
    i32_MemoryVolume = i32_MemoryPerSlice * serie->matrix.z * serie->num_time_series;


    serie->i16_QuaternionCode = __bswap_16(ps_Header->qform_code);
    serie->ps_Quaternion = memory_quaternion_new ();
    serie->ps_QuternationOffset = memory_quaternion_new ();

    serie->ps_Quaternion->I = __bswap_32(ps_Header->quatern_b);
    serie->ps_Quaternion->J = __bswap_32(ps_Header->quatern_c);
    serie->ps_Quaternion->K = __bswap_32(ps_Header->quatern_d);

    serie->ps_Quaternion->W = sqrt(1.0 - (serie->ps_Quaternion->I * serie->ps_Quaternion->I +
                                          serie->ps_Quaternion->J * serie->ps_Quaternion->J +
                                          serie->ps_Quaternion->K * serie->ps_Quaternion->K));



    serie->ps_QuternationOffset->I = __bswap_32(ps_Header->qoffset_x);
    serie->ps_QuternationOffset->J = __bswap_32(ps_Header->qoffset_y);
    serie->ps_QuternationOffset->K = __bswap_32(ps_Header->qoffset_z);


    serie->data = calloc (1, i32_MemoryVolume);
    serie->pv_OutOfBlobValue = calloc (1, i16_BytesToRead);

    if (serie->data != NULL)
    {
      int i32_Offset;
      if (pc_Image==NULL)
      {
        // Image and header file are the same;
        i32_Offset=sizeof(nifti_1_header);
        b_NIFTII_ReadVolumeToMemory ((char *)pc_Filename, i32_Offset, i32_MemoryVolume, serie->data);

        memory_serie_convert_data_big_to_little_endian(serie);

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
  int i32_PixelsInSlice, i32_MemoryPerSlice, i32_MemoryVolume;

  nifti_1_header *ps_Header;
  ps_Header = calloc (1, sizeof (nifti_1_header));
  if (ps_Header == NULL) return 1;

  memcpy (ps_Header, serie->pv_Header, sizeof (nifti_1_header));

  ps_Header->sizeof_hdr = MIN_HEADER_SIZE;


  ps_Header->dim[0] = (serie->num_time_series>1) ? 3 : 4;
  ps_Header->dim[1] = serie->matrix.x;
  ps_Header->dim[2] = serie->matrix.y;
  ps_Header->dim[3] = serie->matrix.z;
  ps_Header->dim[4] = serie->num_time_series;

  ps_Header->pixdim[1] = 3;
  ps_Header->pixdim[1] = serie->pixel_dimension.x;
  ps_Header->pixdim[2] = serie->pixel_dimension.y;
  ps_Header->pixdim[3] = serie->pixel_dimension.z;

  ps_Header->scl_slope = serie->slope;
  ps_Header->scl_inter = serie->offset;

  ps_Header->datatype = i16_NIFTII_ConvertMemoryDataTypeToNIFTII(serie->data_type);
  ps_Header->bitpix = i16_NIFTII_GetBitPix (serie->raw_data_type);

  i16_BytesToWrite = i16_NIFTII_GetMemorySizePerElement (serie->raw_data_type);
  i32_PixelsInSlice = serie->matrix.x * serie->matrix.y;
  i32_MemoryPerSlice = i16_BytesToWrite * i32_PixelsInSlice;
  i32_MemoryVolume = i32_MemoryPerSlice * serie->matrix.z;


  // If the File should be saved as two files
  if (pc_ImageFile == NULL)
  {
    ps_Header->vox_offset = sizeof(nifti_1_header);
    memccpy (ps_Header->magic, "n+1", 1, 3);

    b_NIFTII_WriteHeaderToFile (pc_File, ps_Header);
    b_NIFTII_WriteImageToFile (pc_File, 1, i32_MemoryVolume, sizeof(nifti_1_header), serie->data);
  }
  else
  {
    ps_Header->vox_offset = 0;
    memccpy (ps_Header->magic, "ni1", 1, 3);

    b_NIFTII_WriteHeaderToFile (pc_File, ps_Header);
    b_NIFTII_WriteImageToFile (pc_ImageFile, 1, i32_MemoryVolume, 0, serie->data);
  }

  free (ps_Header);
  ps_Header = NULL;

  return 0;
}

