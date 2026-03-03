#ifndef HELP_BROWSER_DIALOG_H
#define HELP_BROWSER_DIALOG_H

#include <QDialog>

class QListWidget;
class QTextBrowser;

class HelpBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpBrowserDialog(QWidget *parent = nullptr);

private:
    void setupUi();
    void populateToc();
    QString helpContent() const;

    QListWidget *m_tocList;
    QTextBrowser *m_browser;
};

#endif // HELP_BROWSER_DIALOG_H
