#include "sidebar_widget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QStyle>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
{
    setObjectName("sidebar");
    m_layout->setContentsMargins(0, 8, 0, 8);
    m_layout->setSpacing(0);
    m_layout->addStretch();
}

int SidebarWidget::addItem(const QString &iconPath, const QString &label)
{
    auto *button = new QPushButton(QIcon(iconPath), label, this);
    button->setFlat(true);
    button->setIconSize(QSize(20, 20));
    button->setCursor(Qt::PointingHandCursor);

    int index = m_buttons.size();
    m_buttons.append(button);

    // Insert before the stretch
    m_layout->insertWidget(m_layout->count() - 1, button);

    connect(button, &QPushButton::clicked, this, [this, index]() {
        onButtonClicked(index);
    });

    return index;
}

void SidebarWidget::addSeparator()
{
    auto *separator = new QLabel(this);
    separator->setObjectName("sidebarSeparator");
    m_layout->insertWidget(m_layout->count() - 1, separator);
}

void SidebarWidget::addSectionTitle(const QString &title)
{
    auto *label = new QLabel(title, this);
    label->setObjectName("sidebarTitle");
    m_layout->insertWidget(m_layout->count() - 1, label);
}

void SidebarWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_buttons.size())
        return;

    m_currentIndex = index;

    for (int i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i]->setProperty("active", i == index);
        m_buttons[i]->style()->unpolish(m_buttons[i]);
        m_buttons[i]->style()->polish(m_buttons[i]);
    }

    emit currentChanged(index);
}

int SidebarWidget::currentIndex() const
{
    return m_currentIndex;
}

void SidebarWidget::onButtonClicked(int index)
{
    setCurrentIndex(index);
}
