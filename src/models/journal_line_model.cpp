#include "journal_line_model.h"

JournalLineModel::JournalLineModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // Start with 2 empty lines
    m_lines.resize(2);
}

int JournalLineModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(m_lines.size());
}

int JournalLineModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

QVariant JournalLineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_lines.size()))
        return {};

    const auto &line = m_lines[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColAccount:
            return static_cast<qlonglong>(line.accountId);
        case ColDebit:
            if (line.debitCents == 0 && role == Qt::DisplayRole) return QString();
            return QString::number(static_cast<double>(line.debitCents) / 100.0, 'f', 2);
        case ColCredit:
            if (line.creditCents == 0 && role == Qt::DisplayRole) return QString();
            return QString::number(static_cast<double>(line.creditCents) / 100.0, 'f', 2);
        }
    }

    if (role == Qt::TextAlignmentRole && (index.column() == ColDebit || index.column() == ColCredit)) {
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    }

    return {};
}

QVariant JournalLineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case ColAccount: return "Account";
    case ColDebit:   return "Debit";
    case ColCredit:  return "Credit";
    }
    return {};
}

Qt::ItemFlags JournalLineModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool JournalLineModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto &line = m_lines[static_cast<size_t>(index.row())];

    switch (index.column()) {
    case ColAccount:
        line.accountId = value.toLongLong();
        break;
    case ColDebit:
        line.debitCents = static_cast<int64_t>(value.toDouble() * 100.0 + 0.5);
        break;
    case ColCredit:
        line.creditCents = static_cast<int64_t>(value.toDouble() * 100.0 + 0.5);
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {role});
    emit totalsChanged();
    return true;
}

void JournalLineModel::addLine()
{
    int row = static_cast<int>(m_lines.size());
    beginInsertRows(QModelIndex(), row, row);
    m_lines.push_back({});
    endInsertRows();
}

void JournalLineModel::removeLine(int row)
{
    if (row < 0 || row >= static_cast<int>(m_lines.size()) || m_lines.size() <= 2)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_lines.erase(m_lines.begin() + row);
    endRemoveRows();
    emit totalsChanged();
}

void JournalLineModel::setLines(const std::vector<EditableJournalLine> &lines)
{
    beginResetModel();
    m_lines = lines;
    if (m_lines.size() < 2) m_lines.resize(2);
    endResetModel();
    emit totalsChanged();
}

int64_t JournalLineModel::totalDebitCents() const
{
    int64_t total = 0;
    for (const auto &line : m_lines) total += line.debitCents;
    return total;
}

int64_t JournalLineModel::totalCreditCents() const
{
    int64_t total = 0;
    for (const auto &line : m_lines) total += line.creditCents;
    return total;
}

bool JournalLineModel::isBalanced() const
{
    int64_t d = totalDebitCents();
    int64_t c = totalCreditCents();
    return d > 0 && d == c;
}

int JournalLineModel::validLineCount() const
{
    int count = 0;
    for (const auto &line : m_lines) {
        if (line.accountId > 0 && (line.debitCents > 0 || line.creditCents > 0))
            ++count;
    }
    return count;
}
