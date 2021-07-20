// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "base/enumoptions.h"
#include "math/matrix3.h"

// Forward Declarations
class Cell;
class Data1D;
class ProcessPool;

// Basic Box Definition
class Box
{
    public:
    // Box Type Enum
    enum class BoxType
    {
        NonPeriodic,     /* Non-periodic system - cubic box, but no minimum image calculation */
        Cubic,           /* Cubic box with A == B == C, alphe == beta == gamma == 90 */
        Orthorhombic,    /* Orthorhombic box with A != B != C, alphe == beta == gamma = 90 */
        MonoclinicAlpha, /* Monoclinic box with A != B != C, alpha != 90, and beta == gamma == 90 */
        MonoclinicBeta,  /* Monoclinic box with A != B != C, beta != 90, and alpha == gamma == 90 */
        MonoclinicGamma, /* Monoclinic box with A != B != C, gamma != 90, and alpha == beta == 90 */
        Triclinic        /* Triclinic box with A != B != C, alpha != beta != gamma != 90 */
    };
    // Return enum options for BoxType
    static EnumOptions<BoxType> boxTypes();
    Box(Box::BoxType boxType, const Vec3<double> lengths, const Vec3<double> angles);
    virtual ~Box() = default;
    Box &operator=(const Box &source) = default;

    /*
     * Basic Definition
     */
    public:
    protected:
    // Box type
    BoxType type_;
    // Box lengths
    double a_, b_, c_;
    // Reciprocal Box lengths
    double ra_, rb_, rc_;
    // Box angles
    double alpha_, beta_, gamma_;
    // Flags stating periodicity along x, y, and z
    Vec3<bool> periodic_;
    // Axes
    Matrix3 axes_;
    // Axes as simple array
    std::array<double, 9> axesArray_;
    // Inverse axes
    Matrix3 inverseAxes_;
    // Inverse axes as simple array
    std::array<double, 9> inverseAxesArray_;
    // Reciprocal axes
    Matrix3 reciprocalAxes_;
    // Volume
    double volume_;
    // Reciprocal volume
    double reciprocalVolume_;

    public:
    // Finalise Box, storing volume and reciprocal and inverted axes
    void finalise();
    // Return Box type
    BoxType type() const;
    // Return volume
    double volume() const;
    // Return axis lengths
    Vec3<double> axisLengths() const;
    // Return axis length specified
    double axisLength(int n) const;
    // Return axis angles
    Vec3<double> axisAngles() const;
    // Return axis angle specified
    double axisAngle(int n) const;
    // Return axis matrix
    const Matrix3 &axes() const;
    // Return inverse axes matrix
    const Matrix3 &inverseAxes() const;
    // Return reciprocal box volume
    double reciprocalVolume() const;
    // Return reciprocal axis lengths
    Vec3<double> reciprocalAxisLengths() const;
    // Return reciprocal axes matrix
    const Matrix3 &reciprocalAxes() const;
    // Scale Box lengths by specified factors
    void scale(Vec3<double> scaleFactors);

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    inline virtual void toReal(Vec3<double> &r) const = 0;
    // Return specified fractional coordinates converted to real-space coordinates
    Vec3<double> getReal(Vec3<double> r) const;
    // Convert specified real-space coordinates to fractional coordinates
    inline virtual void toFractional(Vec3<double> &r) const = 0;

    /*
     * Minimum Image Calculation
     */
    protected:
    // Wrap fractional coordinate into Box assuming it can be no more than half a fractional Box length away in any one
    // direction
    inline void wrap(Vec3<double> &rFrac) const
    {
        if (rFrac.x < -0.5)
            rFrac.x += 1.0;
        else if (rFrac.x > 0.5)
            rFrac.x -= 1.0;
        if (rFrac.y < -0.5)
            rFrac.y += 1.0;
        else if (rFrac.y > 0.5)
            rFrac.y -= 1.0;
        if (rFrac.z < -0.5)
            rFrac.z += 1.0;
        else if (rFrac.z > 0.5)
            rFrac.z -= 1.0;
    }

    public:
    // Return minimum image coordinates of r1 with respect to r2
    virtual Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const = 0;
    // Return minimum image vector from r1 to r2
    virtual Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const = 0;
    // Return minimum image distance from r1 to r2
    virtual double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const = 0;
    // Return minimum image squared distance from r1 to r2
    virtual double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const = 0;

    /*
     * Geometry Calculation
     */
    public:
    // Return angle (in degrees) between coordinates
    double angleInDegrees(const Vec3<double> &i, const Vec3<double> &j, const Vec3<double> &k) const;
    // Return angle (in degrees) between supplied normalised vectors
    static double angleInDegrees(const Vec3<double> &normji, const Vec3<double> &normjk);
    // Return angle (in degrees) between supplied normalised vectors (storing dot product)
    static double angleInDegrees(const Vec3<double> &normji, const Vec3<double> &normjk, double &dotProduct);
    // Return literal angle (in degrees) between coordinates, without applying minimum image convention
    static double literalAngleInDegrees(const Vec3<double> &i, const Vec3<double> &j, const Vec3<double> &k);
    // Return torsion (in degrees) between supplied unnormalised vectors
    static double torsionInDegrees(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl);
    // Return torsion (in degrees) between supplied unnormalised vectors, storing cross products and magnitude in supplied
    // variables
    static double torsionInDegrees(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl,
                                   Vec3<double> &xpj, double &magxpj, Vec3<double> &xpk, double &magxpk);
    // Return torsion (in radians) between supplied unnormalised vectors
    static double torsionInRadians(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl);
    // Return torsion (in radians) between supplied unnormalised vectors, storing cross products and magnitude in supplied
    // variables
    static double torsionInRadians(const Vec3<double> &vecji, const Vec3<double> &vecjk, const Vec3<double> &veckl,
                                   Vec3<double> &xpj, double &magxpj, Vec3<double> &xpk, double &magxpk);

