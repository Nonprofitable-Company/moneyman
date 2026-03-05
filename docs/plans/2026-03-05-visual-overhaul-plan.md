# Visual Overhaul Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Transform MoneyMan from default Qt Widgets styling to a modern, warm UI with coral accent, sidebar navigation, Inter font, custom SVG icons, and system-following dark mode.

**Architecture:** A `ThemeManager` singleton loads QSS stylesheets from Qt resources based on OS color scheme preference. Navigation moves from `QTabWidget` + `QDockWidget` to a custom `SidebarWidget` + `QStackedWidget`. All styling is driven by QSS files, with minimal hardcoded styles in C++.

**Tech Stack:** Qt6 Widgets, Qt Resource System (.qrc), QSS stylesheets, SVG icons, Inter font (bundled TTF).

**Design doc:** `docs/plans/2026-03-05-visual-overhaul-design.md`

---

### Task 1: Qt Resource File and Bundled Inter Font

**Files:**
- Create: `resources/resources.qrc`
- Create: `resources/fonts/` (directory for Inter font)
- Modify: `CMakeLists.txt:85` (add resource file to executable)
- Modify: `src/main.cpp` (load font at startup)

**Step 1: Download Inter font**

Download Inter Variable TTF from https://github.com/rsms/inter/releases and place at `resources/fonts/Inter-Variable.ttf`. As a fallback, the app will use the system default font.

**Step 2: Create the Qt resource file**

Create `resources/resources.qrc`:
```xml
<RCC>
  <qresource prefix="/">
    <file>fonts/Inter-Variable.ttf</file>
  </qresource>
</RCC>
```

**Step 3: Add resource file to CMakeLists.txt**

In `CMakeLists.txt`, find the line:
```cmake
add_executable(moneyman src/main.cpp)
```
Change it to:
```cmake
add_executable(moneyman src/main.cpp resources/resources.qrc)
```

Also add `Qt6::Svg` to the `find_package` line:
```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql PrintSupport Svg)
```

And to `target_link_libraries` for `moneyman_lib`:
```cmake
target_link_libraries(moneyman_lib PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
    Qt6::Svg
    ${SQLCIPHER_LIBRARIES}
)
```

**Step 4: Load font in main.cpp**

Modify `src/main.cpp` to load Inter at startup:

```cpp
#include "app/main_window.h"
#include <QApplication>
#include <QFontDatabase>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MoneyMan");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("The Nonprofitable Company");

    // Load bundled Inter font
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Inter-Variable.ttf");
    if (fontId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            QFont defaultFont(families.first(), 10);
            app.setFont(defaultFont);
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
```

**Step 5: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles successfully. If the font file doesn't exist yet, the resource compiler will error. That is expected and will be fixed once the font is downloaded.

**Step 6: Commit**

Stage `resources/`, `CMakeLists.txt`, and `src/main.cpp`. Commit with message: `feat: add Qt resource system and bundled Inter font`

---

### Task 2: SVG Icons

**Files:**
- Create: `resources/icons/dashboard.svg`
- Create: `resources/icons/trial-balance.svg`
- Create: `resources/icons/general-ledger.svg`
- Create: `resources/icons/income-statement.svg`
- Create: `resources/icons/balance-sheet.svg`
- Create: `resources/icons/journal-entries.svg`
- Create: `resources/icons/audit-log.svg`
- Create: `resources/icons/accounts.svg`
- Create: `resources/icons/refresh.svg`
- Create: `resources/icons/add.svg`
- Create: `resources/icons/export-csv.svg`
- Create: `resources/icons/export-pdf.svg`
- Create: `resources/icons/backup.svg`
- Modify: `resources/resources.qrc`

**Step 1: Create icons directory and SVG files**

Create each SVG as a 24x24 monochrome stroke icon. Sidebar icons use white (`#FFFFFF`) stroke. Toolbar icons use gray (`#6B7280`) stroke. No fill, stroke-width 2, round caps and joins.

`resources/icons/dashboard.svg` (4 grid squares):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <rect x="3" y="3" width="7" height="7" rx="1"/>
  <rect x="14" y="3" width="7" height="7" rx="1"/>
  <rect x="3" y="14" width="7" height="7" rx="1"/>
  <rect x="14" y="14" width="7" height="7" rx="1"/>
</svg>
```

`resources/icons/trial-balance.svg` (T-account):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <line x1="12" y1="2" x2="12" y2="22"/>
  <line x1="4" y1="6" x2="11" y2="6"/>
  <line x1="13" y1="6" x2="20" y2="6"/>
  <line x1="4" y1="10" x2="11" y2="10"/>
  <line x1="13" y1="10" x2="20" y2="10"/>
  <line x1="4" y1="18" x2="11" y2="18"/>
  <line x1="13" y1="18" x2="20" y2="18"/>
  <line x1="4" y1="14" x2="20" y2="14"/>
</svg>
```

`resources/icons/general-ledger.svg` (book):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20"/>
  <path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z"/>
  <line x1="8" y1="7" x2="16" y2="7"/>
  <line x1="8" y1="11" x2="14" y2="11"/>
</svg>
```

`resources/icons/income-statement.svg` (trend line):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <polyline points="22 7 13.5 15.5 8.5 10.5 2 17"/>
  <polyline points="16 7 22 7 22 13"/>
</svg>
```

`resources/icons/balance-sheet.svg` (split table):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <rect x="3" y="3" width="18" height="18" rx="2"/>
  <line x1="3" y1="9" x2="21" y2="9"/>
  <line x1="12" y1="9" x2="12" y2="21"/>
</svg>
```

`resources/icons/journal-entries.svg` (document with lines):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/>
  <polyline points="14 2 14 8 20 8"/>
  <line x1="8" y1="13" x2="16" y2="13"/>
  <line x1="8" y1="17" x2="12" y2="17"/>
</svg>
```

`resources/icons/audit-log.svg` (shield with checkmark):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>
  <polyline points="9 12 11 14 15 10"/>
</svg>
```

`resources/icons/accounts.svg` (credit card):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <rect x="2" y="5" width="20" height="14" rx="2"/>
  <line x1="2" y1="10" x2="22" y2="10"/>
</svg>
```

`resources/icons/refresh.svg` (circular arrow, gray for toolbar):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#6B7280" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <polyline points="23 4 23 10 17 10"/>
  <path d="M20.49 15a9 9 0 1 1-2.12-9.36L23 10"/>
</svg>
```

`resources/icons/add.svg` (circle with plus, gray for toolbar):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#6B7280" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <circle cx="12" cy="12" r="10"/>
  <line x1="12" y1="8" x2="12" y2="16"/>
  <line x1="8" y1="12" x2="16" y2="12"/>
</svg>
```

