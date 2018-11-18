/**
 *  @file vmd.h
 *  @brief Library for VMD file in MikuMikuDance(MMD)
 *  @author ihm4
 */

#ifndef _H_VMDLIB_VMD_
#define _H_VMDLIB_VMD_

#include <stdint.h>
#include <stdbool.h>

// Error definitions and the global variable to store error code
#define VMDLIB_E_INIT (0x0000)
#define VMDLIB_E_FH   (0x0001)    /* failed to achieve file handler */
#define VMDLIB_E_FT   (0x0002)    /* invalid file type */
#define VMDLIB_E_ME   (0x0003)    /* memory allocation error */
#define VMDLIB_E_IV   (0xffff)    /* invalid call */
extern int VMD_ERROR;

// Magic number of VMD type file
#define VMDLIB_MAGIC ("Vocaloid Motion Data 0002\0\0\0\0")

// debug function taken from https://stackoverflow.com/a/1941336
#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

/*
 * VMDデータ構造は以下の通り

  struct Header header; // ヘッダ

  uint32_t boneCount; // ボーンキーフレーム数
  struct BoneSingleFrame bone[boneCount]; // ボーン要素データ

  uint32_t morphCount; // 表情キーフレーム数
  struct MorphSingleFrame morph[morphCount]; // 表情要素データ

  uint32_t cameraCount; // カメラキーフレーム数
  struct CameraSingleFrame camera[cameraCount]; // カメラ要素データ

  uint32_t lightCount; // 照明キーフレーム数
  struct LightSingleFrame light[lightCount]; // 照明要素データ
  // MMDv6.19以前で保存されたVMDはここまで

  uint32_t sshadowCount; // セルフ影キーフレーム数
  struct SelfShadowSingleFrame sShadow[sshadowCount]; // セルフ影要素データ
  // MMDv7.39.x64以前で保存されたVMDはここまで

  uint32_t showIKCount; // モデル表示・IK on/offキーフレーム数
  struct ShowIKSingleFrame showIK[showIKCount]; // 表示･IK on/off要素データ
 */

// ヘッダ
typedef struct
{
  char header[30]; // "Vocaloid Motion Data 0002\0\0\0\0\0" 30byte
  // (MMDver2以前のvmdは"Vocaloid Motion Data file\0")
  char model_name[20]; // モデル名 20byte(MMDver2以前のvmdは10byte)
  // 内容がカメラ,照明,セルフ影の場合は"カメラ・照明\0on Data"となる
} VMDHeader;

//ボーンキーフレーム要素データ(111Bytes/要素)
typedef struct
{
  char name[15]; // "センター\0"などのボーン名の文字列 15byte
  uint32_t frame; // フレーム番号
  float x; // ボーンのX軸位置,位置データがない場合は0
  float y; // ボーンのY軸位置,位置データがない場合は0
  float z; // ボーンのZ軸位置,位置データがない場合は0
  float qx; // ボーンのクォータニオンX回転,データがない場合は0
  float qy; // ボーンのクォータニオンY回転,データがない場合は0
  float qz; // ボーンのクォータニオンZ回転,データがない場合は0
  float qw; // ボーンのクォータニオンW回転,データがない場合は1
  char bezier[64]; // 補間パラメータ
} __attribute__((packed)) VMDBoneSingleFrame;
/*
   補間パラメータは4点のベジェ曲線(0,0),(x1,y1),(x2,y2),(127,127)で
   表している．各軸のパラメータを
   X軸の補間パラメータ　(X_x1,X_y1),(X_x2,X_y2)
   Y軸の補間パラメータ　(Y_x1,Y_y1),(Y_x2,Y_y2)
   Z軸の補間パラメータ　(Z_x1,Z_y1),(Z_x2,Z_y2)
   回転の補間パラメータ (R_x1,R_y1),(R_x2,R_y2)
   とした時、補間パラメータは以下の通り.
   X_x1,Y_x1,Z_x1,R_x1,X_y1,Y_y1,Z_y1,R_y1,
   X_x2,Y_x2,Z_x2,R_x2,X_y2,Y_y2,Z_y2,R_y2,
   Y_x1,Z_x1,R_x1,X_y1,Y_y1,Z_y1,R_y1,X_x2,
   Y_x2,Z_x2,R_x2,X_y2,Y_y2,Z_y2,R_y2, 01,
   Z_x1,R_x1,X_y1,Y_y1,Z_y1,R_y1,X_x2,Y_x2,
   Z_x2,R_x2,X_y2,Y_y2,Z_y2,R_y2, 01, 00,
   R_x1,X_y1,Y_y1,Z_y1,R_y1,X_x2,Y_x2,Z_x2,
   R_x2,X_y2,Y_y2,Z_y2,R_y2, 01, 00, 00
   */


//表情キーフレーム要素データ(23Bytes/要素)
typedef struct
{
  char name[15]; // "まばたき\0"などの表情名の文字列 15byte
  uint32_t frame; // フレーム番号
  float value; // 表情値(0～1)
} __attribute__((packed)) VMDMorphSingleFrame;


