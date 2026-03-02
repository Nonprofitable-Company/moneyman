#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class Database;
class AccountsWidget;
class DashboardWidget;
class TrialBalanceWidget;
class GeneralLedgerWidget;
class IncomeStatementWidget;
class BalanceSheetWidget;
class JournalListWidget;
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
    void onBackup();
    void onRestore();
    void onChangeKey();
    void refreshAllReports();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    Database *m_database;
    QString m_passphrase;
    AccountsWidget *m_accountsWidget;
    QDockWidget *m_accountsDock;
    QTabWidget *m_reportTabs;
    DashboardWidget *m_dashboardWidget;
    TrialBalanceWidget *m_trialBalanceWidget;
    GeneralLedgerWidget *m_generalLedgerWidget;
    IncomeStatementWidget *m_incomeStatementWidget;
    BalanceSheetWidget *m_balanceSheetWidget;
    JournalListWidget *m_journalListWidget;
    AuditLogWidget *m_auditLogWidget;
};

#endif // MAIN_WINDOW_H
