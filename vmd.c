/**
 *  @file vmd.c
 *  @brief Library for VMD file in MikuMikuDance(MMD)
 *  @author ihm4
 *  @note
 *    true and false are treated as `true` or `false` since this
 *    library uses `stdbool.h`.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "vmd.h"

int VMD_ERROR;

/**
 * @brief Check magic number of VMD file
 *  Internally called function
 * @param (data) data read from VMD file
 * @return boolean
 */
int __VMDCheckHeader(void* data){
  VMDHeader* target = (VMDHeader*)data;
  if ( memcmp(VMDLIB_MAGIC, target->header, sizeof(target->header)) != 0 ){
    return false;
  }
  return true;
}

int __VMDCompareBoneFrameNumber(const void* a, const void* b){
  return ((VMDBoneSingleFrame*)a)->frame - ((VMDBoneSingleFrame*)b)->frame;
}

int __VMDCompareMorphFrameNumber(const void* a, const void* b){
  return ((VMDMorphSingleFrame*)a)->frame - ((VMDMorphSingleFrame*)b)->frame;
}

int __VMDCompareCameraFrameNumber(const void* a, const void* b){
  return ((VMDCameraSingleFrame*)a)->frame - ((VMDCameraSingleFrame*)b)->frame;
}

int __VMDCompareLightFrameNumber(const void* a, const void* b){
  return ((VMDLightSingleFrame*)a)->frame - ((VMDLightSingleFrame*)b)->frame;
}

int __VMDCompareShadowFrameNumber(const void* a, const void* b){
  return ((VMDShadowSingleFrame*)a)->frame - ((VMDShadowSingleFrame*)b)->frame;
}

int __VMDCompareIKFrameNumber(const void* a, const void* b){
  return ((VMDIKSingleFrame*)a)->frame - ((VMDIKSingleFrame*)b)->frame;
}

/**
 * @brief qsort() for VMD Frames
 *  VMD data is constructed by several types of frames e.g. bone, camera, and
 *  so on. These frames are typically not sorted. This function do qsort() for
 *  any type of frames in VMD data by selecting the frame type with one of the
 *  aurguments called `type`.
 * @param (data) same as data for qsort()
 * @param (num) same as num for qsort()
 * @param (size) same as size for qsor()
 * @param (type) specify the type of data choosen from `VMDStructType`
 * @return void
 */
void VMDqsort(void* data, size_t num, size_t size, VMDStructType type){
  void* funcp = NULL;
  if ( data == NULL ) {
    fprintf(stderr,"NULL was passed to %s, type was %d\n", __func__, type);
    return;
  }

  // select which compare function to use
  switch(type){
    case VMDL_BONE:
      funcp = __VMDCompareBoneFrameNumber; break;
    case VMDL_MORPH:
      funcp = __VMDCompareMorphFrameNumber; break;
    case VMDL_CAMERA:
      funcp = __VMDCompareCameraFrameNumber; break;
    case VMDL_LIGHT:
      funcp = __VMDCompareLightFrameNumber; break;
    case VMDL_SHADOW:
      funcp = __VMDCompareShadowFrameNumber; break;
    case VMDL_IK:
      funcp = __VMDCompareIKFrameNumber; break;
    default:
      fprintf( stderr, "%s : invalid type %d\n", __func__, type);
  }
  qsort(data, num, size, funcp);
  return;
}

/**
 * @note You must release returned pointer by VMDReleaseVMDFile()
 *       after you used it
 * @brief Load VMD file and create VMD structure
 * @param (fname) VMD file name to be read
 * @return pointer of VMDFile created inside this function
 */
