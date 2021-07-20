// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "math/constants.h"
#include "math/mathfunc.h"
#include <cmath>
#include <fmt/core.h>

// 3D vector
template <class T> class Vec3
{
    public:
    Vec3<T>() : x(T()), y(T()), z(T()){};
    Vec3<T>(T xx, T yy, T zz) : x(xx), y(yy), z(zz){};
    // Components of vector
    T x, y, z;

    /*
     * Set / adjust / retrieve
     */
    public:
    // Set the vector to 0,0,0
    void zero()
    {
        x = 0;
        y = 0;
        z = 0;
    }
    // Set the specific element to value
    void set(int index, T value)
    {
        if (index == 0)
            x = value;
        else if (index == 1)
            y = value;
        else if (index == 2)
            z = value;
    }
    // Set all three values simultaneously
    void set(T newX, T newY, T newZ)
    {
        x = newX;
        y = newY;
        z = newZ;
    }
    // Add value to single component
    void add(int index, T delta)
    {
        if (index == 0)
            x += delta;
        else if (index == 1)
            y += delta;
        else if (index == 2)
            z += delta;
    }
    // Add values to all three values simultaneously
    void add(T dx, T dy, T dz)
    {
        x += dx;
        y += dy;
        z += dz;
    }
    // Returns the value of the specified element
    T get(int index) const
    {
        if (index == 0)
            return x;
        else if (index == 1)
            return y;
        else if (index == 2)
            return z;

        throw std::runtime_error(fmt::format("Vec3 - element index {} is out of bounds.", index));
        return T();
    }

    /*
     * Operators
     */
    public:
    void operator=(const T value)
    {
        x = value;
        y = value;
        z = value;
    }
    // Operators + and +=
    void operator+=(const T value)
    {
        x += value;
        y += value;
        z += value;
    }
    void operator+=(const Vec3<T> &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }
    Vec3<T> operator+(const T value) const
    {
        Vec3<T> result;
        result.x = x + value;
        result.y = y + value;
        result.z = z + value;
        return result;
    }
    Vec3<T> operator+(const Vec3<T> &v) const
    {
        Vec3<T> result;
        result.x = x + v.x;
        result.y = y + v.y;
        result.z = z + v.z;
        return result;
    }
    // Operators - and -=
    inline Vec3<T> operator-() const { return Vec3<T>(-x, -y, -z); }
    void operator-=(const T value)
    {
        x -= value;
        y -= value;
        z -= value;
    }
    void operator-=(const Vec3<T> &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }
    inline Vec3<T> operator-(const T value) const { return Vec3<T>(x - value, y - value, z - value); }
    inline Vec3<T> operator-(const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    // Operators / and /=
    void operator/=(const T value)
    {
        x /= value;
        y /= value;
        z /= value;
    }
    void operator/=(const Vec3<T> &v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
    }
    Vec3<T> operator/(const T value) const { return Vec3<T>(x / value, y / value, z / value); }
    Vec3<T> operator/(const Vec3<T> &v) const { return Vec3<T>(x / v.x, y / v.y, z / v.z); }
    // Operators * and *=
    Vec3<T> operator*(const T value) const { return Vec3<T>(x * value, y * value, z * value); }
    void operator*=(const T value)
    {
        x *= value;
        y *= value;
        z *= value;
    }
    Vec3<T> operator*(const Vec3<T> &v) const
    {
        // Cross Product
        return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
    T &operator[](int index)
    {
        if (index == 0)
            return x;
        else if (index == 1)
            return y;
        else if (index == 2)
            return z;

        throw std::runtime_error(fmt::format("Vec3 - array access failed - index {} is out of bounds.", index));

        static T dummy;
        return dummy;
    }

    /*
     * Methods
     */
    public:
    // Return vector of absolute elements
    Vec3<T> abs() const { return Vec3<T>(fabs(x), fabs(y), fabs(z)); }
    // Returns the largest absolute value of the vector
    T absMax() const
    {
        T a = (fabs(x) < fabs(y)) ? fabs(y) : fabs(x);
        return (a < fabs(z)) ? fabs(z) : a;
    }
    // Returns the index of the maximum absolute-valued element in the vector
    int absMaxElement() const
    {
        if ((fabs(x) >= fabs(y)) && (fabs(x) >= fabs(z)))
            return 0;
        if ((fabs(y) >= fabs(x)) && (fabs(y) >= fabs(z)))
            return 1;
        return 2;
    }
    // Returns the smallest absolute value of the vector
    T absMin() const
    {
        T a = (fabs(x) > fabs(y)) ? fabs(y) : fabs(x);
        return (a > fabs(z)) ? fabs(z) : a;
    }
    // Returns the index of the minimum absolute-valued element in the vector
    int absMinElement() const
    {
        if ((fabs(x) <= fabs(y)) && (fabs(x) <= fabs(z)))
            return 0;
        if ((fabs(y) <= fabs(x)) && (fabs(y) <= fabs(z)))
            return 1;
        return 2;
    }
    // Dot product between this and supplied vector
    double dp(const Vec3<T> &v) const { return (x * v.x + y * v.y + z * v.z); }
    // Normalise and return original magnitude
    double magAndNormalise()
    {
        double mag = sqrt(x * x + y * y + z * z);
        if (mag < 1.0E-8)
            zero();
        else
        {
            x /= mag;
            y /= mag;
            z /= mag;
        }
        return mag;
    }
    // Normalise and return original magnitude squared
    double magSqAndNormalise()
    {
        double magSq = x * x + y * y + z * z;
        double mag = sqrt(magSq);
        if (mag < 1.0E-8)
            zero();
        else
        {
            x /= mag;
            y /= mag;
            z /= mag;
        }
        return magSq;
    }
    // Calculate vector magnitude
    inline double magnitude() const { return sqrt(x * x + y * y + z * z); }
    // Calculate square of vector magnitude
    inline double magnitudeSq() const { return x * x + y * y + z * z; }
    // Returns the largest value of the vector
    T max() const
    {
        T a = (x < y) ? y : x;
        return (a < z) ? z : a;
    }
    // Returns the maximum valued element in the vector
    int maxElement() const
    {
        if ((x >= y) && (x >= z))
            return 0;
        if ((y >= x) && (y >= z))
            return 1;
        return 2;
    }
    // Returns the smallest value of the vector
    T min() const
    {
        T a = (x > y) ? y : x;
        return (a > z) ? z : a;
    }
    // Returns the minimum valued element in the vector
    int minElement() const
    {
        if ((x <= y) && (x <= z))
            return 0;
        if ((y <= x) && (y <= z))
            return 1;
        return 2;
    }
    // Multiply elements of this vector with factors supplied
    void multiply(double facx, double facy, double facz)
    {
        x *= facx;
        y *= facy;
        z *= facz;
    }
    // Multiply elements of this vector by those of supplied vector
    void multiply(Vec3<double> v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
    }
    // Normalise the vector to unity
    void normalise()
    {
        double mag = sqrt(x * x + y * y + z * z);
        if (mag < 1.0E-8)
            zero();
        else
        {
            x /= mag;
            y /= mag;
            z /= mag;
        }
    }
    // Returns an orthogonal, normalised unit vector
    Vec3<T> orthogonal() const
    {
        Vec3<T> result;
        const int maxComponent = absMaxElement();
        if (maxComponent == 0)
        {
            // X component is largest so return XP with (0,1,0)
            result.x = -z;
            result.y = 0.0;
            result.z = x;
        }
        else if (maxComponent == 1)
        {
            // Y component is largest, so return XP with (0,0,1)
            result.x = y;
            result.y = -x;
            result.z = 0.0;
        }
        else
        {
            // Z component is largest, so return XP with (1,0,0)
            result.x = 0.0;
            result.y = z;
            result.z = -y;
        }
        result.normalise();
        return result;
    }
    // Orthogonalise (Gram-Schmidt) w.r.t. supplied vector
    void orthogonalise(const Vec3<T> &reference)
    {
        double sourcemag = reference.magnitude();
        double dpovermagsq = dp(reference) / (sourcemag * sourcemag);
        x = x - dpovermagsq * reference.x;
        y = y - dpovermagsq * reference.y;
        z = z - dpovermagsq * reference.z;
    }
    // Orthogonalise (two vectors)
    void orthogonalise(const Vec3<T> &reference1, const Vec3<T> &reference2)
    {
        // This routine actually generates the orthogonal vector via the cross-product
        // We also calculate the scalar resolute (dp) to ensure the new vector points in the same direction
        Vec3<T> newvec = reference1 * reference2;
        newvec.normalise();
        double dp = newvec.dp(*this);
        if (dp < 0.0)
            newvec *= -1.0;
        *this = newvec;
    }
    // Prints the contents of the vector
    void print() const { fmt::print("{} {} {}\n", x, y, z); }
    // Generate random unit vector
    void randomUnit()
    {
        // Generates a random unit vector
        x = DissolveMath::random() - 0.5;
        y = DissolveMath::random() - 0.5;
        z = DissolveMath::random() - 0.5;
        normalise();
    }
    // Convert spherical who,phi,theta coordinates into cartesian x,y,z
    void toCartesian()
    {
        // x = rho, y = theta (inclination), z = phi (azimuthal angle), assuming that phi and theta are in degrees
        T newx, newy, newz;
        y /= DEGRAD;
        z /= DEGRAD;
        newx = x * sin(y) * cos(z);
        newy = x * sin(y) * sin(z);
        newz = x * cos(y);
        set(newx, newy, newz);
    }
    // Convert cartesian x,y,z coordinates into spherical (rho,phi/zenith,theta/azimuthal)
    void toSpherical()
    {
        T rho, phi, theta;
        rho = magnitude();
        theta = acos(z / rho);
        phi = atan2(y, x);
        set(rho, phi * DEGRAD, theta * DEGRAD);
    }
    // Set all three values simultaneously, converting supplied spherical coordinates to cartesian
    void fromSpherical(T r, T phi, T theta)
    {
        x = r;
        y = phi;
        z = theta;
        toCartesian();
    }
    // Return a unit vector along the specified direction
    static Vec3<T> unit(int index)
    {
        if (index == 0)
            return Vec3<T>(1, 0, 0);
        else if (index == 1)
            return Vec3<T>(0, 1, 0);
        else if (index == 2)
            return Vec3<T>(0, 0, 1);

        throw std::runtime_error(fmt::format("Vec3 - unit() generation failed - index {} is out of bounds.", index));
        return Vec3<T>();
    }
    // Swap the two specified elements
    void swap(int a, int b)
    {
        T temp = get(a);
        set(a, get(b));
        set(b, temp);
    }
};