//カメラキーフレーム要素データ(61Bytes/要素)
typedef struct
{
  uint32_t frame; // フレーム番号
  float distance; // 目標点とカメラの距離(目標点がカメラ前面でマイナス)
  float x; // 目標点のX軸位置
  float y; // 目標点のY軸位置
  float z; // 目標点のZ軸位置
  float rx; // カメラのx軸回転(rad)(MMD数値入力のマイナス値)
  float ry; // カメラのy軸回転(rad)
  float rz; // カメラのz軸回転(rad)
  char bezier[24]; // 補間パラメータ
  uint32_t viewAngle; // 視野角(deg)
  char parth; // パースペクティブ, 0:ON, 1:OFF
} __attribute__((packed)) VMDCameraSingleFrame;
/*
   補間パラメータは4点のベジェ曲線(0,0),(x1,y1),(x2,y2),(127,127)で
   表している.各軸のパラメータを
   X軸の補間パラメータ　 (X_x1,X_y1),(X_x2,X_y2)
   Y軸の補間パラメータ　 (Y_x1,Y_y1),(Y_x2,Y_y2)
   Z軸の補間パラメータ　 (Z_x1,Z_y1),(Z_x2,Z_y2)
   回転の補間パラメータ　(R_x1,R_y1),(R_x2,R_y2)
   距離の補間パラメータ　(L_x1,L_y1),(L_x2,L_y2)
   視野角の補間パラメータ(V_x1,V_y1),(V_x2,V_y2)
   とした時、補間パラメータは以下の通り.
   X_x1 X_x2 X_y1 X_y2
   Y_x1 Y_x2 Y_y1 Y_y2
   Z_x1 Z_x2 Z_y1 Z_y2
   R_x1 R_x2 R_y1 R_y2
   L_x1 L_x2 L_y1 L_y2
   V_x1 V_x2 V_y1 V_y2
   */


//照明キーフレーム要素データ(28Bytes/要素)
typedef struct
{
  uint32_t frame; // フレーム番号
  float r; // 照明色赤(MMD入力値を256で割った値)
  float g; // 照明色緑(MMD入力値を256で割った値)
  float b; // 照明色青(MMD入力値を256で割った値)
  float x; // 照明x位置(MMD入力値)
  float y; // 照明y位置(MMD入力値)
  float z; // 照明z位置(MMD入力値)
} __attribute__((packed)) VMDLightSingleFrame;


//セルフ影キーフレーム要素データ(9Bytes/要素)
typedef struct
{
  uint32_t frame; // フレーム番号
  char type; // セルフシャドウ種類, 0:OFF, 1:mode1, 2:mode2
  float distance ; // シャドウ距離(MMD入力値Lを(10000-L)/100000とした値)
} __attribute__((packed)) VMDShadowSingleFrame;

typedef struct
{
  char name[20]; // "右足ＩＫ\0"などのIKボーン名の文字列 20byte
  char on_off; // IKのon/off, 0:OFF, 1:ON
} __attribute__((packed)) VMDInfoIK;

//モデル表示・IK on/offキーフレーム要素データ((9+21*IK数)Bytes/要素)
typedef struct
{
  uint32_t frame; // フレーム番号
  char show; // モデル表示, 0:OFF, 1:ON
  uint32_t ik_count; // 記録するIKの数
  VMDInfoIK ik; // IK on/off情報配列
} __attribute__((packed)) VMDIKSingleFrame;

typedef struct {
  uint32_t           num_frames;
  VMDBoneSingleFrame *frames;
} __attribute__((packed)) VMDBoneFrames;

typedef struct {
  uint32_t            num_frames;
  VMDMorphSingleFrame *frames;
} __attribute__((packed)) VMDMorphFrames;

typedef struct {
  uint32_t             num_frames;
  VMDCameraSingleFrame *frames;
} __attribute__((packed)) VMDCameraFrames;

typedef struct {
  uint32_t            num_frames;
  VMDLightSingleFrame *frames;
} __attribute__((packed)) VMDLightFrames;

typedef struct {
  uint32_t            num_frames;
  VMDShadowSingleFrame *frames;
} __attribute__((packed)) VMDShadowFrames;

typedef struct {
  uint32_t             num_frames;
  VMDIKSingleFrame *frames;
} __attribute__((packed)) VMDIKFrames;

// Whole data
typedef struct {
  VMDHeader       header;
  VMDBoneFrames   bone_frames;
  VMDMorphFrames  morph_frames;
  VMDCameraFrames camera_frames;
  VMDLightFrames  light_frames;
  VMDShadowFrames shadow_frames;
  VMDIKFrames     ik_frames;
} __attribute__((packed)) VMDFile;

typedef enum {
  VMDL_BONE,
  VMDL_MORPH,
  VMDL_CAMERA,
  VMDL_LIGHT,
  VMDL_SHADOW,
  VMDL_IK
} VMDStructType;

// function definitions
int __VMDCheckHeader(void*);
int __VMDCompareBoneFrameNumber(const void*, const void*);
int __VMDCompareMorphFrameNumber(const void*, const void*);
int __VMDCompareCameraFrameNumber(const void*, const void*);
int __VMDCompareLightFrameNumber(const void*, const void*);
int __VMDCompareShadowFrameNumber(const void*, const void*);
int __VMDCompareIKFrameNumber(const void*, const void*);
void VMDqsort(void*, size_t, size_t, VMDStructType);
VMDFile* VMDLoadFromFile(const char*);
bool VMDWriteToFile(VMDFile*, char* );
void VMDReleaseVMDFile(VMDFile*);
void VMDSortAllFrames(VMDFile*);
void VMDDisplayData(VMDFile*);
void VMDDumpAllBone2CSV(VMDFile*);
void VMDDumpAllMorph2CSV(VMDFile*);

#endif /* _H_VMDLIB_VMD_ */
