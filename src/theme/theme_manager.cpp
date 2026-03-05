#include "theme_manager.h"

#include <QApplication>
#include <QFile>
#include <QMenu>
#include <QActionGroup>
#include <QStyleHints>

ThemeManager *ThemeManager::instance()
{
    static ThemeManager s_instance;
    return &s_instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
}

void ThemeManager::initialize()
{
    auto *hints = QGuiApplication::styleHints();
    connect(hints, &QStyleHints::colorSchemeChanged, this, [this]() {
        if (m_theme == Theme::System) {
            applyTheme(detectSystemDark());
        }
    });

    setTheme(Theme::System);
}

void ThemeManager::setTheme(Theme theme)
{
    m_theme = theme;
    bool dark = false;

    switch (theme) {
    case Theme::System:
        dark = detectSystemDark();
        break;
    case Theme::Light:
        dark = false;
        break;
    case Theme::Dark:
        dark = true;
        break;
    }

    applyTheme(dark);
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_theme;
}

bool ThemeManager::isDark() const
{
    return m_isDark;
}

void ThemeManager::populateMenu(QMenu *menu)
{
    auto *group = new QActionGroup(menu);
    group->setExclusive(true);

    auto *systemAction = menu->addAction("&System Theme");
    systemAction->setCheckable(true);
    systemAction->setChecked(m_theme == Theme::System);
    group->addAction(systemAction);

    auto *lightAction = menu->addAction("&Light Theme");
    lightAction->setCheckable(true);
    lightAction->setChecked(m_theme == Theme::Light);
    group->addAction(lightAction);

    auto *darkAction = menu->addAction("&Dark Theme");
    darkAction->setCheckable(true);
    darkAction->setChecked(m_theme == Theme::Dark);
    group->addAction(darkAction);

    connect(systemAction, &QAction::triggered, this, [this]() { setTheme(Theme::System); });
    connect(lightAction, &QAction::triggered, this, [this]() { setTheme(Theme::Light); });
    connect(darkAction, &QAction::triggered, this, [this]() { setTheme(Theme::Dark); });
}

void ThemeManager::applyTheme(bool dark)
{
    m_isDark = dark;
    QString qss = loadStyleSheet(dark ? "dark" : "light");
    qApp->setStyleSheet(qss);
    emit themeChanged(dark);
}

bool ThemeManager::detectSystemDark() const
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

QString ThemeManager::loadStyleSheet(const QString &name) const
{
    QFile file(QString(":/themes/%1.qss").arg(name));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QString::fromUtf8(file.readAll());
}
