#include "app/main_window.h"
#include "theme/theme_manager.h"
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

    ThemeManager::instance()->initialize();

    MainWindow window;
    if (!window.initDatabase())
        return 0;
    window.show();

    return app.exec();
}