VMDFile* VMDLoadFromFile(const char* fname){
  FILE *fp = NULL;
  uint32_t offset = 0;
  uint32_t cpy_size = 0;
  char *content = NULL;
  VMDFile* vf = NULL;
  size_t fsize;

  // open file and check
  fp = fopen(fname, "rb");
  if ( fp == NULL ) {
    printf("File open error.\n");
    VMD_ERROR = VMDLIB_E_FH;
    return NULL;
  }

  // check length of file
  fseek(fp, 0, SEEK_END);
  fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  DEBUG_PRINT("Size of file : %d\n", fsize);

  // load entire file, then close
  // After reading whole file data then copy each structure, this is
  // inefficient especially for memory usage. But read out each structure data
  // cause fread() warning (unused return value)
  content = malloc(fsize + 1);
  if ( content == NULL ){
    printf("Insufficient memory.\n");
    VMD_ERROR = VMDLIB_E_ME;
    fclose(fp);
    return NULL;
  }

  if (fread(content, fsize, 1, fp) == 0 ) {
    fprintf(stderr, "File read error\n");
    fclose(fp);
    fp = NULL;
    return NULL;
  }
  fclose(fp);
  fp = NULL;

  // check file type
  if ( __VMDCheckHeader(content) == false ){
    printf("The file is not VMD file!\n");
    VMD_ERROR = VMDLIB_E_FT;
    free(content);
    return NULL;
  }

  // Allocate memory and set data. I don't know proper way to do this(same
  // procedure for differenct type, something like generic functions in C ++),
  // so codes below are copy-pasted...  I hope someone greate improve this.
  vf = malloc(sizeof(VMDFile));
  cpy_size = sizeof(VMDHeader);
  memcpy((void*)(&vf->header), (const void*)content, cpy_size);
  offset += cpy_size;

  vf->bone_frames.num_frames = ((VMDBoneFrames*)(content+offset))->num_frames;
  offset += sizeof(uint32_t);
  if(vf->bone_frames.num_frames != 0){
    cpy_size = sizeof(VMDBoneSingleFrame)*vf->bone_frames.num_frames;
    vf->bone_frames.frames = malloc(cpy_size);
    memcpy((void*)vf->bone_frames.frames, (const void*)(content+offset), cpy_size);
    offset += cpy_size;
  } else {
    vf->bone_frames.frames = NULL;
  }

  vf->morph_frames.num_frames = ((VMDMorphFrames*)(content+offset))->num_frames;
  offset += sizeof(uint32_t);
  if(vf->morph_frames.num_frames != 0){
    cpy_size = sizeof(VMDMorphSingleFrame)*vf->morph_frames.num_frames;
    vf->morph_frames.frames = malloc(cpy_size);
    memcpy((void*)vf->morph_frames.frames, (const void*)(content+offset), cpy_size);
    offset += cpy_size;
  } else {
    vf->morph_frames.frames = NULL;
  }

  vf->camera_frames.num_frames = ((VMDCameraFrames*)(content+offset))->num_frames;
  offset += sizeof(uint32_t);
  if(vf->camera_frames.num_frames != 0){
    cpy_size = sizeof(VMDCameraSingleFrame)*vf->camera_frames.num_frames;
    vf->camera_frames.frames = malloc(cpy_size);
    memcpy((void*)vf->camera_frames.frames, (const void*)(content+offset), cpy_size);
    offset += cpy_size;
  } else {
    vf->camera_frames.frames = NULL;
  }

  vf->light_frames.num_frames = ((VMDLightFrames*)(content+offset))->num_frames;
  offset += sizeof(uint32_t);
  if(vf->light_frames.num_frames != 0){
    cpy_size = sizeof(VMDLightSingleFrame)*vf->light_frames.num_frames;
    vf->light_frames.frames = malloc(cpy_size);
    memcpy((void*)vf->light_frames.frames, (const void*)(content+offset), cpy_size);
    offset += cpy_size;
  } else {
    vf->light_frames.frames = NULL;
  }

  vf->shadow_frames.num_frames = ((VMDShadowFrames*)(content+offset))->num_frames;
  offset += sizeof(uint32_t);
  if(vf->shadow_frames.num_frames != 0){
    cpy_size = sizeof(VMDShadowSingleFrame)*vf->shadow_frames.num_frames;
    vf->shadow_frames.frames = malloc(cpy_size);
    memcpy((void*)vf->shadow_frames.frames, (const void*)(content+offset), cpy_size);
    offset += cpy_size;
  } else {
    vf->shadow_frames.frames = NULL;
  }

  vf->ik_frames.num_frames = ((VMDIKFrames*)(content+offset))->num_frames;
  offset += sizeof(uint32_t);
  if(vf->ik_frames.num_frames != 0){
    cpy_size = sizeof(VMDIKSingleFrame)*vf->ik_frames.num_frames;
    vf->ik_frames.frames = malloc(cpy_size);
    memcpy((void*)vf->ik_frames.frames, (const void*)(content+offset), cpy_size);
    offset += cpy_size;
  } else {
    vf->ik_frames.frames = NULL;
  }

  free(content);
  return vf;
}

