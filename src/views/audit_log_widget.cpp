#include "audit_log_widget.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QToolBar>
#include <QApplication>
#include <QStyle>
#include "utils/csv_export.h"
#include "utils/print_report.h"

AuditLogWidget::AuditLogWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_table(new QTableWidget(this))
{
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"#", "Timestamp", "Action", "Details"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->verticalHeader()->hide();
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 160);
    m_table->setColumnWidth(2, 160);

    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh", this, &AuditLogWidget::refresh);
    toolbar->addAction(style->standardIcon(QStyle::SP_DialogSaveButton),
        "Export CSV", this, &AuditLogWidget::exportCsv);
    toolbar->addAction(style->standardIcon(QStyle::SP_FileDialogDetailedView),
        "Export PDF", this, [this]() {
            printReportToPdf(m_table, "Audit Log", this, "audit_log.pdf");
        });

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(m_table, 1);

    refresh();
}

void AuditLogWidget::refresh()
{
    auto entries = m_db->allAuditLog();
    m_table->setRowCount(static_cast<int>(entries.size()));

    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        const auto &entry = entries[static_cast<size_t>(i)];

        auto *idItem = new QTableWidgetItem(QString::number(entry.id));
        idItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        auto *tsItem = new QTableWidgetItem(entry.timestamp);
        auto *actionItem = new QTableWidgetItem(entry.action);
        auto *detailsItem = new QTableWidgetItem(entry.details);

        m_table->setItem(i, 0, idItem);
        m_table->setItem(i, 1, tsItem);
        m_table->setItem(i, 2, actionItem);
        m_table->setItem(i, 3, detailsItem);
    }
}

void AuditLogWidget::exportCsv()
{
    exportTableToCsv(m_table, this, "audit_log.csv");
}
