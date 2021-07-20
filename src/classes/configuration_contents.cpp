// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/atomtype.h"
#include "classes/box.h"
#include "classes/cell.h"
#include "classes/configuration.h"
#include "classes/species.h"
#include <memory>

// Clear contents of Configuration, leaving other definitions intact
void Configuration::empty()
{
    molecules_.clear();
    atoms_.clear();
    usedAtomTypes_.clear();
    box_ = std::make_unique<CubicBox>(1.0);
    cells_.clear();
    appliedSizeFactor_ = 1.0;
    speciesPopulations_.clear();

    ++contentsVersion_;
}

// Return specified used type
std::shared_ptr<AtomType> Configuration::usedAtomType(int index) { return usedAtomTypes_.atomType(index); }

// Return specified used type data
AtomTypeData &Configuration::usedAtomTypeData(int index) { return usedAtomTypes_[index]; }

// Return first AtomTypeData for this Configuration
const AtomTypeData &Configuration::usedAtomTypes() const { return usedAtomTypes_.first(); }

// Return AtomTypeList for this Configuration
const AtomTypeList &Configuration::usedAtomTypesList() const { return usedAtomTypes_; }

// Return number of atom types used in this Configuration
int Configuration::nUsedAtomTypes() const { return usedAtomTypes_.nItems(); }

// Adjust population of specified Species in the Configuration
void Configuration::adjustSpeciesPopulation(const Species *sp, int delta)
{
    auto it = std::find_if(speciesPopulations_.begin(), speciesPopulations_.end(),
                           [sp](const auto &data) { return data.first == sp; });
    if (it == speciesPopulations_.end())
    {
        if (delta < 0)
            throw(std::runtime_error(
                fmt::format("Can't decrease population of Species '{}' as it is not in the list.\n", sp->name())));
        speciesPopulations_.emplace_back(sp, delta);
    }
    else
        it->second += delta;
}

// Return Species populations within the Configuration
const std::vector<std::pair<const Species *, int>> &Configuration::speciesPopulations() const { return speciesPopulations_; }

// Return if the specified Species is present in the Configuration
bool Configuration::containsSpecies(const Species *sp)
{
    return std::find_if(speciesPopulations_.begin(), speciesPopulations_.end(),
                        [sp](const auto &data) { return data.first == sp; }) != speciesPopulations_.end();
}

// Return the total atomic mass present in the Configuration
double Configuration::atomicMass() const
{
    double mass = 0.0;

    // Get total molar mass in configuration
    for (const auto &spPop : speciesPopulations_)
        mass += spPop.first->mass() * spPop.second;

    // Convert to absolute mass
    return mass / AVOGADRO;
}

// Return the atomic density of the Configuration
double Configuration::atomicDensity() const { return nAtoms() / box_->volume(); }

// Return the chemical density (g/cm3) of the Configuration
double Configuration::chemicalDensity() const { return atomicMass() / (box_->volume() / 1.0E24); }

// Return version of current contents
int Configuration::contentsVersion() const { return contentsVersion_; }

// Increment version of current contents
void Configuration::incrementContentsVersion() { ++contentsVersion_; }

// Add Molecule to Configuration based on the supplied Species
std::shared_ptr<Molecule>
Configuration::addMolecule(const Species *sp, OptionalReferenceWrapper<const std::vector<Vec3<double>>> sourceCoordinates)
{
    // Create the new Molecule object and set its Species pointer
    std::shared_ptr<Molecule> newMolecule = std::make_shared<Molecule>();
    newMolecule->setArrayIndex(molecules_.size());
    molecules_.push_back(newMolecule);
    newMolecule->setSpecies(sp);

    // Update the relevant Species population
    adjustSpeciesPopulation(sp, 1);

    // Add Atoms from Species to the Molecule, using either species coordinates or those from the source CoordinateSet
    if (sourceCoordinates)
    {
        auto r = sourceCoordinates->get();
        for (auto n = 0; n < sp->nAtoms(); ++n)
            addAtom(&sp->atom(n), newMolecule, r[n]);
    }
    else
    {
        for (auto n = 0; n < sp->nAtoms(); ++n)
            addAtom(&sp->atom(n), newMolecule, sp->atom(n).r());
    }
    return newMolecule;
}

