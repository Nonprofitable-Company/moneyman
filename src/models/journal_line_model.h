#ifndef JOURNAL_LINE_MODEL_H
#define JOURNAL_LINE_MODEL_H

#include <QAbstractTableModel>
#include <vector>
#include <cstdint>

class Database;

struct EditableJournalLine {
    int64_t accountId = 0;
    int64_t debitCents = 0;
    int64_t creditCents = 0;
};

class JournalLineModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column { ColAccount = 0, ColDebit, ColCredit, ColCount };

    explicit JournalLineModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void addLine();
    void removeLine(int row);

    const std::vector<EditableJournalLine>& lines() const { return m_lines; }
    int64_t totalDebitCents() const;
    int64_t totalCreditCents() const;
    bool isBalanced() const;
    int validLineCount() const;

signals:
    void totalsChanged();

private:
    std::vector<EditableJournalLine> m_lines;
};

#endif // JOURNAL_LINE_MODEL_H
