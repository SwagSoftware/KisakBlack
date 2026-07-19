#include "r_dpvs_static.h"
#include "r_dvars.h"
#include "r_debug.h"
#include <qcommon/threads.h>

char __cdecl TestOccluders(const float (*bounds)[3], int numOccluders, float (*plane)[4])
{
    float v5; // [esp+4h] [ebp-858h]
    float v6; // [esp+8h] [ebp-854h]
    float v7; // [esp+Ch] [ebp-850h]
    float v8; // [esp+10h] [ebp-84Ch]
    int v9; // [esp+14h] [ebp-848h]
    int v10; // [esp+18h] [ebp-844h]
    int v11; // [esp+1Ch] [ebp-840h]
    int v12; // [esp+20h] [ebp-83Ch]
    float v13; // [esp+24h] [ebp-838h]
    float v14; // [esp+28h] [ebp-834h]
    float v15; // [esp+2Ch] [ebp-830h]
    float v16; // [esp+30h] [ebp-82Ch]
    int v17; // [esp+34h] [ebp-828h]
    int v18; // [esp+38h] [ebp-824h]
    int v19; // [esp+3Ch] [ebp-820h]
    int v20; // [esp+40h] [ebp-81Ch]
    float v21; // [esp+44h] [ebp-818h]
    float v22; // [esp+48h] [ebp-814h]
    float v23; // [esp+4Ch] [ebp-810h]
    float v24; // [esp+50h] [ebp-80Ch]
    int v25; // [esp+54h] [ebp-808h]
    int v26; // [esp+58h] [ebp-804h]
    int v27; // [esp+5Ch] [ebp-800h]
    int v28; // [esp+60h] [ebp-7FCh]
    float v29; // [esp+64h] [ebp-7F8h]
    float v30; // [esp+68h] [ebp-7F4h]
    float v31; // [esp+6Ch] [ebp-7F0h]
    float v32; // [esp+70h] [ebp-7ECh]
    int v33; // [esp+74h] [ebp-7E8h]
    int v34; // [esp+78h] [ebp-7E4h]
    int v35; // [esp+7Ch] [ebp-7E0h]
    int v36; // [esp+80h] [ebp-7DCh]
    int v37; // [esp+84h] [ebp-7D8h]
    int v38; // [esp+88h] [ebp-7D4h]
    int v39; // [esp+8Ch] [ebp-7D0h]
    int v40; // [esp+90h] [ebp-7CCh]
    __int64 v41; // [esp+144h] [ebp-718h]
    __int64 v42; // [esp+1F8h] [ebp-664h]
    __int64 v43; // [esp+2ACh] [ebp-5B0h]
    __int64 v44; // [esp+360h] [ebp-4FCh]
    __int64 v45; // [esp+3E8h] [ebp-474h]
    float v46; // [esp+4C4h] [ebp-398h]
    float v47; // [esp+574h] [ebp-2E8h]
    float v48; // [esp+624h] [ebp-238h]
    float v49; // [esp+6D4h] [ebp-188h]
    __int64 p5; // [esp+7E4h] [ebp-78h]
    __int64 p5_8; // [esp+7ECh] [ebp-70h]
    __int64 p1; // [esp+7F4h] [ebp-68h]
    __int64 p1_8; // [esp+7FCh] [ebp-60h]
    __int64 p3; // [esp+804h] [ebp-58h]
    __int64 p3_8; // [esp+80Ch] [ebp-50h]
    __int64 p2; // [esp+814h] [ebp-48h]
    __int64 p2_8; // [esp+81Ch] [ebp-40h]
    __int64 p4; // [esp+824h] [ebp-38h]
    __int64 p4_8; // [esp+82Ch] [ebp-30h]
    int i; // [esp+834h] [ebp-28h]
    __int64 max; // [esp+838h] [ebp-24h]
    __int64 max_8; // [esp+840h] [ebp-1Ch]
    __int64 min; // [esp+848h] [ebp-14h]
    __int64 min_8; // [esp+850h] [ebp-Ch]

    if ( !numOccluders )
        return 0;
    min = *(_QWORD *)g_keepXYZ.v & *(_QWORD *)&(*bounds)[0];
    min_8 = *(_QWORD *)&g_keepXYZ.unitVec[2].packed & *(_QWORD *)&(*bounds)[2];
    max = *(_QWORD *)g_keepXYZ.v & *(_QWORD *)&(*bounds)[3];
    max_8 = *(_QWORD *)&g_keepXYZ.unitVec[2].packed & *(_QWORD *)&(*bounds)[5];
    for ( i = 0; i < numOccluders; ++i )
    {
        p1 = *(_QWORD *)&(*plane)[0];
        p1_8 = *(_QWORD *)&(*plane)[2];
        p2 = *(_QWORD *)&(*plane)[4];
        p2_8 = *(_QWORD *)&(*plane)[6];
        p3 = *(_QWORD *)&(*plane)[8];
        p3_8 = *(_QWORD *)&(*plane)[10];
        p4 = *(_QWORD *)&(*plane)[12];
        p4_8 = *(_QWORD *)&(*plane)[14];
        p5 = *(_QWORD *)&(*plane)[16];
        p5_8 = *(_QWORD *)&(*plane)[18];
        plane += 5;
        if ( *(float *)&p5 <= 0.0 )
            v40 = 0;
        else
            v40 = -1;
        LODWORD(v45) = v40;
        if ( *((float *)&p5 + 1) <= 0.0 )
            v39 = 0;
        else
            v39 = -1;
        HIDWORD(v45) = v39;
        if ( *(float *)&p5_8 <= 0.0 )
            v38 = 0;
        else
            v38 = -1;
        if ( *((float *)&p5_8 + 1) <= 0.0 )
            v37 = 0;
        else
            v37 = -1;
        if ( *(float *)&p4 <= 0.0 )
            v36 = 0;
        else
            v36 = -1;
        LODWORD(v44) = v36;
        if ( *((float *)&p4 + 1) <= 0.0 )
            v35 = 0;
        else
            v35 = -1;
        HIDWORD(v44) = v35;
        if ( *(float *)&p4_8 <= 0.0 )
            v34 = 0;
        else
            v34 = -1;
        if ( *((float *)&p4_8 + 1) <= 0.0 )
            v33 = 0;
        else
            v33 = -1;
        v49 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v36 | min & ~v36))
                                                                + (float)(*((float *)&p4 + 1)
                                                                                * COERCE_FLOAT((max & v44 | min & (unsigned __int64)~v44) >> 32)))
                                                + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v34 | min_8 & ~v34)))
                                + (float)(*((float *)&p4_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v33 | HIDWORD(min_8) & ~v33)))
                - *((float *)&p4_8 + 1);
        if ( (float)(v49
                             - (float)((float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                                             + (float)(*((float *)&p5 + 1)
                                                                                                             * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                                             + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                                             + (float)(*((float *)&p5_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                                             - *((float *)&p5_8 + 1))) < 0.0 )
            v32 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                    - *((float *)&p5_8 + 1);
        else
            v32 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v36 | min & ~v36))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v44 | min & (unsigned __int64)~v44) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v34 | min_8 & ~v34)))
                                    + (float)(*((float *)&p4_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v33 | HIDWORD(min_8) & ~v33)))
                    - *((float *)&p4_8 + 1);
        if ( (float)(v49
                             - (float)((float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                                             + (float)(*((float *)&p5 + 1)
                                                                                                             * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                                             + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                                             + (float)(*((float *)&p5_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                                             - *((float *)&p5_8 + 1))) < 0.0 )
            v31 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                    - *((float *)&p5_8 + 1);
        else
            v31 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v36 | min & ~v36))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v44 | min & (unsigned __int64)~v44) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v34 | min_8 & ~v34)))
                                    + (float)(*((float *)&p4_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v33 | HIDWORD(min_8) & ~v33)))
                    - *((float *)&p4_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v36 | min & ~v36))
                                                                                             + (float)(*((float *)&p4 + 1)
                                                                                                             * COERCE_FLOAT((max & v44 | min & (unsigned __int64)~v44) >> 32)))
                                                                             + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v34 | min_8 & ~v34)))
                                                             + (float)(*((float *)&p4_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v33 | HIDWORD(min_8) & ~v33)))
                                             - *((float *)&p4_8 + 1))
                             - (float)((float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                                             + (float)(*((float *)&p5 + 1)
                                                                                                             * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                                             + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                                             + (float)(*((float *)&p5_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                                             - *((float *)&p5_8 + 1))) < 0.0 )
            v30 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                    - *((float *)&p5_8 + 1);
        else
            v30 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v36 | min & ~v36))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v44 | min & (unsigned __int64)~v44) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v34 | min_8 & ~v34)))
                                    + (float)(*((float *)&p4_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v33 | HIDWORD(min_8) & ~v33)))
                    - *((float *)&p4_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v36 | min & ~v36))
                                                                                             + (float)(*((float *)&p4 + 1)
                                                                                                             * COERCE_FLOAT((max & v44 | min & (unsigned __int64)~v44) >> 32)))
                                                                             + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v34 | min_8 & ~v34)))
                                                             + (float)(*((float *)&p4_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v33 | HIDWORD(min_8) & ~v33)))
                                             - *((float *)&p4_8 + 1))
                             - (float)((float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                                             + (float)(*((float *)&p5 + 1)
                                                                                                             * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                                             + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                                             + (float)(*((float *)&p5_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                                             - *((float *)&p5_8 + 1))) < 0.0 )
            v29 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v40 | min & ~v40))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v45 | min & (unsigned __int64)~v45) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v38 | min_8 & ~v38)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v37 | HIDWORD(min_8) & ~v37)))
                    - *((float *)&p5_8 + 1);
        else
            v29 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v36 | min & ~v36))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v44 | min & (unsigned __int64)~v44) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v34 | min_8 & ~v34)))
                                    + (float)(*((float *)&p4_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v33 | HIDWORD(min_8) & ~v33)))
                    - *((float *)&p4_8 + 1);
        if ( *(float *)&p3 <= 0.0 )
            v28 = 0;
        else
            v28 = -1;
        LODWORD(v43) = v28;
        if ( *((float *)&p3 + 1) <= 0.0 )
            v27 = 0;
        else
            v27 = -1;
        HIDWORD(v43) = v27;
        if ( *(float *)&p3_8 <= 0.0 )
            v26 = 0;
        else
            v26 = -1;
        if ( *((float *)&p3_8 + 1) <= 0.0 )
            v25 = 0;
        else
            v25 = -1;
        v48 = (float)((float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v28 | min & ~v28))
                                                                + (float)(*((float *)&p3 + 1)
                                                                                * COERCE_FLOAT((max & v43 | min & (unsigned __int64)~v43) >> 32)))
                                                + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v26 | min_8 & ~v26)))
                                + (float)(*((float *)&p3_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v25 | HIDWORD(min_8) & ~v25)))
                - *((float *)&p3_8 + 1);
        if ( (float)(v48 - v32) < 0.0 )
            v24 = v32;
        else
            v24 = (float)((float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v28 | min & ~v28))
                                                                    + (float)(*((float *)&p3 + 1)
                                                                                    * COERCE_FLOAT((max & v43 | min & (unsigned __int64)~v43) >> 32)))
                                                    + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v26 | min_8 & ~v26)))
                                    + (float)(*((float *)&p3_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v25 | HIDWORD(min_8) & ~v25)))
                    - *((float *)&p3_8 + 1);
        if ( (float)(v48 - v31) < 0.0 )
            v23 = v31;
        else
            v23 = (float)((float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v28 | min & ~v28))
                                                                    + (float)(*((float *)&p3 + 1)
                                                                                    * COERCE_FLOAT((max & v43 | min & (unsigned __int64)~v43) >> 32)))
                                                    + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v26 | min_8 & ~v26)))
                                    + (float)(*((float *)&p3_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v25 | HIDWORD(min_8) & ~v25)))
                    - *((float *)&p3_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v28 | min & ~v28))
                                                                                             + (float)(*((float *)&p3 + 1)
                                                                                                             * COERCE_FLOAT((max & v43 | min & (unsigned __int64)~v43) >> 32)))
                                                                             + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v26 | min_8 & ~v26)))
                                                             + (float)(*((float *)&p3_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v25 | HIDWORD(min_8) & ~v25)))
                                             - *((float *)&p3_8 + 1))
                             - v30) < 0.0 )
            v22 = v30;
        else
            v22 = (float)((float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v28 | min & ~v28))
                                                                    + (float)(*((float *)&p3 + 1)
                                                                                    * COERCE_FLOAT((max & v43 | min & (unsigned __int64)~v43) >> 32)))
                                                    + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v26 | min_8 & ~v26)))
                                    + (float)(*((float *)&p3_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v25 | HIDWORD(min_8) & ~v25)))
                    - *((float *)&p3_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v28 | min & ~v28))
                                                                                             + (float)(*((float *)&p3 + 1)
                                                                                                             * COERCE_FLOAT((max & v43 | min & (unsigned __int64)~v43) >> 32)))
                                                                             + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v26 | min_8 & ~v26)))
                                                             + (float)(*((float *)&p3_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v25 | HIDWORD(min_8) & ~v25)))
                                             - *((float *)&p3_8 + 1))
                             - v29) < 0.0 )
            v21 = v29;
        else
            v21 = (float)((float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v28 | min & ~v28))
                                                                    + (float)(*((float *)&p3 + 1)
                                                                                    * COERCE_FLOAT((max & v43 | min & (unsigned __int64)~v43) >> 32)))
                                                    + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v26 | min_8 & ~v26)))
                                    + (float)(*((float *)&p3_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v25 | HIDWORD(min_8) & ~v25)))
                    - *((float *)&p3_8 + 1);
        if ( *(float *)&p2 <= 0.0 )
            v20 = 0;
        else
            v20 = -1;
        LODWORD(v42) = v20;
        if ( *((float *)&p2 + 1) <= 0.0 )
            v19 = 0;
        else
            v19 = -1;
        HIDWORD(v42) = v19;
        if ( *(float *)&p2_8 <= 0.0 )
            v18 = 0;
        else
            v18 = -1;
        if ( *((float *)&p2_8 + 1) <= 0.0 )
            v17 = 0;
        else
            v17 = -1;
        v47 = (float)((float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v20 | min & ~v20))
                                                                + (float)(*((float *)&p2 + 1)
                                                                                * COERCE_FLOAT((max & v42 | min & (unsigned __int64)~v42) >> 32)))
                                                + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v18 | min_8 & ~v18)))
                                + (float)(*((float *)&p2_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v17 | HIDWORD(min_8) & ~v17)))
                - *((float *)&p2_8 + 1);
        if ( (float)(v47 - v24) < 0.0 )
            v16 = v24;
        else
            v16 = (float)((float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v20 | min & ~v20))
                                                                    + (float)(*((float *)&p2 + 1)
                                                                                    * COERCE_FLOAT((max & v42 | min & (unsigned __int64)~v42) >> 32)))
                                                    + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v18 | min_8 & ~v18)))
                                    + (float)(*((float *)&p2_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v17 | HIDWORD(min_8) & ~v17)))
                    - *((float *)&p2_8 + 1);
        if ( (float)(v47 - v23) < 0.0 )
            v15 = v23;
        else
            v15 = (float)((float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v20 | min & ~v20))
                                                                    + (float)(*((float *)&p2 + 1)
                                                                                    * COERCE_FLOAT((max & v42 | min & (unsigned __int64)~v42) >> 32)))
                                                    + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v18 | min_8 & ~v18)))
                                    + (float)(*((float *)&p2_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v17 | HIDWORD(min_8) & ~v17)))
                    - *((float *)&p2_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v20 | min & ~v20))
                                                                                             + (float)(*((float *)&p2 + 1)
                                                                                                             * COERCE_FLOAT((max & v42 | min & (unsigned __int64)~v42) >> 32)))
                                                                             + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v18 | min_8 & ~v18)))
                                                             + (float)(*((float *)&p2_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v17 | HIDWORD(min_8) & ~v17)))
                                             - *((float *)&p2_8 + 1))
                             - v22) < 0.0 )
            v14 = v22;
        else
            v14 = (float)((float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v20 | min & ~v20))
                                                                    + (float)(*((float *)&p2 + 1)
                                                                                    * COERCE_FLOAT((max & v42 | min & (unsigned __int64)~v42) >> 32)))
                                                    + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v18 | min_8 & ~v18)))
                                    + (float)(*((float *)&p2_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v17 | HIDWORD(min_8) & ~v17)))
                    - *((float *)&p2_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v20 | min & ~v20))
                                                                                             + (float)(*((float *)&p2 + 1)
                                                                                                             * COERCE_FLOAT((max & v42 | min & (unsigned __int64)~v42) >> 32)))
                                                                             + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v18 | min_8 & ~v18)))
                                                             + (float)(*((float *)&p2_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v17 | HIDWORD(min_8) & ~v17)))
                                             - *((float *)&p2_8 + 1))
                             - v21) < 0.0 )
            v13 = v21;
        else
            v13 = (float)((float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v20 | min & ~v20))
                                                                    + (float)(*((float *)&p2 + 1)
                                                                                    * COERCE_FLOAT((max & v42 | min & (unsigned __int64)~v42) >> 32)))
                                                    + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v18 | min_8 & ~v18)))
                                    + (float)(*((float *)&p2_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v17 | HIDWORD(min_8) & ~v17)))
                    - *((float *)&p2_8 + 1);
        if ( *(float *)&p1 <= 0.0 )
            v12 = 0;
        else
            v12 = -1;
        LODWORD(v41) = v12;
        if ( *((float *)&p1 + 1) <= 0.0 )
            v11 = 0;
        else
            v11 = -1;
        HIDWORD(v41) = v11;
        if ( *(float *)&p1_8 <= 0.0 )
            v10 = 0;
        else
            v10 = -1;
        if ( *((float *)&p1_8 + 1) <= 0.0 )
            v9 = 0;
        else
            v9 = -1;
        v46 = (float)((float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v12 | min & ~v12))
                                                                + (float)(*((float *)&p1 + 1)
                                                                                * COERCE_FLOAT((max & v41 | min & (unsigned __int64)~v41) >> 32)))
                                                + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v10 | min_8 & ~v10)))
                                + (float)(*((float *)&p1_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v9 | HIDWORD(min_8) & ~v9)))
                - *((float *)&p1_8 + 1);
        if ( (float)(v46 - v16) < 0.0 )
            v8 = v16;
        else
            v8 = (float)((float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v12 | min & ~v12))
                                                                 + (float)(*((float *)&p1 + 1)
                                                                                 * COERCE_FLOAT((max & v41 | min & (unsigned __int64)~v41) >> 32)))
                                                 + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v10 | min_8 & ~v10)))
                                 + (float)(*((float *)&p1_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v9 | HIDWORD(min_8) & ~v9)))
                 - *((float *)&p1_8 + 1);
        if ( (float)(v46 - v15) < 0.0 )
            v7 = v15;
        else
            v7 = (float)((float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v12 | min & ~v12))
                                                                 + (float)(*((float *)&p1 + 1)
                                                                                 * COERCE_FLOAT((max & v41 | min & (unsigned __int64)~v41) >> 32)))
                                                 + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v10 | min_8 & ~v10)))
                                 + (float)(*((float *)&p1_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v9 | HIDWORD(min_8) & ~v9)))
                 - *((float *)&p1_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v12 | min & ~v12))
                                                                                             + (float)(*((float *)&p1 + 1)
                                                                                                             * COERCE_FLOAT((max & v41 | min & (unsigned __int64)~v41) >> 32)))
                                                                             + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v10 | min_8 & ~v10)))
                                                             + (float)(*((float *)&p1_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v9 | HIDWORD(min_8) & ~v9)))
                                             - *((float *)&p1_8 + 1))
                             - v14) < 0.0 )
            v6 = v14;
        else
            v6 = (float)((float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v12 | min & ~v12))
                                                                 + (float)(*((float *)&p1 + 1)
                                                                                 * COERCE_FLOAT((max & v41 | min & (unsigned __int64)~v41) >> 32)))
                                                 + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v10 | min_8 & ~v10)))
                                 + (float)(*((float *)&p1_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v9 | HIDWORD(min_8) & ~v9)))
                 - *((float *)&p1_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v12 | min & ~v12))
                                                                                             + (float)(*((float *)&p1 + 1)
                                                                                                             * COERCE_FLOAT((max & v41 | min & (unsigned __int64)~v41) >> 32)))
                                                                             + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v10 | min_8 & ~v10)))
                                                             + (float)(*((float *)&p1_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v9 | HIDWORD(min_8) & ~v9)))
                                             - *((float *)&p1_8 + 1))
                             - v13) < 0.0 )
            v5 = v13;
        else
            v5 = (float)((float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v12 | min & ~v12))
                                                                 + (float)(*((float *)&p1 + 1)
                                                                                 * COERCE_FLOAT((max & v41 | min & (unsigned __int64)~v41) >> 32)))
                                                 + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v10 | min_8 & ~v10)))
                                 + (float)(*((float *)&p1_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v9 | HIDWORD(min_8) & ~v9)))
                 - *((float *)&p1_8 + 1);
        if ( v8 < 0.0 || v7 < 0.0 || v6 < 0.0 || v5 < 0.0 )
            return 1;
    }
    return 0;
}

