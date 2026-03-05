# MoneyMan Visual Overhaul Design

**Date:** 2026-03-05
**Goal:** Transform MoneyMan from functional Qt Widgets app to a modern, warm, visually polished experience.
**Direction:** Modern & Warm aesthetic with coral/orange accent, sidebar navigation, bundled Inter font, custom SVG icons, and system-following dark mode.

## 1. Project Structure & Theme System

### New files

```
src/theme/
  theme_manager.h        # ThemeManager class
  theme_manager.cpp      # OS detection, QSS loading, theme toggle

resources/
  resources.qrc          # Qt resource file
  fonts/
    Inter-Variable.ttf   # Bundled Inter font
  icons/                 # SVG icons (~13)
    dashboard.svg
    trial-balance.svg
    general-ledger.svg
    income-statement.svg
    balance-sheet.svg
    journal-entries.svg
    audit-log.svg
    accounts.svg
    refresh.svg
    add.svg
    export-csv.svg
    export-pdf.svg
    backup.svg
  themes/
    light.qss
    dark.qss

src/views/
  sidebar_widget.h       # Custom sidebar navigation
  sidebar_widget.cpp
```

### ThemeManager

- Singleton owned by QApplication
- Detects OS dark/light preference via QStyleHints::colorScheme() (Qt 6.5+)
- Loads matching .qss from Qt resources at startup
- Exposes setTheme(Light | Dark | System) slot
- Emits themeChanged() signal
- Window menu: three radio options (System / Light / Dark)

### Color Palette

| Token          | Light     | Dark      | Usage                          |
|----------------|-----------|-----------|--------------------------------|
| bg-primary     | #FFFFFF   | #1A1A2E   | Main content background        |
| bg-secondary   | #F9FAFB   | #16213E   | Cards, sidebar                 |
| bg-sidebar     | #1E293B   | #0F0F23   | Sidebar (dark in both themes)  |
| text-primary   | #1F2937   | #E5E7EB   | Body text                      |
| text-secondary | #6B7280   | #9CA3AF   | Muted text, labels             |
| accent         | #F97316   | #FB923C   | Primary accent (coral)         |
| accent-hover   | #EA580C   | #F97316   | Hover state                    |
| success        | #16A34A   | #4ADE80   | In balance, positive           |
| danger         | #DC2626   | #F87171   | Out of balance, negative       |
| border         | #E5E7EB   | #2D2D4A   | Borders, dividers              |

## 2. Sidebar Navigation

### Layout

```
+--------+------------------------------------------+
| Menu Bar                                          |
+--------+------------------------------------------+
|        | Toolbar (per-view)                       |
| SIDE   +------------------------------------------+
| BAR    |                                          |
|        |         QStackedWidget                   |
| * Dash |         (content area)                   |
|   TB   |                                          |
|   GL   |                                          |
|   IS   |                                          |
|   BS   |                                          |
|   JE   |                                          |
|   Log  |                                          |
|        |                                          |
| -----  |                                          |
|   Accts|                                          |
+--------+------------------------------------------+
| Status Bar                                        |
+---------------------------------------------------+
```

### SidebarWidget (new: src/views/sidebar_widget.h/.cpp)

- Custom QWidget with QVBoxLayout
- Fixed width ~200px (icon + label)
- Dark background (bg-sidebar) in both themes
- Nav items: QPushButton with setFlat(true), styled via QSS
  - 20x20 SVG icon + label text
  - 3px left coral accent bar on active item
  - Subtle hover highlight
- Separator before "Accounts" (different view category)
- Clicking sets QStackedWidget index and updates active state
- Ctrl+1 through Ctrl+7 shortcuts wired to stacked widget

### MainWindow changes

- Remove QDockWidget for accounts
- Remove QTabWidget
- Central widget: QHBoxLayout with SidebarWidget + QStackedWidget
- Chart of Accounts becomes a page in QStackedWidget

## 3. Dashboard & Cards

### Metric Cards (updated makeMetricCard)

- 8px border-radius
- Faux shadow: 1px darker bottom/right border
- 3px left coral border on Net Income card
- Typography: title 11px secondary, value 22px bold primary (Inter)
- Semantic colors: Net Income green/red, Trial Balance green/red, others neutral

### Dashboard Layout

- 3x3 grid with 16px spacing (up from 12px)
- Cards with 16px padding
- Recent tables: section headers as QLabel 14px semibold (not HTML <b>)
- Tables: no outer border, subtle row separators, rounded container

### Other report views

- Styled via QSS (minimal C++ changes)
- Table headers: warm gray background
- Alternating rows: bg-primary / bg-secondary
- Toolbar actions: new SVG icons

## 4. Typography, Icons & Polish

### Font

- Inter Variable (open-source, bundled in .qrc)
- Loaded via QFontDatabase::addApplicationFont() at startup
- Set as QApplication::setFont(), base size 13px
- Weights: Regular (400) body, SemiBold (600) labels, Bold (700) metric values

### SVG Icons (~13)

- Monochrome, stroke-style
- Sidebar: 20x20, white at rest, coral when active
- Toolbar: 18x18, text-secondary at rest, text-primary on hover
- Bundled in resources.qrc under :/icons/

### Polish Details

| Element       | Treatment                                                    |
|---------------|--------------------------------------------------------------|
| Buttons       | 6px radius, coral bg primary, outlined secondary             |
| Input fields  | 8px radius, 1px border, coral 2px focus ring                 |
| Tables        | No outer grid, 1px bottom border per row, darker header bg   |
| Scrollbars    | 8px thin, rounded, semi-transparent, visible on hover only   |
| Dialogs       | 24px padding, coral primary button, gray cancel              |
| Status bar    | Subtle top border, muted background, smaller font            |

### Dark Mode

- Sidebar stays dark in both themes
- Cards: lighter-than-background fill, not border-heavy
- Accent shifts #F97316 -> #FB923C for contrast
- Semantic greens/reds shift lighter for readability
