#include "MainWindow.hpp"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("sb-coms");
    QApplication::setOrganizationDomain("sb-coms.local");
    QApplication::setApplicationName("sb-coms");
    QApplication::setApplicationVersion("0.3.1");
    QApplication::setWindowIcon(QIcon(":/icons/icon_comms.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription("sb-coms desktop client");
    parser.addHelpOption();

    QCommandLineOption instanceOption(
        QStringList{"i", "instance"},
        "Etiqueta legible para distinguir múltiples clientes.",
        "name",
        "Cliente"
    );
    parser.addOption(instanceOption);
    parser.process(app);

    const QString instanceName = parser.value(instanceOption);
    QApplication::setApplicationDisplayName("SB Comms " + instanceName);

    MainWindow window(instanceName);
    window.show();

    return app.exec();
}
