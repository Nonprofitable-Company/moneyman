#include "add_account_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>

AddAccountDialog::AddAccountDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Add Account");
    setMinimumWidth(350);

    auto *form = new QFormLayout;

    m_codeSpinBox = new QSpinBox(this);
    m_codeSpinBox->setRange(1000, 9999);
    m_codeSpinBox->setValue(1000);
    form->addRow("Account Code:", m_codeSpinBox);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("e.g., Cash, Accounts Payable");
    form->addRow("Account Name:", m_nameEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Asset", "asset");
    m_typeCombo->addItem("Liability", "liability");
    m_typeCombo->addItem("Equity", "equity");
    m_typeCombo->addItem("Revenue", "revenue");
    m_typeCombo->addItem("Expense", "expense");
    form->addRow("Account Type:", m_typeCombo);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

int AddAccountDialog::accountCode() const
{
    return m_codeSpinBox->value();
}

QString AddAccountDialog::accountName() const
{
    return m_nameEdit->text().trimmed();
}

QString AddAccountDialog::accountType() const
{
    return m_typeCombo->currentData().toString();
}