`resources/icons/export-csv.svg` (document with table lines, gray):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#6B7280" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/>
  <polyline points="14 2 14 8 20 8"/>
  <line x1="8" y1="13" x2="16" y2="13"/>
  <line x1="8" y1="17" x2="16" y2="17"/>
  <line x1="10" y1="9" x2="10" y2="9"/>
</svg>
```

`resources/icons/export-pdf.svg` (document with P, gray):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#6B7280" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/>
  <polyline points="14 2 14 8 20 8"/>
  <path d="M9 15v-2h2a1 1 0 1 0 0-2H9"/>
</svg>
```

`resources/icons/backup.svg` (download arrow, gray):
```xml
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#6B7280" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>
  <polyline points="7 10 12 15 17 10"/>
  <line x1="12" y1="15" x2="12" y2="3"/>
</svg>
```

**Step 2: Update resources.qrc with all icons**

Update `resources/resources.qrc`:
```xml
<RCC>
  <qresource prefix="/">
    <file>fonts/Inter-Variable.ttf</file>
    <file>icons/dashboard.svg</file>
    <file>icons/trial-balance.svg</file>
    <file>icons/general-ledger.svg</file>
    <file>icons/income-statement.svg</file>
    <file>icons/balance-sheet.svg</file>
    <file>icons/journal-entries.svg</file>
    <file>icons/audit-log.svg</file>
    <file>icons/accounts.svg</file>
    <file>icons/refresh.svg</file>
    <file>icons/add.svg</file>
    <file>icons/export-csv.svg</file>
    <file>icons/export-pdf.svg</file>
    <file>icons/backup.svg</file>
  </qresource>
</RCC>
```

**Step 3: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles. Resource compiler bundles SVGs and font into binary.

**Step 4: Commit**

Stage `resources/`. Commit with message: `feat: add SVG icons and update resource file`

---

### Task 3: ThemeManager Class

**Files:**
- Create: `src/theme/theme_manager.h`
- Create: `src/theme/theme_manager.cpp`
- Modify: `CMakeLists.txt` (add source files to moneyman_lib)
- Modify: `src/main.cpp` (initialize ThemeManager)

**Step 1: Create theme_manager.h**

Create `src/theme/theme_manager.h`:
```cpp
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
```

**Step 2: Create theme_manager.cpp**

Create `src/theme/theme_manager.cpp`:
```cpp
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
```

**Step 3: Add to CMakeLists.txt**

In `CMakeLists.txt`, add to the `moneyman_lib` source list (after `src/app/main_window.h`):
```cmake
    src/theme/theme_manager.cpp
    src/theme/theme_manager.h
```

**Step 4: Initialize in main.cpp**

Add to `src/main.cpp` includes:
```cpp
#include "theme/theme_manager.h"
```

In `main()`, after font setup and before `MainWindow window;`:
```cpp
    ThemeManager::instance()->initialize();
```

**Step 5: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles. Theme won't visually change yet because the QSS files don't exist in resources yet.

**Step 6: Run tests**

```
cd build && ctest --output-on-failure
```
Expected: All existing tests pass.

**Step 7: Commit**

Stage `src/theme/`, `CMakeLists.txt`, `src/main.cpp`. Commit with message: `feat: add ThemeManager with system-follow and manual override`

---

### Task 4: QSS Stylesheets (Light and Dark)

**Files:**
- Create: `resources/themes/light.qss`
- Create: `resources/themes/dark.qss`
- Modify: `resources/resources.qrc` (add theme files)

**Step 1: Create light.qss**

