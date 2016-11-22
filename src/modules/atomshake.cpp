/*
	*** AtomShake Module
	*** src/modules/atomshake.cpp
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

#include "main/duq.h"
#include "modules/atomshake.h"
#include "classes/box.h"
#include "classes/cell.h"
#include "classes/changestore.h"
#include "classes/energykernel.h"
#include "base/processpool.h"
#include "base/timer.h"
#include "math/matrix4.h"

// Static Members
List<Module> AtomShake::instances_;

/*
 * Constructor / Destructor
 */

// Constructor
AtomShake::AtomShake() : Module()
{
	// Add to instances list and set unique name for this instance
	uniqueName_.sprintf("%s_%02i", name(), instances_.nItems());
	instances_.own(this);

	// Setup variables / control parameters
	addVariable("CutoffDistance", -1.0);
	addVariable("ShakesPerAtom", 1);
	addVariable("TargetAcceptanceRate", 0.33);
	addVariable("StepSize", 0.05);
}

// Destructor
AtomShake::~AtomShake()
{
}

/*
 * Instances
 */

// Create instance of this module
List<Module>& AtomShake::instances()
{
	return instances_;
}

// Create instance of this module
Module* AtomShake::createInstance()
{
	return new AtomShake;
}

/*
 * Definition
 */

// Return name of module
const char* AtomShake::name()
{
	return "AtomShake";
}

// Return brief description of module
const char* AtomShake::brief()
{
	return "Perform atomic Monte Carlo on all atoms";
}

// Return instance type for module
Module::InstanceType AtomShake::instanceType()
{
	return Module::MultipleInstance;
}

// Whether the Module has a pre-processing stage
bool AtomShake::hasPreProcessing()
{
	return false;
}

// Whether the Module has a processing stage
bool AtomShake::hasProcessing()
{
	return true;
}

// Whether the Module has a post-processing stage
bool AtomShake::hasPostProcessing()
{
	return false;
}

// Modules upon which this Module depends to have run first
const char* AtomShake::dependentModules()
{
	return "";
}

/*
 * Targets
 */

// Return the maximum number of Configurations the Module can target (or -1 for any number)
int AtomShake::nTargetableConfigurations()
{
	return 1;
}

// Return the maximum number of Samples the Module can target (or -1 for any number)
int AtomShake::nTargetableSamples()
{
	return 0;
}

/*
 * Method
 */

// Perform setup tasks for module
bool AtomShake::setup(ProcessPool& procPool)
{
	return true;
}

// Execute pre-processing stage
bool AtomShake::preProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}

