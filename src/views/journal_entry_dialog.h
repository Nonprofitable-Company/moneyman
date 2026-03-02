#ifndef JOURNAL_ENTRY_DIALOG_H
#define JOURNAL_ENTRY_DIALOG_H

#include <QDialog>

class QDateEdit;
class QLineEdit;
class QTableView;
class QLabel;
class QPushButton;
class Database;
class JournalLineModel;

class JournalEntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JournalEntryDialog(Database *db, QWidget *parent = nullptr);

private slots:
    void onAddLine();
    void onRemoveLine();
    void onTotalsChanged();
    void onPost();

private:
    Database *m_db;
    JournalLineModel *m_lineModel;

    QDateEdit *m_dateEdit;
    QLineEdit *m_descriptionEdit;
    QTableView *m_lineTable;
    QLabel *m_debitTotalLabel;
    QLabel *m_creditTotalLabel;
    QLabel *m_balanceStatusLabel;
    QPushButton *m_postButton;
};

#endif // JOURNAL_ENTRY_DIALOG_H