Create `resources/themes/light.qss`:
```css
/* === MoneyMan Light Theme === */

/* --- Global --- */
QWidget {
    background-color: #FFFFFF;
    color: #1F2937;
    font-size: 13px;
}

QMainWindow {
    background-color: #F9FAFB;
}

/* --- Menu Bar --- */
QMenuBar {
    background-color: #FFFFFF;
    color: #1F2937;
    border-bottom: 1px solid #E5E7EB;
    padding: 2px 0;
}

QMenuBar::item {
    padding: 6px 12px;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #F3F4F6;
}

QMenu {
    background-color: #FFFFFF;
    color: #1F2937;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    padding: 4px;
}

QMenu::item {
    padding: 6px 24px;
    border-radius: 4px;
}

QMenu::item:selected {
    background-color: #FFF7ED;
    color: #F97316;
}

QMenu::separator {
    height: 1px;
    background-color: #E5E7EB;
    margin: 4px 8px;
}

/* --- Tool Bar --- */
QToolBar {
    background-color: #FFFFFF;
    border-bottom: 1px solid #E5E7EB;
    spacing: 4px;
    padding: 4px 8px;
}

QToolBar::separator {
    width: 1px;
    background-color: #E5E7EB;
    margin: 4px 4px;
}

QToolButton {
    background-color: transparent;
    color: #6B7280;
    border: none;
    border-radius: 6px;
    padding: 6px 10px;
}

QToolButton:hover {
    background-color: #F3F4F6;
    color: #1F2937;
}

QToolButton:pressed {
    background-color: #E5E7EB;
}

/* --- Buttons --- */
QPushButton {
    background-color: #F3F4F6;
    color: #1F2937;
    border: 1px solid #E5E7EB;
    border-radius: 6px;
    padding: 6px 16px;
    font-weight: 500;
}

QPushButton:hover {
    background-color: #E5E7EB;
    border-color: #D1D5DB;
}

QPushButton:pressed {
    background-color: #D1D5DB;
}

QPushButton#primaryButton {
    background-color: #F97316;
    color: #FFFFFF;
    border: none;
}

QPushButton#primaryButton:hover {
    background-color: #EA580C;
}

QPushButton#primaryButton:pressed {
    background-color: #DC5B0C;
}

/* --- Input Fields --- */
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateEdit {
    background-color: #FFFFFF;
    color: #1F2937;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    padding: 6px 10px;
    selection-background-color: #FDBA74;
    selection-color: #1F2937;
}

QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QDateEdit:focus {
    border: 2px solid #F97316;
    padding: 5px 9px;
}

QComboBox::drop-down {
    border: none;
    padding-right: 8px;
}

QComboBox QAbstractItemView {
    background-color: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    selection-background-color: #FFF7ED;
    selection-color: #F97316;
}

/* --- Tables --- */
QTableWidget, QTableView {
    background-color: #FFFFFF;
    alternate-background-color: #F9FAFB;
    color: #1F2937;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    gridline-color: transparent;
    selection-background-color: #FFF7ED;
    selection-color: #1F2937;
}

QTableWidget::item, QTableView::item {
    padding: 4px 8px;
    border-bottom: 1px solid #F3F4F6;
}

QHeaderView::section {
    background-color: #F9FAFB;
    color: #6B7280;
    border: none;
    border-bottom: 2px solid #E5E7EB;
    padding: 8px;
    font-weight: 600;
    font-size: 12px;
}

/* --- Scrollbars --- */
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background: #D1D5DB;
    border-radius: 4px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background: #9CA3AF;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background: transparent;
    height: 8px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background: #D1D5DB;
    border-radius: 4px;
    min-width: 30px;
}

QScrollBar::handle:horizontal:hover {
    background: #9CA3AF;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

/* --- Status Bar --- */
QStatusBar {
    background-color: #F9FAFB;
    color: #6B7280;
    border-top: 1px solid #E5E7EB;
    font-size: 12px;
    padding: 2px 8px;
}

/* --- Sidebar --- */
QWidget#sidebar {
    background-color: #1E293B;
    min-width: 200px;
    max-width: 200px;
}

QWidget#sidebar QPushButton {
    background-color: transparent;
    color: #94A3B8;
    border: none;
    border-radius: 0;
    text-align: left;
    padding: 10px 16px 10px 16px;
    font-size: 13px;
    font-weight: 500;
}

QWidget#sidebar QPushButton:hover {
    background-color: #334155;
    color: #E2E8F0;
}

QWidget#sidebar QPushButton[active="true"] {
    background-color: #334155;
    color: #FFFFFF;
    border-left: 3px solid #F97316;
    padding-left: 13px;
}

QWidget#sidebar QLabel#sidebarSeparator {
    background-color: #334155;
    max-height: 1px;
    min-height: 1px;
    margin: 8px 16px;
}

QWidget#sidebar QLabel#sidebarTitle {
    color: #64748B;
    font-size: 11px;
    font-weight: 700;
    padding: 16px 16px 4px 16px;
}

/* --- Dialog Styling --- */
QDialog {
    background-color: #FFFFFF;
}

QDialogButtonBox QPushButton {
    min-width: 80px;
}

/* --- Frames / Cards --- */
QFrame#metricCard {
    background-color: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-bottom: 2px solid #D1D5DB;
    border-right: 2px solid #D1D5DB;
    border-radius: 8px;
    padding: 16px;
}

QFrame#metricCardAccent {
    background-color: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-left: 3px solid #F97316;
    border-bottom: 2px solid #D1D5DB;
    border-right: 2px solid #D1D5DB;
    border-radius: 8px;
    padding: 16px;
}

QLabel#metricTitle {
    color: #6B7280;
    font-size: 11px;
    font-weight: 700;
}

QLabel#metricValue {
    color: #1F2937;
    font-size: 22px;
    font-weight: 700;
}

QLabel#metricValueSuccess {
    color: #16A34A;
    font-size: 22px;
    font-weight: 700;
}

QLabel#metricValueDanger {
    color: #DC2626;
    font-size: 22px;
    font-weight: 700;
}

QLabel#sectionHeader {
    color: #1F2937;
    font-size: 14px;
    font-weight: 600;
    padding: 4px 0;
}

/* --- Status Labels --- */
QLabel#statusSuccess {
    color: #16A34A;
    font-weight: bold;
}

QLabel#statusDanger {
    color: #DC2626;
    font-weight: bold;
}
```

**Step 2: Create dark.qss**

