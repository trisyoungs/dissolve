// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/box.h"
#include "base/processpool.h"
#include "classes/cell.h"
#include "math/data1d.h"
#include "math/interpolator.h"
#include <algorithm>

Box::Box(Box::BoxType boxType, const Vec3<double> lengths, const Vec3<double> angles)
    : type_(boxType), a_(lengths.x), b_(lengths.y), c_(lengths.z), ra_(1.0 / lengths.x), rb_(1.0 / lengths.y),
      rc_(1.0 / lengths.z), alpha_(angles.x), beta_(angles.y), gamma_(angles.z)
{
    // Set periodicity flags
    periodic_.set(type_ != BoxType::NonPeriodic, type_ != BoxType::NonPeriodic, type_ != BoxType::NonPeriodic);

    // Construct axes matrix
    axes_.setIdentity();

    // Determine cosines of box angles, zeroing if below some tolerance to clean up matrices
    auto cosAlpha = cos(alpha_ / DEGRAD);
    if (fabs(cosAlpha) < 1.0e-10)
        cosAlpha = 0;
    auto cosBeta = cos(beta_ / DEGRAD);
    if (fabs(cosBeta) < 1.0e-10)
        cosBeta = 0;
    auto cosGamma = cos(gamma_ / DEGRAD);
    if (fabs(cosGamma) < 1.0e-10)
        cosGamma = 0;

    // We set A={1,0,0}, so cos(gamma) equals 'x' of the B vector
    axes_.setColumn(1, cosGamma, sqrt(1.0 - cosGamma * cosGamma), 0.0);

    // The C vector can now be determined in parts.
    // - C.x = cos(beta)    (since {1,0,0}{x,y,z} = {1}{x} = cos(beta))
    // - C.y can be determined by completing the dot product between the B and C vectors
    // - C.z is the remainder of the unit vector
    axes_[6] = cosBeta;
    axes_[7] = (cosAlpha - axes_[3] * axes_[6]) / axes_[4];
    axes_[8] = sqrt(1.0 - axes_[6] * axes_[6] - axes_[7] * axes_[7]);

    // Multiply the unit vectors to have the correct lengths
    axes_.columnMultiply(0, a_);
    axes_.columnMultiply(1, b_);
    axes_.columnMultiply(2, c_);

    // Copy axes array
    axesArray_ = axes_.matrix();

    // Calculate box volume
    volume_ = axes_.determinant();

    // Calculate inverse axes
    inverseAxes_ = axes_;
    inverseAxes_.invert();
    inverseAxesArray_ = inverseAxes_.matrix();

    /*
     * Calculate reciprocal axes and volume.
     * Reciprocal cell vectors are perpendicular to normal cell axes - calculate from cross products of normal cell vectors.
     */
    reciprocalAxes_.setColumn(0, axes_.columnAsVec3(1) * axes_.columnAsVec3(2));
    reciprocalAxes_.setColumn(1, axes_.columnAsVec3(2) * axes_.columnAsVec3(0));
    reciprocalAxes_.setColumn(2, axes_.columnAsVec3(0) * axes_.columnAsVec3(1));
    reciprocalAxes_.columnMultiply(0, TWOPI / volume_);
    reciprocalAxes_.columnMultiply(1, TWOPI / volume_);
    reciprocalAxes_.columnMultiply(2, TWOPI / volume_);
    reciprocalVolume_ = (reciprocalAxes_.columnAsVec3(1) * reciprocalAxes_.columnAsVec3(2)).dp(reciprocalAxes_.columnAsVec3(0));
}

/*
 * Basic Definition
 */

// Return enum options for BoxType
EnumOptions<Box::BoxType> Box::boxTypes()
{
    return EnumOptions<Box::BoxType>("BoxType", {{Box::BoxType::NonPeriodic, "NonPeriodic"},
                                                 {Box::BoxType::Cubic, "Cubic"},
                                                 {Box::BoxType::Orthorhombic, "Orthorhombic"},
                                                 {Box::BoxType::MonoclinicAlpha, "MonoclinicAlpha"},
                                                 {Box::BoxType::MonoclinicBeta, "MonoclinicBeta"},
                                                 {Box::BoxType::MonoclinicGamma, "MonoclinicGamma"},
                                                 {Box::BoxType::Triclinic, "Triclinic"}});
}

// Return Box type
Box::BoxType Box::type() const { return type_; }

// Return volume
double Box::volume() const { return volume_; }

// Return axis lengths
Vec3<double> Box::axisLengths() const
{
    return Vec3<double>(axes_.columnMagnitude(0), axes_.columnMagnitude(1), axes_.columnMagnitude(2));
}

// Return axis length specified
double Box::axisLength(int n) const
{
    assert(n >= 0 && n < 3);

    return axes_.columnMagnitude(n);
}

// Return axis angles
Vec3<double> Box::axisAngles() const { return Vec3<double>(axisAngle(0), axisAngle(1), axisAngle(2)); }

