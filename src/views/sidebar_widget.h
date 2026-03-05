#ifndef SIDEBAR_WIDGET_H
#define SIDEBAR_WIDGET_H

#include <QWidget>
#include <QVector>

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

private:
    void onButtonClicked(int index);

    QVBoxLayout *m_layout;
    QVector<QPushButton *> m_buttons;
    int m_currentIndex = -1;
};

#endif // SIDEBAR_WIDGET_H
