#include "ParticlePrototype.h"

void CopyPPbyActiveData(ParticlePrototype& dest, const ParticlePrototypeOverride& src) {
    // --- 基本生命サイクル ---
    if (src.hasLifetime) dest.lifetime = src.lifetime;
    if (src.hasLifetimeRandomness) dest.lifetimeRandomness = src.lifetimeRandomness;

    /// --- 軌跡（Part0 も Part1 以降も同じく上書き） ---
    auto applyTrack = [&](ParticlePartTrack& t) {
        // --- 初始位置 ---
        if (src.hasStartPosition)       t.startPosition = src.startPosition;
        if (src.hasStartPositionRandom) t.startPositionRandom = src.startPositionRandom;
		// --- 初速 ---
        if (src.hasStartVelocity)       t.startVelocity = src.startVelocity;
        if (src.hasStartVelocityRandom) t.startVelocityRandom = src.startVelocityRandom;
        if (src.hasStartSpeedRandom)    t.startSpeedRandom = src.startSpeedRandom;
		// --- 大小 ---
        if (src.hasStartScale)          t.startScale = src.startScale;
        if (src.hasEndScale)            t.endScale = src.endScale;
        if (src.hasStartScaleRandom)    t.startScaleRandom = src.startScaleRandom;
        if (src.hasEndScaleRandom)      t.endScaleRandom = src.endScaleRandom;
        if (src.hasIsConstantScale)     t.isConstantScale = src.isConstantScale;
		// --- 旋轉 ---
        if (src.hasStartRotation)       t.startRotation = src.startRotation;
        if (src.hasEndRotation)         t.endRotation = src.endRotation;
        if (src.hasStartRotationRandom) t.startRotationRandom = src.startRotationRandom;
        if (src.hasEndRotationRandom)   t.endRotationRandom = src.endRotationRandom;
        if (src.hasIsConstantRotation)  t.isConstantRotation = src.isConstantRotation;
		// --- UV ---
        if (src.hasUVTranslateStart)    t.uvTranslateStart = src.uvTranslateStart;
        if (src.hasUVTranslateEnd)      t.uvTranslateEnd = src.uvTranslateEnd;
        if (src.hasUVRotateStart)       t.uvRotateStart = src.uvRotateStart;
        if (src.hasUVRotateEnd)         t.uvRotateEnd = src.uvRotateEnd;
		if (src.hasUVScaleStart)        t.uvScaleStart = src.uvScaleStart;
		if (src.hasUVScaleEnd)          t.uvScaleEnd = src.uvScaleEnd;
        };
    applyTrack(dest.mainTrack);
    for (auto& t : dest.partTracks) applyTrack(t);

    // --- 顏色 ---
    if (src.hasStartColor)              dest.startColor = src.startColor;
    if (src.hasEndColor)                dest.endColor = src.endColor;
    if (src.hasStartColorRandom)        dest.startColorRandom = src.startColorRandom;
    if (src.hasEndColorRandom)          dest.endColorRandom = src.endColorRandom;
    if (src.hasIsConstantColor)         dest.isConstantColor = src.isConstantColor;

    // --- 行為 ---
    if (src.hasGravity)                 dest.gravity = src.gravity;
    if (src.hasDamping)                 dest.damping = src.damping;
    if (src.hasDampingRandom)           dest.dampingRandom = src.dampingRandom;

    // --- 發射設定 ---
    if (src.hasEmitRate)                dest.emitRate = src.emitRate;
    if (src.hasEmitIntervalRandom)      dest.emitIntervalRandom = src.emitIntervalRandom;
    if (src.hasBurstCount)              dest.burstCount = src.burstCount;
    if (src.hasEmitNumRandom)           dest.emitNumRandom = src.emitNumRandom;

    // --- 外觀 ---
    if (src.hasRenderType)              dest.renderType = src.renderType;
    if (src.hasObjectList)              dest.objectList = src.objectList;
    if (src.hasSpriteList)              dest.spriteList = src.spriteList;

    // --- 路徑 ---
    if (src.hasModelPath)               dest.modelPath = src.modelPath;
    if (src.hasTexturePath)             dest.texturePath = src.texturePath;
}
