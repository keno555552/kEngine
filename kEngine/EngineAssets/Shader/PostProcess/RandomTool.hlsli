#include "whiteNoise.hlsli"

float whiteNoise(float2 texcoord, float time)
{
    float random = rand2dTo1d(texcoord * time);
    return random;
}

/// Type of Random Noise
// 0: None
// 1: white noise(Ran2dTo1d)
// 

float getNoise(float noiseType, float2 texcoord, float noiseTime)
{
    if (noiseType == 1)
        return whiteNoise(texcoord, noiseTime == 0 ? 10 : noiseTime);
    //if (noiseType == 1)
    //    return hashNoise(uv);
    //if (noiseType == 2)
    //    return gridNoise(uv);
    //if (noiseType == 3)
    //    return fakePerlin(uv);
    //if (noiseType == 4)
    //    return tvStatic(uv, time);

    return 0.0;
}
