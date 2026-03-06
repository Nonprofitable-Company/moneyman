#ifndef SIDEBAR_WIDGET_H
#define SIDEBAR_WIDGET_H

#include <QWidget>
#include <QVector>
#include <QString>

class QPushButton;
class QVBoxLayout;

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);

    int addItem(const QString &iconPath, const QString &label);
    void addSeparator();
    void addSectionTitle(const QString &title);
    void setCurrentIndex(int index);
    int currentIndex() const;

signals:
    void currentChanged(int index);

private slots:
    void onThemeChanged(bool dark);

private:
    void onButtonClicked(int index);
    void reloadIcons();

    QVBoxLayout *m_layout;
    QVector<QPushButton *> m_buttons;
    QVector<QString> m_iconPaths;
    int m_currentIndex = -1;
    bool m_dark = true;
};

#endif // SIDEBAR_WIDGET_H