// Execute Method
bool AtomShake::process(DUQ& duq, ProcessPool& procPool)
{
	/*
	* Perform an Atom shake
	* 
	* This is a parallel routine, with processes operating in groups.
	 */

	// Get target Configuration
	Configuration* cfg = targetConfigurations_.firstItem();

	// Retrieve control parameters from Configuration
	const double cutoffDistance = variableAsDouble(cfg, "CutoffDistance") < 0.0 ? duq.pairPotentialRange() : variableAsDouble(cfg, "CutoffDistance");
	const int nShakesPerAtom = variableAsInt(cfg, "ShakesPerAtom");
	const double targetAcceptanceRate = variableAsDouble(cfg, "TargetAcceptanceRate");
	double stepSize = variableAsDouble(cfg, "StepSize");
	const double termScale = 1.0;
	const double rRT = 1.0/(.008314472*cfg->temperature());

	// Print argument/parameter summary
	Messenger::print("AtomShake: Cutoff distance is %f\n", cutoffDistance);
	Messenger::print("AtomShake: Performing %i shake(s) per Atom\n", nShakesPerAtom);
	Messenger::print("AtomShake: Translation step is %f Angstroms, target acceptance rate is %f.\n", stepSize, targetAcceptanceRate);

	// Initialise the Cell distributor
	const bool willBeModified = true, allowRepeats = false;
	cfg->initialiseCellDistribution();

	// Create a local ChangeStore and EnergyKernel
	ChangeStore changeStore(procPool);
	EnergyKernel kernel(procPool, cfg, duq.potentialMap(), cutoffDistance);

	// Initialise the random number buffer
	procPool.initialiseRandomBuffer(ProcessPool::Group);

	// Enter calculation loop until no more Cells are available
	int cellId, shake, n, nbr;
	int nTries = 0, nAccepted = 0;
	bool accept;
	double currentEnergy, intraEnergy, newEnergy, newIntraEnergy, delta, totalDelta = 0.0;
	Cell* cell;
	Grain* grainI;
	Vec3<double> centre, rDelta;
	Atom** cellAtoms;

	Timer timer;
	procPool.resetAccumulatedTime();
	while (cellId = cfg->nextAvailableCell(procPool, willBeModified, allowRepeats), cellId != Cell::AllCellsComplete)
	{
		// Check for valid cell
		if (cellId == Cell::NoCellsAvailable)
		{
			// No valid cell, but still need to enter into change distribution with other processes
			changeStore.distributeAndApply(cfg);
			cfg->finishedWithCell(procPool, willBeModified, cellId);
			continue;
		}
		cell = cfg->cell(cellId);
		Messenger::printVerbose("AtomShake: Cell %i now the target on process %s, containing %i Grains interacting with %i neighbours.\n", cellId, procPool.processInfo(), cell->nGrains(), cell->nTotalCellNeighbours());

		/*
		 * Calculation Begins
		 */

		// Set current atom targets in ChangeStore (entire cell contents)
		changeStore.add(cell);

		// Loop over atoms in this cell
		cellAtoms = cell->atoms().objects();
		for (n = 0; n < cell->atoms().nItems(); ++n)
		{
			// Check for unused atom
			Atom* i = cellAtoms[n];

			// Get the atom's grain pointer
			grainI = i->grain();

			// Calculate reference intramolecular energy for atom, including intramolecular terms through the atom's grain
			currentEnergy = kernel.energy(i, ProcessPool::OverGroupProcesses);
			intraEnergy = kernel.fullIntraEnergy(grainI, termScale);

			// Loop over number of shakes per atom
			for (shake=0; shake<nShakesPerAtom; ++shake)
			{
				// Create a random translation vector
				rDelta.set(procPool.randomPlusMinusOne()*stepSize, procPool.randomPlusMinusOne()*stepSize, procPool.randomPlusMinusOne()*stepSize);

				// Translate atom and calculate new energy
				i->translateCoordinates(rDelta);
				newEnergy = kernel.energy(i, ProcessPool::OverGroupProcesses);
				newIntraEnergy = kernel.fullIntraEnergy(grainI, termScale);
				
				// Trial the transformed atom position
				delta = (newEnergy + newIntraEnergy) - (currentEnergy + intraEnergy);
// 				printf("delta = %f\n", delta);
				accept = delta < 0 ? true : (procPool.random() < exp(-delta*rRT));

				if (accept)
				{
// 					Messenger::print("Accepts move with delta %f\n", delta);
					// Accept new (current) position of target Atom
					changeStore.updateAtom(n);
					currentEnergy = newEnergy;
					intraEnergy = newIntraEnergy;
					totalDelta += delta;
					++nAccepted;
				}
				else changeStore.revert(n);
				
				++nTries;
			}

		}

		// Store modifications to Atom positions ready for broadcast later
		changeStore.storeAndReset();

		/*
		 * Calculation End
		 */

		// Distribute coordinate changes to all processes
		changeStore.distributeAndApply(cfg);
		changeStore.reset();

		// Must unlock the Cell when we are done with it!
		cfg->finishedWithCell(procPool, willBeModified, cellId);
	}
	timer.stop();

	// Grains have moved, so refold and update locations
	cfg->updateGrains();

	// Collect statistics from process group leaders
	if (!procPool.allSum(&nAccepted, 1, ProcessPool::Leaders)) return false;
	if (!procPool.allSum(&nTries, 1, ProcessPool::Leaders)) return false;
	if (!procPool.allSum(&totalDelta, 1, ProcessPool::Leaders)) return false;
	if (procPool.groupLeader())
	{
		double rate = double(nAccepted)/nTries;

		Messenger::print("AtomShake: Overall acceptance rate was %4.2f% (%i of %i attempted moves) (%s work, %s comms)\n", 100.0*rate, nAccepted, nTries, timer.timeString(), procPool.accumulatedTimeString());
		Messenger::print("AtomShake: Total energy delta was %10.4e kJ/mol.\n", totalDelta);

		// Adjust step size
		stepSize *= rate/targetAcceptanceRate;
// 		if (stepSize_ < 0.05) stepSize_ = 0.05;
// 		else if (stepSize_ > maxTranslationStep_) stepSize_ = maxTranslationStep_;
// 		if (rotationStep_ < 3.0) rotationStep_ = 3.0;
	}

	// Store updated parameter values
	if (!procPool.broadcast(&stepSize, 1, 0, ProcessPool::Group)) return false;
	setVariable(cfg, "StepSize", stepSize);
	Messenger::print("AtomShake: Updated translation step is %f Angstroms.\n", stepSize);
	
	// Increment configuration changeCount_
	if (nAccepted > 0) cfg->incrementCoordinateIndex();

	// Update total energy
	cfg->registerEnergyChange(totalDelta);
	cfg->accumulateEnergyChange();

	return true;
}