/**
 * @brief Write data into specified file
 * @param (vf) pointer to VMDFile
 * @param (fname) a name of a file to be written
 * @return bool : succeed or not
 */
bool VMDWriteToFile(VMDFile *vf, char* fname){
  FILE *fp;

  if ( vf == NULL ) {
    VMD_ERROR = VMDLIB_E_IV;
    return false;
  }

  // file check
  fp = fopen(fname, "wb");
  if ( fp == NULL ) {
    DEBUG_PRINT("File open error.\n");
    VMD_ERROR = VMDLIB_E_FH;
    return false;
  }

  fwrite(&(vf->header), sizeof(VMDHeader), 1, fp);
  fwrite(&(vf->bone_frames.num_frames),
         sizeof(vf->bone_frames.num_frames), 1, fp);
  if ( vf->bone_frames.frames != NULL ) {
    fwrite(vf->bone_frames.frames,
           sizeof(vf->bone_frames.frames[0]), vf->bone_frames.num_frames, fp);
  }

  fwrite(&(vf->morph_frames.num_frames),
         sizeof(vf->morph_frames.num_frames), 1, fp);
  if ( vf->morph_frames.frames != NULL ) {
    fwrite(vf->morph_frames.frames,
           sizeof(vf->morph_frames.frames[0]), vf->morph_frames.num_frames, fp);
  }

  fwrite(&(vf->camera_frames.num_frames),
         sizeof(vf->camera_frames.num_frames), 1, fp);
  if ( vf->camera_frames.frames != NULL ) {
    fwrite(vf->camera_frames.frames,
           sizeof(vf->camera_frames.frames[0]), vf->camera_frames.num_frames, fp);
  }

  fwrite(&(vf->light_frames.num_frames),
         sizeof(vf->light_frames.num_frames), 1, fp);
  if ( vf->light_frames.frames != NULL ) {
    fwrite(vf->light_frames.frames,
           sizeof(vf->light_frames.frames[0]), vf->light_frames.num_frames, fp);
  }

  fwrite(&(vf->shadow_frames.num_frames),
         sizeof(vf->shadow_frames.num_frames), 1, fp);
  if ( vf->shadow_frames.frames != NULL ) {
    fwrite(vf->shadow_frames.frames,
           sizeof(vf->shadow_frames.frames[0]), vf->shadow_frames.num_frames, fp);
  }

  fwrite(&(vf->ik_frames.num_frames),
         sizeof(vf->ik_frames.num_frames), 1, fp);
  if ( vf->ik_frames.frames != NULL ) {
    fwrite(vf->ik_frames.frames,
           sizeof(vf->ik_frames.frames[0]), vf->ik_frames.num_frames, fp);
  }

  fclose(fp);
  fp = NULL;

  return true;
}

/**
 * @brief Release allocated VMDFile structure
 * @param (vf) a pointer to VMDFile structure
 * @return void
 */
