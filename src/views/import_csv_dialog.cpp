#include "import_csv_dialog.h"
#include "db/database.h"
#include "utils/csv_import.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>

ImportCsvDialog::ImportCsvDialog(Database *db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
    , m_preview(new QTableWidget(this))
    , m_statusLabel(new QLabel("Select a CSV file with columns: code, name, type, currency", this))
{
    setWindowTitle("Import Chart of Accounts from CSV");
    setMinimumSize(600, 400);

    auto *browseBtn = new QPushButton("Browse...", this);
    auto *importBtn = new QPushButton("Import", this);
    auto *cancelBtn = new QPushButton("Cancel", this);
    importBtn->setEnabled(false);

    m_preview->setColumnCount(4);
    m_preview->setHorizontalHeaderLabels({"Code", "Name", "Type", "Currency"});
    m_preview->horizontalHeader()->setStretchLastSection(true);
    m_preview->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(browseBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(importBtn);
    btnLayout->addWidget(cancelBtn);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_preview);
    layout->addLayout(btnLayout);

    connect(browseBtn, &QPushButton::clicked, this, &ImportCsvDialog::onBrowse);
    connect(importBtn, &QPushButton::clicked, this, &ImportCsvDialog::onImport);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_importBtn = importBtn;
}

void ImportCsvDialog::onBrowse()
{
    QString path = QFileDialog::getOpenFileName(this, "Select CSV File",
        QString(), "CSV Files (*.csv);;All Files (*)");
    if (path.isEmpty())
        return;

    m_filePath = path;
    QString error;
    auto rows = parseCsvFile(path, error);
    if (!error.isEmpty()) {
        QMessageBox::warning(this, "Import Error", error);
        return;
    }

    if (rows.empty()) {
        m_statusLabel->setText("File is empty.");
        return;
    }

    // Check if first row is a header
    int startRow = 0;
    if (rows[0].size() >= 2) {
        QString first = rows[0][0].toLower();
        if (first == "code" || first == "account code" || first == "acct")
            startRow = 1;
    }

    m_preview->setRowCount(0);
    for (size_t i = startRow; i < rows.size(); ++i) {
        const auto &fields = rows[i];
        if (fields.size() < 3) continue; // need at least code, name, type

        int row = m_preview->rowCount();
        m_preview->insertRow(row);
        m_preview->setItem(row, 0, new QTableWidgetItem(fields[0]));
        m_preview->setItem(row, 1, new QTableWidgetItem(fields[1]));
        m_preview->setItem(row, 2, new QTableWidgetItem(fields[2]));
        m_preview->setItem(row, 3, new QTableWidgetItem(
            fields.size() > 3 ? fields[3] : "USD"));
    }

    m_statusLabel->setText(QString("Preview: %1 accounts to import from %2")
        .arg(m_preview->rowCount()).arg(path));

    m_importBtn->setEnabled(m_preview->rowCount() > 0);
}

void ImportCsvDialog::onImport()
{
    int success = 0;
    int failed = 0;

    for (int i = 0; i < m_preview->rowCount(); ++i) {
        bool ok = false;
        int code = m_preview->item(i, 0)->text().toInt(&ok);
        if (!ok) { ++failed; continue; }

        QString name = m_preview->item(i, 1)->text();
        QString type = m_preview->item(i, 2)->text().toLower();
        QString currency = m_preview->item(i, 3)->text().toUpper();
        if (currency.isEmpty()) currency = "USD";

        if (m_db->createAccount(code, name, type, currency)) {
            ++success;
        } else {
            ++failed;
        }
    }

    m_importedCount = success;

    if (failed > 0) {
        QMessageBox::information(this, "Import Complete",
            QString("Imported %1 accounts. %2 failed (duplicate codes or invalid data).")
                .arg(success).arg(failed));
    }

    if (success > 0)
        accept();
}
