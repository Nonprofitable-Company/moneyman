#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class Database;
class AccountsWidget;
class TrialBalanceWidget;
class GeneralLedgerWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewJournalEntry();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    Database *m_database;
    AccountsWidget *m_accountsWidget;
    TrialBalanceWidget *m_trialBalanceWidget;
    GeneralLedgerWidget *m_generalLedgerWidget;
};

#endif // MAIN_WINDOW_H
