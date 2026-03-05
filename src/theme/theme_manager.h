#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>

class QMenu;

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class Theme { System, Light, Dark };
    Q_ENUM(Theme)

    static ThemeManager *instance();

    void initialize();
    void setTheme(Theme theme);
    Theme currentTheme() const;
    bool isDark() const;

    void populateMenu(QMenu *menu);

signals:
    void themeChanged(bool isDark);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    void applyTheme(bool dark);
    bool detectSystemDark() const;
    QString loadStyleSheet(const QString &name) const;

    Theme m_theme = Theme::System;
    bool m_isDark = false;
};

#endif // THEME_MANAGER_H
