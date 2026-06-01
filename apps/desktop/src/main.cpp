#include "MainWindow.hpp"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("sb-coms");

    QCommandLineParser parser;
    parser.setApplicationDescription("sb-coms desktop client");
    parser.addHelpOption();

    QCommandLineOption instanceOption(
        QStringList{"i", "instance"},
        "Human-readable instance label, useful when running multiple clients.",
        "name",
        "Client"
    );
    parser.addOption(instanceOption);
    parser.process(app);

    const QString instanceName = parser.value(instanceOption);
    QApplication::setApplicationDisplayName("sb-coms " + instanceName);

    MainWindow window(instanceName);
    window.show();

    return app.exec();
}