float (*__cdecl TestOccludersPartial(const float (*bounds)[3], int *numOccludersPtr, float (*plane)[4]))[4]
{
    __int64 *v3; // ecx
    __int64 *v4; // ecx
    __int64 *v5; // ecx
    __int64 *v6; // ecx
    __int64 *v7; // ecx
    __int64 *v9; // eax
    __int64 *v10; // eax
    __int64 *v11; // eax
    __int64 *v12; // eax
    __int64 *v13; // eax
    _QWORD *v14; // eax
    _QWORD *v15; // eax
    _QWORD *v16; // eax
    _QWORD *v17; // eax
    _QWORD *v18; // eax
    _QWORD *v19; // eax
    _QWORD *v20; // eax
    _QWORD *v21; // eax
    _QWORD *v22; // eax
    _QWORD *v23; // eax
    float v25; // [esp+4h] [ebp-E94h]
    float v26; // [esp+8h] [ebp-E90h]
    float v27; // [esp+Ch] [ebp-E8Ch]
    float v28; // [esp+10h] [ebp-E88h]
    float v29; // [esp+14h] [ebp-E84h]
    float v30; // [esp+18h] [ebp-E80h]
    float v31; // [esp+1Ch] [ebp-E7Ch]
    float v32; // [esp+20h] [ebp-E78h]
    float v33; // [esp+24h] [ebp-E74h]
    float v34; // [esp+28h] [ebp-E70h]
    float v35; // [esp+2Ch] [ebp-E6Ch]
    float v36; // [esp+30h] [ebp-E68h]
    float v37; // [esp+34h] [ebp-E64h]
    float v38; // [esp+38h] [ebp-E60h]
    float v39; // [esp+3Ch] [ebp-E5Ch]
    float v40; // [esp+40h] [ebp-E58h]
    float v42; // [esp+48h] [ebp-E50h]
    float v43; // [esp+4Ch] [ebp-E4Ch]
    float v44; // [esp+50h] [ebp-E48h]
    float v45; // [esp+54h] [ebp-E44h]
    float v46; // [esp+58h] [ebp-E40h]
    float v47; // [esp+5Ch] [ebp-E3Ch]
    float v48; // [esp+60h] [ebp-E38h]
    float v49; // [esp+64h] [ebp-E34h]
    float v50; // [esp+68h] [ebp-E30h]
    float v51; // [esp+6Ch] [ebp-E2Ch]
    float v52; // [esp+70h] [ebp-E28h]
    float v53; // [esp+74h] [ebp-E24h]
    float v54; // [esp+78h] [ebp-E20h]
    float v55; // [esp+7Ch] [ebp-E1Ch]
    float v56; // [esp+80h] [ebp-E18h]
    float v57; // [esp+84h] [ebp-E14h]
    int v58; // [esp+88h] [ebp-E10h]
    int v59; // [esp+8Ch] [ebp-E0Ch]
    int v60; // [esp+90h] [ebp-E08h]
    int v61; // [esp+94h] [ebp-E04h]
    int v62; // [esp+98h] [ebp-E00h]
    int v63; // [esp+9Ch] [ebp-DFCh]
    int v64; // [esp+A0h] [ebp-DF8h]
    int v65; // [esp+A4h] [ebp-DF4h]
    int v66; // [esp+A8h] [ebp-DF0h]
    int v67; // [esp+ACh] [ebp-DECh]
    int v68; // [esp+B0h] [ebp-DE8h]
    int v69; // [esp+B4h] [ebp-DE4h]
    int v70; // [esp+B8h] [ebp-DE0h]
    int v71; // [esp+BCh] [ebp-DDCh]
    int v72; // [esp+C0h] [ebp-DD8h]
    int v73; // [esp+C4h] [ebp-DD4h]
    int v74; // [esp+C8h] [ebp-DD0h]
    int v75; // [esp+CCh] [ebp-DCCh]
    int v76; // [esp+D0h] [ebp-DC8h]
    int v77; // [esp+D4h] [ebp-DC4h]
    float v78; // [esp+144h] [ebp-D54h]
    float v79; // [esp+1D0h] [ebp-CC8h]
    float v80; // [esp+25Ch] [ebp-C3Ch]
    float v81; // [esp+2E8h] [ebp-BB0h]
    float v82; // [esp+348h] [ebp-B50h]
    float v83; // [esp+3F8h] [ebp-AA0h]
    float v84; // [esp+484h] [ebp-A14h]
    float v85; // [esp+510h] [ebp-988h]
    __int64 v86; // [esp+6A4h] [ebp-7F4h]
    __int64 v87; // [esp+6B8h] [ebp-7E0h]
    __int64 v88; // [esp+6CCh] [ebp-7CCh]
    __int64 v89; // [esp+6E0h] [ebp-7B8h]
    __int64 v90; // [esp+6F4h] [ebp-7A4h]
    float v91; // [esp+B18h] [ebp-380h]
    float v92; // [esp+B78h] [ebp-320h]
    __int64 t4; // [esp+D2Ch] [ebp-16Ch]
    __int64 t4_8; // [esp+D34h] [ebp-164h]
    __int64 t1; // [esp+D3Ch] [ebp-15Ch]
    __int64 t1_8; // [esp+D44h] [ebp-154h]
    __int64 t5; // [esp+D4Ch] [ebp-14Ch]
    __int64 t5_8; // [esp+D54h] [ebp-144h]
    __int64 t3; // [esp+D5Ch] [ebp-13Ch]
    __int64 t3_8; // [esp+D64h] [ebp-134h]
    int idx; // [esp+D6Ch] [ebp-12Ch]
    __int64 t2; // [esp+D70h] [ebp-128h]
    __int64 t2_8; // [esp+D78h] [ebp-120h]
    __int64 p5; // [esp+DA0h] [ebp-F8h]
    __int64 p5_8; // [esp+DA8h] [ebp-F0h]
    __int64 p1; // [esp+DD0h] [ebp-C8h]
    __int64 p1_8; // [esp+DD8h] [ebp-C0h]
    __int64 p3; // [esp+DE0h] [ebp-B8h]
    __int64 p3_8; // [esp+DE8h] [ebp-B0h]
    __int64 p2; // [esp+E30h] [ebp-68h]
    __int64 p2_8; // [esp+E38h] [ebp-60h]
    __int64 p4; // [esp+E50h] [ebp-48h]
    __int64 p4_8; // [esp+E58h] [ebp-40h]
    __int64 max; // [esp+E70h] [ebp-28h]
    __int64 max_8; // [esp+E78h] [ebp-20h]
    __int64 min; // [esp+E80h] [ebp-18h]
    __int64 min_8; // [esp+E88h] [ebp-10h]
    int planeIdx; // [esp+E94h] [ebp-4h]

    if ( ((unsigned __int8)plane & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_static.cpp",
                    288,
                    0,
                    "%s",
                    "(uint32_t(plane) & 0xf) == 0") )
    {
        __debugbreak();
    }
    min = *(_QWORD *)g_keepXYZ.v & *(_QWORD *)&(*bounds)[0];
    min_8 = *(_QWORD *)&g_keepXYZ.unitVec[2].packed & *(_QWORD *)&(*bounds)[2];
    max = *(_QWORD *)g_keepXYZ.v & *(_QWORD *)&(*bounds)[3];
    max_8 = *(_QWORD *)&g_keepXYZ.unitVec[2].packed & *(_QWORD *)&(*bounds)[5];
    for ( planeIdx = 0; planeIdx < *numOccludersPtr; ++planeIdx )
    {
        v3 = (__int64 *)&(*plane)[20 * planeIdx];
        p1 = *v3;
        p1_8 = v3[1];
        v4 = (__int64 *)&(*plane)[20 * planeIdx + 4];
        p2 = *v4;
        p2_8 = v4[1];
        v5 = (__int64 *)&(*plane)[20 * planeIdx + 8];
        p3 = *v5;
        p3_8 = v5[1];
        v6 = (__int64 *)&(*plane)[20 * planeIdx + 12];
        p4 = *v6;
        p4_8 = v6[1];
        v7 = (__int64 *)&(*plane)[20 * planeIdx + 16];
        p5 = *v7;
        p5_8 = v7[1];
        if ( *(float *)&p1 <= 0.0 )
            v77 = 0;
        else
            v77 = -1;
        LODWORD(v90) = v77;
        if ( *((float *)&p1 + 1) <= 0.0 )
            v76 = 0;
        else
            v76 = -1;
        HIDWORD(v90) = v76;
        if ( *(float *)&p1_8 <= 0.0 )
            v75 = 0;
        else
            v75 = -1;
        if ( *((float *)&p1_8 + 1) <= 0.0 )
            v74 = 0;
        else
            v74 = -1;
        if ( *(float *)&p2 <= 0.0 )
            v73 = 0;
        else
            v73 = -1;
        LODWORD(v89) = v73;
        if ( *((float *)&p2 + 1) <= 0.0 )
            v72 = 0;
        else
            v72 = -1;
        HIDWORD(v89) = v72;
        if ( *(float *)&p2_8 <= 0.0 )
            v71 = 0;
        else
            v71 = -1;
        if ( *((float *)&p2_8 + 1) <= 0.0 )
            v70 = 0;
        else
            v70 = -1;
        if ( *(float *)&p3 <= 0.0 )
            v69 = 0;
        else
            v69 = -1;
        LODWORD(v88) = v69;
        if ( *((float *)&p3 + 1) <= 0.0 )
            v68 = 0;
        else
            v68 = -1;
        HIDWORD(v88) = v68;
        if ( *(float *)&p3_8 <= 0.0 )
            v67 = 0;
        else
            v67 = -1;
        if ( *((float *)&p3_8 + 1) <= 0.0 )
            v66 = 0;
        else
            v66 = -1;
        if ( *(float *)&p4 <= 0.0 )
            v65 = 0;
        else
            v65 = -1;
        LODWORD(v87) = v65;
        if ( *((float *)&p4 + 1) <= 0.0 )
            v64 = 0;
        else
            v64 = -1;
        HIDWORD(v87) = v64;
        if ( *(float *)&p4_8 <= 0.0 )
            v63 = 0;
        else
            v63 = -1;
        if ( *((float *)&p4_8 + 1) <= 0.0 )
            v62 = 0;
        else
            v62 = -1;
        if ( *(float *)&p5 <= 0.0 )
            v61 = 0;
        else
            v61 = -1;
        LODWORD(v86) = v61;
        if ( *((float *)&p5 + 1) <= 0.0 )
            v60 = 0;
        else
            v60 = -1;
        HIDWORD(v86) = v60;
        if ( *(float *)&p5_8 <= 0.0 )
            v59 = 0;
        else
            v59 = -1;
        if ( *((float *)&p5_8 + 1) <= 0.0 )
            v58 = 0;
        else
            v58 = -1;
        v92 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v61 | min & ~v61))
                                                                + (float)(*((float *)&p5 + 1)
                                                                                * COERCE_FLOAT((max & v86 | min & (unsigned __int64)~v86) >> 32)))
                                                + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v59 | min_8 & ~v59)))
                                + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v58 | HIDWORD(min_8) & ~v58)))
                - *((float *)&p5_8 + 1);
        v91 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v65 | min & ~v65))
                                                                + (float)(*((float *)&p4 + 1)
                                                                                * COERCE_FLOAT((max & v87 | min & (unsigned __int64)~v87) >> 32)))
                                                + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v63 | min_8 & ~v63)))
                                + (float)(*((float *)&p4_8 + 1)
                                                * COERCE_FLOAT((max_8 & __PAIR64__(v62, v63) | min_8 & ~__PAIR64__(v62, v63)) >> 32)))
                - *((float *)&p4_8 + 1);
        if ( (float)(v91 - v92) < 0.0 )
            v57 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v61 | min & ~v61))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v86 | min & (unsigned __int64)~v86) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v59 | min_8 & ~v59)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v58 | HIDWORD(min_8) & ~v58)))
                    - *((float *)&p5_8 + 1);
        else
            v57 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v65 | min & ~v65))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v87 | min & (unsigned __int64)~v87) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v63 | min_8 & ~v63)))
                                    + (float)(*((float *)&p4_8 + 1)
                                                    * COERCE_FLOAT((max_8 & __PAIR64__(v62, v63) | min_8 & ~__PAIR64__(v62, v63)) >> 32)))
                    - *((float *)&p4_8 + 1);
        if ( (float)(v91 - v92) < 0.0 )
            v56 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v61 | min & ~v61))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v86 | min & (unsigned __int64)~v86) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v59 | min_8 & ~v59)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v58 | HIDWORD(min_8) & ~v58)))
                    - *((float *)&p5_8 + 1);
        else
            v56 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v65 | min & ~v65))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v87 | min & (unsigned __int64)~v87) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v63 | min_8 & ~v63)))
                                    + (float)(*((float *)&p4_8 + 1)
                                                    * COERCE_FLOAT((max_8 & __PAIR64__(v62, v63) | min_8 & ~__PAIR64__(v62, v63)) >> 32)))
                    - *((float *)&p4_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v65 | min & ~v65))
                                                                                             + (float)(*((float *)&p4 + 1)
                                                                                                             * COERCE_FLOAT((max & v87 | min & (unsigned __int64)~v87) >> 32)))
                                                                             + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v63 | min_8 & ~v63)))
                                                             + (float)(*((float *)&p4_8 + 1)
                                                                             * COERCE_FLOAT((max_8 & __PAIR64__(v62, v63) | min_8 & ~__PAIR64__(v62, v63)) >> 32)))
                                             - *((float *)&p4_8 + 1))
                             - (float)((float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v61 | min & ~v61))
                                                                                             + (float)(*((float *)&p5 + 1)
                                                                                                             * COERCE_FLOAT((max & v86 | min & (unsigned __int64)~v86) >> 32)))
                                                                             + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v59 | min_8 & ~v59)))
                                                             + (float)(*((float *)&p5_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v58 | HIDWORD(min_8) & ~v58)))
                                             - *((float *)&p5_8 + 1))) < 0.0 )
            v55 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v61 | min & ~v61))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v86 | min & (unsigned __int64)~v86) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v59 | min_8 & ~v59)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v58 | HIDWORD(min_8) & ~v58)))
                    - *((float *)&p5_8 + 1);
        else
            v55 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v65 | min & ~v65))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v87 | min & (unsigned __int64)~v87) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v63 | min_8 & ~v63)))
                                    + (float)(*((float *)&p4_8 + 1)
                                                    * COERCE_FLOAT((max_8 & __PAIR64__(v62, v63) | min_8 & ~__PAIR64__(v62, v63)) >> 32)))
                    - *((float *)&p4_8 + 1);
        if ( (float)((float)((float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v65 | min & ~v65))
                                                                                             + (float)(*((float *)&p4 + 1)
                                                                                                             * COERCE_FLOAT((max & v87 | min & (unsigned __int64)~v87) >> 32)))
                                                                             + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v63 | min_8 & ~v63)))
                                                             + (float)(*((float *)&p4_8 + 1)
                                                                             * COERCE_FLOAT((max_8 & __PAIR64__(v62, v63) | min_8 & ~__PAIR64__(v62, v63)) >> 32)))
                                             - *((float *)&p4_8 + 1))
                             - (float)((float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v61 | min & ~v61))
                                                                                             + (float)(*((float *)&p5 + 1)
                                                                                                             * COERCE_FLOAT((max & v86 | min & (unsigned __int64)~v86) >> 32)))
                                                                             + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v59 | min_8 & ~v59)))
                                                             + (float)(*((float *)&p5_8 + 1)
                                                                             * COERCE_FLOAT(HIDWORD(max_8) & v58 | HIDWORD(min_8) & ~v58)))
                                             - *((float *)&p5_8 + 1))) < 0.0 )
            v54 = (float)((float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(max & v61 | min & ~v61))
                                                                    + (float)(*((float *)&p5 + 1)
                                                                                    * COERCE_FLOAT((max & v86 | min & (unsigned __int64)~v86) >> 32)))
                                                    + (float)(*(float *)&p5_8 * COERCE_FLOAT(max_8 & v59 | min_8 & ~v59)))
                                    + (float)(*((float *)&p5_8 + 1) * COERCE_FLOAT(HIDWORD(max_8) & v58 | HIDWORD(min_8) & ~v58)))
                    - *((float *)&p5_8 + 1);
        else
            v54 = (float)((float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(max & v65 | min & ~v65))
                                                                    + (float)(*((float *)&p4 + 1)
                                                                                    * COERCE_FLOAT((max & v87 | min & (unsigned __int64)~v87) >> 32)))
                                                    + (float)(*(float *)&p4_8 * COERCE_FLOAT(max_8 & v63 | min_8 & ~v63)))
                                    + (float)(*((float *)&p4_8 + 1)
                                                    * COERCE_FLOAT((max_8 & __PAIR64__(v62, v63) | min_8 & ~__PAIR64__(v62, v63)) >> 32)))
                    - *((float *)&p4_8 + 1);
        v85 = (float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(max & v69 | min & ~v69))
                                                + (float)(*((float *)&p3 + 1) * COERCE_FLOAT((max & v88 | min & (unsigned __int64)~v88) >> 32)))
                                + (float)(*(float *)&p3_8 * COERCE_FLOAT(max_8 & v67 | min_8 & ~v67)))
                + (float)(*((float *)&p3_8 + 1)
                                * COERCE_FLOAT((max_8 & __PAIR64__(v66, v67) | min_8 & ~__PAIR64__(v66, v67)) >> 32));
        if ( (float)((float)(v85 - *((float *)&p3_8 + 1)) - v57) < 0.0 )
            v53 = v57;
        else
            v53 = v85 - *((float *)&p3_8 + 1);
        if ( (float)((float)(v85 - *((float *)&p3_8 + 1)) - v56) < 0.0 )
            v52 = v56;
        else
            v52 = v85 - *((float *)&p3_8 + 1);
        if ( (float)((float)(v85 - *((float *)&p3_8 + 1)) - v55) < 0.0 )
            v51 = v55;
        else
            v51 = v85 - *((float *)&p3_8 + 1);
        if ( (float)((float)(v85 - *((float *)&p3_8 + 1)) - v54) < 0.0 )
            v50 = v54;
        else
            v50 = v85 - *((float *)&p3_8 + 1);
        v84 = (float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(max & v73 | min & ~v73))
                                                + (float)(*((float *)&p2 + 1) * COERCE_FLOAT((max & v89 | min & (unsigned __int64)~v89) >> 32)))
                                + (float)(*(float *)&p2_8 * COERCE_FLOAT(max_8 & v71 | min_8 & ~v71)))
                + (float)(*((float *)&p2_8 + 1)
                                * COERCE_FLOAT((max_8 & __PAIR64__(v70, v71) | min_8 & ~__PAIR64__(v70, v71)) >> 32));
        if ( (float)((float)(v84 - *((float *)&p2_8 + 1)) - v53) < 0.0 )
            v49 = v53;
        else
            v49 = v84 - *((float *)&p2_8 + 1);
        if ( (float)((float)(v84 - *((float *)&p2_8 + 1)) - v52) < 0.0 )
            v48 = v52;
        else
            v48 = v84 - *((float *)&p2_8 + 1);
        if ( (float)((float)(v84 - *((float *)&p2_8 + 1)) - v51) < 0.0 )
            v47 = v51;
        else
            v47 = v84 - *((float *)&p2_8 + 1);
        if ( (float)((float)(v84 - *((float *)&p2_8 + 1)) - v50) < 0.0 )
            v46 = v50;
        else
            v46 = v84 - *((float *)&p2_8 + 1);
        v83 = (float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(max & v77 | min & ~v77))
                                                + (float)(*((float *)&p1 + 1) * COERCE_FLOAT((max & v90 | min & (unsigned __int64)~v90) >> 32)))
                                + (float)(*(float *)&p1_8 * COERCE_FLOAT(max_8 & v75 | min_8 & ~v75)))
                + (float)(*((float *)&p1_8 + 1)
                                * COERCE_FLOAT((max_8 & __PAIR64__(v74, v75) | min_8 & ~__PAIR64__(v74, v75)) >> 32));
        if ( (float)((float)(v83 - *((float *)&p1_8 + 1)) - v49) < 0.0 )
            v45 = v49;
        else
            v45 = v83 - *((float *)&p1_8 + 1);
        if ( (float)((float)(v83 - *((float *)&p1_8 + 1)) - v48) < 0.0 )
            v44 = v48;
        else
            v44 = v83 - *((float *)&p1_8 + 1);
        if ( (float)((float)(v83 - *((float *)&p1_8 + 1)) - v47) < 0.0 )
            v43 = v47;
        else
            v43 = v83 - *((float *)&p1_8 + 1);
        if ( (float)((float)(v83 - *((float *)&p1_8 + 1)) - v46) < 0.0 )
            v42 = v46;
        else
            v42 = v83 - *((float *)&p1_8 + 1);
        if ( v45 < 0.0 || v44 < 0.0 || v43 < 0.0 || v42 < 0.0 )
        {
            *numOccludersPtr = 0;
            return 0;
        }
        v82 = (float)((float)((float)(*(float *)&p5 * COERCE_FLOAT(min & v61 | max & ~v61))
                                                + (float)(*((float *)&p5 + 1) * COERCE_FLOAT((min & v86 | max & (unsigned __int64)~v86) >> 32)))
                                + (float)(*(float *)&p5_8 * COERCE_FLOAT(min_8 & v59 | max_8 & ~v59)))
                + (float)(*((float *)&p5_8 + 1)
                                * COERCE_FLOAT((min_8 & __PAIR64__(v58, v59) | max_8 & ~__PAIR64__(v58, v59)) >> 32));
        v81 = (float)((float)((float)(*(float *)&p4 * COERCE_FLOAT(min & v65 | max & ~v65))
                                                + (float)(*((float *)&p4 + 1) * COERCE_FLOAT((min & v87 | max & (unsigned __int64)~v87) >> 32)))
                                + (float)(*(float *)&p4_8 * COERCE_FLOAT(min_8 & v63 | max_8 & ~v63)))
                + (float)(*((float *)&p4_8 + 1)
                                * COERCE_FLOAT((min_8 & __PAIR64__(v62, v63) | max_8 & ~__PAIR64__(v62, v63)) >> 32));
        if ( (float)((float)(v81 - *((float *)&p4_8 + 1)) - (float)(v82 - *((float *)&p5_8 + 1))) < 0.0 )
            v40 = v82 - *((float *)&p5_8 + 1);
        else
            v40 = v81 - *((float *)&p4_8 + 1);
        if ( (float)((float)(v81 - *((float *)&p4_8 + 1)) - (float)(v82 - *((float *)&p5_8 + 1))) < 0.0 )
            v39 = v82 - *((float *)&p5_8 + 1);
        else
            v39 = v81 - *((float *)&p4_8 + 1);
        if ( (float)((float)(v81 - *((float *)&p4_8 + 1)) - (float)(v82 - *((float *)&p5_8 + 1))) < 0.0 )
            v38 = v82 - *((float *)&p5_8 + 1);
        else
            v38 = v81 - *((float *)&p4_8 + 1);
        if ( (float)((float)(v81 - *((float *)&p4_8 + 1)) - (float)(v82 - *((float *)&p5_8 + 1))) < 0.0 )
            v37 = v82 - *((float *)&p5_8 + 1);
        else
            v37 = v81 - *((float *)&p4_8 + 1);
        v80 = (float)((float)((float)(*(float *)&p3 * COERCE_FLOAT(min & v69 | max & ~v69))
                                                + (float)(*((float *)&p3 + 1) * COERCE_FLOAT((min & v88 | max & (unsigned __int64)~v88) >> 32)))
                                + (float)(*(float *)&p3_8 * COERCE_FLOAT(min_8 & v67 | max_8 & ~v67)))
                + (float)(*((float *)&p3_8 + 1)
                                * COERCE_FLOAT((min_8 & __PAIR64__(v66, v67) | max_8 & ~__PAIR64__(v66, v67)) >> 32));
        if ( (float)((float)(v80 - *((float *)&p3_8 + 1)) - v40) < 0.0 )
            v36 = v40;
        else
            v36 = v80 - *((float *)&p3_8 + 1);
        if ( (float)((float)(v80 - *((float *)&p3_8 + 1)) - v39) < 0.0 )
            v35 = v39;
        else
            v35 = v80 - *((float *)&p3_8 + 1);
        if ( (float)((float)(v80 - *((float *)&p3_8 + 1)) - v38) < 0.0 )
            v34 = v38;
        else
            v34 = v80 - *((float *)&p3_8 + 1);
        if ( (float)((float)(v80 - *((float *)&p3_8 + 1)) - v37) < 0.0 )
            v33 = v37;
        else
            v33 = v80 - *((float *)&p3_8 + 1);
        v79 = (float)((float)((float)(*(float *)&p2 * COERCE_FLOAT(min & v73 | max & ~v73))
                                                + (float)(*((float *)&p2 + 1) * COERCE_FLOAT((min & v89 | max & (unsigned __int64)~v89) >> 32)))
                                + (float)(*(float *)&p2_8 * COERCE_FLOAT(min_8 & v71 | max_8 & ~v71)))
                + (float)(*((float *)&p2_8 + 1)
                                * COERCE_FLOAT((min_8 & __PAIR64__(v70, v71) | max_8 & ~__PAIR64__(v70, v71)) >> 32));
        if ( (float)((float)(v79 - *((float *)&p2_8 + 1)) - v36) < 0.0 )
            v32 = v36;
        else
            v32 = v79 - *((float *)&p2_8 + 1);
        if ( (float)((float)(v79 - *((float *)&p2_8 + 1)) - v35) < 0.0 )
            v31 = v35;
        else
            v31 = v79 - *((float *)&p2_8 + 1);
        if ( (float)((float)(v79 - *((float *)&p2_8 + 1)) - v34) < 0.0 )
            v30 = v34;
        else
            v30 = v79 - *((float *)&p2_8 + 1);
        if ( (float)((float)(v79 - *((float *)&p2_8 + 1)) - v33) < 0.0 )
            v29 = v33;
        else
            v29 = v79 - *((float *)&p2_8 + 1);
        v78 = (float)((float)((float)(*(float *)&p1 * COERCE_FLOAT(min & v77 | max & ~v77))
                                                + (float)(*((float *)&p1 + 1) * COERCE_FLOAT((min & v90 | max & (unsigned __int64)~v90) >> 32)))
                                + (float)(*(float *)&p1_8 * COERCE_FLOAT(min_8 & v75 | max_8 & ~v75)))
                + (float)(*((float *)&p1_8 + 1)
                                * COERCE_FLOAT((min_8 & __PAIR64__(v74, v75) | max_8 & ~__PAIR64__(v74, v75)) >> 32));
        if ( (float)((float)(v78 - *((float *)&p1_8 + 1)) - v32) < 0.0 )
            v28 = v32;
        else
            v28 = v78 - *((float *)&p1_8 + 1);
        if ( (float)((float)(v78 - *((float *)&p1_8 + 1)) - v31) < 0.0 )
            v27 = v31;
        else
            v27 = v78 - *((float *)&p1_8 + 1);
        if ( (float)((float)(v78 - *((float *)&p1_8 + 1)) - v30) < 0.0 )
            v26 = v30;
        else
            v26 = v78 - *((float *)&p1_8 + 1);
        if ( (float)((float)(v78 - *((float *)&p1_8 + 1)) - v29) < 0.0 )
            v25 = v29;
        else
            v25 = v78 - *((float *)&p1_8 + 1);
        if ( v28 >= 0.0 && v27 >= 0.0 && v26 >= 0.0 && v25 >= 0.0 )
        {
            idx = --*numOccludersPtr;
            v9 = (__int64 *)&(*plane)[20 * *numOccludersPtr];
            t1 = *v9;
            t1_8 = v9[1];
            v10 = (__int64 *)&(*plane)[20 * *numOccludersPtr + 4];
            t2 = *v10;
            t2_8 = v10[1];
            v11 = (__int64 *)&(*plane)[20 * *numOccludersPtr + 8];
            t3 = *v11;
            t3_8 = v11[1];
            v12 = (__int64 *)&(*plane)[20 * *numOccludersPtr + 12];
            t4 = *v12;
            t4_8 = v12[1];
            v13 = (__int64 *)&(*plane)[20 * *numOccludersPtr + 16];
            t5 = *v13;
            t5_8 = v13[1];
            v14 = (uint64_t*)&(*plane)[20 * *numOccludersPtr];
            *v14 = p1;
            v14[1] = p1_8;
            v15 = (uint64_t *)&(*plane)[20 * idx + 4];
            *v15 = p2;
            v15[1] = p2_8;
            v16 = (uint64_t *)&(*plane)[20 * idx + 8];
            *v16 = p3;
            v16[1] = p3_8;
            v17 = (uint64_t *)&(*plane)[20 * idx + 12];
            *v17 = p4;
            v17[1] = p4_8;
            v18 = (uint64_t *)&(*plane)[20 * idx + 16];
            *v18 = p5;
            v18[1] = p5_8;
            v19 = (uint64_t *)&(*plane)[20 * planeIdx];
            *v19 = t1;
            v19[1] = t1_8;
            v20 = (uint64_t *)&(*plane)[20 * planeIdx + 4];
            *v20 = t2;
            v20[1] = t2_8;
            v21 = (uint64_t *)&(*plane)[20 * planeIdx + 8];
            *v21 = t3;
            v21[1] = t3_8;
            v22 = (uint64_t *)&(*plane)[20 * planeIdx + 12];
            *v22 = t4;
            v22[1] = t4_8;
            v23 = (uint64_t *)&(*plane)[20 * planeIdx + 16];
            *v23 = t5;
            v23[1] = t5_8;
        }
    }
    return plane;
}

