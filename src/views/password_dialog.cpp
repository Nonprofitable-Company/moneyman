#include "password_dialog.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>

PasswordDialog::PasswordDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_passwordEdit(new QLineEdit(this))
    , m_newPasswordEdit(nullptr)
    , m_confirmEdit(nullptr)
    , m_errorLabel(new QLabel(this))
{
    setWindowTitle(mode == Unlock ? "Unlock Database" : "Change Encryption Key");
    setMinimumWidth(350);

    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter passphrase...");

    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->hide();

    auto *form = new QFormLayout;
    form->addRow(mode == Unlock ? "Passphrase:" : "Current passphrase:", m_passwordEdit);

    if (mode == ChangeKey) {
        m_newPasswordEdit = new QLineEdit(this);
        m_newPasswordEdit->setEchoMode(QLineEdit::Password);
        m_newPasswordEdit->setPlaceholderText("New passphrase...");
        m_confirmEdit = new QLineEdit(this);
        m_confirmEdit->setEchoMode(QLineEdit::Password);
        m_confirmEdit->setPlaceholderText("Confirm new passphrase...");
        form->addRow("New passphrase:", m_newPasswordEdit);
        form->addRow("Confirm:", m_confirmEdit);
    }

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &PasswordDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_errorLabel);
    layout->addWidget(buttons);

    m_passwordEdit->setFocus();
}

QString PasswordDialog::password() const
{
    return m_passwordEdit->text();
}

QString PasswordDialog::newPassword() const
{
    return m_newPasswordEdit ? m_newPasswordEdit->text() : QString();
}

void PasswordDialog::onAccept()
{
    if (m_passwordEdit->text().isEmpty()) {
        m_errorLabel->setText("Passphrase cannot be empty.");
        m_errorLabel->show();
        return;
    }

    if (m_mode == ChangeKey) {
        if (m_newPasswordEdit->text().isEmpty()) {
            m_errorLabel->setText("New passphrase cannot be empty.");
            m_errorLabel->show();
            return;
        }
        if (m_newPasswordEdit->text() != m_confirmEdit->text()) {
            m_errorLabel->setText("New passphrases do not match.");
            m_errorLabel->show();
            return;
        }
    }

    accept();
}
