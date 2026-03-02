#include "main_window.h"
#include "db/database.h"
#include "views/accounts_widget.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
    , m_accountsWidget(nullptr)
{
    setupUi();
    setupMenuBar();
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
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}