// Remove all Molecules of the target Species from the Configuration
void Configuration::removeMolecules(const Species *sp)
{
    molecules_.erase(std::remove_if(molecules_.begin(), molecules_.end(),
                                    [&, sp](auto &mol) {
                                        if (mol->species() == sp)
                                        {
                                            for (auto &i : mol->atoms())
                                                atoms_.erase(std::find(atoms_.begin(), atoms_.end(), i));
                                            adjustSpeciesPopulation(mol->species(), -1);
                                            return true;
                                        }
                                        else
                                            return false;
                                    }),
                     molecules_.end());
}

// Remove specified Molecules from the Configuration
void Configuration::removeMolecules(const std::vector<std::shared_ptr<Molecule>> &molecules)
{
    molecules_.erase(std::remove_if(molecules_.begin(), molecules_.end(),
                                    [&, molecules](const auto &mol) {
                                        if (std::find(molecules.begin(), molecules.end(), mol) != molecules.end())
                                        {
                                            for (auto &i : mol->atoms())
                                                atoms_.erase(std::find(atoms_.begin(), atoms_.end(), i));
                                            adjustSpeciesPopulation(mol->species(), -1);
                                            return true;
                                        }
                                        else
                                            return false;
                                    }),
                     molecules_.end());
}

// Return number of Molecules in Configuration
int Configuration::nMolecules() const { return molecules_.size(); }

// Return Molecule vector
std::vector<std::shared_ptr<Molecule>> &Configuration::molecules() { return molecules_; }
const std::vector<std::shared_ptr<Molecule>> &Configuration::molecules() const { return molecules_; }

// Return nth Molecule
std::shared_ptr<Molecule> Configuration::molecule(int n) { return molecules_[n]; }

// Add new Atom to Configuration, with Molecule parent specified
std::shared_ptr<Atom> Configuration::addAtom(const SpeciesAtom *sourceAtom, const std::shared_ptr<Molecule> &molecule,
                                             Vec3<double> r)
{
    // Create new Atom object and set its source pointer
    auto newAtom = std::make_shared<Atom>();
    newAtom->setArrayIndex(atoms_.size());
    atoms_.push_back(newAtom);
    newAtom->setSpeciesAtom(sourceAtom);

    // Register the Atom in the specified Molecule (this will also set the Molecule pointer in the Atom)
    molecule->addAtom(newAtom);

    // Set the position
    newAtom->setCoordinates(r);

    // Update our typeIndex (non-isotopic) and set local and master type indices
    AtomTypeData &atd = usedAtomTypes_.add(sourceAtom->atomType(), 1);
    newAtom->setLocalTypeIndex(atd.listIndex());
    newAtom->setMasterTypeIndex(sourceAtom->atomType()->index());

    return newAtom;
}

// Return number of Atoms in Configuration
int Configuration::nAtoms() const { return atoms_.size(); }

// Return Atom array
std::vector<std::shared_ptr<Atom>> &Configuration::atoms() { return atoms_; }

const std::vector<std::shared_ptr<Atom>> &Configuration::atoms() const { return atoms_; }

// Return nth atom
std::shared_ptr<Atom> Configuration::atom(int n)
{
    assert(n >= 0 && n < atoms_.size());
    return atoms_[n];
}

// Scale contents of the box by the specified factors along each axis
void Configuration::scaleContents(Vec3<double> scaleFactors)
{
    Vec3<double> r, cog;
    for (auto &mol : molecules_)
    {
        // If the related species has a periodic box, scale atom positions rather than COG position
        if (mol->species()->box()->type() != Box::BoxType::NonPeriodic)
        {
            for (auto &i : mol->atoms())
            {
                r = i->r();
                box()->toFractional(r);
                r.multiply(scaleFactors);
                box()->toReal(r);
                i->setCoordinates(r);
            }
        }
        else
        {
            // First, work out the centre of geometry of the Molecule, and fold it into the Box
            cog = box()->fold(mol->centreOfGeometry(box()));

            // Scale centre of geometry along axes by supplied factors
            r = cog;
            box()->toFractional(r);
            r.multiply(scaleFactors);
            box()->toReal(r);

            // Loop over Atoms in Molecule, setting new coordinates as we go
            for (auto &i : mol->atoms())
                i->setCoordinates(r + box()->minimumVector(i->r(), cog));
        }
    }
}
