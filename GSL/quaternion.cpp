#include "quaternion.h"
#include "vector4d.h"
#include "matrix4x4.h"
#include <cmath>

gsl::Quaternion::Quaternion(GLfloat sIn, GLfloat iIn, GLfloat jIn, GLfloat kIn)
    : s{sIn}, i{iIn}, j{jIn}, k{kIn}
{

}

gsl::Quaternion::Quaternion(GLfloat scalar, const gsl::vec3 &v)
    : s{scalar}, i{v.x}, j{v.y}, k{v.z}
{

}

gsl::Quaternion::Quaternion(const gsl::Quaternion::Pair &pair)
    : s{pair.s}, i{pair.v.x}, j{pair.v.y}, k{pair.v.z}
{

}

gsl::Quaternion::Pair::Pair(GLfloat scalar, const gsl::vec3 &vector)
    : s{scalar}, v{vector}
{

}

namespace gsl {
gsl::quat gsl::Quaternion::operator+(const gsl::quat &quat) const
{
    return gsl::quat{s + quat.s, i + quat.i, j + quat.j, k + quat.k};
}
}

gsl::quat gsl::Quaternion::operator+(GLfloat scalar) const
{
    return gsl::quat{s + scalar, i, j, k};
}

gsl::quat operator+(GLfloat scalar, const gsl::quat &quat)
{
    return gsl::quat{scalar + quat.s, quat.i, quat.j, quat.k};
}

namespace gsl {
gsl::quat gsl::Quaternion::operator-(const gsl::quat &quat) const
{
    return gsl::quat{s - quat.s, i - quat.i, j - quat.j, k - quat.k};
}
}

gsl::quat gsl::Quaternion::operator-(GLfloat scalar) const
{
    return gsl::quat{s - scalar, i, j, k};
}

gsl::quat operator-(GLfloat scalar, const gsl::quat &quat)
{
    return gsl::quat{scalar - quat.s, quat.i, quat.j, quat.k};
}

gsl::quat gsl::Quaternion::operator*(const gsl::quat &quat) const
{
//    auto v1 = toPair();
//    auto v2 = toPair();
//    return gsl::quat::Pair{v1.s * v2.s - v1.v * v2.v, v1.s * v2.v + v2.s * v1.v + v1.v ^ v2.v};

    // Unsure whether or not this is even faster or if it even matters at all
    return gsl::quat{
        s * quat.s - i * quat.i - j * quat.j - k * quat.k,
        s * quat.i + i * quat.s + j * quat.k - k * quat.j,
        s * quat.j - i * quat.k + j * quat.s + k * quat.i,
        s * quat.k + i * quat.j - j * quat.i + k * quat.s
    };
}

gsl::quat gsl::Quaternion::operator*(GLfloat scalar) const
{
    return gsl::quat{s * scalar, s * i, s * j, s * k};
}

namespace gsl {
gsl::quat operator*(GLfloat scalar, const gsl::quat& quat)
{
    return gsl::quat{scalar * quat.s, scalar * quat.i, scalar * quat.j, scalar * quat.k};
}
}

gsl::quat gsl::Quaternion::rot(GLfloat angle, const gsl::vec3 &axis) const
{
    return gsl::quat{std::cos(angle) / 2, axis * (std::sin(angle) / 2)};
}

gsl::vec3 gsl::Quaternion::rotatePoint(const gsl::vec3 &p, const gsl::quat &rot)
{
    auto Pq = rot * gsl::quat{0, p} * rot.inverse();
    return gsl::vec3{Pq.i, Pq.j, Pq.k};
}

gsl::vec3 gsl::Quaternion::rotatePointUnit(const gsl::vec3 &p, const gsl::quat &rot)
{
    // For unit vectors the inverse of a quaternion is
    // q^-1 = (s, -v)
    auto Pq = rot * gsl::quat{0, p} * gsl::quat{rot.s, -rot.i, -rot.j, -rot.k};
    return gsl::vec3{Pq.i, Pq.j, Pq.k};

    // Alernative:
//    auto qp = rot.toPair();
//    return (qp.s * qp.s) * p + qp.v * (p * qp.v) + 2.f * qp.s * (qp.v ^ p) + qp.v ^ (qp.v ^ p);
}

gsl::mat4 gsl::Quaternion::toMat() const
{
    return gsl::mat4{
        1.f - 2.f * j * j - 2.f * k * k,    2.f * i * j - 2.f * s * k,      2.f * i * k + 2.f * s * j,      0.f,
        2.f * i * j + 2.f * s * k,      1.f - 2.f * i * i - 2.f * k * k,    2.f * j * k - 2.f * s * i,      0.f,
        2.f * i * k - 2.f * s * j,      2.f * j * k + 2.f * s * i,      1.f - 2.f * i * i - 2.f * j * j,    0.f,
        0.f,                            0.f,                                0.f,                            1.f
    };
}

gsl::quat gsl::Quaternion::conj() const
{
    return gsl::quat{s, -i, -j, -k};
}

GLfloat gsl::Quaternion::sizeSqrd() const
{
    auto qp = toPair();
    return qp.s * qp.s + qp.v * qp.v;
}

gsl::quat gsl::Quaternion::inverse() const
{
    return (1.f / sizeSqrd()) * gsl::quat{s, -i, -j, -k};
}

gsl::vec4 gsl::Quaternion::toVec4() const
{
    return gsl::vec4{s, i, j, k};
}

gsl::Quaternion::Pair gsl::Quaternion::toPair() const
{
    return gsl::quat::Pair{s, {i, j, k}};
}