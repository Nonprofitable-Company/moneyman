#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class Database;
class AccountsWidget;
class TrialBalanceWidget;
class GeneralLedgerWidget;
class IncomeStatementWidget;
class BalanceSheetWidget;
class QTabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewJournalEntry();
    void refreshAllReports();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    Database *m_database;
    AccountsWidget *m_accountsWidget;
    QTabWidget *m_reportTabs;
    TrialBalanceWidget *m_trialBalanceWidget;
    GeneralLedgerWidget *m_generalLedgerWidget;
    IncomeStatementWidget *m_incomeStatementWidget;
    BalanceSheetWidget *m_balanceSheetWidget;
};

#endif // MAIN_WINDOW_H