Create `resources/themes/dark.qss`:
```css
/* === MoneyMan Dark Theme === */

/* --- Global --- */
QWidget {
    background-color: #1A1A2E;
    color: #E5E7EB;
    font-size: 13px;
}

QMainWindow {
    background-color: #1A1A2E;
}

/* --- Menu Bar --- */
QMenuBar {
    background-color: #1A1A2E;
    color: #E5E7EB;
    border-bottom: 1px solid #2D2D4A;
    padding: 2px 0;
}

QMenuBar::item {
    padding: 6px 12px;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #16213E;
}

QMenu {
    background-color: #16213E;
    color: #E5E7EB;
    border: 1px solid #2D2D4A;
    border-radius: 8px;
    padding: 4px;
}

QMenu::item {
    padding: 6px 24px;
    border-radius: 4px;
}

QMenu::item:selected {
    background-color: #2D2D4A;
    color: #FB923C;
}

QMenu::separator {
    height: 1px;
    background-color: #2D2D4A;
    margin: 4px 8px;
}

/* --- Tool Bar --- */
QToolBar {
    background-color: #1A1A2E;
    border-bottom: 1px solid #2D2D4A;
    spacing: 4px;
    padding: 4px 8px;
}

QToolBar::separator {
    width: 1px;
    background-color: #2D2D4A;
    margin: 4px 4px;
}

QToolButton {
    background-color: transparent;
    color: #9CA3AF;
    border: none;
    border-radius: 6px;
    padding: 6px 10px;
}

QToolButton:hover {
    background-color: #16213E;
    color: #E5E7EB;
}

QToolButton:pressed {
    background-color: #2D2D4A;
}

/* --- Buttons --- */
QPushButton {
    background-color: #16213E;
    color: #E5E7EB;
    border: 1px solid #2D2D4A;
    border-radius: 6px;
    padding: 6px 16px;
    font-weight: 500;
}

QPushButton:hover {
    background-color: #2D2D4A;
    border-color: #3D3D5C;
}

QPushButton:pressed {
    background-color: #3D3D5C;
}

QPushButton#primaryButton {
    background-color: #FB923C;
    color: #1A1A2E;
    border: none;
}

QPushButton#primaryButton:hover {
    background-color: #F97316;
}

QPushButton#primaryButton:pressed {
    background-color: #EA580C;
}

/* --- Input Fields --- */
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateEdit {
    background-color: #16213E;
    color: #E5E7EB;
    border: 1px solid #2D2D4A;
    border-radius: 8px;
    padding: 6px 10px;
    selection-background-color: #FB923C;
    selection-color: #1A1A2E;
}

QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QDateEdit:focus {
    border: 2px solid #FB923C;
    padding: 5px 9px;
}

QComboBox::drop-down {
    border: none;
    padding-right: 8px;
}

QComboBox QAbstractItemView {
    background-color: #16213E;
    border: 1px solid #2D2D4A;
    border-radius: 8px;
    selection-background-color: #2D2D4A;
    selection-color: #FB923C;
}

/* --- Tables --- */
QTableWidget, QTableView {
    background-color: #1A1A2E;
    alternate-background-color: #16213E;
    color: #E5E7EB;
    border: 1px solid #2D2D4A;
    border-radius: 8px;
    gridline-color: transparent;
    selection-background-color: #2D2D4A;
    selection-color: #E5E7EB;
}

QTableWidget::item, QTableView::item {
    padding: 4px 8px;
    border-bottom: 1px solid #16213E;
}

QHeaderView::section {
    background-color: #16213E;
    color: #9CA3AF;
    border: none;
    border-bottom: 2px solid #2D2D4A;
    padding: 8px;
    font-weight: 600;
    font-size: 12px;
}

/* --- Scrollbars --- */
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background: #2D2D4A;
    border-radius: 4px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background: #3D3D5C;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background: transparent;
    height: 8px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background: #2D2D4A;
    border-radius: 4px;
    min-width: 30px;
}

QScrollBar::handle:horizontal:hover {
    background: #3D3D5C;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

/* --- Status Bar --- */
QStatusBar {
    background-color: #16213E;
    color: #9CA3AF;
    border-top: 1px solid #2D2D4A;
    font-size: 12px;
    padding: 2px 8px;
}

/* --- Sidebar --- */
QWidget#sidebar {
    background-color: #0F0F23;
    min-width: 200px;
    max-width: 200px;
}

QWidget#sidebar QPushButton {
    background-color: transparent;
    color: #94A3B8;
    border: none;
    border-radius: 0;
    text-align: left;
    padding: 10px 16px 10px 16px;
    font-size: 13px;
    font-weight: 500;
}

QWidget#sidebar QPushButton:hover {
    background-color: #1A1A2E;
    color: #E2E8F0;
}

QWidget#sidebar QPushButton[active="true"] {
    background-color: #1A1A2E;
    color: #FFFFFF;
    border-left: 3px solid #FB923C;
    padding-left: 13px;
}

QWidget#sidebar QLabel#sidebarSeparator {
    background-color: #1A1A2E;
    max-height: 1px;
    min-height: 1px;
    margin: 8px 16px;
}

QWidget#sidebar QLabel#sidebarTitle {
    color: #64748B;
    font-size: 11px;
    font-weight: 700;
    padding: 16px 16px 4px 16px;
}

/* --- Dialog Styling --- */
QDialog {
    background-color: #1A1A2E;
}

QDialogButtonBox QPushButton {
    min-width: 80px;
}

/* --- Frames / Cards --- */
QFrame#metricCard {
    background-color: #16213E;
    border: 1px solid #2D2D4A;
    border-bottom: 2px solid #1A1A2E;
    border-right: 2px solid #1A1A2E;
    border-radius: 8px;
    padding: 16px;
}

QFrame#metricCardAccent {
    background-color: #16213E;
    border: 1px solid #2D2D4A;
    border-left: 3px solid #FB923C;
    border-bottom: 2px solid #1A1A2E;
    border-right: 2px solid #1A1A2E;
    border-radius: 8px;
    padding: 16px;
}

QLabel#metricTitle {
    color: #9CA3AF;
    font-size: 11px;
    font-weight: 700;
}

QLabel#metricValue {
    color: #E5E7EB;
    font-size: 22px;
    font-weight: 700;
}

QLabel#metricValueSuccess {
    color: #4ADE80;
    font-size: 22px;
    font-weight: 700;
}

QLabel#metricValueDanger {
    color: #F87171;
    font-size: 22px;
    font-weight: 700;
}

QLabel#sectionHeader {
    color: #E5E7EB;
    font-size: 14px;
    font-weight: 600;
    padding: 4px 0;
}

/* --- Status Labels --- */
QLabel#statusSuccess {
    color: #4ADE80;
    font-weight: bold;
}

QLabel#statusDanger {
    color: #F87171;
    font-weight: bold;
}
```

**Step 3: Update resources.qrc**

Add to `resources/resources.qrc` inside the `<qresource>` tag:
```xml
    <file>themes/light.qss</file>
    <file>themes/dark.qss</file>
```

**Step 4: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles. Running the app now shows themed UI.

**Step 5: Run tests**

```
cd build && ctest --output-on-failure
```
Expected: All tests pass.

**Step 6: Commit**

Stage `resources/`. Commit with message: `feat: add light and dark QSS theme stylesheets`

---

### Task 5: SidebarWidget

**Files:**
- Create: `src/views/sidebar_widget.h`
- Create: `src/views/sidebar_widget.cpp`
- Modify: `CMakeLists.txt` (add source files to moneyman_lib)

**Step 1: Create sidebar_widget.h**

Create `src/views/sidebar_widget.h`:
```cpp
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
```

**Step 2: Create sidebar_widget.cpp**

Create `src/views/sidebar_widget.cpp`:
```cpp
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
```

**Step 3: Add to CMakeLists.txt**

In `CMakeLists.txt`, add to the `moneyman_lib` source list:
```cmake
    src/views/sidebar_widget.cpp
    src/views/sidebar_widget.h
```

**Step 4: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles. SidebarWidget is not wired into MainWindow yet.

**Step 5: Commit**

Stage `src/views/sidebar_widget.h`, `src/views/sidebar_widget.cpp`, `CMakeLists.txt`. Commit with message: `feat: add SidebarWidget for navigation`

---

### Task 6: Refactor MainWindow (Replace QTabWidget + QDockWidget with Sidebar)

**Files:**
- Modify: `src/app/main_window.h`
- Modify: `src/app/main_window.cpp`

This is the largest task. Replace QTabWidget and QDockWidget with SidebarWidget + QStackedWidget.

**Step 1: Update main_window.h**

Replace the full contents of `src/app/main_window.h`:
```cpp
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class Database;
class AccountsWidget;
class DashboardWidget;
class TrialBalanceWidget;
class GeneralLedgerWidget;
class IncomeStatementWidget;
class BalanceSheetWidget;
class JournalListWidget;
class AuditLogWidget;
class SidebarWidget;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewJournalEntry();
    void onBackup();
    void onRestore();
    void onChangeKey();
    void refreshAllReports();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    Database *m_database;
    QString m_passphrase;
    SidebarWidget *m_sidebar;
    QStackedWidget *m_stack;
    AccountsWidget *m_accountsWidget;
    DashboardWidget *m_dashboardWidget;
    TrialBalanceWidget *m_trialBalanceWidget;
    GeneralLedgerWidget *m_generalLedgerWidget;
    IncomeStatementWidget *m_incomeStatementWidget;
    BalanceSheetWidget *m_balanceSheetWidget;
    JournalListWidget *m_journalListWidget;
    AuditLogWidget *m_auditLogWidget;
};

#endif // MAIN_WINDOW_H
```

**Step 2: Update main_window.cpp**

