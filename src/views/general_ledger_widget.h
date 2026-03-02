#ifndef GENERAL_LEDGER_WIDGET_H
#define GENERAL_LEDGER_WIDGET_H

#include <QWidget>

class QComboBox;
class QTableWidget;
class Database;

class GeneralLedgerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralLedgerWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onAccountChanged(int index);

private:
    void populateAccountSelector();
    void loadLedger(int64_t accountId);

    Database *m_db;
    QComboBox *m_accountCombo;
    QTableWidget *m_table;
};

#endif // GENERAL_LEDGER_WIDGET_H
