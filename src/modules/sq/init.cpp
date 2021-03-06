// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "keywords/types.h"
#include "math/averaging.h"
#include "modules/bragg/bragg.h"
#include "modules/rdf/rdf.h"
#include "modules/sq/sq.h"

// Perform any necessary initialisation for the Module
void SQModule::initialise()
{
    // Control
    keywords_.add("Control", new ModuleKeyword<const RDFModule>("RDF"), "SourceRDFs", "Source RDFs to transform into S(Q)");
    keywords_.add("Control", new DoubleKeyword(0.05, 1.0e-5), "QDelta", "Step size in Q for S(Q) calculation");
    keywords_.add("Control", new DoubleKeyword(30.0, -1.0), "QMax", "Maximum Q for calculated S(Q)");
    keywords_.add("Control", new DoubleKeyword(0.01, 0.0), "QMin", "Minimum Q for calculated S(Q)");
    keywords_.add("Control", new Function1DKeyword({Functions::Function1D::GaussianC2, {0.0, 0.02}}), "QBroadening",
                  "Instrument broadening function to apply when calculating S(Q)");
    keywords_.add("Control", new EnumOptionsKeyword<WindowFunction::Form>(WindowFunction::forms()), "WindowFunction",
                  "Window function to apply in Fourier-transform of g(r) to S(Q)");
    keywords_.add("Control", new IntegerKeyword(1, 1), "Averaging",
                  "Number of historical partial sets to combine into final partials", "<1>");
    keywords_.add(
        "Control",
        new EnumOptionsKeyword<Averaging::AveragingScheme>(Averaging::averagingSchemes() = Averaging::LinearAveraging),
        "AveragingScheme", "Weighting scheme to use when averaging partials", "<Linear>");

    // Bragg Scattering
    keywords_.add("Bragg Scattering", new ModuleKeyword<const BraggModule>("Bragg"), "IncludeBragg",
                  "Include Bragg scattering from specified module");
    keywords_.add("Bragg Scattering", new Function1DKeyword, "BraggQBroadening",
                  "Broadening function to apply to Bragg reflections when generating S(Q)");

    // Export
    keywords_.add("Export", new BoolKeyword(false), "Save", "Whether to save partials to disk after calculation",
                  "<True|False>");
}
