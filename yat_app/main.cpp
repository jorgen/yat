#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuick/QQuickWindow>

int main(int argc, char* argv[])
{
    // do this before constructing QGuiApplication so that the custom logging rules will override the setFilterRules below
    qputenv("QT_LOGGING_CONF", QDir::homePath().toLocal8Bit() + "/.config/QtProject/qtlogging.ini");
    QGuiApplication app(argc, argv);
    QLoggingCategory::setFilterRules(QStringLiteral("yat.*.debug=false\nyat.*.info=false"));
    QQmlApplicationEngine engine(QUrl("qrc:///main.qml"));
    return app.exec();
}