Replace the full contents of `src/app/main_window.cpp`. Key changes:
- Remove `QDockWidget` and `QTabWidget` includes and usage
- Add `SidebarWidget` and `QStackedWidget`
- Replace old dark mode toggle in Window menu with `ThemeManager::populateMenu()`
- Use new SVG icons instead of `QStyle::SP_*`
- Wire sidebar to stacked widget
- Keep all existing functionality (backup, restore, journal entry, etc.)

```cpp
#include "main_window.h"
#include "db/database.h"
#include "views/dashboard_widget.h"
#include "views/accounts_widget.h"
#include "views/journal_entry_dialog.h"
#include "views/trial_balance_widget.h"
#include "views/general_ledger_widget.h"
#include "views/income_statement_widget.h"
#include "views/balance_sheet_widget.h"
#include "views/journal_list_widget.h"
#include "views/audit_log_widget.h"
#include "views/close_period_dialog.h"
#include "views/password_dialog.h"
#include "views/import_csv_dialog.h"
#include "views/help_browser_dialog.h"
#include "views/sidebar_widget.h"
#include "theme/theme_manager.h"
#include "models/account_model.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QShortcut>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
    , m_sidebar(nullptr)
    , m_stack(nullptr)
    , m_accountsWidget(nullptr)
    , m_dashboardWidget(nullptr)
    , m_trialBalanceWidget(nullptr)
    , m_generalLedgerWidget(nullptr)
    , m_incomeStatementWidget(nullptr)
    , m_balanceSheetWidget(nullptr)
    , m_journalListWidget(nullptr)
    , m_auditLogWidget(nullptr)
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    PasswordDialog pwDialog(PasswordDialog::Unlock, this);
    if (pwDialog.exec() != QDialog::Accepted) {
        QMessageBox::warning(this, "No Passphrase",
            "A passphrase is required to open the database.");
    } else {
        m_passphrase = pwDialog.password();
        if (!m_database->open(QString(), m_passphrase)) {
            QMessageBox::critical(this, "Database Error",
                "Failed to open database: " + m_database->lastError()
                + "\n\nThe passphrase may be incorrect.");
        }
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle("MoneyMan \u2014 The Nonprofitable Company");
    resize(1200, 800);

    // Create sidebar
    m_sidebar = new SidebarWidget(this);
    m_sidebar->addSectionTitle("REPORTS");
    m_sidebar->addItem(":/icons/dashboard.svg", "  Dashboard");
    m_sidebar->addItem(":/icons/trial-balance.svg", "  Trial Balance");
    m_sidebar->addItem(":/icons/general-ledger.svg", "  General Ledger");
    m_sidebar->addItem(":/icons/income-statement.svg", "  Income Statement");
    m_sidebar->addItem(":/icons/balance-sheet.svg", "  Balance Sheet");
    m_sidebar->addItem(":/icons/journal-entries.svg", "  Journal Entries");
    m_sidebar->addItem(":/icons/audit-log.svg", "  Audit Log");
    m_sidebar->addSeparator();
    m_sidebar->addSectionTitle("DATA");
    m_sidebar->addItem(":/icons/accounts.svg", "  Chart of Accounts");

    // Create stacked widget with all pages
    m_stack = new QStackedWidget(this);
    m_dashboardWidget = new DashboardWidget(m_database, this);
    m_trialBalanceWidget = new TrialBalanceWidget(m_database, this);
    m_generalLedgerWidget = new GeneralLedgerWidget(m_database, this);
    m_incomeStatementWidget = new IncomeStatementWidget(m_database, this);
    m_balanceSheetWidget = new BalanceSheetWidget(m_database, this);
    m_journalListWidget = new JournalListWidget(m_database, this);
    m_auditLogWidget = new AuditLogWidget(m_database, this);
    m_accountsWidget = new AccountsWidget(m_database, this);

    m_stack->addWidget(m_dashboardWidget);      // 0
    m_stack->addWidget(m_trialBalanceWidget);    // 1
    m_stack->addWidget(m_generalLedgerWidget);   // 2
    m_stack->addWidget(m_incomeStatementWidget); // 3
    m_stack->addWidget(m_balanceSheetWidget);    // 4
    m_stack->addWidget(m_journalListWidget);     // 5
    m_stack->addWidget(m_auditLogWidget);        // 6
    m_stack->addWidget(m_accountsWidget);        // 7

    connect(m_sidebar, &SidebarWidget::currentChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
    m_sidebar->setCurrentIndex(0);

    // Central widget: sidebar + stack in horizontal layout
    auto *central = new QWidget(this);
    auto *hbox = new QHBoxLayout(central);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    hbox->addWidget(m_sidebar);
    hbox->addWidget(m_stack, 1);
    setCentralWidget(central);
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Backup Database...", this, &MainWindow::onBackup);
    fileMenu->addAction("&Restore Database...", this, &MainWindow::onRestore);
    fileMenu->addAction("&Import Accounts CSV...", this, [this]() {
        ImportCsvDialog dialog(m_database, this);
        if (dialog.exec() == QDialog::Accepted) {
            m_accountsWidget->model()->refresh();
            refreshAllReports();
            statusBar()->showMessage(
                QString("Imported %1 accounts").arg(dialog.importedCount()), 5000);
        }
    });
    fileMenu->addAction("Change Encryption &Key...", this, &MainWindow::onChangeKey);
    fileMenu->addSeparator();
    fileMenu->addAction("Close &Period...", this, [this]() {
        ClosePeriodDialog dialog(m_database, this);
        if (dialog.exec() == QDialog::Accepted) {
            m_accountsWidget->model()->refresh();
            refreshAllReports();
            statusBar()->showMessage("Fiscal period closed successfully", 5000);
        }
    });
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto *txnMenu = menuBar()->addMenu("&Transactions");
    txnMenu->addAction(QIcon(":/icons/add.svg"),
        "&New Journal Entry...", QKeySequence(Qt::CTRL | Qt::Key_J),
        this, &MainWindow::onNewJournalEntry);

    auto *reportsMenu = menuBar()->addMenu("&Reports");
    reportsMenu->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh &All Reports", QKeySequence(Qt::CTRL | Qt::Key_R),
        this, &MainWindow::refreshAllReports);

    // Window menu with theme selection
    auto *windowMenu = menuBar()->addMenu("&Window");
    ThemeManager::instance()->populateMenu(windowMenu);

    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&User Guide", QKeySequence::HelpContents, this, [this]() {
        HelpBrowserDialog dialog(this);
        dialog.exec();
    });
    helpMenu->addSeparator();
    helpMenu->addAction("&About MoneyMan", this, [this]() {
        QMessageBox::about(this, "About MoneyMan",
            "<h2>MoneyMan v0.1.0</h2>"
            "<p>Double-entry bookkeeping for The Nonprofitable Company.</p>"
            "<p>Features: Chart of Accounts, Journal Entries, Trial Balance, "
            "General Ledger, Income Statement, Balance Sheet, Audit Log, "
            "Templates, Fiscal Periods, CSV/PDF Export, Encrypted Database.</p>"
            "<p>Built with Qt6 and SQLCipher.</p>");
    });

    // Navigation shortcuts (Ctrl+1 through Ctrl+8)
    for (int i = 0; i < m_stack->count() && i < 9; ++i) {
        auto *shortcut = new QShortcut(
            QKeySequence(Qt::CTRL | static_cast<Qt::Key>(Qt::Key_1 + i)), this);
        connect(shortcut, &QShortcut::activated, this, [this, i]() {
            m_sidebar->setCurrentIndex(i);
        });
    }
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    toolbar->addAction(QIcon(":/icons/add.svg"),
        "New Journal Entry", this, &MainWindow::onNewJournalEntry);
    toolbar->addSeparator();
    toolbar->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh Reports", this, &MainWindow::refreshAllReports);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready \u2014 " + m_database->databasePath());
}

void MainWindow::onNewJournalEntry()
{
    JournalEntryDialog dialog(m_database, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_accountsWidget->model()->refresh();
        refreshAllReports();
        statusBar()->showMessage("Journal entry posted successfully", 5000);
    }
}

void MainWindow::onBackup()
{
    QString dest = QFileDialog::getSaveFileName(this, "Backup Database",
        "moneyman_backup.db", "SQLite Database (*.db)");
    if (dest.isEmpty()) return;

    QString srcPath = m_database->databasePath();
    m_database->close();

    bool ok = QFile::copy(srcPath, dest);

    if (!m_database->open(srcPath, m_passphrase)) {
        QMessageBox::critical(this, "Error",
            "Failed to reopen database after backup.");
        return;
    }

    if (ok) {
        statusBar()->showMessage("Backup saved to " + dest, 5000);
        m_database->logAudit("BACKUP", "Backed up to " + dest);
    } else {
        QMessageBox::warning(this, "Backup Failed",
            "Could not copy database file. The destination may already exist.");
    }
}

void MainWindow::onRestore()
{
    QString src = QFileDialog::getOpenFileName(this, "Restore Database",
        QString(), "SQLite Database (*.db)");
    if (src.isEmpty()) return;

    auto reply = QMessageBox::warning(this, "Confirm Restore",
        "This will replace all current data with the backup.\n\n"
        "This action cannot be undone. Continue?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QString destPath = m_database->databasePath();
    m_database->close();

    QFile::remove(destPath);
    bool ok = QFile::copy(src, destPath);

    if (!ok) {
        QMessageBox::critical(this, "Restore Failed",
            "Could not copy backup file.");
        m_database->open(destPath, m_passphrase);
        return;
    }

    if (!m_database->open(destPath, m_passphrase)) {
        QMessageBox::critical(this, "Error",
            "Failed to open restored database: " + m_database->lastError());
        return;
    }

    m_database->logAudit("RESTORE", "Restored from " + src);
    m_accountsWidget->model()->refresh();
    refreshAllReports();
    statusBar()->showMessage("Database restored from " + src, 5000);
}

void MainWindow::onChangeKey()
{
    PasswordDialog dialog(PasswordDialog::ChangeKey, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (m_database->changeEncryptionKey(dialog.newPassword())) {
        m_passphrase = dialog.newPassword();
        statusBar()->showMessage("Encryption key changed successfully", 5000);
    } else {
        QMessageBox::warning(this, "Error",
            "Failed to change encryption key: " + m_database->lastError());
    }
}

void MainWindow::refreshAllReports()
{
    m_dashboardWidget->refresh();
    m_trialBalanceWidget->refresh();
    m_generalLedgerWidget->refresh();
    m_incomeStatementWidget->refresh();
    m_balanceSheetWidget->refresh();
    m_journalListWidget->refresh();
    m_auditLogWidget->refresh();
}
```

