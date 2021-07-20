// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/cellarray.h"
#include "classes/box.h"
#include "classes/cell.h"

/*
 * Cell Data
 */

// Return number of Cells for box
int CellArray::nCells() const { return static_cast<int>(cells_.size()); }

// Return cell divisions along each axis
Vec3<int> CellArray::divisions() const { return divisions_; }

// Return real Cell dimensions
Vec3<double> CellArray::realCellSize() const { return realCellSize_; }

// Return cell extents out from given central cell
Vec3<int> CellArray::extents() const { return extents_; }

// Retrieve Cell with (wrapped) grid reference specified
const Cell *CellArray::cell(int x, int y, int z) const
{
    x = x % divisions_.x;
    if (x < 0)
        x += divisions_.x;
    y = y % divisions_.y;
    if (y < 0)
        y += divisions_.y;
    z = z % divisions_.z;
    if (z < 0)
        z += divisions_.z;
    return &cells_[x * divisions_.y * divisions_.z + y * divisions_.z + z];
}

// Retrieve Cell with id specified
const Cell *CellArray::cell(int id) const
{
    assert(id >= 0 && id < cells_.size());

    return &cells_[id];
}

// Return Cell which contains specified coordinate
const Cell *CellArray::cell(const Vec3<double> r) const
{
    auto foldFracR = box_->foldFrac(r);
    Vec3<int> indices;
    indices.x = foldFracR.x / fractionalCellSize_.x;
    indices.y = foldFracR.y / fractionalCellSize_.y;
    indices.z = foldFracR.z / fractionalCellSize_.z;
    indices.x %= divisions_.x;
    indices.y %= divisions_.y;
    indices.z %= divisions_.z;

    return &cells_[indices.x * divisions_.y * divisions_.z + indices.y * divisions_.z + indices.z];
}
Cell *CellArray::cell(const Vec3<double> r)
{
    auto foldFracR = box_->foldFrac(r);
    Vec3<int> indices;
    indices.x = foldFracR.x / fractionalCellSize_.x;
    indices.y = foldFracR.y / fractionalCellSize_.y;
    indices.z = foldFracR.z / fractionalCellSize_.z;
    indices.x %= divisions_.x;
    indices.y %= divisions_.y;
    indices.z %= divisions_.z;

    return &cells_[indices.x * divisions_.y * divisions_.z + indices.y * divisions_.z + indices.z];
}

// Check if it is possible for any pair of Atoms in the supplied cells to be within the specified distance
bool CellArray::withinRange(const Cell *a, const Cell *b, double distance)
{
    assert(a != nullptr);
    assert(b != nullptr);

    // We need both the minimum image centroid-centroid distance, as well as the integer mim grid-reference delta
    Vec3<int> u = mimGridDelta(a, b);

    /*
     * We now have the minimum image integer grid vector from Cell a to Cell b.
     * Subtract 1 from any vector that is not zero (adding 1 to negative indices and -1 to positive indices.
     * This has the effect of shortening the vector to account for atoms being at the near edges / corners of the two cells.
     */
    u.x -= DissolveMath::sgn(u.x);
    u.y -= DissolveMath::sgn(u.y);
    u.z -= DissolveMath::sgn(u.z);

    // Turn this grid reference delta into a real distamce by multiplying by the Cell axes_ matrix
    auto v = axes_ * Vec3<double>(u.x, u.y, u.z);

    // Check ths vector magnitude against the supplied distance
    return (v.magnitude() <= distance);
}