// Return axis angle specified
double Box::axisAngle(int n) const
{
    assert(n >= 0 && n < 3);

    Vec3<double> u, v;
    u = axes_.columnAsVec3((n + 1) % 3);
    v = axes_.columnAsVec3((n + 2) % 3);
    u.normalise();
    v.normalise();
    double dp = u.dp(v);
    return acos(dp) * DEGRAD;
}

// Return axes matrix
const Matrix3 &Box::axes() const { return axes_; }

// Return inverse axes matrix
const Matrix3 &Box::inverseAxes() const { return inverseAxes_; }

// Return reciprocal box volume
double Box::reciprocalVolume() const { return reciprocalVolume_; }

// Return reciprocal axis lengths
Vec3<double> Box::reciprocalAxisLengths() const
{
    return Vec3<double>(reciprocalAxes_.columnMagnitude(0), reciprocalAxes_.columnMagnitude(1),
                        reciprocalAxes_.columnMagnitude(2));
}

// Return reciprocal axes matrix
const Matrix3 &Box::reciprocalAxes() const { return reciprocalAxes_; }

// Scale Box lengths by specified factors
void Box::scale(Vec3<double> scaleFactors)
{
    // Make a check here for cubic boxes, in which case all elements in scaleFactors must be equal
    if (type_ == BoxType::Cubic)
    {
        auto abSame = (fabs(scaleFactors.x - scaleFactors.y) < 1.0e-5);
        auto acSame = (fabs(scaleFactors.x - scaleFactors.z) < 1.0e-5);
        if (!abSame || !acSame)
            throw(std::runtime_error(fmt::format("Irregular scaling of cubic box requested (scale factors = {}, {}, {}\n",
                                                 scaleFactors.x, scaleFactors.y, scaleFactors.z)));
    }

    // Scale lengths
    a_ *= scaleFactors.x;
    b_ *= scaleFactors.y;
    c_ *= scaleFactors.z;
    ra_ = 1.0 / a_;
    rb_ = 1.0 / b_;
    rc_ = 1.0 / c_;

    // Scale axes
    axes_.columnMultiply(scaleFactors);
    axes_.print();
    // Calculate box volume
    volume_ = axes_.determinant();

    // Calculate inverse axes
    inverseAxes_ = axes_;
    inverseAxes_.invert();

    // Calculate reciprocal axes and volume
    // Reciprocal cell vectors are perpendicular to normal cell axes.
    // Calculate from cross products of normal cell vectors
    reciprocalAxes_.setColumn(0, axes_.columnAsVec3(1) * axes_.columnAsVec3(2));
    reciprocalAxes_.setColumn(1, axes_.columnAsVec3(2) * axes_.columnAsVec3(0));
    reciprocalAxes_.setColumn(2, axes_.columnAsVec3(0) * axes_.columnAsVec3(1));
    reciprocalAxes_.columnMultiply(0, TWOPI / volume_);
    reciprocalAxes_.columnMultiply(1, TWOPI / volume_);
    reciprocalAxes_.columnMultiply(2, TWOPI / volume_);
    reciprocalVolume_ = (reciprocalAxes_.columnAsVec3(1) * reciprocalAxes_.columnAsVec3(2)).dp(reciprocalAxes_.columnAsVec3(0));
}

/*
 * Coordinate Conversion
 */

// Return specified fractional coordinates converted to real-space coordinates
Vec3<double> Box::getReal(Vec3<double> r) const
{
    toReal(r);
    return r;
}

/*
 * Geometry Calculation
 */

// Return angle (in degrees) between coordinates
double Box::angleInDegrees(const Vec3<double> &i, const Vec3<double> &j, const Vec3<double> &k) const
{
    static Vec3<double> vecji, vecjk;
    vecji = minimumVector(j, i);
    vecjk = minimumVector(j, k);

    // Normalise vectors
    vecji.normalise();
    vecjk.normalise();

    // Determine Angle
    return angleInDegrees(vecji, vecjk);
}

// Return angle (in degrees) between supplied normalised vectors
double Box::angleInDegrees(const Vec3<double> &normji, const Vec3<double> &normjk) { return acos(normji.dp(normjk)) * DEGRAD; }

// Return angle (in degrees) between supplied normalised vectors
double Box::angleInDegrees(const Vec3<double> &normji, const Vec3<double> &normjk, double &dotProduct)
{
    dotProduct = normji.dp(normjk);
    return acos(dotProduct) * DEGRAD;
}

// Return literal angle (in degrees) between coordinates, without applying minimum image convention
double Box::literalAngleInDegrees(const Vec3<double> &i, const Vec3<double> &j, const Vec3<double> &k)
{
    static Vec3<double> vecji, vecjk;
    vecji = i - j;
    vecji.normalise();
    vecjk = k - j;
    vecjk.normalise();
    return acos(vecji.dp(vecjk)) * DEGRAD;
}

// Return torsion (in degrees) between supplied unnormalised vectors
double Box::torsionInDegrees(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl)
{
    // Calculate cross products and torsion angle formed (in radians)
    Vec3<double> xpj, xpk;
    double magxpj, magxpk;

    return torsionInRadians(vecji, vecjk, veckl, xpj, magxpj, xpk, magxpk) * DEGRAD;
}

