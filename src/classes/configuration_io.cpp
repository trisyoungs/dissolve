// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "classes/box.h"
#include "classes/configuration.h"
#include "classes/species.h"
#include <algorithm>

// Write through specified LineParser
bool Configuration::serialise(LineParser &parser) const
{
    if (!parser.writeLineF("'{}'  {}  # nMolecules\n", name(), molecules_.size()))
        return false;

    // Write unit cell (box) lengths and angles
    const auto lengths = box()->axisLengths();
    const auto angles = box()->axisAngles();
    if (!parser.writeLineF("{:12e} {:12e} {:12e}  {}  {}  {}\n", lengths.x, lengths.y, lengths.z, appliedSizeFactor_,
                           requestedSizeFactor_, DissolveSys::btoa(box()->type() == Box::BoxType::NonPeriodic)))
        return false;
    if (!parser.writeLineF("{:12e} {:12e} {:12e}\n", angles.x, angles.y, angles.z))
        return false;

    // Write total number of Molecules
    if (!parser.writeLineF("{}\n", molecules_.size()))
        return false;

    // Write Molecule types - write sequential Molecules with same type as single line
    auto moleculeCount = 0;
    const Species *lastType = nullptr;
    for (const auto &molecule : molecules_)
    {
        // If the last Molecule's Species is the same as this one, increment counter and move on
        if (lastType == molecule->species())
        {
            ++moleculeCount;
            continue;
        }

        // Species is different between this molecule and the last - write this info, and reset the counter
        if (lastType && (!parser.writeLineF("{}  '{}'\n", moleculeCount, lastType->name())))
            return false;
        moleculeCount = 1;
        lastType = molecule->species();
    }
    // Write final molecule count / type
    if ((moleculeCount > 0) && (!parser.writeLineF("{}  '{}'\n", moleculeCount, lastType->name())))
        return false;

    // Write all Atoms - for each write index and coordinates
    if (!parser.writeLineF("{}  # nAtoms\n", atoms_.size()))
        return false;
    for (const auto &i : atoms_)
    {
        if (!parser.writeLineF("{} {} {} {}\n", i->molecule()->arrayIndex(), i->x(), i->y(), i->z()))
            return false;
    }

    return true;
}

// Read through specified LineParser
bool Configuration::read(LineParser &parser, const std::vector<std::unique_ptr<Species>> &availableSpecies,
                         double pairPotentialRange)
{
    // Clear current contents of Configuration
    empty();

    // Read configuration name and nMolecules
    if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
        return false;
    setName(parser.argsv(0));

    /*
     * Read box definition
     * Lengths, along with atomic coordinates, reflect the applied size factor.
     * Create box with unscaled lengths - they will be scaled according to the size factor at the end of the routine.
     */
    if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
        return false;
    appliedSizeFactor_ = parser.argd(3);
    requestedSizeFactor_ = parser.argd(4);
    auto nonPeriodic = parser.argb(5);
    const auto lengths = parser.arg3d(0) / appliedSizeFactor_;

    if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
        return false;
    const auto angles = parser.arg3d(0);

    createBox(lengths, angles, nonPeriodic);

    // Read total number of Molecules to expect
    if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
        return false;
    const auto expectedNMols = parser.argi(0);

    // Read Species types for Molecules
    auto nMolsRead = 0;
    while (nMolsRead < expectedNMols)
    {
        // Read line containing number of molecules and Species name
        if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
            return false;

        auto it = std::find_if(availableSpecies.cbegin(), availableSpecies.cend(),
                               [&](const auto &sp) { return DissolveSys::sameString(sp->name(), parser.argsv(1)); });

        if (it == availableSpecies.cend())
        {
            return Messenger::error("Unrecognised Species '{}' found in Configuration '{}' in restart file.\n", parser.argsv(1),
                                    name());
        }
        auto &sp = *it;

        // Set Species pointers for this range of Molecules
        auto nMols = parser.argi(0);
        for (auto n = 0; n < nMols; ++n)
            addMolecule(sp.get());

        // Increase our counter
        nMolsRead += parser.argi(0);
    }

    // Read in Atoms
    if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
        return false;
    auto nAtoms = parser.argi(0);
    for (auto n = 0; n < nAtoms; ++n)
    {
        // Each line contains molecule ID and coordinates only
        if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
            return false;

        atom(n)->setCoordinates(parser.arg3d(1));
    }

    // Finalise used AtomType list
    usedAtomTypes_.finalise();

    // Set-up Cells for the Box
    cells_.generate(box_.get(), requestedCellDivisionLength_, pairPotentialRange);

    // Scale box and cells according to the applied size factor
    scaleBox({appliedSizeFactor_, appliedSizeFactor_, appliedSizeFactor_});

    // Update Cell locations for Atoms
    updateCellContents();

    return true;
}
