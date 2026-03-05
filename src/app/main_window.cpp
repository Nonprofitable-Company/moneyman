#include "main_window.h"
#include "db/database.h"
#include "views/dashboard_widget.h"
#include "views/accounts_widget.h"
#include "views/journal_entry_dialog.h"
#include "views/trial_balance_widget.h"
#include "views/general_ledger_widget.h"
#include "views/income_statement_widget.h"
#include "views/balance_sheet_widget.h"
#include "views/journal_list_widget.h"
#include "views/audit_log_widget.h"
#include "views/close_period_dialog.h"
#include "views/password_dialog.h"
#include "views/import_csv_dialog.h"
#include "views/help_browser_dialog.h"
#include "views/sidebar_widget.h"
#include "theme/theme_manager.h"
#include "models/account_model.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QShortcut>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
    , m_sidebar(nullptr)
    , m_stack(nullptr)
    , m_accountsWidget(nullptr)
    , m_dashboardWidget(nullptr)
    , m_trialBalanceWidget(nullptr)
    , m_generalLedgerWidget(nullptr)
    , m_incomeStatementWidget(nullptr)
    , m_balanceSheetWidget(nullptr)
    , m_journalListWidget(nullptr)
    , m_auditLogWidget(nullptr)
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    PasswordDialog pwDialog(PasswordDialog::Unlock, this);
    if (pwDialog.exec() != QDialog::Accepted) {
        QMessageBox::warning(this, "No Passphrase",
            "A passphrase is required to open the database.");
    } else {
        m_passphrase = pwDialog.password();
        if (!m_database->open(QString(), m_passphrase)) {
            QMessageBox::critical(this, "Database Error",
                "Failed to open database: " + m_database->lastError()
                + "\n\nThe passphrase may be incorrect.");
        }
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle("MoneyMan \u2014 The Nonprofitable Company");
    resize(1200, 800);

    // Create sidebar
    m_sidebar = new SidebarWidget(this);
    m_sidebar->addSectionTitle("REPORTS");
    m_sidebar->addItem(":/icons/dashboard.svg", "  Dashboard");
    m_sidebar->addItem(":/icons/trial-balance.svg", "  Trial Balance");
    m_sidebar->addItem(":/icons/general-ledger.svg", "  General Ledger");
    m_sidebar->addItem(":/icons/income-statement.svg", "  Income Statement");
    m_sidebar->addItem(":/icons/balance-sheet.svg", "  Balance Sheet");
    m_sidebar->addItem(":/icons/journal-entries.svg", "  Journal Entries");
    m_sidebar->addItem(":/icons/audit-log.svg", "  Audit Log");
    m_sidebar->addSeparator();
    m_sidebar->addSectionTitle("DATA");
    m_sidebar->addItem(":/icons/accounts.svg", "  Chart of Accounts");

    // Create stacked widget with all pages
    m_stack = new QStackedWidget(this);
    m_dashboardWidget = new DashboardWidget(m_database, this);
    m_trialBalanceWidget = new TrialBalanceWidget(m_database, this);
    m_generalLedgerWidget = new GeneralLedgerWidget(m_database, this);
    m_incomeStatementWidget = new IncomeStatementWidget(m_database, this);
    m_balanceSheetWidget = new BalanceSheetWidget(m_database, this);
    m_journalListWidget = new JournalListWidget(m_database, this);
    m_auditLogWidget = new AuditLogWidget(m_database, this);
    m_accountsWidget = new AccountsWidget(m_database, this);

    m_stack->addWidget(m_dashboardWidget);      // 0
    m_stack->addWidget(m_trialBalanceWidget);    // 1
    m_stack->addWidget(m_generalLedgerWidget);   // 2
    m_stack->addWidget(m_incomeStatementWidget); // 3
    m_stack->addWidget(m_balanceSheetWidget);    // 4
    m_stack->addWidget(m_journalListWidget);     // 5
    m_stack->addWidget(m_auditLogWidget);        // 6
    m_stack->addWidget(m_accountsWidget);        // 7

    connect(m_sidebar, &SidebarWidget::currentChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
    m_sidebar->setCurrentIndex(0);

    // Central widget: sidebar + stack in horizontal layout
    auto *central = new QWidget(this);
    auto *hbox = new QHBoxLayout(central);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    hbox->addWidget(m_sidebar);
    hbox->addWidget(m_stack, 1);
    setCentralWidget(central);
}

void MainWindow::setupMenuBar()
{
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
    fileMenu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto *txnMenu = menuBar()->addMenu("&Transactions");
    txnMenu->addAction(QIcon(":/icons/add.svg"),
        "&New Journal Entry...", QKeySequence(Qt::CTRL | Qt::Key_J),
        this, &MainWindow::onNewJournalEntry);

    auto *reportsMenu = menuBar()->addMenu("&Reports");
    reportsMenu->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh &All Reports", QKeySequence(Qt::CTRL | Qt::Key_R),
        this, &MainWindow::refreshAllReports);

    // Window menu with theme selection
    auto *windowMenu = menuBar()->addMenu("&Window");
    ThemeManager::instance()->populateMenu(windowMenu);

    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&User Guide", QKeySequence::HelpContents, this, [this]() {
        HelpBrowserDialog dialog(this);
        dialog.exec();
    });
    helpMenu->addSeparator();
    helpMenu->addAction("&About MoneyMan", this, [this]() {
        QMessageBox::about(this, "About MoneyMan",
            "<h2>MoneyMan v0.1.0</h2>"
            "<p>Double-entry bookkeeping for The Nonprofitable Company.</p>"
            "<p>Features: Chart of Accounts, Journal Entries, Trial Balance, "
            "General Ledger, Income Statement, Balance Sheet, Audit Log, "
            "Templates, Fiscal Periods, CSV/PDF Export, Encrypted Database.</p>"
            "<p>Built with Qt6 and SQLCipher.</p>");
    });

    // Navigation shortcuts (Ctrl+1 through Ctrl+8)
    for (int i = 0; i < m_stack->count() && i < 9; ++i) {
        auto *shortcut = new QShortcut(
            QKeySequence(Qt::CTRL | static_cast<Qt::Key>(Qt::Key_1 + i)), this);
        connect(shortcut, &QShortcut::activated, this, [this, i]() {
            m_sidebar->setCurrentIndex(i);
        });
    }
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    toolbar->addAction(QIcon(":/icons/add.svg"),
        "New Journal Entry", this, &MainWindow::onNewJournalEntry);
    toolbar->addSeparator();
    toolbar->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh Reports", this, &MainWindow::refreshAllReports);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready \u2014 " + m_database->databasePath());
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

    QString srcPath = m_database->databasePath();
    m_database->close();

    bool ok = QFile::copy(srcPath, dest);

    if (!m_database->open(srcPath, m_passphrase)) {
        QMessageBox::critical(this, "Error",
            "Failed to reopen database after backup.");
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

    QFile::remove(destPath);
    bool ok = QFile::copy(src, destPath);

    if (!ok) {
        QMessageBox::critical(this, "Restore Failed",
            "Could not copy backup file.");
        m_database->open(destPath, m_passphrase);
        return;
    }

    if (!m_database->open(destPath, m_passphrase)) {
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
        m_passphrase = dialog.newPassword();
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
    m_journalListWidget->refresh();
    m_auditLogWidget->refresh();
}
