#include "journal_list_widget.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QToolBar>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QIcon>
#include "utils/csv_export.h"
#include "utils/print_report.h"
#include "attachments_dialog.h"

JournalListWidget::JournalListWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_table(new QTableWidget(this))
    , m_filterEdit(new QLineEdit(this))
{
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"#", "Date", "Description", "Lines", "Attach", "Status"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->verticalHeader()->hide();
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 100);
    m_table->setColumnWidth(3, 60);
    m_table->setColumnWidth(4, 60);
    m_table->setColumnWidth(5, 80);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_table, &QTableWidget::customContextMenuRequested,
            this, &JournalListWidget::onContextMenu);

    m_filterEdit->setPlaceholderText("Search entries by description...");
    m_filterEdit->setClearButtonEnabled(true);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &JournalListWidget::onFilterChanged);

    auto *toolbar = new QToolBar(this);
    toolbar->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh", this, &JournalListWidget::refresh);
    toolbar->addAction(QIcon(":/icons/export-csv.svg"),
        "Export CSV", this, &JournalListWidget::exportCsv);
    toolbar->addAction(QIcon(":/icons/export-pdf.svg"),
        "Export PDF", this, [this]() {
            printReportToPdf(m_table, "Journal Entries", this, "journal_entries.pdf");
        });

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_filterEdit);
    layout->addWidget(m_table, 1);

    refresh();
}

void JournalListWidget::refresh()
{
    auto entries = m_db->allJournalEntries();
    m_table->setRowCount(static_cast<int>(entries.size()));

    QString filter = m_filterEdit->text().toLower();

    for (size_t i = 0; i < entries.size(); ++i) {
        const auto &entry = entries[i];
        int row = static_cast<int>(i);

        bool isVoided = entry.description.startsWith("[VOIDED]") ||
                        entry.description.startsWith("[VOID");
        QString status = isVoided ? "Voided" : "Posted";

        auto attachments = m_db->attachmentsForEntry(entry.id);
        int attachCount = static_cast<int>(attachments.size());

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(entry.id)));
        m_table->setItem(row, 1, new QTableWidgetItem(entry.date));
        m_table->setItem(row, 2, new QTableWidgetItem(entry.description));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(entry.lines.size())));
        m_table->setItem(row, 4, new QTableWidgetItem(attachCount > 0 ? QString::number(attachCount) : ""));
        m_table->setItem(row, 5, new QTableWidgetItem(status));

        // Store entry ID for context menu
        m_table->item(row, 0)->setData(Qt::UserRole, static_cast<qlonglong>(entry.id));

        // Apply filter visibility
        bool matches = filter.isEmpty() || entry.description.toLower().contains(filter)
                       || entry.date.contains(filter);
        m_table->setRowHidden(row, !matches);

        if (isVoided) {
            for (int c = 0; c < 6; ++c) {
                m_table->item(row, c)->setForeground(Qt::gray);
            }
        }
    }
}

void JournalListWidget::onFilterChanged(const QString &text)
{
    Q_UNUSED(text)
    refresh();
}

void JournalListWidget::onContextMenu(const QPoint &pos)
{
    QModelIndex index = m_table->indexAt(pos);
    if (!index.isValid()) return;

    int row = index.row();
    auto *idItem = m_table->item(row, 0);
    if (!idItem) return;

    int64_t entryId = idItem->data(Qt::UserRole).toLongLong();
    QString desc = m_table->item(row, 2)->text();
    bool isVoided = desc.startsWith("[VOIDED]") || desc.startsWith("[VOID");

    QMenu menu(this);

    if (!isVoided) {
        menu.addAction("Void Entry...", this, [this, entryId]() {
            auto reply = QMessageBox::question(this, "Void Journal Entry",
                QString("Void journal entry #%1?\n\n"
                        "This will create a reversing entry to zero out all balances.")
                    .arg(entryId));
            if (reply != QMessageBox::Yes) return;

            if (!m_db->voidJournalEntry(entryId)) {
                QMessageBox::warning(this, "Error",
                    "Failed to void entry: " + m_db->lastError());
                return;
            }
            refresh();
        });
    }

    menu.addAction("View Details...", this, [this, entryId]() {
        auto entry = m_db->journalEntryById(entryId);
        QString details = QString("Entry #%1\nDate: %2\nDescription: %3\n\nLines:\n")
            .arg(entry.id).arg(entry.date, entry.description);
        for (const auto &line : entry.lines) {
            auto acct = m_db->accountById(line.accountId);
            details += QString("  %1: Dr %2  Cr %3\n")
                .arg(acct.name)
                .arg(QString::number(static_cast<double>(line.debitCents) / 100.0, 'f', 2))
                .arg(QString::number(static_cast<double>(line.creditCents) / 100.0, 'f', 2));
        }
        QMessageBox::information(this, "Journal Entry Details", details);
    });

    menu.addAction("Attachments...", this, [this, entryId]() {
        AttachmentsDialog dialog(m_db, entryId, this);
        dialog.exec();
        refresh();
    });

    menu.exec(m_table->viewport()->mapToGlobal(pos));
}

void JournalListWidget::exportCsv()
{
    exportTableToCsv(m_table, this, "journal_entries.csv");
}
