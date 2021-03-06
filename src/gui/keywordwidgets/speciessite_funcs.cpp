// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/coredata.h"
#include "classes/species.h"
#include "classes/speciessite.h"
#include "gui/keywordwidgets/speciessite.h"
#include "templates/variantpointer.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpacerItem>

SpeciesSiteKeywordWidget::SpeciesSiteKeywordWidget(QWidget *parent, KeywordBase *keyword, const CoreData &coreData)
    : KeywordDropDown(this), KeywordWidgetBase(coreData)
{
    // Create and set up the UI for our widget in the drop-down's widget container
    ui_.setupUi(dropWidget());

    // Cast the pointer up into the parent class type
    keyword_ = dynamic_cast<SpeciesSiteKeyword *>(keyword);
    if (!keyword_)
        Messenger::error("Couldn't cast base keyword '{}' into SpeciesSiteKeyword.\n", keyword->name());
    else
    {
        // Set current information
        updateWidgetValues(coreData_);
    }
}

/*
 * Widgets
 */
void SpeciesSiteKeywordWidget::siteRadioButton_clicked(bool checked)
{
    if (refreshing_)
        return;

    QRadioButton *radioButton = qobject_cast<QRadioButton *>(sender());
    if (!radioButton)
        return;

    // Retrieve the SpeciesSite from the radioButton
    SpeciesSite *site = VariantPointer<SpeciesSite>(radioButton->property("SpeciesSite"));
    if (!site)
        return;

    keyword_->data() = site;
    keyword_->setAsModified();

    updateSummaryText();

    emit(keywordValueChanged(keyword_->optionMask()));
}

/*
 * Update
 */

// Update value displayed in widget
void SpeciesSiteKeywordWidget::updateValue() { updateWidgetValues(coreData_); }

// Update widget values data based on keyword data
void SpeciesSiteKeywordWidget::updateWidgetValues(const CoreData &coreData)
{
    refreshing_ = true;

    // Clear all tabs from our tab widget
    ui_.SpeciesTabs->clear();

    // Create button group for the radio buttons
    auto *buttonGroup = new QButtonGroup(this);

    // Add new tabs in, one for each defined Species, and each containing checkboxes for each available site
    for (const auto &sp : coreData_.species())
    {
        // Create the widget to hold our checkboxes for this Species
        auto *widget = new QWidget();
        auto *layout = new QVBoxLayout;
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setSpacing(4);
        widget->setLayout(layout);
        widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        // Are there sites defined?
        if (sp->nSites() == 0)
        {
            layout->addWidget(new QLabel("No sites defined."));
        }
        else
        {
            // Loop over sites defined in this Species
            for (auto &site : sp->sites())
            {
                auto *radioButton = new QRadioButton(QString::fromStdString(std::string(site->name())));
                if (keyword_->data() == site.get())
                    radioButton->setChecked(true);
                connect(radioButton, SIGNAL(clicked(bool)), this, SLOT(siteRadioButton_clicked(bool)));
                radioButton->setProperty("SpeciesSite", VariantPointer<SpeciesSite>(site.get()));
                layout->addWidget(radioButton);
                buttonGroup->addButton(radioButton);

                // If this keyword demands oriented sites, disable the radio button if the site has no axes
                if (keyword_->axesRequired() && (!site->hasAxes()))
                    radioButton->setDisabled(true);
            }

            // Add on a vertical spacer to take up any extra space at the foot of the widget
            layout->addSpacing(0);
        }

        // Create the page in the tabs
        ui_.SpeciesTabs->addTab(widget, QString::fromStdString(std::string(sp->name())));
    }

    updateSummaryText();

    refreshing_ = false;
}

// Update keyword data based on widget values
void SpeciesSiteKeywordWidget::updateKeywordData()
{
    // Not relevant - Handled via checkbox callbacks
}

// Update summary text
void SpeciesSiteKeywordWidget::updateSummaryText()
{
    if (keyword_->data())
        setSummaryText(
            QString::fromStdString(fmt::format("{} ({})", keyword_->data()->name(), keyword_->data()->parent()->name())));
    else
        setSummaryText("<None>");
}
