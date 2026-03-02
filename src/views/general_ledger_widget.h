#ifndef GENERAL_LEDGER_WIDGET_H
#define GENERAL_LEDGER_WIDGET_H

#include <QWidget>

class QComboBox;
class QTableWidget;
class QDateEdit;
class QCheckBox;
class Database;

class GeneralLedgerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralLedgerWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();
    void exportCsv();

private slots:
    void onAccountChanged(int index);
    void onDateFilterChanged();
    void onContextMenu(const QPoint &pos);

private:
    void populateAccountSelector();
    void loadLedger(int64_t accountId);

    Database *m_db;
    QComboBox *m_accountCombo;
    QTableWidget *m_table;
    QCheckBox *m_dateFilterCheck;
    QDateEdit *m_fromDate;
    QDateEdit *m_toDate;
};

#endif // GENERAL_LEDGER_WIDGET_H
