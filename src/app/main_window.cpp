#include "main_window.h"
#include "db/database.h"
#include "views/accounts_widget.h"
#include "views/journal_entry_dialog.h"
#include "views/trial_balance_widget.h"
#include "models/account_model.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
    , m_accountsWidget(nullptr)
    , m_trialBalanceWidget(nullptr)
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
    resize(1024, 768);

    // Chart of Accounts as a dock widget
    m_accountsWidget = new AccountsWidget(m_database, this);
    auto *dock = new QDockWidget("Chart of Accounts", this);
    dock->setWidget(m_accountsWidget);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Trial Balance as a dock widget
    m_trialBalanceWidget = new TrialBalanceWidget(m_database, this);
    auto *tbDock = new QDockWidget("Trial Balance", this);
    tbDock->setWidget(m_trialBalanceWidget);
    tbDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, tbDock);
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto *reportsMenu = menuBar()->addMenu("&Reports");
    reportsMenu->addAction("Refresh &Trial Balance", QKeySequence(Qt::CTRL | Qt::Key_T),
                           m_trialBalanceWidget, &TrialBalanceWidget::refresh);

    auto *txnMenu = menuBar()->addMenu("&Transactions");
    txnMenu->addAction("&New Journal Entry...", QKeySequence(Qt::CTRL | Qt::Key_J),
                        this, &MainWindow::onNewJournalEntry);
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->addAction("New Journal Entry", this, &MainWindow::onNewJournalEntry);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::onNewJournalEntry()
{
    JournalEntryDialog dialog(m_database, this);
    if (dialog.exec() == QDialog::Accepted) {
        // Refresh account balances after posting
        m_accountsWidget->model()->refresh();
        m_trialBalanceWidget->refresh();
        statusBar()->showMessage("Journal entry posted successfully", 5000);
    }
}
