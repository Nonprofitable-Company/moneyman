#include "main_window.h"
#include "db/database.h"
#include "views/dashboard_widget.h"
#include "views/accounts_widget.h"
#include "views/journal_entry_dialog.h"
#include "views/trial_balance_widget.h"
#include "views/general_ledger_widget.h"
#include "views/income_statement_widget.h"
#include "views/balance_sheet_widget.h"
#include "views/audit_log_widget.h"
#include "views/close_period_dialog.h"
#include "views/password_dialog.h"
#include "views/import_csv_dialog.h"
#include "models/account_model.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <QTabWidget>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QFileDialog>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
    , m_accountsWidget(nullptr)
    , m_accountsDock(nullptr)
    , m_reportTabs(nullptr)
    , m_trialBalanceWidget(nullptr)
    , m_generalLedgerWidget(nullptr)
    , m_incomeStatementWidget(nullptr)
    , m_balanceSheetWidget(nullptr)
    , m_auditLogWidget(nullptr)
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    // Prompt for encryption passphrase
    PasswordDialog pwDialog(PasswordDialog::Unlock, this);
    if (pwDialog.exec() != QDialog::Accepted) {
        // User cancelled — open with default key so UI still loads
        m_database->open();
    } else if (!m_database->open(QString(), pwDialog.password())) {
        QMessageBox::critical(this, "Database Error",
            "Failed to open database: " + m_database->lastError()
            + "\n\nThe passphrase may be incorrect.");
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle("MoneyMan — The Nonprofitable Company");
    resize(1200, 800);

    // Chart of Accounts as a dock widget on the left
    m_accountsWidget = new AccountsWidget(m_database, this);
    m_accountsDock = new QDockWidget("Chart of Accounts", this);
    m_accountsDock->setWidget(m_accountsWidget);
    m_accountsDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable
                                | QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::LeftDockWidgetArea, m_accountsDock);

    // Reports as tabbed central widget
    m_reportTabs = new QTabWidget(this);
    m_dashboardWidget = new DashboardWidget(m_database, this);
    m_trialBalanceWidget = new TrialBalanceWidget(m_database, this);
    m_generalLedgerWidget = new GeneralLedgerWidget(m_database, this);
    m_incomeStatementWidget = new IncomeStatementWidget(m_database, this);
    m_balanceSheetWidget = new BalanceSheetWidget(m_database, this);
    m_auditLogWidget = new AuditLogWidget(m_database, this);

    m_reportTabs->addTab(m_dashboardWidget, "Dashboard");
    m_reportTabs->addTab(m_trialBalanceWidget, "Trial Balance");
    m_reportTabs->addTab(m_generalLedgerWidget, "General Ledger");
    m_reportTabs->addTab(m_incomeStatementWidget, "Income Statement");
    m_reportTabs->addTab(m_balanceSheetWidget, "Balance Sheet");
    m_reportTabs->addTab(m_auditLogWidget, "Audit Log");

    setCentralWidget(m_reportTabs);
}

