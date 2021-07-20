// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "base/version.h"
#include "classes/atom.h"
#include "classes/atomtypelist.h"
#include "classes/box.h"
#include "classes/cellarray.h"
#include "classes/molecule.h"
#include "classes/sitestack.h"
#include "genericitems/list.h"
#include "io/import/coordinates.h"
#include "math/data1d.h"
#include "math/histogram1d.h"
#include "math/interpolator.h"
#include "module/layer.h"
#include "procedure/procedure.h"
#include "templates/vector3.h"
#include <deque>
#include <memory>

// Forward Declarations
class Cell;
class PotentialMap;
class ProcessPool;
class Species;

// Configuration
class Configuration
{
    public:
    Configuration();
    ~Configuration();
    Configuration(const Configuration &source) = delete;
    Configuration(Configuration &&source) = delete;
    void operator=(Configuration &source) = delete;
    // Clear all data
    void clear();

    /*
     * Definition
     */
    private:
    // Name of the Configuration
    std::string name_;
    // Nice name (generated from name_) used for output files
    std::string niceName_;
    // Procedure to generate the Configuration
    Procedure generator_;
    // File / format of input coordinates file, if provided
    CoordinateImportFileFormat inputCoordinates_;
    // Temperature of this configuration (K)
    double temperature_;

    public:
    // Set name of the Configuration
    void setName(std::string_view name);
    // Return name of the Configuration
    std::string_view name() const;
    // Return nice name of the Configuration
    std::string_view niceName() const;
    // Return the current generator
    Procedure &generator();
    // Create the Configuration according to its generator Procedure
    bool generate(ProcessPool &procPool, double pairPotentialRange);
    // Return import coordinates file / format
    CoordinateImportFileFormat &inputCoordinates();
    // Initialise (generate or load) the basic contents of the Configuration
    bool initialiseContent(ProcessPool &procPool, double pairPotentialRange, bool emptyCurrentContent = false);
    // Set configuration temperature
    void setTemperature(double t);
    // Return configuration temperature
    double temperature() const;

    /*
     * Content
     */
    private:
    // Species populations present in the Configuration
    std::vector<std::pair<const Species *, int>> speciesPopulations_;
    // AtomType list, containing unique (non-isotopic) atom types over all Species used in this configuration
    AtomTypeList usedAtomTypes_;
    // Contents version, incremented whenever Configuration content or Atom positions change
    VersionCounter contentsVersion_;
    // Molecule vector
    std::vector<std::shared_ptr<Molecule>> molecules_;
    // Atom vector
    std::vector<std::shared_ptr<Atom>> atoms_;

    public:
    // Empty contents of Configuration, leaving core definitions intact
    void empty();
    // Return specified used type
    std::shared_ptr<AtomType> usedAtomType(int index);
    // Return specified used type data
    AtomTypeData &usedAtomTypeData(int index);
    // Return first AtomTypeData for this Configuration
    const AtomTypeData &usedAtomTypes() const;
    // Return AtomTypeList for this Configuration
    const AtomTypeList &usedAtomTypesList() const;
    // Return number of atom types used in this Configuration
    int nUsedAtomTypes() const;
    // Adjust population of specified Species in the Configuration
    void adjustSpeciesPopulation(const Species *sp, int delta);
    // Return Species populations within the Configuration
    const std::vector<std::pair<const Species *, int>> &speciesPopulations() const;
    // Return if the specified Species is present in the Configuration
    bool containsSpecies(const Species *sp);
    // Return the total atomic mass present in the Configuration
    double atomicMass() const;
    // Return the atomic density of the Configuration
    double atomicDensity() const;
    // Return the chemical density (g/cm3) of the Configuration
    double chemicalDensity() const;
    // Return version of current contents
    int contentsVersion() const;
    // Increment version of current contents
    void incrementContentsVersion();
    // Add Molecule to Configuration based on the supplied Species
    std::shared_ptr<Molecule>
    addMolecule(const Species *sp, OptionalReferenceWrapper<const std::vector<Vec3<double>>> sourceCoordinates = std::nullopt);
    // Remove all Molecules of the target Species from the Configuration
    void removeMolecules(const Species *sp);
    // Remove specified Molecules from the Configuration
    void removeMolecules(const std::vector<std::shared_ptr<Molecule>> &molecules);
    // Return number of Molecules in Configuration
    int nMolecules() const;
    // Return Molecule vector
    std::vector<std::shared_ptr<Molecule>> &molecules();
    const std::vector<std::shared_ptr<Molecule>> &molecules() const;
    // Return nth Molecule
    std::shared_ptr<Molecule> molecule(int n);
    // Add new Atom to Configuration
    std::shared_ptr<Atom> addAtom(const SpeciesAtom *sourceAtom, const std::shared_ptr<Molecule> &molecule,
                                  Vec3<double> r = Vec3<double>());
    // Return number of Atoms in Configuration
    int nAtoms() const;
    // Return Atom array
    std::vector<std::shared_ptr<Atom>> &atoms();
    const std::vector<std::shared_ptr<Atom>> &atoms() const;
    // Return nth Atom
    std::shared_ptr<Atom> atom(int n);
    // Scale contents of the box by the specified factors along each axis
    void scaleContents(Vec3<double> scaleFactors);

