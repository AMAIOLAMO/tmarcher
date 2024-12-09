#pragma once
#ifndef _CAMERA_H
#define _CAMERA_H

#include "cxvec.h"

struct Camera {
    Vec3f pos;
    Vec3f look_at;
    float zoom;

    void move(Vec3f offset);

    // calculates the screen frustrum to be able to reflect our rays out
    Vec3f screen_center() const;

    // calculate the spacial basis vectors of the camera
    Vec3f forward() const;

    Vec3f right() const;
    Vec3f up() const;
};


#endif // _CAMERA_H
