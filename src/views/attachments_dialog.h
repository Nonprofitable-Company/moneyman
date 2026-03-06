#ifndef ATTACHMENTS_DIALOG_H
#define ATTACHMENTS_DIALOG_H

#include <QDialog>
#include <cstdint>

class Database;
class QListWidget;

class AttachmentsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AttachmentsDialog(Database *db, int64_t entryId, QWidget *parent = nullptr);

private slots:
    void onAdd();
    void onOpen();
    void onRemove();

private:
    void refreshList();

    Database *m_db;
    int64_t m_entryId;
    QListWidget *m_list;
};

#endif // ATTACHMENTS_DIALOG_H
