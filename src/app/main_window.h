#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class Database;
class AccountsWidget;
class TrialBalanceWidget;
class GeneralLedgerWidget;
class IncomeStatementWidget;
class BalanceSheetWidget;
class AuditLogWidget;
class QTabWidget;
class QDockWidget;

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
    QDockWidget *m_accountsDock;
    QTabWidget *m_reportTabs;
    TrialBalanceWidget *m_trialBalanceWidget;
    GeneralLedgerWidget *m_generalLedgerWidget;
    IncomeStatementWidget *m_incomeStatementWidget;
    BalanceSheetWidget *m_balanceSheetWidget;
    AuditLogWidget *m_auditLogWidget;
};

#endif // MAIN_WINDOW_H