void VMDReleaseVMDFile(VMDFile *vf){
  if ( vf == NULL ) {
    DEBUG_PRINT("VMDFile pointer is null");
    return;
  }

  if (vf->bone_frames.frames != NULL) free(vf->bone_frames.frames);
  if (vf->morph_frames.frames != NULL) free(vf->morph_frames.frames);
  if (vf->camera_frames.frames != NULL) free(vf->camera_frames.frames);
  if (vf->light_frames.frames != NULL) free(vf->light_frames.frames);
  if (vf->shadow_frames.frames != NULL) free(vf->shadow_frames.frames);
  if (vf->ik_frames.frames != NULL) free(vf->ik_frames.frames);
  free(vf);
  return;
}

/**
 * @brief sort all frames of VMDFile
 * @param (vf) a pointer to VMDFile structure
 * @return void
 * @sa VMDqsort
 */
void VMDSortAllFrames(VMDFile* vf){
  VMDqsort(vf->bone_frames.frames, vf->bone_frames.num_frames,
           sizeof(VMDBoneSingleFrame), VMDL_BONE);
  VMDqsort(vf->morph_frames.frames, vf->morph_frames.num_frames,
           sizeof(VMDMorphSingleFrame), VMDL_MORPH);
  VMDqsort(vf->camera_frames.frames, vf->camera_frames.num_frames,
           sizeof(VMDCameraSingleFrame), VMDL_CAMERA);
  VMDqsort(vf->light_frames.frames, vf->light_frames.num_frames,
           sizeof(VMDCameraSingleFrame), VMDL_LIGHT);
  VMDqsort(vf->shadow_frames.frames, vf->shadow_frames.num_frames,
           sizeof(VMDCameraSingleFrame), VMDL_SHADOW);
  VMDqsort(vf->ik_frames.frames, vf->ik_frames.num_frames,
           sizeof(VMDCameraSingleFrame), VMDL_IK);
  return;
}

/**
 * @brief display summary data for CLI
 * @param (vf) a pointer to VMDFile
 * @return void
 */
void VMDDisplayData(VMDFile* vf){
  printf("[Model    Name]: %s\n", vf->header.model_name);
  printf("[Bone   Frames]: %d\n", vf->bone_frames.num_frames);
  printf("[Morph  Frames]: %d\n", vf->morph_frames.num_frames);
  printf("[Camera Frames]: %d\n", vf->camera_frames.num_frames);
  printf("[Light  Frames]: %d\n", vf->light_frames.num_frames);
  printf("[Shadow Frames]: %d\n", vf->shadow_frames.num_frames);
  printf("[IK     Frames]: %d\n", vf->ik_frames.num_frames);
}

/**
 * @brief print out bone frames in CSV format
 * @param (vf) a pointer to VMDFile
 * @return void
 */
void VMDDumpAllBone2CSV(VMDFile* vf){
  printf("name,frame,x,y,z,qx,qy,qz,qw\n");
  for ( int i = 0; i < vf->bone_frames.num_frames; i++ ) {
    printf("%s,%d,%f,%f,%f,%f,%f,%f,%f\n",
           vf->bone_frames.frames[i].name,
           vf->bone_frames.frames[i].frame,
           vf->bone_frames.frames[i].x,
           vf->bone_frames.frames[i].y,
           vf->bone_frames.frames[i].z,
           vf->bone_frames.frames[i].qx,
           vf->bone_frames.frames[i].qy,
           vf->bone_frames.frames[i].qz,
           vf->bone_frames.frames[i].qw);
  }
}

/**
 * @brief print out morph frames in CSV format
 * @param (vf) a pointer to VMDFile
 * @return void
 */
void VMDDumpAllMorph2CSV(VMDFile* vf){
  printf("name,frame,value");
  for ( int i = 0; i < vf->morph_frames.num_frames; i++ ) {
    printf("%s,%d,%f",
           vf->morph_frames.frames[i].name,
           vf->morph_frames.frames[i].frame,
           vf->morph_frames.frames[i].value);
  }
}