bool __cdecl R_CalcSurfaceNoDynamicShadow(unsigned int bits, GfxSurface *localSurfaces, GfxDrawSurf *localDrawSurfaces)
{
    unsigned int v3; // ecx
    GfxDrawSurf *surfaceMaterials; // [esp+1Ch] [ebp-30h]
    unsigned int bit; // [esp+30h] [ebp-1Ch]
    unsigned int indexLow; // [esp+34h] [ebp-18h]
    bool index0; // [esp+3Ch] [ebp-10h]
    bool index2; // [esp+40h] [ebp-Ch]
    bool index1; // [esp+48h] [ebp-4h]

    index0 = dpvsGlob.sunShadow.viewDir[0] < 0.0;
    index1 = dpvsGlob.sunShadow.viewDir[1] < 0.0;
    index2 = dpvsGlob.sunShadow.viewDir[2] < 0.0;
    indexLow = 0;
    bit = 0x80000000;
    while ( bits )
    {
        if ( (bits & bit) != 0 )
        {
            bits &= ~bit;
            surfaceMaterials = &localDrawSurfaces[indexLow];
            v3 = (((float)((float)((float)((float)(localSurfaces[indexLow].bounds[index0][0] * dpvsGlob.sunShadow.viewDir[0])
                                                                     + (float)(localSurfaces[indexLow].bounds[index1][1] * dpvsGlob.sunShadow.viewDir[1]))
                                                     + (float)(localSurfaces[indexLow].bounds[index2][2] * dpvsGlob.sunShadow.viewDir[2]))
                                     - dpvsGlob.sunShadow.viewDirDist) > dpvsGlob.sunShadow.sunShadowDrawDist) << 25)
                 | HIDWORD(surfaceMaterials->packed) & 0xFDFFFFFF;
            //*(unsigned int *)&surfaceMaterials->fields = surfaceMaterials->fields;
            surfaceMaterials->packed = surfaceMaterials->packed;
            HIDWORD(surfaceMaterials->packed) = v3;
        }
        ++indexLow;
        bit >>= 1;
    }
    return 0;
}

