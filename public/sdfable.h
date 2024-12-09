#pragma once
#ifndef _SDFABLE_H
#define _SDFABLE_H

#include "cxvec.h"

// global uniform time to be used everywhere to calculate time based rendering
class SDFable {
public:
    virtual float dist(Vec3f pt) const = 0;
};

#endif // _SDFABLE_H