void MainWindow::setupMenuBar()
{
    auto *style = QApplication::style();

    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Backup Database...", this, &MainWindow::onBackup);
    fileMenu->addAction("&Restore Database...", this, &MainWindow::onRestore);
    fileMenu->addAction("&Import Accounts CSV...", this, [this]() {
        ImportCsvDialog dialog(m_database, this);
        if (dialog.exec() == QDialog::Accepted) {
            m_accountsWidget->model()->refresh();
            refreshAllReports();
            statusBar()->showMessage(
                QString("Imported %1 accounts").arg(dialog.importedCount()), 5000);
        }
    });
    fileMenu->addAction("Change Encryption &Key...", this, &MainWindow::onChangeKey);
    fileMenu->addSeparator();
    fileMenu->addAction("Close &Period...", this, [this]() {
        ClosePeriodDialog dialog(m_database, this);
        if (dialog.exec() == QDialog::Accepted) {
            m_accountsWidget->model()->refresh();
            refreshAllReports();
            statusBar()->showMessage("Fiscal period closed successfully", 5000);
        }
    });
    fileMenu->addSeparator();
    auto *quitAction = fileMenu->addAction(
        style->standardIcon(QStyle::SP_DialogCloseButton),
        "&Quit", QKeySequence::Quit, this, &QWidget::close);
    Q_UNUSED(quitAction)

    auto *txnMenu = menuBar()->addMenu("&Transactions");
    txnMenu->addAction(
        style->standardIcon(QStyle::SP_FileDialogNewFolder),
        "&New Journal Entry...", QKeySequence(Qt::CTRL | Qt::Key_J),
        this, &MainWindow::onNewJournalEntry);

    auto *reportsMenu = menuBar()->addMenu("&Reports");
    reportsMenu->addAction(
        style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh &All Reports", QKeySequence(Qt::CTRL | Qt::Key_R),
        this, &MainWindow::refreshAllReports);

    // Window menu for toggling dock visibility
    auto *windowMenu = menuBar()->addMenu("&Window");
    windowMenu->addAction(m_accountsDock->toggleViewAction());
}

void MainWindow::setupToolBar()
{
    auto *style = QApplication::style();
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    toolbar->addAction(
        style->standardIcon(QStyle::SP_FileDialogNewFolder),
        "New Journal Entry", this, &MainWindow::onNewJournalEntry);
    toolbar->addSeparator();
    toolbar->addAction(
        style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh Reports", this, &MainWindow::refreshAllReports);
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

void MainWindow::onBackup()
{
    QString dest = QFileDialog::getSaveFileName(this, "Backup Database",
        "moneyman_backup.db", "SQLite Database (*.db)");
    if (dest.isEmpty()) return;

    // Close DB, copy file, reopen
    QString srcPath = m_database->databasePath();
    m_database->close();

    bool ok = QFile::copy(srcPath, dest);

    if (!m_database->open(srcPath)) {
        QMessageBox::critical(this, "Error", "Failed to reopen database after backup.");
        return;
    }

    if (ok) {
        statusBar()->showMessage("Backup saved to " + dest, 5000);
        m_database->logAudit("BACKUP", "Backed up to " + dest);
    } else {
        QMessageBox::warning(this, "Backup Failed",
            "Could not copy database file. The destination may already exist.");
    }
}

void MainWindow::onRestore()
{
    QString src = QFileDialog::getOpenFileName(this, "Restore Database",
        QString(), "SQLite Database (*.db)");
    if (src.isEmpty()) return;

    auto reply = QMessageBox::warning(this, "Confirm Restore",
        "This will replace all current data with the backup.\n\n"
        "This action cannot be undone. Continue?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QString destPath = m_database->databasePath();
    m_database->close();

    // Remove current and copy backup over
    QFile::remove(destPath);
    bool ok = QFile::copy(src, destPath);

    if (!ok) {
        QMessageBox::critical(this, "Restore Failed",
            "Could not copy backup file.");
        m_database->open(destPath);
        return;
    }

    if (!m_database->open(destPath)) {
        QMessageBox::critical(this, "Error",
            "Failed to open restored database: " + m_database->lastError());
        return;
    }

    m_database->logAudit("RESTORE", "Restored from " + src);
    m_accountsWidget->model()->refresh();
    refreshAllReports();
    statusBar()->showMessage("Database restored from " + src, 5000);
}

void MainWindow::onChangeKey()
{
    PasswordDialog dialog(PasswordDialog::ChangeKey, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (m_database->changeEncryptionKey(dialog.newPassword())) {
        statusBar()->showMessage("Encryption key changed successfully", 5000);
    } else {
        QMessageBox::warning(this, "Error",
            "Failed to change encryption key: " + m_database->lastError());
    }
}

void MainWindow::refreshAllReports()
{
    m_dashboardWidget->refresh();
    m_trialBalanceWidget->refresh();
    m_generalLedgerWidget->refresh();
    m_incomeStatementWidget->refresh();
    m_balanceSheetWidget->refresh();
    m_auditLogWidget->refresh();
}
