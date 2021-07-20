// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "procedure/nodes/add.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "classes/box.h"
#include "classes/configuration.h"
#include "classes/coredata.h"
#include "classes/species.h"
#include "keywords/types.h"
#include "procedure/nodes/regionbase.h"

AddProcedureNode::AddProcedureNode(const Species *sp, const NodeValue &population, const NodeValue &density,
                                   Units::DensityUnits densityUnits)
    : ProcedureNode(ProcedureNode::NodeType::Add)
{
    // Set up keywords
    keywords_.add("Control", new SpeciesKeyword(sp), "Species", "Target species to add");
    keywords_.add("Control", new NodeValueKeyword(this, population), "Population", "Population of the target species to add");
    keywords_.add("Control",
                  new EnumOptionsKeyword<AddProcedureNode::BoxActionStyle>(boxActionStyles() =
                                                                               AddProcedureNode::BoxActionStyle::AddVolume),
                  "BoxAction", "Action to take on the Box geometry / volume on addition of the species");
    keywords_.add("Control", new BoolKeyword(true), "ScaleA", "Scale box length A when modifying volume");
    keywords_.add("Control", new BoolKeyword(true), "ScaleB", "Scale box length B when modifying volume");
    keywords_.add("Control", new BoolKeyword(true), "ScaleC", "Scale box length C when modifying volume");
    keywords_.add("Control",
                  new NodeValueEnumOptionsKeyword<Units::DensityUnits>(this, density, Units::densityUnits() = densityUnits),
                  "Density", "Density at which to add the target species");
    keywords_.add("Control", new BoolKeyword(true), "Rotate", "Whether to randomly rotate molecules on insertion");
    keywords_.add("Control",
                  new EnumOptionsKeyword<AddProcedureNode::PositioningType>(positioningTypes() =
                                                                                AddProcedureNode::PositioningType::Random),
                  "Positioning", "Positioning type for individual molecules");
    keywords_.add("Control", new NodeKeyword(this, ProcedureNode::NodeClass::Region, true), "Region",
                  "Region into which to add the species");
}

/*
 * Identity
 */

// Return whether specified context is relevant for this node type
bool AddProcedureNode::isContextRelevant(ProcedureNode::NodeContext context)
{
    return (context == ProcedureNode::GenerationContext);
}

// Return whether a name for the node must be provided
bool AddProcedureNode::mustBeNamed() const { return false; }

/*
 * Node Data
 */

// Return enum option info for PositioningType
EnumOptions<AddProcedureNode::BoxActionStyle> AddProcedureNode::boxActionStyles()
{
    return EnumOptions<AddProcedureNode::BoxActionStyle>("BoxAction",
                                                         {{AddProcedureNode::BoxActionStyle::None, "None"},
                                                          {AddProcedureNode::BoxActionStyle::AddVolume, "AddVolume"},
                                                          {AddProcedureNode::BoxActionStyle::ScaleVolume, "ScaleVolume"},
                                                          {AddProcedureNode::BoxActionStyle::Set, "Set"}});
}

// Return enum option info for PositioningType
EnumOptions<AddProcedureNode::PositioningType> AddProcedureNode::positioningTypes()
{
    return EnumOptions<AddProcedureNode::PositioningType>("PositioningType",
                                                          {{AddProcedureNode::PositioningType::Central, "Central"},
                                                           {AddProcedureNode::PositioningType::Current, "Current"},
                                                           {AddProcedureNode::PositioningType::Random, "Random"},
                                                           {AddProcedureNode::PositioningType::Region, "Region"}});
}

/*
 * Execute
 */

// Prepare any necessary data, ready for execution
bool AddProcedureNode::prepare(Configuration *cfg, std::string_view prefix, GenericList &targetList)
{
    // If positioning type is 'Region', must have a suitable node defined
    auto positioning = keywords_.enumeration<AddProcedureNode::PositioningType>("Positioning");
    auto *regionNode = keywords_.retrieve<const ProcedureNode *>("Region");
    if (positioning == AddProcedureNode::PositioningType::Region && !regionNode)
        return Messenger::error("A valid region must be specified with the 'Region' keyword.\n");
    else if (positioning != AddProcedureNode::PositioningType::Region && regionNode)
        Messenger::warn("A region has been specified ({}) but the positioning type is set to '{}'\n", regionNode->name(),
                        AddProcedureNode::positioningTypes().keyword(positioning));

    // Check scalable axes definitions
    if (!keywords_.asBool("ScaleA") && !keywords_.asBool("ScaleB") && !keywords_.asBool("ScaleC"))
        return Messenger::error("Must have at least one scalable box axis!\n");

    return true;
}

