#include "app/main_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MoneyMan");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("The Nonprofitable Company");

    MainWindow window;
    window.show();

    return app.exec();
}