**Step 3: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles without errors. The app now shows sidebar navigation instead of tabs.

**Step 4: Run tests**

```
cd build && ctest --output-on-failure
```
Expected: All tests pass.

**Step 5: Commit**

Stage `src/app/main_window.h` and `src/app/main_window.cpp`. Commit with message: `feat: replace tab/dock navigation with sidebar + stacked widget`

---

### Task 7: Update Dashboard Cards to Use QSS Object Names

**Files:**
- Modify: `src/views/dashboard_widget.h`
- Modify: `src/views/dashboard_widget.cpp`

The dashboard currently uses hardcoded `setStyleSheet()` calls. Switch to QSS object names so theme stylesheets control appearance.

**Step 1: Update dashboard_widget.h**

Replace `src/views/dashboard_widget.h`:
```cpp
#ifndef DASHBOARD_WIDGET_H
#define DASHBOARD_WIDGET_H

#include <QWidget>

class Database;
class QLabel;
class QTableWidget;
class QFrame;

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    struct MetricCard {
        QFrame *frame;
        QLabel *title;
        QLabel *value;
    };

    MetricCard makeMetricCard(const QString &title, bool accent = false);

    Database *m_db;
    MetricCard m_totalAssets;
    MetricCard m_totalLiabilities;
    MetricCard m_totalEquity;
    MetricCard m_totalRevenue;
    MetricCard m_totalExpenses;
    MetricCard m_netIncome;
    MetricCard m_accountCount;
    MetricCard m_entryCount;
    MetricCard m_balanceStatus;
    QTableWidget *m_recentEntries;
    QTableWidget *m_recentAudit;
};

#endif // DASHBOARD_WIDGET_H
```

**Step 2: Update dashboard_widget.cpp**

