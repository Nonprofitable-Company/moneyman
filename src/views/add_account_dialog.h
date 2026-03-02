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

    int accountCode() const;
    QString accountName() const;
    QString accountType() const;

private:
    QSpinBox *m_codeSpinBox;
    QLineEdit *m_nameEdit;
    QComboBox *m_typeCombo;
};

#endif // ADD_ACCOUNT_DIALOG_H
