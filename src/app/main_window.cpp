#include "main_window.h"
#include "db/database.h"
#include "views/accounts_widget.h"
#include "views/journal_entry_dialog.h"
#include "views/trial_balance_widget.h"
#include "views/general_ledger_widget.h"
#include "views/income_statement_widget.h"
#include "views/balance_sheet_widget.h"
#include "models/account_model.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <QTabWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
    , m_accountsWidget(nullptr)
    , m_reportTabs(nullptr)
    , m_trialBalanceWidget(nullptr)
    , m_generalLedgerWidget(nullptr)
    , m_incomeStatementWidget(nullptr)
    , m_balanceSheetWidget(nullptr)
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    if (!m_database->open()) {
        QMessageBox::critical(this, "Database Error",
            "Failed to open database: " + m_database->lastError());
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle("MoneyMan — The Nonprofitable Company");
    resize(1200, 800);

    // Chart of Accounts as a dock widget on the left
    m_accountsWidget = new AccountsWidget(m_database, this);
    auto *dock = new QDockWidget("Chart of Accounts", this);
    dock->setWidget(m_accountsWidget);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Reports as tabbed central widget
    m_reportTabs = new QTabWidget(this);
    m_trialBalanceWidget = new TrialBalanceWidget(m_database, this);
    m_generalLedgerWidget = new GeneralLedgerWidget(m_database, this);
    m_incomeStatementWidget = new IncomeStatementWidget(m_database, this);
    m_balanceSheetWidget = new BalanceSheetWidget(m_database, this);

    m_reportTabs->addTab(m_trialBalanceWidget, "Trial Balance");
    m_reportTabs->addTab(m_generalLedgerWidget, "General Ledger");
    m_reportTabs->addTab(m_incomeStatementWidget, "Income Statement");
    m_reportTabs->addTab(m_balanceSheetWidget, "Balance Sheet");

    setCentralWidget(m_reportTabs);
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto *txnMenu = menuBar()->addMenu("&Transactions");
    txnMenu->addAction("&New Journal Entry...", QKeySequence(Qt::CTRL | Qt::Key_J),
                        this, &MainWindow::onNewJournalEntry);

    auto *reportsMenu = menuBar()->addMenu("&Reports");
    reportsMenu->addAction("Refresh &All Reports", QKeySequence(Qt::CTRL | Qt::Key_R),
                           this, &MainWindow::refreshAllReports);
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->addAction("New Journal Entry", this, &MainWindow::onNewJournalEntry);
    toolbar->addSeparator();
    toolbar->addAction("Refresh Reports", this, &MainWindow::refreshAllReports);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::onNewJournalEntry()
{
    JournalEntryDialog dialog(m_database, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_accountsWidget->model()->refresh();
        refreshAllReports();
        statusBar()->showMessage("Journal entry posted successfully", 5000);
    }
}

void MainWindow::refreshAllReports()
{
    m_trialBalanceWidget->refresh();
    m_generalLedgerWidget->refresh();
    m_incomeStatementWidget->refresh();
    m_balanceSheetWidget->refresh();
}
