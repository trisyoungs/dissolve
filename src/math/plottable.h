/*
	*** Plottable Data
	*** src/math/plottable.h
	Copyright T. Youngs 2012-2018

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISSOLVE_PLOTTABLE_H
#define DISSOLVE_PLOTTABLE_H

#include "math/sampleddouble.h"
#include "base/charstring.h"
#include "templates/array.h"

// Forward Declarations
template <class T> class Array3D;

// Plottable
class Plottable
{
	public:
	// Plottable Types
	enum PlottableType {
		OneAxisPlottable, 		/* Contains data points plotted against one axis (x) */
		TwoAxisPlottable,		/* Contains data points plotted against two axes (x and y) */
		ThreeAxisPlottable		/* Contains data points plotted againas three axes (x, y, and z) */
	};
	// Constructor
	Plottable(PlottableType type);

	
	/*
	 * Basic Information
	 */
	private:
	// Type of plottable
	PlottableType type_;

	protected:
	// Name of plottable
	CharString name_;

	public:
	// Set name of plottable
	void setName(const char* name);
	// Return name of plottable
	const char* name() const;


	/*
	 * Axis Information
	 */
	public:
	// Return number of points along x axis
	virtual int nXAxisPoints() const = 0;
	// Return x axis Array
	virtual const Array<double>& constXAxis() const = 0;
	// Return number of points along y axis
	virtual int nYAxisPoints() const;
	// Return y axis Array
	virtual const Array<double>& constYAxis() const;
	// Return number of points along z axis
	virtual int nZAxisPoints() const;
	// Return z axis Array
	virtual const Array<double>& constZAxis() const;


	/*
	 * Values / Errors
	 */
	public:
	// Return values Array
	virtual const Array<double>& constValues() const;
	// Return values Array
	virtual const Array2D<double>& constValues2D() const;
	// Return three-dimensional values Array
	virtual const Array3D<double>& constValues3D() const;
	// Return number of data points present in the whole dataset
	virtual int nDataPoints() const = 0;
	// Return minimum value over all data points
	virtual double minValue() const = 0;
	// Return maximum value over all data points
	virtual double maxValue() const = 0;
	// Return whether the values have associated errors
	virtual bool valuesHaveErrors() const;
	// Return errors Array
	virtual const Array<double>& constErrors() const;
	// Return errors Array
	virtual const Array2D<double>& constErrors2D() const;
	// Return three-dimensional errors Array
	virtual const Array3D<double>& constErrors3D() const;
};

#endif
