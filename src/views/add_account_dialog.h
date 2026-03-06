#ifndef ADD_ACCOUNT_DIALOG_H
#define ADD_ACCOUNT_DIALOG_H

#include <QDialog>

class QSpinBox;
class QLineEdit;
class QComboBox;

class AddAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAccountDialog(QWidget *parent = nullptr);

    void setEditMode(int code, const QString &name, const QString &type,
                     const QString &taxCategory = QString());

    int accountCode() const;
    QString accountName() const;
    QString accountType() const;
    QString accountCurrency() const;
    QString taxCategory() const;

private:
    void updateTaxCategorySuggestion();

    QSpinBox *m_codeSpinBox;
    QLineEdit *m_nameEdit;
    QComboBox *m_typeCombo;
    QComboBox *m_currencyCombo;
    QComboBox *m_taxCategoryCombo;
    bool m_taxCategoryManuallySet = false;
};

#endif // ADD_ACCOUNT_DIALOG_H
