#ifndef VECTOR2D_H
#define VECTOR2D_H

#include "gltypes.h"
#include "mathfwd.h"
#include <cmath>
#include <iostream>
#include <QJsonArray>

namespace gsl
{

/** A mathematical 2 dimensional vector.
 * @brief A mathematical 2 dimensional vector.
 */
class Vector2D
{
public:
    //Constructors
    Vector2D(GLfloat x_in = 0.f, GLfloat y_in = 0.f);
    Vector2D(const int v);
    Vector2D(const double v);
    Vector2D(const gsl::ivec2& v);

    //Operators
    Vector2D operator+(const Vector2D &rhs) const;      // v + v
    Vector2D operator-(const Vector2D &rhs) const;      // v - v
    Vector2D& operator+=(const Vector2D &rhs);          // v += v
    Vector2D& operator-=(const Vector2D &rhs);          // v -= v
    Vector2D operator-() const;                     // -v
    Vector2D operator*(GLfloat lhs) const;          // v * f
    const Vector2D& operator =(const Vector2D &rhs);    // v = v

    //Functions
    GLfloat length() const;
    void normalize();
    Vector2D normalized();
    static GLfloat cross(const Vector2D &v1, const Vector2D &v2);
    static GLfloat dot(const Vector2D &v1, const Vector2D &v2);

    //Getters and setters
    GLfloat getX() const;
    void setX(const GLfloat &value);

    GLfloat getY() const;
    void setY(const GLfloat &value);

    QJsonArray toJSON();
    void fromJSON(const QJsonArray &array);

    GLfloat* data() { return &x; }

    //Friend functions
    friend std::ostream& operator<<(std::ostream &output, const Vector2D &rhs)
    {
        output << "(" << rhs.x << "," << rhs.y << ")";
        return output;
    }

    GLfloat x;
    GLfloat y;
};





/** Integer vector2.
 * @brief Integer vector2.
 */
class IVector2D
{
public:
    IVector2D(int _x, int _y);
    bool operator== (const gsl::ivec2& rhs) const;
    IVector2D operator+ (int rhs) const;
    IVector2D operator- (int rhs) const;
    IVector2D operator* (int rhs) const;
    IVector2D operator/ (int rhs) const;

    int x;
    int y;
};

} //namespace

#endif // VECTOR2D_H
