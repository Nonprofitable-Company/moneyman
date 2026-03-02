#include "main_window.h"
#include "db/database.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
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

    auto *central = new QLabel("MoneyMan Double-Entry Bookkeeping", this);
    central->setAlignment(Qt::AlignCenter);
    setCentralWidget(central);
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
