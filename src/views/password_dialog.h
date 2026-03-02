#ifndef PASSWORD_DIALOG_H
#define PASSWORD_DIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { Unlock, ChangeKey };

    explicit PasswordDialog(Mode mode = Unlock, QWidget *parent = nullptr);

    QString password() const;
    QString newPassword() const; // Only for ChangeKey mode

private slots:
    void onAccept();

private:
    Mode m_mode;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_newPasswordEdit;
    QLineEdit *m_confirmEdit;
    QLabel *m_errorLabel;
};

#endif // PASSWORD_DIALOG_H
