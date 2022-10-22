/**
 *
 * @param retArray 返回数组
 * @param pArray 传入数组
 * @param arrLen 数组大小
 * @param retMax 归一化上界
 * @param retMin 归一化下节
 */
void array_normalize(int16_t retArray[],int16_t inArray[],uint16_t arrLen,int16_t retMax,int16_t retMin){
    int16_t  arrMax,arrMin;
    arrMax = arrMin = inArray[0];
    for(uint16_t i = 0; i < arrLen; ++i){
        if(arrMax < inArray[i]) arrMax = inArray[i];
        if(arrMin > inArray[i]) arrMin = inArray[i];
    }
    double p = 1.0 * (retMax - retMin)/(arrMax - arrMin);
    for(uint16_t i = 0; i < arrLen; ++i){
        retArray[i] = lround(p * inArray[i] + retMin);
    }
}


void array_normalize_self(int16_t Array[],uint16_t arrLen,int16_t retMax,int16_t retMin){
    int16_t  arrMax,arrMin;
    arrMax = arrMin = Array[0];
    for(uint16_t i = 0; i < arrLen; ++i){
        if(arrMax < Array[i]) arrMax = Array[i];
        if(arrMin > Array[i]) arrMin = Array[i];
    }
    double p = 1.0 * (retMax - retMin)/(arrMax - arrMin);
    for(uint16_t i = 0; i < arrLen; ++i){
        Array[i] = lround(p * Array[i] + retMin);
    }
}


void array_generate_sin_waveform(int16_t *retArray,uint16_t arrLen,uint16_t NumPerCycle,int16_t retMax,int16_t retMin){
    double PI = 3.1416;
    double step = 2 * PI / NumPerCycle;
    uint16_t refer = (retMax - retMin) / 2 + retMin;
    for (uint16_t i = 0; i < arrLen; ++i) {
        retArray[i] = sin(i * step) / 2 * (retMax - retMin) + refer;
    }
}