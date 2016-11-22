/*
	*** KVector
	*** src/classes/kvector.h
	Copyright T. Youngs 2012-2016

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUQ_KVECTOR_H
#define DUQ_KVECTOR_H

#include "templates/vector3.h"
#include "templates/array.h"

// Forward Declarations
class BraggPeak;

// K-Vector
class KVector
{
	public:
	// Constructor
	KVector(int h = 0, int k = 0, int l = 0, BraggPeak* braggPeak_ = 0, int nAtomTypes = 0);
	// Destructor
	~KVector();
	// Copy constructor
	KVector(const KVector& source);
	// Operator=
	void operator=(const KVector& source);
	// List pointers
	KVector* prev, *next;


	/*
	 * KVector Data
	 */
	private:
	// Integer hkl indices of vector
	Vec3<int> hkl_;
	// Associated BraggPeak
	BraggPeak* braggPeak_;
	// Contributions to this kvector from individual atom types
	Array<double> cosTerms_, sinTerms_;

	public:
	// Return hkl indices
	const Vec3<int>& hkl() const;
	// Return h index
	const int h() const;
	// Return k index
	const int k() const;
	// Return l index
	const int l() const;
	// Return associated BraggPeak
	const BraggPeak* braggPeak() const;
	// Zero cos/sin term arrays
	void zeroCosSinTerms();
	// Add value to cosTerm index specified
	void addCosTerm(int atomTypeIndex, double value);
	// Add value to sinTerm index specified
	void addSinTerm(int atomTypeIndex, double value);
	// Calculate intensities and sum into associated BraggPeak
	void calculateIntensities();
	// Return specified intensity
	double intensity(int typeI, int typeJ);
};

#endif