// Check if minimum image calculation is necessary for any potential pair of atoms in the supplied cells
bool CellArray::minimumImageRequired(const Cell *a, const Cell *b, double distance)
{
    assert(a != nullptr);
    assert(b != nullptr);

    // Check every pair of corners between the Cell two grid references, and determine if minimum image calculation would be
    // required
    Vec3<int> i, j;
    Vec3<double> r;
    for (auto iCorner = 0; iCorner < 8; ++iCorner)
    {
        // Set integer vertex of corner on 'central' box
        i.set(a->gridReference().x + (iCorner & 1 ? 1 : 0), a->gridReference().y + (iCorner & 2 ? 1 : 0),
              a->gridReference().z + (iCorner & 4 ? 1 : 0));

        for (auto jCorner = 0; jCorner < 8; ++jCorner)
        {
            // Set integer vertex of corner on 'other' box
            j.set(b->gridReference().x + (jCorner & 1 ? 1 : 0), b->gridReference().y + (jCorner & 2 ? 1 : 0),
                  b->gridReference().z + (jCorner & 4 ? 1 : 0));

            // Check literal distance between points - if it's less than the distance specified we can continue (no
            // mim required)
            j -= i;
            r.set(j.x, j.y, j.z);
            r = axes_ * r;
            if (r.magnitude() < distance)
                continue;

            // Check minimum image distance between points - if it's less than the distance specified we require mim
            // for this cell pair
            j = mimGridDelta(j);
            r.set(j.x, j.y, j.z);
            r = axes_ * r;
            if (r.magnitude() < distance)
                return true;
        }
    }

    return false;
}

// Return the minimum image grid delta between the two specified Cells
Vec3<int> CellArray::mimGridDelta(const Cell *a, const Cell *b) const
{
    Vec3<int> u = b->gridReference() - a->gridReference();
    return mimGridDelta(u);
}

// Return the minimum image equivalent of the supplied grid delta
Vec3<int> CellArray::mimGridDelta(Vec3<int> delta) const
{
    if (delta.x > divisions_.x * 0.5)
        delta.x -= divisions_.x;
    else if (delta.x < -divisions_.x * 0.5)
        delta.x += divisions_.x;
    if (delta.y > divisions_.y * 0.5)
        delta.y -= divisions_.y;
    else if (delta.y < -divisions_.y * 0.5)
        delta.y += divisions_.y;
    if (delta.z > divisions_.z * 0.5)
        delta.z -= divisions_.z;
    else if (delta.z < -divisions_.z * 0.5)
        delta.z += divisions_.z;
    return delta;
}

/*
 * Cell Neighbour
 */

// Add neighbour to cell vector
void CellArray::addNeighbour(const Cell &cell, const Cell &nbr, bool useMim)
{
    auto &cellVector = neighbours_[cell.index()];
    auto it = std::find(cellVector.begin(), cellVector.end(), nbr);
    if (it != cellVector.end())
    {
        // Neighbour already exists - make sure it is consistent
        assert(it->neighbour_.index() == nbr.index());
        assert(it->requiresMIM_ == useMim);
    }
    else
        cellVector.emplace_back(nbr, useMim);
}

// Construct cell neighbour pairs
void CellArray::createCellNeighbourPairs()
{
    auto nPairs = cells_.size() * ((cells_.size() + 1) / 2);
    neighbourPairs_.clear();
    neighbourPairs_.reserve(nPairs);
    for (auto &cell : cells_)
    {
        // Cell with itself
        neighbourPairs_.emplace_back(cell, cell, false);

        // Central cell neighbours not requiring minimum image
        for (auto *otherCell : cell.cellNeighbours())
            if (cell.index() < otherCell->index())
                neighbourPairs_.emplace_back(cell, *otherCell, false);

        // Central cell neighbours not requiring minimum image
        for (auto *otherCell : cell.mimCellNeighbours())
            if (cell.index() < otherCell->index())
                neighbourPairs_.emplace_back(cell, *otherCell, true);
    }
}

// Return neighbour vector for specified cell, including self as first item
const std::vector<CellNeighbour> &CellArray::neighbours(const Cell &cell) const { return neighbours_[cell.index()]; }

// Return vector of all unique cell neighbour pairs
const std::vector<CellNeighbourPair> &CellArray::getCellNeighbourPairs() const { return neighbourPairs_; }

