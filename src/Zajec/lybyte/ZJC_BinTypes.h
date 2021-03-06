#ifndef __ZJC_BinTypes_H__
#define __ZJC_BinTypes_H__

#ifdef __cplusplus
extern "C" {
#endif

//  - --- - --- - --- - --- -

#include "../ZJC_CommonTypes.h"

#define   BUILD_FRAC  0x01
#define   BUILD_JOJ   0x02

//  - --- - --- - --- - --- -

typedef struct JOJ_COLOR_COMPRESSED {

    uchar chroma_u;
    uchar chroma_v;
    uchar luma;
    uchar alpha;

} JOJPIX;

//  - --- - --- - --- - --- -

JOJPIX rgba_to_joj       (float r, float g, float b, float a);
void   joj_to_rgba       (float pixel[4], JOJPIX* joj);

uchar  bitsize           (uchar x);
uchar  usbitsize         (ushort x);

int    nthbit            (uchar b, int n);
int    uinthbit          (uint b, int n);

int    takebits          (uchar b, uint iStart, uint iEnd);

float  frac_tofloat      (uint frac, uint maxval, float fac, uint shift);
uchar  float_tofrac      (float v, float maxval, float fraction, uint fac, uint shift);

//  - --- - --- - --- - --- -

typedef struct RAWVERT3D {

    float co       [3];
    float normal   [3];
    float tangent  [3];
    float bhand;
    float uv       [2];

} RWV3D;

typedef struct RAWBOX3D {

    float co[24];

} RWB3D;

//  - --- - --- - --- - --- -

/*
     _________________________________________________________________________________
    |  � VP3D_64BITS � VERTEX_PACKED_3D �                                             |
    |_________________________________________________________________________________|
    |  FRST_UINT32                          |  SCND_UINT32                            |
    |_______________________________________|_________________________________________|
    |_byte0___|_byte1___|_byte2___|_byte3___|_byte0___|_byte1___|_byte2___|_byte3_____|
    | �       | �       | �       | �       |      �  |         |   �     |  �        |
    | 10000000| 10000000| 10000000| 10000000| 00000100| 00000000| 00100000| 01000000  |
    |_________|_________|_________|_________|______.__|_________|___._____|__.________|
    |_vpx_____|_vpy_____|_vpz_____|_nnx_____+xxn___|_ttt____________|_uvx____|_uvy____|
    |         |         |         |                |                |        |        |
    | 8       | 8       | 8       | 13             | 13             | 7      | 7      |
    |_________|_________|_________|________________|________________|________|________|

    the packing is pretty tight so I'll explain it

    vertex position (first 24 bits) follows the FRAC8 model: (idex(0, 256) - 128) * (1/32)

    vertex normals is where it gets interesting. we do nnx + xxn,
    which is (last 8 bits of FRST_UINT32) | (first 5 bits of SCND_UINT32 << 8)

    this 13bit model is fairly unique. the x element's model is FRAC6: (idex(0, 64) - 32) * (1/32)
    while the y element's model is FRAC5: (idex(0, 32) - 16) * (1/16)

    the last two bits store the sign of the z element and the bitangent handedness

    how do we compute the z element of a 13bit pack unit vector?

        z = 0; d = 1 - ( pow(x, 2) + pow(y, 2) )
        if(d > 0) { z = sqrt(d) * sign; }

    and the bitangent

        b = cross(n, t) * handedness

    the ttt element (SCND_UINT32, bits 5-18) for the tangent can then make use of an extra bit
    so both x and y elements follow the FRAC6 model, last bit is the sign of z
    z is then computed at shader runtime, same as the normal vector's

    texture coordinates (uvx, uvy) occupy the last 14 bits
    they follow the UFRAC7 model: idex(0, 128) * (1/128)
    this squishing of the uvs is a necessity but I'm still salty about it

    also a lot of distortion is caused by this roguish packing scheme,
    so fine details are bound to be blurred or lost and normal mapping gets wobbly
    I get those oldschool aesthetics for real, you post-process worshipping schmucks

*/

typedef struct VertexPacked3D {

    uint frac1;                                 // vpx, vpy, vpz, nnx
    uint frac2;                                 // xxn, ttt, uvx, uvy

} VP3D;

// and all that explaining just for two ints in a struct!
// guess I could make this a single long but GLSL wouldn't like that

typedef struct BoxPacked3D {

    uint frac1;                                 // v1x  v1y  v1z  v2x
    uint frac2;                                 // v2y  v2z  v3x  v3y
    uint frac3;                                 // v3z  v4x  v4y  v4z
    uint frac4;                                 // v5x  v5y  v5z  v6x
    uint frac5;                                 // v6y  v6z  v7x  v7y
    uint frac6;                                 // v7z  v8x  v8y  v8z

} BP3D;

void ZJC_pack_rawvert(VP3D* vert, RWV3D* data);
void ZJC_pack_rawbox (BP3D* box,  RWB3D* data);

//  - --- - --- - --- - --- -

#ifdef __cplusplus
}
#endif

#endif //__ZJC_BinTypes_H__
