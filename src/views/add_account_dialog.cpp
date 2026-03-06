#include "add_account_dialog.h"
#include "accounting/tax_categories.h"

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

    m_currencyCombo = new QComboBox(this);
    m_currencyCombo->addItem("USD");
    m_currencyCombo->addItem("EUR");
    m_currencyCombo->addItem("GBP");
    m_currencyCombo->addItem("CAD");
    m_currencyCombo->addItem("JPY");
    m_currencyCombo->addItem("AUD");
    m_currencyCombo->addItem("CHF");
    m_currencyCombo->setEditable(true);
    form->addRow("Currency:", m_currencyCombo);

    m_taxCategoryCombo = new QComboBox(this);
    m_taxCategoryCombo->setEditable(true);
    m_taxCategoryCombo->addItem("");
    m_taxCategoryCombo->addItems(standardTaxCategories());
    form->addRow("Tax Category:", m_taxCategoryCombo);

    // Mark as manually set when user edits the combo directly
    connect(m_taxCategoryCombo, &QComboBox::currentTextChanged, this, [this]() {
        if (m_taxCategoryCombo->hasFocus())
            m_taxCategoryManuallySet = true;
    });

    // Auto-suggest code range when type changes
    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, [this]() {
        if (!m_codeSpinBox->isEnabled()) return; // edit mode — don't change code
        QString type = m_typeCombo->currentData().toString();
        int base = 1000;
        if (type == "asset")         base = 1000;
        else if (type == "liability") base = 2000;
        else if (type == "equity")   base = 3000;
        else if (type == "revenue")  base = 4000;
        else if (type == "expense")  base = 5000;
        m_codeSpinBox->setValue(base);
        updateTaxCategorySuggestion();
    });

    // Auto-suggest tax category when name changes
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this]() {
        updateTaxCategorySuggestion();
    });

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void AddAccountDialog::setEditMode(int code, const QString &name, const QString &type,
                                    const QString &taxCategory)
{
    setWindowTitle("Edit Account");
    m_codeSpinBox->setValue(code);
    m_codeSpinBox->setEnabled(false);
    m_nameEdit->setText(name);
    int idx = m_typeCombo->findData(type);
    if (idx >= 0) m_typeCombo->setCurrentIndex(idx);
    m_taxCategoryCombo->setCurrentText(taxCategory);
    m_taxCategoryManuallySet = true; // don't override existing value
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

QString AddAccountDialog::accountCurrency() const
{
    return m_currencyCombo->currentText().trimmed().toUpper();
}

QString AddAccountDialog::taxCategory() const
{
    return m_taxCategoryCombo->currentText().trimmed();
}

void AddAccountDialog::updateTaxCategorySuggestion()
{
    if (m_taxCategoryManuallySet) return;
    QString suggested = suggestTaxCategory(
        m_codeSpinBox->value(), m_nameEdit->text(),
        m_typeCombo->currentData().toString());
    m_taxCategoryCombo->setCurrentText(suggested);
}
