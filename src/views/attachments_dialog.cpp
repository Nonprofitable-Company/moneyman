#include "attachments_dialog.h"
#include "db/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTemporaryDir>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>

AttachmentsDialog::AttachmentsDialog(Database *db, int64_t entryId, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
    , m_entryId(entryId)
    , m_list(new QListWidget(this))
{
    setWindowTitle(QString("Attachments — Entry #%1").arg(entryId));
    setMinimumSize(400, 300);

    auto *addBtn = new QPushButton("Add...", this);
    auto *openBtn = new QPushButton("Open", this);
    auto *removeBtn = new QPushButton("Remove", this);
    auto *closeBtn = new QPushButton("Close", this);

    connect(addBtn, &QPushButton::clicked, this, &AttachmentsDialog::onAdd);
    connect(openBtn, &QPushButton::clicked, this, &AttachmentsDialog::onOpen);
    connect(removeBtn, &QPushButton::clicked, this, &AttachmentsDialog::onRemove);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto *btnLayout = new QVBoxLayout;
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(openBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_list, 1);
    mainLayout->addLayout(btnLayout);

    refreshList();
}

void AttachmentsDialog::refreshList()
{
    m_list->clear();
    auto metas = m_db->attachmentsForEntry(m_entryId);
    for (const auto &meta : metas) {
        auto *item = new QListWidgetItem(
            QString("%1  (%2)").arg(meta.filename, meta.createdAt), m_list);
        item->setData(Qt::UserRole, static_cast<qlonglong>(meta.id));
    }
}

void AttachmentsDialog::onAdd()
{
    QString path = QFileDialog::getOpenFileName(this, "Attach File", QString(),
        "Supported Files (*.png *.jpg *.jpeg *.pdf);;All Files (*)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not read file: " + path);
        return;
    }

    QByteArray data = file.readAll();
    QString filename = QFileInfo(path).fileName();
    QMimeDatabase mimeDb;
    QString mimeType = mimeDb.mimeTypeForFile(path).name();

    if (m_db->addAttachment(m_entryId, filename, mimeType, data) < 0) {
        QMessageBox::warning(this, "Error",
            "Failed to add attachment: " + m_db->lastError());
        return;
    }

    refreshList();
}

void AttachmentsDialog::onOpen()
{
    auto *item = m_list->currentItem();
    if (!item) return;

    int64_t attachId = item->data(Qt::UserRole).toLongLong();
    auto row = m_db->attachmentById(attachId);
    if (row.id == 0) return;

    // Write BLOB to temp file and open with system viewer
    static QTemporaryDir tmpDir;
    if (!tmpDir.isValid()) return;

    QString tmpPath = tmpDir.path() + "/" + QString::number(row.id) + "_" + row.filename;
    QFile tmpFile(tmpPath);
    if (!tmpFile.open(QIODevice::WriteOnly)) return;
    tmpFile.write(row.data);
    tmpFile.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(tmpPath));
}

void AttachmentsDialog::onRemove()
{
    auto *item = m_list->currentItem();
    if (!item) return;

    int64_t attachId = item->data(Qt::UserRole).toLongLong();
    if (!m_db->deleteAttachment(attachId)) {
        QMessageBox::warning(this, "Error",
            "Failed to remove attachment: " + m_db->lastError());
        return;
    }

    refreshList();
}
