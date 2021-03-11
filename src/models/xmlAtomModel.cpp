#include "models/xmlAtomModel.h"
#include "classes/atomtype.h"
#include "data/elements.h"
#include <QColor>
#include <pugixml.hpp>

XmlAtomModel::XmlAtomModel(Dissolve &dissolve) : dissolve_(dissolve) {}

void XmlAtomModel::readFile(const pugi::xml_node &root)
{
    beginResetModel();
    atoms_.clear();

    auto types = dissolve_.atomTypes();

    for (auto &b : root.select_nodes("/ForceField/AtomTypes/Type"))
    {
        // Find the first node with the same element.  Use this as our best guess for the AtomType
        auto it = std::find_if(types.begin(), types.end(),
                               [&b](auto t) { return Elements::symbol(t->Z()) == b.node().attribute("element").as_string(); });

        atoms_.emplace_back(b.node().attribute("name").as_string(), b.node().attribute("class").as_string(),
                            b.node().attribute("element").as_string(), b.node().attribute("mass").as_double(),
                            // Use our best guess for the AtomType or
                            // mark it as missing.
                            it == types.end() ? -1 : (*it)->index());
    }

    endResetModel();
}

int XmlAtomModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return atoms_.size();
}

int XmlAtomModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant XmlAtomModel::data(const QModelIndex &index, int role) const
{
    int type;

    if (role == Qt::ForegroundRole)
    {
        switch (index.column())
        {
            case 4:
                type = std::get<4>(atoms_[index.row()]);
                if (type < 0 || type >= dissolve_.nAtomTypes())
                    return QColor("Red");
                else
                    return QVariant();
            default:
                return QVariant();
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (index.column())
    {
        case 0:
            return std::get<0>(atoms_[index.row()]).c_str();
        case 1:
            return std::get<1>(atoms_[index.row()]).c_str();
        case 2:
            return std::get<2>(atoms_[index.row()]).c_str();
        case 3:
	    return std::get<3>(atoms_[index.row()]);
        case 4:
            type = std::get<4>(atoms_[index.row()]);
            if (type < 0 || type >= dissolve_.nAtomTypes())
                return "Missing";
            return dissolve_.atomType(type)->name().data();
        default:
            return QVariant();
    }
}

QVariant XmlAtomModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section)
    {
        case 0:
            return "Name";
        case 1:
            return "Class";
        case 2:
            return "Element";
        case 3:
            return "Mass";
        case 4:
            return "Type";
        default:
            return QVariant();
    }
}

Qt::ItemFlags XmlAtomModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = Qt::ItemIsEnabled;
    if (index.column() == 4)
        result |= Qt::ItemIsEditable;
    return result;
}

bool XmlAtomModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= atoms_.size() || index.row() < 0)
        return false;

    // Only set the linked AtomType
    if (index.column() != 4)
        return false;

    // Find the requested AtomType
    auto type = dissolve_.findAtomType(value.toString().toStdString());
    if (!type)
        return false;

    std::get<4>(atoms_[index.row()]) = type->index();

    return true;
}

void XmlAtomModel::clear()
{
    beginResetModel();
    atoms_.clear();
    endResetModel();
}

std::vector<ForcefieldAtomType> XmlAtomModel::toVector()
{
    std::vector<ForcefieldAtomType> result;
    for (auto &at : atoms_)
    {
        auto type = dissolve_.atomType(std::get<4>(at));
        result.emplace_back(type->Z(), type->index(), type->name(), "", "", type->charge(), type->shortRangeParameters());
    }
    return result;
}

std::map<std::string, std::string> XmlAtomModel::toMap()
{
    std::map<std::string, std::string> result;
    for (auto &at : atoms_)
    {
        result[std::get<0>(at)] = dissolve_.atomType(std::get<4>(at))->name();
    }
    return result;
}