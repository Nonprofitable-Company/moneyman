#ifndef STARTUP_DIALOG_H
#define STARTUP_DIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;

class StartupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartupDialog(QWidget *parent = nullptr);

    QString databasePath() const { return m_databasePath; }
    QString passphrase() const { return m_passphrase; }
    bool encrypted() const { return m_encrypted; }

private slots:
    void onCreateNew();
    void onOpenExisting();
    void onOpenRecent();

private:
    bool promptPassphrase(bool isNew);

    QString m_databasePath;
    QString m_passphrase;
    bool m_encrypted = false;

    QPushButton *m_recentButton;
    QString m_recentPath;
    bool m_recentEncrypted = false;
};

#endif // STARTUP_DIALOG_H