// Execute node, targetting the supplied Configuration
bool AddProcedureNode::execute(ProcessPool &procPool, Configuration *cfg, std::string_view prefix, GenericList &targetList)
{
    auto requestedPopulation = keywords_.asInt("Population");
    auto *sp = keywords_.retrieve<const Species *>("Species");
    if (!sp)
        return Messenger::error("No Species set in Add node.\n");
    const auto nAtomsToAdd = requestedPopulation * sp->nAtoms();

    // Can't add the Species if it has any missing core information
    if (!sp->checkSetUp())
        return Messenger::error("Can't add Species '{}' because it is not set up correctly.\n", sp->name());

    Messenger::print("[Add] Adding species '{}' - population is {}.\n", sp->name(), requestedPopulation);

    // If a density was not given, just add new molecules to the current box without adjusting its size
    auto &densityAndUnits = keywords_.retrieve<Venum<NodeValue, Units::DensityUnits>>("Density");
    double density = densityAndUnits.value().asDouble();
    auto boxAction = keywords_.enumeration<AddProcedureNode::BoxActionStyle>("BoxAction");
    Vec3<bool> scalableAxes(keywords_.asBool("ScaleA"), keywords_.asBool("ScaleB"), keywords_.asBool("ScaleC"));
    if (boxAction == AddProcedureNode::BoxActionStyle::None)
        Messenger::print("[Add] Current box geometry / volume will remain as-is.\n");
    else if (boxAction == AddProcedureNode::BoxActionStyle::AddVolume)
    {
        Messenger::print("[Add] Current box volume will be increased to accommodate volume of new species.\n");

        // Get current cell volume
        auto currentVolume = cfg->box()->volume();

        // Determine volume required to contain the population of the specified Species at the requested density
        auto requiredVolume = 0.0;
        if (densityAndUnits.enumeration() == Units::AtomsPerAngstromUnits)
            requiredVolume = nAtomsToAdd / density;
        else
            requiredVolume = ((sp->mass() * requestedPopulation) / AVOGADRO) / (density / 1.0E24);

        Messenger::print("[Add] Density for new species is {} {}.\n", density,
                         Units::densityUnits().keyword(densityAndUnits.enumeration()));
        Messenger::print("[Add] Required volume for new species is {} cubic Angstroms.\n", requiredVolume);

        // If the current box has no atoms in it, absorb the current volume rather than adding to it
        if (cfg->nAtoms() > 0)
            requiredVolume += currentVolume;
        else
            Messenger::print("[Add] Current box is empty, so new volume will be set to exactly {} cubic Angstroms.\n",
                             requiredVolume);

        auto scaleFactors = cfg->box()->scaleFactors(requiredVolume, scalableAxes);

        // Scale existing contents
        cfg->scaleContents(scaleFactors);

        // Scale the current Box so there is enough space for our new species
        cfg->scaleBox(scaleFactors);

        Messenger::print("[Add] New box volume is {:e} cubic Angstroms - scale factors were ({},{},{}).\n",
                         cfg->box()->volume(), scaleFactors.x, scaleFactors.y, scaleFactors.z);
    }
    else if (boxAction == AddProcedureNode::BoxActionStyle::ScaleVolume)
    {
        Messenger::print("[Add] Box volume will be set to give supplied density.\n");

        // Get volume required to hold current cell contents at the requested density
        double existingRequiredVolume = 0.0;
        if (densityAndUnits.enumeration() == Units::AtomsPerAngstromUnits)
            existingRequiredVolume = cfg->nAtoms() / density;
        else
            existingRequiredVolume = cfg->atomicMass() / (density / 1.0E24);
        Messenger::print("[Add] Existing contents requires volume of {} cubic Angstroms at specified density.\n",
                         existingRequiredVolume);

        // Determine volume required to contain the population of the specified Species at the requested density
        auto requiredVolume = 0.0;
        if (densityAndUnits.enumeration() == Units::AtomsPerAngstromUnits)
            requiredVolume = nAtomsToAdd / density;
        else
            requiredVolume = ((sp->mass() * requestedPopulation) / AVOGADRO) / (density / 1.0E24);

        Messenger::print("[Add] Required volume for new species is {} cubic Angstroms.\n", requiredVolume);

        // Add on required volume for existing box contents
        if (cfg->nAtoms() > 0)
            requiredVolume += existingRequiredVolume;

        auto scaleFactors = cfg->box()->scaleFactors(requiredVolume, scalableAxes);

        // Scale existing contents
        cfg->scaleContents(scaleFactors);

        // Scale the current Box so there is enough space for our new species
        cfg->scaleBox(scaleFactors);

        Messenger::print("[Add] Current box scaled by ({},{},{}) - new volume is {:e} cubic Angstroms.\n", scaleFactors.x,
                         scaleFactors.y, scaleFactors.z, cfg->box()->volume());
    }
    else if (boxAction == AddProcedureNode::BoxActionStyle::Set)
    {
        Messenger::print("[Add] Box geometry will be set from the species box definition.\n");
        if (sp->box()->type() == Box::BoxType::NonPeriodic)
            return Messenger::error("Target species '{}' is not periodic!.\n", sp->name());

        cfg->createBox(sp->box()->axisLengths(), sp->box()->axisAngles());

        Messenger::print("[Add] Box type is now {}: A = {:10.4e} B = {:10.4e} C = {:10.4e}, alpha = {:10.4e} beta = "
                         "{:10.4e} gamma = {:10.4e}\n",
                         Box::boxTypes().keyword(cfg->box()->type()), cfg->box()->axisLengths().x, cfg->box()->axisLengths().y,
                         cfg->box()->axisLengths().z, cfg->box()->axisAngles().x, cfg->box()->axisAngles().y,
                         cfg->box()->axisAngles().z);

        // Check on the requestedPopulation - we can have exactly one copy and no more
        if (requestedPopulation > 1)
        {
            Messenger::warn("Population for species '{}' reset to 1.\n", sp->name());
            requestedPopulation = 0;
        }
    }

    // Get the positioning type and rotation flag
    auto positioning = keywords_.enumeration<AddProcedureNode::PositioningType>("Positioning");
    auto *regionNode = dynamic_cast<const RegionProcedureNodeBase *>(keywords_.retrieve<const ProcedureNode *>("Region"));
    auto rotate = keywords_.asBool("Rotate");

    Messenger::print("[Add] Positioning type is '{}' and rotation is {}.\n",
                     AddProcedureNode::positioningTypes().keyword(positioning), rotate ? "on" : "off");

    // Now we add the molecules
    procPool.initialiseRandomBuffer(ProcessPool::PoolProcessesCommunicator);
    Vec3<double> r, cog, newCentre, fr;
    auto coordSetIt = sp->coordinateSets().begin();
    Matrix3 transform;
    const auto *box = cfg->box();
    for (auto n = 0; n < requestedPopulation; ++n)
    {
        // Add the Molecule - use coordinate set if one is available
        std::shared_ptr<Molecule> mol;
        if (coordSetIt != sp->coordinateSets().end())
        {
            mol = cfg->addMolecule(sp, *coordSetIt);

            // Move to next coordinate set
            ++coordSetIt;
            if (coordSetIt == sp->coordinateSets().end())
                coordSetIt = sp->coordinateSets().begin();
        }
        else
            mol = cfg->addMolecule(sp);

        // Set / generate position of Molecule
        switch (positioning)
        {
            case (AddProcedureNode::PositioningType::Random):
                fr.set(procPool.random(), procPool.random(), procPool.random());
                newCentre = box->getReal(fr);
                mol->setCentreOfGeometry(box, newCentre);
                break;
            case (AddProcedureNode::PositioningType::Region):
                mol->setCentreOfGeometry(box, box->getReal(regionNode->randomFractionalCoordinate()));
                break;
            case (AddProcedureNode::PositioningType::Central):
                fr.set(0.5, 0.5, 0.5);
                newCentre = box->getReal(fr);
                mol->setCentreOfGeometry(box, newCentre);
                break;
            case (AddProcedureNode::PositioningType::Current):
                break;
            default:
                Messenger::error("Unrecognised positioning type.\n");
                break;
        }

        // Generate and apply a random rotation matrix
        if (rotate)
        {
            transform.createRotationXY(procPool.randomPlusMinusOne() * 180.0, procPool.randomPlusMinusOne() * 180.0);
            mol->transform(box, transform);
        }
    }

    Messenger::print("[Add] New box density is {:e} cubic Angstroms ({} g/cm3).\n", cfg->atomicDensity(),
                     cfg->chemicalDensity());

    return true;
}
