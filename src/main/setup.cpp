/*
	*** dUQ - Setup
	*** src/main/setup.cpp
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
#include "classes/atomtype.h"
#include "classes/species.h"

// Set number of test points to use when calculating Box normalisation arrays
void DUQ::setBoxNormalisationPoints(int nPoints)
{
	boxNormalisationPoints_ = nPoints;
}

// Bin width to use when calculating RMSE between Sample F(Q) and reference data
void DUQ::setRMSEDeltaQ(double dq)
{
	rmseDeltaQ_ = dq;
}

// Set random seed
void DUQ::setSeed(int seed)
{
	seed_ = seed;
}

// Return random seed
int DUQ::seed()
{
	return seed_;
}

// Set maximum number of main loop iterations to perform
void DUQ::setMaxIterations(int nIterations)
{
	maxIterations_ = nIterations;
}

// Return maximum number of main loop iterations to perform
int DUQ::maxIterations()
{
	return maxIterations_;
}

// Set window function to use for all Fourier transforms
void DUQ::setWindowFunction(Data2D::WindowFunction wf)
{
	windowFunction_ = wf;
}

// Setup all simulation data, checking it as we go
bool DUQ::setupSimulation()
{
	Messenger::print("\n");
	Messenger::print("*** SYSTEM SETUP BEGINS ***\n");

	/* Check each defined Species */
	Messenger::print("\n");
	Messenger::print("*** Checking Species definitions...\n");
	for (Species* sp = species_.first(); sp != NULL; sp = sp->next)
	{
		Messenger::print("--- Species '%s'...\n", sp->name());
		if (!sp->checkSetup(atomTypes_)) return false;
	}

	Messenger::print("\n");
	Messenger::print("*** Setting up Configurations...\n");
	int index = 0;
	for (Configuration* cfg = configurations_.first(); cfg != NULL; cfg = cfg->next, ++index)
	{
		Messenger::print("*** Configuration %2i: '%s'\n", index, cfg->name());

		if (!cfg->setup(worldPool_, atomTypes_, pairPotentialRange_, boxNormalisationPoints_)) return false;
	}

	/* Pair Potentials */
	/* We expect a PairPotential to have been defined for every combination of AtomType used in the system */
	Messenger::print("\n");
	Messenger::print("*** Checking PairPotential definitions....\n");
	int nMissingPots = 0;
	for (AtomType* at1 = atomTypes_.first(); at1 != NULL; at1 = at1->next)
	{
		for (AtomType* at2 = at1; at2 != NULL; at2 = at2->next)
		{
			PairPotential* pot = hasPairPotential(at1, at2);
			if (pot == NULL)
			{
				Messenger::error("A PairPotential between AtomTypes '%s' and '%s' is required, but has not been defined.\n", at1->name(), at2->name());
				++nMissingPots;
			}
		}
	}
	if (nMissingPots > 0) return false;

	/* Prepare Samples */
	Messenger::print("\n");
	Messenger::print("*** Preparing Samples...\n");
	if (!setupSamples())
	{
		Messenger::error("Failed to setup Samples.\n");
		return false;
	}

	/* Finalise AtomTypes */

	// Assign indices to atom types
	Messenger::print("--> Assigning indices to master AtomTypes...\n");
	int id = 0;
	for (AtomType* at = atomTypes_.first(); at != NULL; at = at->next) at->setIndex(id++);

	// Set global AtomType indices in all Configurations
	Messenger::print("--> Setting global AtomType indices in Configurations...\n");
	for (Configuration* cfg = configurations_.first(); cfg != NULL; cfg = cfg->next) cfg->setGlobalAtomTypeIndices(atomTypes_);

	// Create PairPotential matrix
	Messenger::print("--> Creating PairPotential matrix (%ix%i)...\n", atomTypes_.nItems(), atomTypes_.nItems());
	if (!potentialMap_.initialise(atomTypes_, pairPotentials_, pairPotentialRange_)) return false;

	/* Construct Pre/Post-Process Lists */

	// Loop over configurations
	for (Configuration* cfg = configurations_.first(); cfg != NULL; cfg = cfg->next)
	{
		// Loop over Modules, checking for those that have pre- or post-processing steps
		for (RefListItem<Module,bool>* ri = cfg->modules(); ri != NULL; ri = ri->next)
		{
			// Grab Module pointer
			Module* module = ri->item;

			// Pre-Processing
			if (module->hasPreProcessing())
			{
				// If the Module's instance type is UniqueInstance, check that it is not already in the list
				if (module->instanceType() == Module::UniqueInstance)
				{
					Module* oldModule = findPreProcessingStage(module->name());
					if (!oldModule) preProcessingStages_.add(module);
				}
				else preProcessingStages_.add(module);
			}

			// Post-Processing
			if (module->hasPostProcessing())
			{
				// If the Module's instance type is UniqueInstance, check that it is not already in the list
				if (module->instanceType() == Module::UniqueInstance)
				{
					Module* oldModule = findPostProcessingStage(module->name());
					if (!oldModule) postProcessingStages_.add(module);
				}
				else postProcessingStages_.add(module);
			}
		}
	}

	return true;
}
