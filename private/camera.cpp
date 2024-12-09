#include "../public/camera.h"

void Camera::move(Vec3f offset) {
    pos += offset;
    look_at += offset;
}

// calculates the screen frustrum to be able to reflect our rays out
Vec3f Camera::screen_center() const {
    return pos + forward() * zoom;
}

// calculate the spacial basis vectors of the camera utilizing basic vector analysis
Vec3f Camera::forward() const {
    return Vec3f_norm(look_at - pos);
}

Vec3f Camera::right() const {
    return Vec3f_norm(Vec3f_cross(Vec3f{0, 1, 0}, forward()));
}

Vec3f Camera::up() const {
    return Vec3f_norm(Vec3f_cross(forward(), right()));
}
