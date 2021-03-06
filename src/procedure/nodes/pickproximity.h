// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "procedure/nodes/node.h"
#include "procedure/nodes/pickbase.h"
#include <memory>

// Forward Declarations
class Molecule;

// Pick by Proximity Node
class PickProximityProcedureNode : public PickProcedureNodeBase
{
    public:
    explicit PickProximityProcedureNode();
    ~PickProximityProcedureNode() override = default;

    /*
     * Execute
     */
    public:
    // Execute node, targetting the supplied Configuration
    bool execute(ProcessPool &procPool, Configuration *cfg, std::string_view prefix, GenericList &targetList) override;
};