/*
 * Generation
 */

// Generate Cells for Box
bool CellArray::generate(const Box *box, double cellSize, double pairPotentialRange)
{
    // There are some conditions to the partitioning:
    // 1) We require a minimum number of cells (minCellsPerSide) per box side
    // 2) TODO Need to check distances properly for monoclinic and triclinic cells
    // 3) Require Cells to be roughly 'cubic' as far as is possible.

    const auto minCellsPerSide = 3;
    const auto tolerance = 0.01;

    box_ = box;

    Messenger::print("Generating cells for box - minimum cells per side is {}, cell size is {}...\n", minCellsPerSide,
                     cellSize);

    // Get Box axis lengths and divide through by cellSize
    Vec3<double> boxLengths(box_->axisLength(0), box_->axisLength(1), box_->axisLength(2));
    Vec3<int> divisions(boxLengths.x / cellSize, boxLengths.y / cellSize, boxLengths.z / cellSize);
    int minEl;
    divisions_.zero();
    realCellSize_.zero();

    Messenger::print("Initial divisions based on cell size are (x,y,z) = ({},{},{})\n", divisions.x, divisions.y, divisions.z);

    // How does the smallest length compare with the PairPotential range?
    if (divisions.min() < minCellsPerSide)
    {
        Messenger::warn("Box size only allows for {} whole divisions of the cell size ({}) along one or more axes, "
                        "while we require at least {}.\n",
                        divisions.min(), cellSize, minCellsPerSide);

        // We must now take the shortest box length and divide by 3 to get the absolute maximum length to use on that side
        minEl = boxLengths.minElement();
        realCellSize_[minEl] = boxLengths[minEl] / minCellsPerSide;
        divisions_[minEl] = minCellsPerSide;
    }
    else
    {
        // All Box lengths are long enough to be partitioned in to at least minCellsPerSide
        // Just find shortest of them and set its exact distance
        minEl = boxLengths.minElement();
        realCellSize_[minEl] = boxLengths[minEl] / divisions[minEl];
        divisions_[minEl] = divisions[minEl];
    }

    Messenger::print("Shortest side (axis {}) will have cell length of {} Angstroms.\n", minEl, realCellSize_[minEl]);

    // Now, set our other cellLengths_ based on the minimum value we have just set
    // We try to get all lengths as similar as possible
    for (auto n = 1; n < 3; ++n)
    {
        auto el = (minEl + n) % 3;
        double x = boxLengths[el] / realCellSize_[minEl];
        double remainder = x - int(x);

        // If we're within a tolerance to the nearest integer, use this number....
        // If not, we can round 'x' to the nearest integer, but can only round up if ppRange allows us to...
        if (remainder > (1.0 - tolerance))
        {
            divisions_[el] = int(x) + 1;
            realCellSize_[el] = boxLengths[el] / divisions_[el];
            Messenger::print("Accepted cell length of {} Angstroms ({} divisions) for axis {}, since it was within "
                             "tolerance (-{:e}).\n",
                             realCellSize_[minEl], divisions_[el], el, remainder);
        }
        else if (remainder < tolerance)
        {
            divisions_[el] = int(x);
            realCellSize_[el] = boxLengths[el] / divisions_[el];
            Messenger::print("Accepted cell length of {} Angstroms ({} divisions) for axis {}, since it was within "
                             "tolerance (+{:e}).\n",
                             realCellSize_[minEl], divisions_[el], el, remainder);
        }
        else if (remainder < 0.5)
        {
            // Can't fit more than half another cell in, so reduce number of divisions...
            divisions_[el] = int(x);
            realCellSize_[el] = boxLengths[el] / divisions_[el];
            Messenger::print("Decreased cell length for axis {} to {} Angstroms ({} divisions).\n", el, realCellSize_[el],
                             divisions_[el]);
        }
        else
        {
            // Can fit more than half another Cell in - can we increase to the next integer?
            divisions_[el] = int(x) + 1;
            realCellSize_[el] = boxLengths[el] / divisions_[el];
            if (realCellSize_[el] < cellSize)
            {
                --divisions_[el];
                realCellSize_[el] = boxLengths[el] / divisions_[el];
                Messenger::print("Forced decrease of cell length for axis {} to {} Angstroms ({} divisions) "
                                 "since increasing it gave a length larger than the cell size.\n",
                                 el, realCellSize_[el], divisions_[el]);
            }
            else
                Messenger::print("Increased cell length for axis {} to {} Angstroms ({} divisions).\n", el, realCellSize_[el],
                                 divisions_[el]);
        }
    }

    // Summarise
    fractionalCellSize_.set(1.0 / divisions_.x, 1.0 / divisions_.y, 1.0 / divisions_.z);
    Messenger::print("Final cell partitioning is (x,y,z) = ({},{},{}), giving {} cells in total.\n", divisions_.x, divisions_.y,
                     divisions_.z, divisions_.x * divisions_.y * divisions_.z);
    Messenger::print("Fractional cell size is ({},{},{}).\n", fractionalCellSize_.x, fractionalCellSize_.y,
                     fractionalCellSize_.z);

    // Construct Cell arrays
    clear();
    auto nCells = divisions_.x * divisions_.y * divisions_.z;
    Messenger::print("Constructing array of {} cells...\n", nCells);
    cells_.resize(nCells);
    Vec3<double> fracCentre(fractionalCellSize_.x * 0.5, 0.0, 0.0);
    auto count = 0;
    for (auto x = 0; x < divisions_.x; ++x)
    {
        fracCentre.y = fractionalCellSize_.y * 0.5;
        for (auto y = 0; y < divisions_.y; ++y)
        {
            fracCentre.z = fractionalCellSize_.z * 0.5;
            for (auto z = 0; z < divisions_.z; ++z)
            {
                cells_[count] = Cell(count, Vec3<int>(x, y, z), box_->getReal(fracCentre));
                fracCentre.z += fractionalCellSize_.z;
                ++count;
            }
            fracCentre.y += fractionalCellSize_.y;
        }
        fracCentre.x += fractionalCellSize_.x;
    }

    // Calculate Cell axes matrix
    axes_ = box_->axes();
    axes_.columnMultiply(fractionalCellSize_);

    // Construct Cell neighbour lists
    Messenger::print("Creating cell neighbour lists...\n");

    // Make a list of integer vectors which we'll then use to pick Cells for the neighbour lists
    Vec3<double> r;
    Matrix3 cellAxes = box_->axes();
    cellAxes.applyScaling(fractionalCellSize_.x, fractionalCellSize_.y, fractionalCellSize_.z);
    extents_.zero();

    // First, establish a maximal extent in principal directions...
    for (auto n = 0; n < 3; ++n)
    {
        do
        {
            r.zero();
            ++extents_[n];
            r[n] = extents_[n];
            r = cellAxes * r;
        } while (r[n] < pairPotentialRange);

        // If we require a larger number of cells than the box physically has along this direction, reduce it accordingly
        if ((extents_[n] * 2 + 1) > divisions_[n])
            extents_[n] = divisions_[n] / 2;
    }
    Messenger::print("Cell extents required to cover PairPotential range are (x,y,z) = ({},{},{}).\n", extents_.x, extents_.y,
                     extents_.z);

    // Now, loop over extent integers and construct list of gridReferences within range
    std::vector<Vec3<int>> neighbourIndices;
    RefList<const Cell> cellNbrs;
    Vec3<int> i, j;
    const Cell *nbr;
    for (auto x = -extents_.x; x <= extents_.x; ++x)
    {
        for (auto y = -extents_.y; y <= extents_.y; ++y)
        {
            for (auto z = -extents_.z; z <= extents_.z; ++z)
            {
                if ((x == 0) && (y == 0) && (z == 0))
                    continue;

                // Check a nominal central cell at (0,0,0) and this grid reference to see if any pairs of
                // corners are in range
                auto close = false;
                for (auto iCorner = 0; iCorner < 8; ++iCorner)
                {
                    // Set integer vertex of corner on 'central' box
                    i.set(iCorner & 1 ? 1 : 0, iCorner & 2 ? 1 : 0, iCorner & 4 ? 1 : 0);

                    for (auto jCorner = 0; jCorner < 8; ++jCorner)
                    {
                        // Set integer vertex of corner on 'other' box
                        j.set(x + (jCorner & 1 ? 1 : 0), y + (jCorner & 2 ? 1 : 0), z + (jCorner & 4 ? 1 : 0));

                        // Get minimum image of vertex j w.r.t. i
                        j = mimGridDelta(j - i);

                        r.set(j.x, j.y, j.z);
                        r = cellAxes * r;
                        if (r.magnitude() < pairPotentialRange)
                        {
                            close = true;
                            break;
                        }
                    }
                    if (close)
                        break;
                }
                if (!close)
                    continue;

                // Check that the cell is not already in the list by querying the cellNbrs RefList
                nbr = cell(x, y, z);
                if (cellNbrs.contains(nbr))
                    continue;
                neighbourIndices.emplace_back(x, y, z);
                cellNbrs.append(nbr);
            }
        }
    }
    Messenger::print("Added {} Cells to representative neighbour list.\n", neighbourIndices.size());

    // Construct neighbour arrays for individual Cells
    neighbours_.clear();
    neighbours_.resize(cells_.size());
    for (auto &nbrVector : neighbours_)
        nbrVector.reserve(neighbourIndices.size() + 1);

    for (auto &cell : cells_)
    {
        addNeighbour(cell, cell, false);

        for (auto &indices : neighbourIndices)
        {
            // Find neighbour with the relative indices provided
            auto *nbr = this->cell(cell.gridReference().x + indices.x, cell.gridReference().y + indices.y,
                                   cell.gridReference().z + indices.z);
            addNeighbour(cell, *nbr, minimumImageRequired(&cell, nbr, pairPotentialRange));
        }
    }

    // Finally, loop over Cells and set neighbours, and construct neighbour matrix
    Messenger::print("Constructing neighbour lists for individual Cells...\n");
    std::vector<const Cell *> nearNeighbours, mimNeighbours;
    Vec3<int> gridRef, delta;
    for (auto &cell : cells_)
    {
        // Grab grid reference of central cell
        gridRef = cell.gridReference();

        // Clear neighbour lists
        nearNeighbours.clear();
        mimNeighbours.clear();

        // Loop over list of (relative) neighbour cell indices
        for (auto nbrIndices : neighbourIndices)
        {
            // Retrieve Cell pointer
            nbr = this->cell(gridRef.x + nbrIndices.x, gridRef.y + nbrIndices.y, gridRef.z + nbrIndices.z);
            if (box_->type() == Box::BoxType::NonPeriodic)
                nearNeighbours.emplace_back(nbr);
            else if (minimumImageRequired(&cell, nbr, pairPotentialRange))
                mimNeighbours.emplace_back(nbr);
            else
                nearNeighbours.emplace_back(nbr);
        }

        // Set up lists in the cell
        cell.addCellNeighbours(nearNeighbours, mimNeighbours);
    }
    createCellNeighbourPairs();
    return true;
}

// Clear Cell arrays
void CellArray::clear() { cells_.clear(); }

/*
 * Operations
 */

// Scale Cells by supplied factors along each axis
void CellArray::scale(Vec3<double> scaleFactors)
{
    realCellSize_.multiply(scaleFactors);
    axes_.columnMultiply(scaleFactors);
}