    /*
     * Periodic Box and Cells
     */
    private:
    // Requested size factor for Box
    double requestedSizeFactor_;
    // Size factor currently applied to Box / Cells
    double appliedSizeFactor_;
    // Periodic Box
    std::unique_ptr<Box> box_;
    // Requested side length for individual Cell
    double requestedCellDivisionLength_;
    // Cell array
    CellArray cells_;

    public:
    // Create Box definition with specified lengths and angles
    void createBox(const Vec3<double> lengths, const Vec3<double> angles, bool nonPeriodic = false);
    // Create Box definition from axes matrix
    void createBox(const Matrix3 axes);
    // Return Box
    const Box *box() const;
    // Scale Box lengths (and associated Cells) by specified factors
    void scaleBox(Vec3<double> scaleFactors);
    // Set requested size factor for Box
    void setRequestedSizeFactor(double factor);
    // Return requested size factor for Box
    double requestedSizeFactor() const;
    // Return last size factor applied to Box / Cells
    double appliedSizeFactor() const;
    // Set requested side length for individual Cell
    void setRequestedCellDivisionLength(double a);
    // Return requested side length for individual Cell
    double requestedCellDivisionLength() const;
    // Return cell array
    CellArray &cells();
    const CellArray &cells() const;
    // Scale Box, Cells, and Molecule geometric centres according to current size factor
    void applySizeFactor(const PotentialMap &potentialMap);

    /*
     * Upkeep
     */
    public:
    // Update Cell contents
    void updateCellContents();
    // Update Cell location of specified Atom
    void updateCellLocation(Atom *i);
    // Update Cell location of specified Molecule
    void updateCellLocation(const std::shared_ptr<Molecule> &mol);
    // Update Cell location of specified Atom indices
    void updateCellLocation(const std::vector<int> &targetAtoms, int indexOffset);

    /*
     * Site Stacks
     */
    private:
    // List of current SiteStacks
    std::vector<std::unique_ptr<SiteStack>> siteStacks_;

    public:
    // Calculate / retrieve stack of sites for specified SpeciesSite
    const SiteStack *siteStack(const SpeciesSite *site);

    /*
     * I/O
     */
    public:
    // Write through specified LineParser
    bool serialise(LineParser &parser) const;
    // Read through specified LineParser
    bool read(LineParser &parser, const std::vector<std::unique_ptr<Species>> &availableSpecies, double pairPotentialRange);

    /*
     * Parallel Comms
     */
    private:
    // Process pool for this Configuration
    ProcessPool processPool_;

    public:
    // Set up process pool for this Configuration
    bool setUpProcessPool(const std::vector<int> &worldRanks);
    // Return process pool for this Configuration
    ProcessPool &processPool();
};