void __cdecl R_AddCellStaticSurfacesInFrustumCmd(DpvsStaticCellCmd *data)
{
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    viewIndex = data->viewIndex;
    g_smodelVisData = g_worldDpvs->smodelVisData[viewIndex];
    g_surfaceVisData = g_worldDpvs->surfaceVisData[viewIndex];
    R_AddCellStaticSurfacesInFrustum(data);
    if ( rg.drawWorld )
        R_AddCellCullGroupsInFrustum(data);
}

int __cdecl R_AddCellStaticSurfacesInFrustum(DpvsStaticCellCmd *dpvsCell)
{
    unsigned int v2; // edx

    unsigned int v5; // [esp+8h] [ebp-15DCh]
    unsigned int bits; // [esp+10h] [ebp-15D4h]
    unsigned int k; // [esp+14h] [ebp-15D0h]
    int j; // [esp+18h] [ebp-15CCh]
    float planebufStorage[320 * 4 + 4];  // +16 bytes slack to hand-align to 16
    float (*occluderPlanes)[4]; // [esp+146Ch] [ebp-178h]
    const GfxCell *cell; // [esp+1470h] [ebp-174h]
    DpvsClipPlanes out; // [esp+1474h] [ebp-170h] BYREF
    GfxAabbTree *tree; // [esp+15C4h] [ebp-20h]
    unsigned int v14; // [esp+15C8h] [ebp-1Ch]
    DpvsPlanes planes; // [esp+15CCh] [ebp-18h]
    unsigned int v16; // [esp+15D4h] [ebp-10h]
    GfxAabbTree *aabbTree; // [esp+15D8h] [ebp-Ch]
    unsigned int i; // [esp+15DCh] [ebp-8h]
    int v19; // [esp+15E0h] [ebp-4h]

    v19 = 0;
    cell = dpvsCell->cell;
    aabbTree = cell->aabbTree;
    if (!aabbTree)
        return v19;

    planes.count = dpvsCell->planeCount;

    iassert(planes.count <= (10 + 4 + 2));
    iassert(planes.count);

    planes.planes = dpvsCell->planes;
    tree = aabbTree;
    for (i = 0; i < planes.count; ++i)
        R_CopyClipPlanes(&planes.planes[i], &out.planes[i]);

    out.count = planes.count;

    if (dpvsCell->viewIndex)
    {
        v19 = R_AddAabbTreeSurfacesInFrustum_r(tree, &out, 1, 0, dpvsGlob.occluderPlanes);
    }
    else
    {
        // The SSE occluder cull (TestOccludersPartial) asserts the plane buffer is
        // 16-byte aligned, but GCC doesn't honor alignas(16) on this stack local
        // here (no stack realignment in the prologue). Hand-align a pointer into the
        // over-allocated storage instead.
        occluderPlanes = (float (*)[4])(((unsigned)planebufStorage + 15u) & ~15u);
        for (j = 0; j < 5 * dpvsGlob.numOccluders; ++j)
        {
            float *v3 = occluderPlanes[j];
            float *v4 = dpvsGlob.occluderPlanes[j];
            v3[0] = v4[0];
            v3[1] = v4[1];
            v3[2] = v4[2];
            v3[3] = v4[3];
        }
        v19 = R_AddAabbTreeSurfacesInFrustum_r(tree, &out, 1, dpvsGlob.numOccluders, occluderPlanes);
    }
    v16 = g_worldDpvs->staticSurfaceCount >> 5;
    for (k = 0; k < v16; ++k)
    {
        bits = g_surfaceVisData[k];
        if (bits)
            R_CalcSurfaceNoDynamicShadow(bits, &g_worldDpvs->surfaces[32 * k], &g_worldDpvs->surfaceMaterials[32 * k]);
    }
    v14 = g_worldDpvs->staticSurfaceCount % 0x20;
    if (v14)
    {
        v5 = g_worldDpvs->staticSurfaceCount >> 5;
        v2 = g_surfaceVisData[v5] & (-1 << (32 - v14));
        if (v2)
            R_CalcSurfaceNoDynamicShadow(v2, &g_worldDpvs->surfaces[32 * v5], &g_worldDpvs->surfaceMaterials[32 * v5]);
    }
    return v19;
}