    /*
     * Utility Routines
     */
    public:
    // Generate a suitable Box given the supplied relative lengths, angles, and volume
    static std::unique_ptr<Box> generate(Vec3<double> lengths, Vec3<double> angles);
    // Return radius of largest possible inscribed sphere for box
    double inscribedSphereRadius() const;
    // Return random coordinate inside Box
    Vec3<double> randomCoordinate() const;
    // Return folded coordinate (i.e. inside current Box)
    Vec3<double> fold(const Vec3<double> &r) const;
    // Return folded fractional coordinate (i.e. inside current Box)
    Vec3<double> foldFrac(const Vec3<double> &r) const;
    // Determine axis scale factors to give requested volume, with scaling ratios provided
    Vec3<double> scaleFactors(double requestedVolume, Vec3<bool> scalableAxes = {true, true, true}) const;
};

// Non-Periodic Box Definition
class NonPeriodicBox : public Box
{
    public:
    NonPeriodicBox(double length = 1.0);
    ~NonPeriodicBox() override = default;

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    void toReal(Vec3<double> &r) const override;
    // Convert specified real-space coordinates to fractional coordinates
    void toFractional(Vec3<double> &r) const override;

    /*
     * Minimum Image Calculations
     */
    public:
    // Return minimum image coordinates of r1 with respect to r2
    Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image vector from r1 to r2
    Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image distance from r1 to r2
    double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image squared distance from r1 to r2
    double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const override;
};

// Cubic Box Definition
class CubicBox : public Box
{
    public:
    CubicBox(double length);
    ~CubicBox() override = default;

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    void toReal(Vec3<double> &r) const override;
    // Convert specified real-space coordinates to fractional coordinates
    void toFractional(Vec3<double> &r) const override;

    /*
     * Minimum Image Calculations
     */
    public:
    // Return minimum image coordinates of r1 with respect to r2
    Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image vector from r1 to r2
    Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image distance from r1 to r2
    double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image squared distance from r1 to r2
    double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const override;

    /*
     * Utility Functions
     */
    public:
    // Return folded coordinate (i.e. inside current Box)
    //    Vec3<double> fold(const Vec3<double> &r) const override;
};

// Orthorhombic Box Definition
class OrthorhombicBox : public Box
{
    public:
    OrthorhombicBox(const Vec3<double> lengths);
    ~OrthorhombicBox() override = default;

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    void toReal(Vec3<double> &r) const override;
    // Convert specified real-space coordinates to fractional coordinates
    void toFractional(Vec3<double> &r) const override;

    /*
     * Minimum Image Calculations
     */
    public:
    // Return minimum image coordinates of r1 with respect to r2
    Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image vector from r1 to r2
    Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image distance from r1 to r2
    double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image squared distance from r1 to r2
    double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const override;
};

// MonoclinicAlpha Box Definition
class MonoclinicAlphaBox : public Box
{
    public:
    MonoclinicAlphaBox(const Vec3<double> lengths, double alpha);
    ~MonoclinicAlphaBox() override = default;

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    void toReal(Vec3<double> &r) const override;
    // Convert specified real-space coordinates to fractional coordinates
    void toFractional(Vec3<double> &r) const override;

    /*
     * Minimum Image Calculations
     */
    public:
    // Return minimum image coordinates of r1 with respect to r2
    Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image vector from r1 to r2
    Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image distance from r1 to r2
    double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image squared distance from r1 to r2
    double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const override;
};

// MonoclinicBeta Box Definition
class MonoclinicBetaBox : public Box
{
    public:
    MonoclinicBetaBox(const Vec3<double> lengths, double beta);
    ~MonoclinicBetaBox() override = default;

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    void toReal(Vec3<double> &r) const override;
    // Convert specified real-space coordinates to fractional coordinates
    void toFractional(Vec3<double> &r) const override;

    /*
     * Minimum Image Calculations
     */
    public:
    // Return minimum image coordinates of r1 with respect to r2
    Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image vector from r1 to r2
    Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image distance from r1 to r2
    double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image squared distance from r1 to r2
    double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const override;
};

// MonoclinicGamma Box Definition
class MonoclinicGammaBox : public Box
{
    public:
    MonoclinicGammaBox(const Vec3<double> lengths, double gamma);
    ~MonoclinicGammaBox() override = default;

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    void toReal(Vec3<double> &r) const override;
    // Convert specified real-space coordinates to fractional coordinates
    void toFractional(Vec3<double> &r) const override;

    /*
     * Minimum Image Calculations
     */
    public:
    // Return minimum image coordinates of r1 with respect to r2
    Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image vector from r1 to r2
    Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image distance from r1 to r2
    double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image squared distance from r1 to r2
    double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const override;
};

// Triclinic Box Definition
class TriclinicBox : public Box
{
    public:
    TriclinicBox(const Vec3<double> lengths, const Vec3<double> angles);
    ~TriclinicBox() override = default;

    /*
     * Coordinate Conversion
     */
    public:
    // Convert specified fractional coordinates to real-space coordinates
    void toReal(Vec3<double> &r) const override;
    // Convert specified real-space coordinates to fractional coordinates
    void toFractional(Vec3<double> &r) const override;

    /*
     * Minimum Image Calculations
     */
    public:
    // Return minimum image coordinates of r1 with respect to r2
    Vec3<double> minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image vector from r1 to r2
    Vec3<double> minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image distance from r1 to r2
    double minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const override;
    // Return minimum image squared distance from r1 to r2
    double minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const override;
};