Replace `src/views/dashboard_widget.cpp`:
```cpp
#include "dashboard_widget.h"
#include "db/database.h"
#include "accounting/account.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QToolBar>
#include <QFont>
#include <QTableWidget>
#include <QHeaderView>
#include <QIcon>
#include <QStyle>

static QString formatCents(int64_t cents)
{
    return QString::number(static_cast<double>(cents) / 100.0, 'f', 2);
}

DashboardWidget::DashboardWidget(Database *db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
{
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh", this, &DashboardWidget::refresh);

    m_totalAssets = makeMetricCard("Total Assets");
    m_totalLiabilities = makeMetricCard("Total Liabilities");
    m_totalEquity = makeMetricCard("Total Equity");
    m_totalRevenue = makeMetricCard("Total Revenue");
    m_totalExpenses = makeMetricCard("Total Expenses");
    m_netIncome = makeMetricCard("Net Income", true);
    m_accountCount = makeMetricCard("Accounts");
    m_entryCount = makeMetricCard("Journal Entries");
    m_balanceStatus = makeMetricCard("Trial Balance");

    auto *grid = new QGridLayout;
    grid->setSpacing(16);

    grid->addWidget(m_totalAssets.frame, 0, 0);
    grid->addWidget(m_totalLiabilities.frame, 0, 1);
    grid->addWidget(m_totalEquity.frame, 0, 2);

    grid->addWidget(m_totalRevenue.frame, 1, 0);
    grid->addWidget(m_totalExpenses.frame, 1, 1);
    grid->addWidget(m_netIncome.frame, 1, 2);

    grid->addWidget(m_accountCount.frame, 2, 0);
    grid->addWidget(m_entryCount.frame, 2, 1);
    grid->addWidget(m_balanceStatus.frame, 2, 2);

    m_recentEntries = new QTableWidget(this);
    m_recentEntries->setColumnCount(3);
    m_recentEntries->setHorizontalHeaderLabels({"Date", "Description", "#Lines"});
    m_recentEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentEntries->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentEntries->setAlternatingRowColors(true);
    m_recentEntries->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_recentEntries->verticalHeader()->hide();
    m_recentEntries->setMaximumHeight(180);

    m_recentAudit = new QTableWidget(this);
    m_recentAudit->setColumnCount(3);
    m_recentAudit->setHorizontalHeaderLabels({"Timestamp", "Action", "Details"});
    m_recentAudit->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentAudit->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentAudit->setAlternatingRowColors(true);
    m_recentAudit->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_recentAudit->verticalHeader()->hide();
    m_recentAudit->setMaximumHeight(180);

    auto *recentLayout = new QHBoxLayout;
    auto *entriesGroup = new QVBoxLayout;
    auto *entriesHeader = new QLabel("Recent Journal Entries", this);
    entriesHeader->setObjectName("sectionHeader");
    entriesGroup->addWidget(entriesHeader);
    entriesGroup->addWidget(m_recentEntries);
    auto *auditGroup = new QVBoxLayout;
    auto *auditHeader = new QLabel("Recent Audit Log", this);
    auditHeader->setObjectName("sectionHeader");
    auditGroup->addWidget(auditHeader);
    auditGroup->addWidget(m_recentAudit);
    recentLayout->addLayout(entriesGroup);
    recentLayout->addLayout(auditGroup);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 16);
    layout->addWidget(toolbar);
    layout->addLayout(grid);
    layout->addLayout(recentLayout);
    layout->addStretch();

    refresh();
}

DashboardWidget::MetricCard DashboardWidget::makeMetricCard(
    const QString &title, bool accent)
{
    MetricCard card;
    card.frame = new QFrame(this);
    card.frame->setObjectName(accent ? "metricCardAccent" : "metricCard");

    card.title = new QLabel(title, card.frame);
    card.title->setObjectName("metricTitle");

    card.value = new QLabel("--", card.frame);
    card.value->setObjectName("metricValue");
    card.value->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *layout = new QVBoxLayout(card.frame);
    layout->addWidget(card.title);
    layout->addWidget(card.value);

    return card;
}

void DashboardWidget::refresh()
{
    auto accounts = m_db->allAccounts();
    auto entries = m_db->allJournalEntries();

    int64_t assets = 0, liabilities = 0, equity = 0, revenue = 0, expenses = 0;

    for (const auto &acct : accounts) {
        AccountType type = accountTypeFromString(acct.type);
        switch (type) {
        case AccountType::Asset:     assets += acct.balanceCents; break;
        case AccountType::Liability: liabilities += acct.balanceCents; break;
        case AccountType::Equity:    equity += acct.balanceCents; break;
        case AccountType::Revenue:   revenue += acct.balanceCents; break;
        case AccountType::Expense:   expenses += acct.balanceCents; break;
        default: break;
        }
    }

    int64_t netIncome = revenue - expenses;

    m_totalAssets.value->setText("$" + formatCents(assets));
    m_totalLiabilities.value->setText("$" + formatCents(liabilities));
    m_totalEquity.value->setText("$" + formatCents(equity));
    m_totalRevenue.value->setText("$" + formatCents(revenue));
    m_totalExpenses.value->setText("$" + formatCents(expenses));

    if (netIncome >= 0) {
        m_netIncome.value->setText("$" + formatCents(netIncome));
        m_netIncome.value->setObjectName("metricValueSuccess");
    } else {
        m_netIncome.value->setText("-$" + formatCents(-netIncome));
        m_netIncome.value->setObjectName("metricValueDanger");
    }
    m_netIncome.value->style()->unpolish(m_netIncome.value);
    m_netIncome.value->style()->polish(m_netIncome.value);

    m_accountCount.value->setText(QString::number(accounts.size()));
    m_entryCount.value->setText(QString::number(entries.size()));

    int64_t totalDebits = 0, totalCredits = 0;
    for (const auto &acct : accounts) {
        AccountType type = accountTypeFromString(acct.type);
        if (isDebitNormal(type)) {
            if (acct.balanceCents >= 0) totalDebits += acct.balanceCents;
            else totalCredits += -acct.balanceCents;
        } else {
            if (acct.balanceCents >= 0) totalCredits += acct.balanceCents;
            else totalDebits += -acct.balanceCents;
        }
    }

    if (totalDebits == totalCredits) {
        m_balanceStatus.value->setText("In Balance");
        m_balanceStatus.value->setObjectName("metricValueSuccess");
    } else {
        m_balanceStatus.value->setText("OUT OF BALANCE");
        m_balanceStatus.value->setObjectName("metricValueDanger");
    }
    m_balanceStatus.value->style()->unpolish(m_balanceStatus.value);
    m_balanceStatus.value->style()->polish(m_balanceStatus.value);

    m_recentEntries->setRowCount(0);
    int entryCount = std::min(static_cast<int>(entries.size()), 5);
    for (int i = 0; i < entryCount; ++i) {
        const auto &e = entries[entries.size() - 1 - static_cast<size_t>(i)];
        int row = m_recentEntries->rowCount();
        m_recentEntries->insertRow(row);
        m_recentEntries->setItem(row, 0, new QTableWidgetItem(e.date));
        m_recentEntries->setItem(row, 1, new QTableWidgetItem(e.description));
        m_recentEntries->setItem(row, 2,
            new QTableWidgetItem(QString::number(e.lines.size())));
    }

    auto auditLog = m_db->allAuditLog();
    m_recentAudit->setRowCount(0);
    int auditCount = std::min(static_cast<int>(auditLog.size()), 5);
    for (int i = 0; i < auditCount; ++i) {
        const auto &a = auditLog[auditLog.size() - 1 - static_cast<size_t>(i)];
        int row = m_recentAudit->rowCount();
        m_recentAudit->insertRow(row);
        m_recentAudit->setItem(row, 0, new QTableWidgetItem(a.timestamp));
        m_recentAudit->setItem(row, 1, new QTableWidgetItem(a.action));
        m_recentAudit->setItem(row, 2, new QTableWidgetItem(a.details));
    }
}
```

