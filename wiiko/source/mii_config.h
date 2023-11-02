#ifndef MII_CONFIG_H
#define MII_CONFIG_H

#include <mii.h>
#define MAX_MIIS 2

const Mii miis[2] = {
    {
        .female = 0,
        .favColor = 6,
        .name = "FISH",
        .height = 25,
        .weight = 9,
        .faceShape = 0,
        .skinColor = 1,
        .facialFeature = 11,
        .hairType = 30,
        .hairColor = 1,
        .hairPart = 0,
        .eyebrowType = 8,
        .eyebrowRotation = 3,
        .eyebrowColor = 7,
        .eyebrowSize = 8,
        .eyebrowVertPos = 7,
        .eyebrowHorizSpacing = 12,
        .eyeType = 3,
        .eyeRotation = 4,
        .eyeVertPos = 1,
        .eyeColor = 0,
        .eyeSize = 4,
        .eyeHorizSpacing = 3,
        .noseType = 2,
        .noseSize = 0,
        .noseVertPos = 3,
        .lipType = 13,
        .lipColor = 0,
        .lipSize = 4,
        .lipVertPos = 2,
        .mustacheType = 3,
        .facialHairColor = 7,
        .mustacheSize = 8,
        .mustacheVertPos = 16
    },
    {
        .female = 1,
        .favColor = 10,
        .name = "JESUS",
        .height = 127,
        .weight = 0,
        .faceShape = 0,
        .skinColor = 3,
        .facialFeature = 9,
        .hairType = 15,
        .hairColor = 1,
        .hairPart = 0,
        .eyebrowType = 12,
        .eyebrowRotation = 5,
        .eyebrowColor = 1,
        .eyebrowSize = 4,
        .eyebrowVertPos = 11,
        .eyebrowHorizSpacing = 2,
        .eyeType = 32,
        .eyeRotation = 4,
        .eyeVertPos = 12,
        .eyeColor = 0,
        .eyeSize = 3,
        .eyeHorizSpacing = 3,
        .noseType = 6,
        .noseSize = 3,
        .noseVertPos = 10,
        .lipType = 15,
        .lipColor = 0,
        .lipSize = 2,
        .lipVertPos = 14,
        .mustacheType = 3,
        .beardType = 3,
        .facialHairColor = 1,
        .mustacheSize = 4,
        .mustacheVertPos = 11
    }
};

#endif