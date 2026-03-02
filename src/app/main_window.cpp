#include "main_window.h"
#include "db/database.h"
#include "views/accounts_widget.h"
#include "views/journal_entry_dialog.h"
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
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

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
        statusBar()->showMessage("Journal entry posted successfully", 5000);
    }
}