// // Perform a world atom shake
// bool DUQ::worldAtomShake(Configuration& cfg, double cutoffDistance, int nShakes, double targetAcceptanceRate, double stepSize)
// {
// 	// Control Parameters
// 	const double termScale = 1.0;	
// 	if (cutoffDistance < 0.0) cutoffDistance = pairPotentialRange_;
// 	const double rRT = 1.0/(.008314472*cfg->temperature());
// 	
// 	// Print argument/parameter summary
// 	Messenger::print("WorldAtomShake: Cutoff distance is %f\n", cutoffDistance);
// 	Messenger::print("WorldAtomShake: Translation step is %f Angstroms, target acceptance rate is %f.\n", stepSize, targetAcceptanceRate);
// 
// 	// Initialise the Cell distributor
// 	const bool willBeModified = true, allowRepeats = false;
// 	cfg->initialiseCellDistribution();
// 
// 	// Create a local ChangeStore and EnergyKernel
// 	ChangeStore changeStore;
// 	EnergyKernel kernel(cfg, potentialMap_, cutoffDistance);
// 
// 	// Initialise the random number buffer
// 	procPool.initialiseRandomBuffer(ProcessPool::Pool);
// 
// 	// Enter calculation loop until no more Cells are available
// 	int cellId, shake, n, i;
// 	int nTries = 0, nAccepted = 0;
// 	bool accept;
// 	double currentEnergy, newEnergy, delta, totalDelta = 0.0;
// 	Cell* cell;
// 	Vec3<double> rDelta;
// 	int nAtoms = cfg->nAtoms();
// 	Atom* atoms = cfg->atoms();
// 
// 	Timer timer;
// 	procPool.resetAccumulatedTime();
// 
// 	// Add entire cell contents to a ChangeStore
// 	for (n=0; n<nAtoms; ++n) changeStore.add(&atoms[n]);
// 
// 	// Calculate reference energy
// 	currentEnergy = interatomicEnergy(cfg) + intramolecularEnergy(cfg);
// 
// 	// Loop over nShakes
// 	for (int n=0; n<nShakes; ++n)
// 	{
// 		// Randomly displace each atom in the system
// 		for (i=0; i<nAtoms; ++i)
// 		{
// 			// Create a random translation vector
// 			rDelta.set(procPool.randomPlusMinusOne()*stepSize, procPool.randomPlusMinusOne()*stepSize, procPool.randomPlusMinusOne()*stepSize);
// 
// 			// Translate atom
// 			atoms[i].translateCoordinates(rDelta);
// 		}
// 
// 		// Update neighbour lists
// 		cfg->recreateCellAtomNeighbourLists(pairPotentialRange_);
// 
// 		// Calculate new energy
// 		newEnergy = interatomicEnergy(cfg) + intramolecularEnergy(cfg);
// 
// 		delta = newEnergy - currentEnergy;
// 		printf("delta = %f\n", delta);
// 		accept = delta < 0 ? true : (procPool.random() < exp(-delta*rRT));
// 
// 		if (accept)
// 		{
// // 			Messenger::print("Accepts move with delta %f\n", delta);
// 			// Accept new (current) positions of atoms
// 			changeStore.updateAll();
// 			currentEnergy = newEnergy;
// 			totalDelta += delta;
// 			++nAccepted;
// 		}
// 		else
// 		{
// 			changeStore.revertAll();
// 			cfg->recreateCellAtomNeighbourLists(pairPotentialRange_);
// 		}
// 	}
// 
// 	// Collect statistics from process group leaders
// 	if (!procPool.allSum(&nAccepted, 1, ProcessPool::Leaders)) return false;
// 	if (!procPool.allSum(&nTries, 1, ProcessPool::Leaders)) return false;
// 	if (!procPool.allSum(&totalDelta, 1, ProcessPool::Leaders)) return false;
// 	if (procPool.groupLeader())
// 	{
// 		double rate = double(nAccepted)/nTries;
// 
// 		Messenger::print("WorldAtomShake: Overall acceptance rate was %4.2f% (%i of %i attempted moves) (%s work, %s comms)\n", 100.0*rate, nAccepted, nTries, timer.timeString(), procPool.accumulatedTimeString());
// 		Messenger::print("WorldAtomShake: Total energy delta was %10.4e kJ/mol.\n", totalDelta);
// 
// 		// Adjust step size
// 		stepSize *= rate/targetAcceptanceRate;
// // 		if (stepSize_ < 0.05) stepSize_ = 0.05;
// // 		else if (stepSize_ > maxTranslationStep_) stepSize_ = maxTranslationStep_;
// // 		if (rotationStep_ < 3.0) rotationStep_ = 3.0;
// 	}
// 
// 	// Store updated parameter values
// 	if (!procPool.broadcast(&stepSize, 1, 0, ProcessPool::Group)) return false;
// // 	stepSizeParam->setValue(stepSize); TODO
// 	Messenger::print("WorldAtomShake: Updated translation step is %f Angstroms.\n", stepSize);
// 	
// 	// Increment configuration changeCount_
// 	if (nAccepted > 0) cfg->incrementCoordinateIndex();
// 
// 	// Update total energy
// 	cfg->registerEnergyChange(totalDelta);
// 	cfg->accumulateEnergyChange();
// }

// Execute post-processing stage
bool AtomShake::postProcess(DUQ& duq, ProcessPool& procPool)
{
	return true;
}
