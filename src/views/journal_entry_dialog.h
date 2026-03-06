#ifndef JOURNAL_ENTRY_DIALOG_H
#define JOURNAL_ENTRY_DIALOG_H

#include <QDialog>
#include <QByteArray>
#include <QString>
#include <vector>

class QDateEdit;
class QLineEdit;
class QTableView;
class QLabel;
class QPushButton;
class QListWidget;
class Database;
class JournalLineModel;

struct PendingAttachment {
    QString filename;
    QString mimeType;
    QByteArray data;
};

class JournalEntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JournalEntryDialog(Database *db, QWidget *parent = nullptr);

private slots:
    void onAddLine();
    void onRemoveLine();
    void onTotalsChanged();
    void onPost();
    void onSaveTemplate();
    void onLoadTemplate();
    void onAddAttachment();
    void onRemoveAttachment();

private:
    Database *m_db;
    JournalLineModel *m_lineModel;

    QDateEdit *m_dateEdit;
    QLineEdit *m_descriptionEdit;
    QTableView *m_lineTable;
    QLabel *m_debitTotalLabel;
    QLabel *m_creditTotalLabel;
    QLabel *m_balanceStatusLabel;
    QPushButton *m_postButton;

    QListWidget *m_attachmentList;
    std::vector<PendingAttachment> m_pendingAttachments;
};

#endif // JOURNAL_ENTRY_DIALOG_H
