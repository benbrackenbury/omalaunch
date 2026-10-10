#include <QFont>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickStyle>
#include <QUrl>
#include <QFile>

#include "backend.h"
#include "systemtheme.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("redstone"));
    app.setApplicationDisplayName(QStringLiteral("Redstone"));
    app.setDesktopFileName(QStringLiteral("redstone"));
    app.setOrganizationName(QStringLiteral("Ben Brackenbury"));
    app.setOrganizationDomain(QStringLiteral("benbrackenbury.com"));
    QIcon icon(QStringLiteral(":/redstone.png"));
    if (icon.isNull())
        icon = QIcon::fromTheme(QStringLiteral("redstone"));
    app.setWindowIcon(icon);

    QQuickStyle::setStyle(QStringLiteral("Material"));

    Backend backend(&app);
    SystemTheme systemTheme(&app);
    backend.setDarkMode(systemTheme.darkMode());
    QObject::connect(&systemTheme, &SystemTheme::darkModeChanged, &backend, &Backend::setDarkMode);

    const QFont interfaceFont = app.font();
    const qreal basePointSize = interfaceFont.pointSizeF() > 0 ? interfaceFont.pointSizeF() : 10.0;
    const auto applyInterfaceFont = [&app, interfaceFont, basePointSize](qreal textScale) {
        QFont scaled = interfaceFont;
        scaled.setPointSizeF(basePointSize * textScale);
        app.setFont(scaled);
    };
    applyInterfaceFont(systemTheme.textScale());
    backend.setTextScale(systemTheme.textScale());
    QObject::connect(&systemTheme, &SystemTheme::textScaleChanged, &backend,
                     [&backend, applyInterfaceFont](qreal textScale) {
        applyInterfaceFont(textScale);
        backend.setTextScale(textScale);
    });

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings)
            qWarning().noquote() << warning.toString();
    });
    engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Could not load Redstone UI; resource available:"
                    << QFile::exists(QStringLiteral(":/Main.qml"));
        return -1;
    }
    return app.exec();
}