**Step 3: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles. Dashboard cards are now styled by QSS.

**Step 4: Run tests**

```
cd build && ctest --output-on-failure
```
Expected: All tests pass.

**Step 5: Commit**

Stage `src/views/dashboard_widget.h` and `src/views/dashboard_widget.cpp`. Commit with message: `feat: update dashboard cards to use QSS object names`

---

### Task 8: Update Report Widgets to Use SVG Icons and QSS Object Names

**Files:**
- Modify: `src/views/trial_balance_widget.cpp`
- Modify: `src/views/general_ledger_widget.cpp`
- Modify: `src/views/income_statement_widget.cpp`
- Modify: `src/views/balance_sheet_widget.cpp`
- Modify: `src/views/journal_list_widget.cpp`
- Modify: `src/views/audit_log_widget.cpp`
- Modify: `src/views/accounts_widget.cpp`

For each widget, apply these changes:

1. Replace `QApplication::style()->standardIcon(QStyle::SP_BrowserReload)` with `QIcon(":/icons/refresh.svg")`
2. Replace `QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton)` with `QIcon(":/icons/export-csv.svg")`
3. Replace `QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView)` with `QIcon(":/icons/export-pdf.svg")`
4. Replace `QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder)` with `QIcon(":/icons/add.svg")`
5. Replace `setStyleSheet("color: green; ...")` with `setObjectName("statusSuccess")` + unpolish/polish
6. Replace `setStyleSheet("color: red; ...")` with `setObjectName("statusDanger")` + unpolish/polish
7. Remove `#include <QApplication>` and `#include <QStyle>` if no longer needed (keep `<QStyle>` if `style()->unpolish` is used)
8. Add `#include <QIcon>` where needed

**Example for trial_balance_widget.cpp toolbar (lines 37-46):**

Replace:
```cpp
    auto *style = QApplication::style();
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(style->standardIcon(QStyle::SP_BrowserReload),
        "Refresh", this, &TrialBalanceWidget::refresh);
    toolbar->addAction(style->standardIcon(QStyle::SP_DialogSaveButton),
        "Export CSV", this, &TrialBalanceWidget::exportCsv);
    toolbar->addAction(style->standardIcon(QStyle::SP_FileDialogDetailedView),
        "Export PDF", this, [this]() {
            printReportToPdf(m_table, "Trial Balance", this, "trial_balance.pdf");
        });
```

With:
```cpp
    auto *toolbar = new QToolBar(this);
    toolbar->addAction(QIcon(":/icons/refresh.svg"),
        "Refresh", this, &TrialBalanceWidget::refresh);
    toolbar->addAction(QIcon(":/icons/export-csv.svg"),
        "Export CSV", this, &TrialBalanceWidget::exportCsv);
    toolbar->addAction(QIcon(":/icons/export-pdf.svg"),
        "Export PDF", this, [this]() {
            printReportToPdf(m_table, "Trial Balance", this, "trial_balance.pdf");
        });
```

**Example for trial_balance_widget.cpp status label (lines 133-141):**

Replace:
```cpp
    if (totalDebits == totalCredits) {
        m_statusLabel->setText("Trial Balance is in balance");
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        ...
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    }
```

With:
```cpp
    if (totalDebits == totalCredits) {
        m_statusLabel->setText("Trial Balance is in balance");
        m_statusLabel->setObjectName("statusSuccess");
    } else {
        ...
        m_statusLabel->setObjectName("statusDanger");
    }
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
```

Apply the same pattern to all seven widget files.

**Step 1: Update all seven files with the changes described above**

**Step 2: Build and verify**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Compiles without errors or warnings.

**Step 3: Run tests**

```
cd build && ctest --output-on-failure
```
Expected: All tests pass.

**Step 4: Commit**

Stage `src/views/`. Commit with message: `feat: update all report widgets to use SVG icons and QSS styling`

---

### Task 9: Build, Test, and Visual Verification

**Step 1: Full clean build**

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```
Expected: Zero errors, zero warnings.

**Step 2: Run all tests**

```
cd build && ctest --output-on-failure
```
Expected: All tests pass.

**Step 3: Launch and visually verify**

```
./build/moneyman
```

Visual checklist:
- [ ] Sidebar appears on the left with dark background
- [ ] Sidebar items have icons and labels
- [ ] Clicking sidebar items switches content
- [ ] Active sidebar item has coral left border
- [ ] Dashboard metric cards have rounded corners and proper spacing
- [ ] Net Income card has coral left accent border
- [ ] Tables have styled headers and alternating rows
- [ ] Scrollbars are thin and rounded
- [ ] Menu bar looks clean with proper padding
- [ ] Status bar has subtle top border
- [ ] Ctrl+1 through Ctrl+8 switches views
- [ ] Window > System Theme / Light Theme / Dark Theme toggle works
- [ ] Dark mode looks correct: dark backgrounds, lighter accent colors
- [ ] All toolbar buttons use SVG icons

**Step 4: Fix any visual issues found during verification**

Iterate on QSS as needed.

**Step 5: Commit any fixes**

Stage all changed files. Commit with message: `fix: visual polish after manual verification`

---

## Summary

| Task | Description | Key Files |
|------|-------------|-----------|
| 1 | Qt resources + Inter font | `resources/resources.qrc`, `CMakeLists.txt`, `src/main.cpp` |
| 2 | SVG icons (13 files) | `resources/icons/*.svg` |
| 3 | ThemeManager class | `src/theme/theme_manager.h/.cpp` |
| 4 | QSS stylesheets (light + dark) | `resources/themes/light.qss`, `dark.qss` |
| 5 | SidebarWidget | `src/views/sidebar_widget.h/.cpp` |
| 6 | MainWindow refactor | `src/app/main_window.h/.cpp` |
| 7 | Dashboard card redesign | `src/views/dashboard_widget.h/.cpp` |
| 8 | Report widget icon/style updates | `src/views/*.cpp` (7 files) |
| 9 | Build, test, visual verification | All |