// Return torsion (in degrees) between supplied unnormalised vectors
double Box::torsionInDegrees(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl, Vec3<double> &xpj,
                             double &magxpj, Vec3<double> &xpk, double &magxpk)
{
    return torsionInRadians(vecji, vecjk, veckl, xpj, magxpj, xpk, magxpk) * DEGRAD;
}

// Return torsion (in radians) between supplied unnormalised vectors
double Box::torsionInRadians(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl)
{
    // Calculate cross products and torsion angle formed (in radians)
    Vec3<double> xpj, xpk;
    double magxpj, magxpk;

    return torsionInRadians(vecji, vecjk, veckl, xpj, magxpj, xpk, magxpk);
}

// Return torsion (in radians) between supplied unnormalised vectors, storing cross products and magnitude in supplied variables
double Box::torsionInRadians(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl, Vec3<double> &xpj,
                             double &magxpj, Vec3<double> &xpk, double &magxpk)
{
    xpj = vecjk * vecji;
    xpk = vecjk * veckl;
    magxpj = xpj.magAndNormalise();
    magxpk = xpk.magAndNormalise();
    return atan2(vecjk.dp(xpj * xpk) / vecjk.magnitude(), xpj.dp(xpk));
}

/*
 * Utility Routines
 */

// Generate a suitable Box given the supplied relative lengths, angles
std::unique_ptr<Box> Box::generate(Vec3<double> lengths, Vec3<double> angles)
{
    // Determine box type from supplied lengths / angles
    auto rightAlpha = (fabs(angles.x - 90.0) < 1.0e-5);
    auto rightBeta = (fabs(angles.y - 90.0) < 1.0e-5);
    auto rightGamma = (fabs(angles.z - 90.0) < 1.0e-5);
    int nRight = rightAlpha + rightBeta + rightGamma;

    if (nRight == 3)
    {
        auto abSame = (fabs(lengths.x - lengths.y) < 1.0e-5);
        auto acSame = (fabs(lengths.x - lengths.z) < 1.0e-5);
        if (abSame && acSame)
            return std::make_unique<CubicBox>(lengths.x);
        else
            return std::make_unique<OrthorhombicBox>(lengths);
    }
    else if (nRight == 2)
    {
        if (!rightAlpha)
            return std::make_unique<MonoclinicAlphaBox>(lengths, angles.x);
        else if (!rightBeta)
            return std::make_unique<MonoclinicBetaBox>(lengths, angles.y);
        else
            return std::make_unique<MonoclinicGammaBox>(lengths, angles.z);
    }
    else
        return std::make_unique<TriclinicBox>(lengths, angles);
}

// Return radius of largest possible inscribed sphere for box
double Box::inscribedSphereRadius() const
{
    // Radius of largest inscribed sphere is the smallest of the three calculated values....
    double mag, diameter, result = 0.0;
    Vec3<double> cross;
    int n, i, j;
    for (n = 0; n < 3; ++n)
    {
        i = (n + 1) % 3;
        j = (n + 2) % 3;
        cross = axes_.columnAsVec3(i) * axes_.columnAsVec3(j);
        mag = cross.magnitude();
        diameter = fabs(axes_.columnAsVec3(n).dp(cross / mag));
        if (n == 0)
            result = diameter;
        else if (diameter < result)
            result = diameter;
    }
    return result * 0.5;
}

// Return random coordinate inside Box
Vec3<double> Box::randomCoordinate() const
{
    Vec3<double> r(DissolveMath::random(), DissolveMath::random(), DissolveMath::random());
    toReal(r);
    return r;
}

// Return folded coordinate (i.e. inside current Box)
Vec3<double> Box::fold(const Vec3<double> &r) const
{
    // Convert coordinate to fractional coords
    auto frac = r;
    toFractional(frac);

    // Fold into Box
    frac.x -= floor(frac.x);
    frac.y -= floor(frac.y);
    frac.z -= floor(frac.z);

    toReal(frac);

    return frac;
}

// Return folded fractional coordinate (i.e. inside current Box)
Vec3<double> Box::foldFrac(const Vec3<double> &r) const
{
    // Convert coordinate to fractional coords
    auto frac = r;
    toFractional(frac);

    // Fold into Box
    frac.x -= floor(frac.x);
    frac.y -= floor(frac.y);
    frac.z -= floor(frac.z);

    return frac;
}

// Determine axis scale factors to give requested volume, with scaling ratios provided
Vec3<double> Box::scaleFactors(double requestedVolume, Vec3<bool> scalableAxes) const
{
    // Sanity check
    if (!scalableAxes.x && !scalableAxes.y && !scalableAxes.z)
        throw(std::runtime_error("No axes specified as scalable, so no scaling factor can be calculated.\n"));

    // Determine root power
    auto rootPower = 1.0 / (scalableAxes.x + scalableAxes.y + scalableAxes.z);

    auto ratio = pow(requestedVolume, rootPower) / pow(volume_, rootPower);

    return {scalableAxes.x ? ratio : 1.0, scalableAxes.y ? ratio : 1.0, scalableAxes.z ? ratio : 1.0};
}