int __cdecl R_AddAabbTreeSurfacesInFrustum_r(
    const GfxAabbTree *tree,
    const DpvsClipPlanes *planes,
    int treeDepth,
    int numOccluders,
    float (*occluderPlanes)[4])
{
    int v6; // [esp+8h] [ebp-1C8h]
    int v7; // [esp+Ch] [ebp-1C4h]
    GfxSurface *v8; // [esp+14h] [ebp-1BCh]
    int v9; // [esp+18h] [ebp-1B8h]
    int v10; // [esp+1Ch] [ebp-1B4h]
    GfxStaticModelInst *minmax; // [esp+20h] [ebp-1B0h]
    const GfxAabbTree *children; // [esp+2Ch] [ebp-1A4h]
    unsigned int childIndex; // [esp+30h] [ebp-1A0h]
    unsigned int childCount; // [esp+34h] [ebp-19Ch]
    unsigned int v15; // [esp+38h] [ebp-198h]
    unsigned __int16 *v16; // [esp+3Ch] [ebp-194h]
    unsigned __int16 *smodelIndexes; // [esp+40h] [ebp-190h]
    unsigned int v18; // [esp+44h] [ebp-18Ch]
    unsigned int i; // [esp+48h] [ebp-188h]
    unsigned int smodelChildIndex; // [esp+4Ch] [ebp-184h]
    unsigned int surfaceCount; // [esp+50h] [ebp-180h]
    unsigned __int16 *v22; // [esp+54h] [ebp-17Ch]
    unsigned __int16 *indices; // [esp+58h] [ebp-178h]
    unsigned int smodelIndexCount; // [esp+5Ch] [ebp-174h]
    DpvsClipPlanes clipPlanes; // [esp+60h] [ebp-170h] BYREF
    const DpvsPlane *plane; // [esp+1B0h] [ebp-20h]
    unsigned int planeCount; // [esp+1B4h] [ebp-1Ch]
    unsigned int smodelIndexIter; // [esp+1B8h] [ebp-18h]
    unsigned int surfIndex; // [esp+1BCh] [ebp-14h]
    unsigned int surfNodeIndex; // [esp+1C0h] [ebp-10h]
    unsigned int planeIndex; // [esp+1C4h] [ebp-Ch]
    int finalTreeDepth; // [esp+1C8h] [ebp-8h]
    unsigned int smodelIndex; // [esp+1CCh] [ebp-4h]

    finalTreeDepth = treeDepth;
    if (numOccluders)
    {
        occluderPlanes = TestOccludersPartial((const float (*)[3])tree, &numOccluders, occluderPlanes);
        if (occluderPlanes)
        {
            if (!numOccluders && r_showOccluders->current.integer >= 3)
                R_AddDebugBox(&frontEndDataOut->debugGlobals, tree->mins, tree->maxs, colorCyan);
        }
    }
    if (occluderPlanes)
    {
        clipPlanes.count = 0;
        planeCount = planes->count;
        planeIndex = 0;
        plane = (const DpvsPlane *)planes;
        while (planeIndex < planeCount)
        {
            if (R_DpvsPlaneMaxSignedDistToBox(plane, tree->mins) <= 0.0)
                return finalTreeDepth;
            if (R_DpvsPlaneMinSignedDistToBox(plane, tree->mins) < 0.0)
                clipPlanes.planes[clipPlanes.count++] = *plane;
            ++planeIndex;
            ++plane;
        }
    }
    else
    {
        clipPlanes.count = 0;
    }
    if (occluderPlanes)
    {
        if (clipPlanes.count || numOccluders)
        {
            if (tree->childCount)
            {
                children = (const GfxAabbTree *)((char *)tree + tree->childrenOffset);
                childCount = tree->childCount;
                for (childIndex = 0; childIndex < childCount; ++childIndex)
                    R_AddAabbTreeSurfacesInFrustum_r(&children[childIndex], &clipPlanes, 0, numOccluders, occluderPlanes);
            }
            else
            {
                if (r_showAabbTrees->current.integer)
                    R_AddDebugBox(&frontEndDataOut->debugGlobals, tree->mins, tree->maxs, colorOrange);
                if (rg.drawSModels)
                {
                    v18 = tree->smodelIndexCount;
                    if (tree->smodelIndexCount)
                    {
                        smodelIndexes = tree->smodelIndexes;
                        for (smodelChildIndex = 0; smodelChildIndex < v18; ++smodelChildIndex)
                        {
                            v9 = numOccluders;
                            v10 = smodelIndexes[smodelChildIndex];
                            if (!g_smodelVisData[v10])
                            {
                                minmax = &g_worldDpvs->smodelInsts[v10];
                                if (!R_CullBoxLeaf(minmax->mins, &clipPlanes))
                                {
                                    if (TestOccluders((const float (*)[3])minmax, v9, occluderPlanes))
                                    {
                                        if (r_showOccluders->current.integer >= 2)
                                            R_AddDebugBox(&frontEndDataOut->debugGlobals, minmax->mins, minmax->maxs, colorOrange);
                                    }
                                    else
                                    {
                                        g_smodelVisData[v10] = 1;
                                    }
                                }
                            }
                        }
                    }
                }
                if (rg.drawWorld)
                {
                    v15 = tree->surfaceCount;
                    if (tree->surfaceCount)
                    {
                        v16 = &g_worldDpvs->sortedSurfIndex[tree->startSurfIndex];
                        for (i = 0; i < v15; ++i)
                        {
                            v6 = numOccluders;
                            v7 = v16[i];
                            if (!g_surfaceVisData[v7])
                            {
                                v8 = &g_worldDpvs->surfaces[v7];
                                if (!R_CullBoxLeaf(v8->bounds[0], &clipPlanes))
                                {
                                    if (TestOccluders(v8->bounds, v6, occluderPlanes))
                                    {
                                        if (r_showOccluders->current.integer >= 2)
                                            R_AddDebugBox(&frontEndDataOut->debugGlobals, v8->bounds[0], v8->bounds[1], colorOrange);
                                    }
                                    else
                                    {
                                        if ((r_showAabbTrees->current.integer & 2) != 0)
                                            R_AddDebugBox(&frontEndDataOut->debugGlobals, v8->bounds[0], v8->bounds[1], colorGreen);
                                        g_surfaceVisData[v7] = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (r_showAabbTrees->current.integer)
                R_AddDebugBox(&frontEndDataOut->debugGlobals, tree->mins, tree->maxs, colorYellow);
            if (rg.drawSModels)
            {
                smodelIndexCount = tree->smodelIndexCount;
                if (tree->smodelIndexCount)
                {
                    indices = tree->smodelIndexes;
                    for (smodelIndexIter = 0; smodelIndexIter < smodelIndexCount; ++smodelIndexIter)
                    {
                        smodelIndex = indices[smodelIndexIter];
                        g_smodelVisData[smodelIndex] = 1;
                    }
                }
            }
            if (rg.drawWorld)
            {
                surfaceCount = tree->surfaceCount;
                if (tree->surfaceCount)
                {
                    v22 = &g_worldDpvs->sortedSurfIndex[tree->startSurfIndex];
                    for (surfNodeIndex = 0; surfNodeIndex < surfaceCount; ++surfNodeIndex)
                    {
                        surfIndex = v22[surfNodeIndex];
                        g_surfaceVisData[surfIndex] = 1;
                    }
                }
            }
        }
    }
    else if (r_showOccluders->current.integer >= 2)
    {
        R_AddDebugBox(&frontEndDataOut->debugGlobals, tree->mins, tree->maxs, colorBlue);
    }
    return finalTreeDepth;
}

double __cdecl R_DpvsPlaneMinSignedDistToBox(const DpvsPlane *plane, const float *minmax)
{
    return (float)((float)(*(const float *)((char *)minmax - plane->side[2] + 28) * plane->coeffs[2])
                             + (float)((float)(*(const float *)((char *)minmax - plane->side[1] + 20) * plane->coeffs[1])
                                             + (float)((float)(*(const float *)((char *)minmax - plane->side[0] + 12) * plane->coeffs[0])
                                                             + plane->coeffs[3])));
}

int __cdecl R_CullBoxLeaf(const float *minmax, DpvsClipPlanes *planes)
{
    DpvsClipPlanes *plane; // [esp+8h] [ebp-8h]
    unsigned int planeIndex; // [esp+Ch] [ebp-4h]

    planeIndex = 0;
    plane = planes;
    while ( planeIndex < planes->count )
    {
        if ( R_DpvsPlaneMaxSignedDistToBox(plane->planes, minmax) <= 0.0 )
            return 1;
        ++planeIndex;
        plane = (DpvsClipPlanes *)((char *)plane + 20);
    }
    return 0;
}

void __cdecl R_CopyClipPlanes(const DpvsPlane *in, DpvsPlane *out)
{
    *out = *in;
}

void __cdecl R_AddCellCullGroupsInFrustum(DpvsStaticCellCmd *dpvsCell)
{
    int v1; // [esp+4h] [ebp-3Ch]
    DpvsPlane *plane; // [esp+Ch] [ebp-34h]
    int v3; // [esp+10h] [ebp-30h]
    const unsigned __int16 *indices; // [esp+1Ch] [ebp-24h]
    GfxCullGroup *group; // [esp+24h] [ebp-1Ch]
    int count; // [esp+28h] [ebp-18h]
    const GfxCell *cell; // [esp+2Ch] [ebp-14h]
    int planeCount; // [esp+30h] [ebp-10h]
    const DpvsPlane *planes; // [esp+34h] [ebp-Ch]
    int groupCount; // [esp+38h] [ebp-8h]
    const int *cullGroup; // [esp+3Ch] [ebp-4h]

    cell = dpvsCell->cell;
    planes = dpvsCell->planes;
    planeCount = dpvsCell->planeCount;
    groupCount = cell->cullGroupCount;
    for (cullGroup = cell->cullGroups; groupCount; ++cullGroup)
    {
        group = &g_worldDpvs->cullGroups[*cullGroup];
        v3 = 0;
        plane = (DpvsPlane *)planes;
        while (v3 < planeCount)
        {
            if (R_DpvsPlaneMaxSignedDistToBox(plane, group->mins) <= 0.0)
            {
                v1 = 1;
                goto LABEL_9;
            }
            ++v3;
            ++plane;
        }
        v1 = 0;
    LABEL_9:
        if (v1)
            break;
        if ((r_showPortals->current.integer & 1) != 0)
            R_AddDebugBox(&frontEndDataOut->debugGlobals, group->mins, group->maxs, colorLtYellow);
        if (!group->surfaceCount)
            break;
        indices = &g_worldDpvs->sortedSurfIndex[group->startSurfIndex];
        for (count = 0; count < group->surfaceCount; ++count)
            g_surfaceVisData[indices[count]] = 1;
        --groupCount;
    }
}

void __cdecl R_AddSkySurfacesDpvs(const DpvsPlane *planes, int planeCount)
{
    int numOccluders; // [esp+0h] [ebp-168h]
    int v3; // [esp+4h] [ebp-164h]
    GfxSurface *v4; // [esp+Ch] [ebp-15Ch]
    DpvsClipPlanes clipPlanes; // [esp+10h] [ebp-158h] BYREF
    int surfIndex; // [esp+160h] [ebp-8h]
    int planeIndex; // [esp+164h] [ebp-4h]

    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_static.cpp",
            1630,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    g_smodelVisData = rgp.world->dpvs.smodelVisData[0];
    g_surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
        R_CopyClipPlanes(&planes[planeIndex], &clipPlanes.planes[planeIndex]);
    clipPlanes.count = planeCount;
    for (surfIndex = 0; surfIndex < rgp.world->skySurfCount; ++surfIndex)
    {
        numOccluders = dpvsGlob.numOccluders;
        v3 = rgp.world->skyStartSurfs[surfIndex];
        if (!g_surfaceVisData[v3])
        {
            v4 = &g_worldDpvs->surfaces[v3];
            if (!R_CullBoxLeaf(v4->bounds[0], &clipPlanes))
            {
                if (TestOccluders(v4->bounds, numOccluders, dpvsGlob.occluderPlanes))
                {
                    if (r_showOccluders->current.integer >= 2)
                        R_AddDebugBox(&frontEndDataOut->debugGlobals, v4->bounds[0], v4->bounds[1], colorOrange);
                }
                else
                {
                    if ((r_showAabbTrees->current.integer & 2) != 0)
                        R_AddDebugBox(&frontEndDataOut->debugGlobals, v4->bounds[0], v4->bounds[1], colorGreen);
                    g_surfaceVisData[v3] = 1;
                }
            }
        }
    }
}
