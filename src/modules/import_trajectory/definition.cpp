// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "modules/import_trajectory/importtraj.h"

// Return type of module
std::string_view ImportTrajectoryModule::type() const { return "Import"; }

// Return category for module
std::string_view ImportTrajectoryModule::category() const { return "Input / Output"; }

// Return brief description of module
std::string_view ImportTrajectoryModule::brief() const { return "Import data to the target Configuration"; }

// Return the number of Configuration targets this Module requires
int ImportTrajectoryModule::nRequiredTargets() const { return Module::ExactlyOneTarget; }
