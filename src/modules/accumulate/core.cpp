// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "modules/accumulate/accumulate.h"

AccumulateModule::AccumulateModule() : Module(Module::ZeroTargets)
{
    // Set unique name for this instance of the Module
    static int instanceId = 0;
    uniqueName_ = fmt::format("{}{:02d}", type(), instanceId++);

    // Initialise Module - set up keywords etc.
    initialise();
}

/*
 * Instances
 */

// Create instance of this module
Module *AccumulateModule::createInstance() const { return new AccumulateModule; }
