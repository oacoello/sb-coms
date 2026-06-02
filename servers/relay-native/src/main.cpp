#include <sb_coms/relay/UdpRelay.hpp>

#include <QCoreApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    sb_coms::relay::UdpRelay relay;
    QObject::connect(&relay, &sb_coms::relay::UdpRelay::statusChanged, [](const QString& message) {
        std::cout << message.toStdString() << '\n';
    });
    QObject::connect(&relay, &sb_coms::relay::UdpRelay::errorOccurred, [](const QString& message) {
        std::cerr << message.toStdString() << '\n';
    });

    if (!relay.start(50000)) {
        return 1;
    }

    return app.exec();
}
