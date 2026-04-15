#include "startup_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFileInfo>

StartupDialog::StartupDialog(QWidget *parent)
    : QDialog(parent)
    , m_recentButton(nullptr)
{
    setWindowTitle("MoneyMan");
    setMinimumWidth(380);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    layout->setContentsMargins(32, 28, 32, 28);

    // Title
    auto *title = new QLabel("MoneyMan", this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto *subtitle = new QLabel("Double-entry bookkeeping", this);
    subtitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(subtitle);

    layout->addSpacing(8);

    // Buttons
    auto *createBtn = new QPushButton("Create New Database...", this);
    auto *openBtn = new QPushButton("Open Existing Database...", this);
    createBtn->setCursor(Qt::PointingHandCursor);
    openBtn->setCursor(Qt::PointingHandCursor);
    createBtn->setMinimumHeight(36);
    openBtn->setMinimumHeight(36);

    layout->addWidget(createBtn);
    layout->addWidget(openBtn);

    connect(createBtn, &QPushButton::clicked, this, &StartupDialog::onCreateNew);
    connect(openBtn, &QPushButton::clicked, this, &StartupDialog::onOpenExisting);

    // Recent shortcut
    QSettings settings;
    m_recentPath = settings.value("lastDatabasePath").toString();
    m_recentEncrypted = settings.value("lastDatabaseEncrypted", false).toBool();

    if (!m_recentPath.isEmpty() && QFileInfo::exists(m_recentPath)) {
        layout->addSpacing(4);
        auto *recentLabel = new QLabel("Recent:", this);
        layout->addWidget(recentLabel);

        QFileInfo fi(m_recentPath);
        QString label = fi.fileName();
        if (m_recentEncrypted)
            label += QString::fromUtf8("  \xF0\x9F\x94\x92"); // lock emoji

        m_recentButton = new QPushButton(label, this);
        m_recentButton->setToolTip(m_recentPath);
        m_recentButton->setFlat(true);
        m_recentButton->setCursor(Qt::PointingHandCursor);
        layout->addWidget(m_recentButton);

        connect(m_recentButton, &QPushButton::clicked,
                this, &StartupDialog::onOpenRecent);
    }

    layout->addSpacing(8);

    // Quit button
    auto *quitBtn = new QPushButton("Quit", this);
    quitBtn->setMinimumHeight(32);
    layout->addWidget(quitBtn);
    connect(quitBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void StartupDialog::onCreateNew()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Create New Database", "moneyman.db",
        "SQLite Database (*.db)");
    if (path.isEmpty())
        return;

    // Ask about encryption
    QDialog setupDlg(this);
    setupDlg.setWindowTitle("Database Options");
    setupDlg.setMinimumWidth(340);

    auto *dlgLayout = new QVBoxLayout(&setupDlg);
    auto *encryptCheck = new QCheckBox("Encrypt database", &setupDlg);
    dlgLayout->addWidget(encryptCheck);

    auto *form = new QFormLayout;
    auto *passEdit = new QLineEdit(&setupDlg);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText("Enter passphrase...");
    passEdit->setEnabled(false);
    auto *confirmEdit = new QLineEdit(&setupDlg);
    confirmEdit->setEchoMode(QLineEdit::Password);
    confirmEdit->setPlaceholderText("Confirm passphrase...");
    confirmEdit->setEnabled(false);
    form->addRow("Passphrase:", passEdit);
    form->addRow("Confirm:", confirmEdit);
    dlgLayout->addLayout(form);

    auto *errorLabel = new QLabel(&setupDlg);
    errorLabel->setStyleSheet("color: red;");
    errorLabel->hide();
    dlgLayout->addWidget(errorLabel);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &setupDlg);
    dlgLayout->addWidget(buttons);

    connect(encryptCheck, &QCheckBox::toggled, passEdit, &QLineEdit::setEnabled);
    connect(encryptCheck, &QCheckBox::toggled, confirmEdit, &QLineEdit::setEnabled);
    connect(buttons, &QDialogButtonBox::rejected, &setupDlg, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &setupDlg, [&]() {
        if (encryptCheck->isChecked()) {
            if (passEdit->text().isEmpty()) {
                errorLabel->setText("Passphrase cannot be empty.");
                errorLabel->show();
                return;
            }
            if (passEdit->text() != confirmEdit->text()) {
                errorLabel->setText("Passphrases do not match.");
                errorLabel->show();
                return;
            }
        }
        setupDlg.accept();
    });

    if (setupDlg.exec() != QDialog::Accepted)
        return;

    m_databasePath = path;
    m_encrypted = encryptCheck->isChecked();
    m_passphrase = m_encrypted ? passEdit->text() : QString();
    accept();
}

void StartupDialog::onOpenExisting()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open Database", QString(),
        "SQLite Database (*.db);;All Files (*)");
    if (path.isEmpty())
        return;

    // Prompt for optional passphrase
    QDialog pwDlg(this);
    pwDlg.setWindowTitle("Open Database");
    pwDlg.setMinimumWidth(340);

    auto *dlgLayout = new QVBoxLayout(&pwDlg);
    auto *hint = new QLabel("Enter passphrase, or leave blank if unencrypted.", &pwDlg);
    hint->setWordWrap(true);
    dlgLayout->addWidget(hint);

    auto *form = new QFormLayout;
    auto *passEdit = new QLineEdit(&pwDlg);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText("Passphrase (optional)...");
    form->addRow("Passphrase:", passEdit);
    dlgLayout->addLayout(form);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &pwDlg);
    dlgLayout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &pwDlg, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &pwDlg, &QDialog::accept);

    if (pwDlg.exec() != QDialog::Accepted)
        return;

    m_databasePath = path;
    m_passphrase = passEdit->text();
    m_encrypted = !m_passphrase.isEmpty();
    accept();
}

void StartupDialog::onOpenRecent()
{
    m_databasePath = m_recentPath;
    m_encrypted = m_recentEncrypted;

    if (m_encrypted) {
        // Need passphrase
        QDialog pwDlg(this);
        pwDlg.setWindowTitle("Unlock Database");
        pwDlg.setMinimumWidth(340);

        auto *dlgLayout = new QVBoxLayout(&pwDlg);
        auto *form = new QFormLayout;
        auto *passEdit = new QLineEdit(&pwDlg);
        passEdit->setEchoMode(QLineEdit::Password);
        passEdit->setPlaceholderText("Enter passphrase...");
        form->addRow("Passphrase:", passEdit);
        dlgLayout->addLayout(form);

        auto *errorLabel = new QLabel(&pwDlg);
        errorLabel->setStyleSheet("color: red;");
        errorLabel->hide();
        dlgLayout->addWidget(errorLabel);

        auto *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &pwDlg);
        dlgLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::rejected, &pwDlg, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, &pwDlg, [&]() {
            if (passEdit->text().isEmpty()) {
                errorLabel->setText("Passphrase cannot be empty.");
                errorLabel->show();
                return;
            }
            pwDlg.accept();
        });

        if (pwDlg.exec() != QDialog::Accepted)
            return;

        m_passphrase = passEdit->text();
    } else {
        m_passphrase.clear();
    }

    accept();
}
