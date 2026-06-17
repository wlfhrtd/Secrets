#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#pragma once

#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>


class SingleInstance : public QObject
{
    Q_OBJECT

public:
    explicit SingleInstance(QObject* parent = nullptr)
        : QObject(parent)
    {}

    bool start(const QString& name)
    {
        m_name = name;

        // send signal if already running and return
        QLocalSocket socket;
        socket.connectToServer(m_name, QIODevice::WriteOnly);

        if (socket.waitForConnected(200))
        {
            socket.write("show");
            socket.flush();
            socket.waitForBytesWritten(200);

            return false;
        }

        // else become server
        server = new QLocalServer(this);
        QLocalServer::removeServer(m_name);

        connect(server, &QLocalServer::newConnection,
                this, &SingleInstance::onNewConnection);

        server->listen(m_name);

        return true;
    }

signals:
    void showRequested();

private slots:
    void onNewConnection()
    {
        auto socket = server->nextPendingConnection();

        connect(
            socket,
            &QLocalSocket::readyRead,
            this,
            [this, socket]()
            {
                const QByteArray data = socket->readAll();

                if (data == "show")
                {
                    emit showRequested();
                }

                socket->disconnectFromServer();
            });
    }

private:
    QString m_name;
    QLocalServer* server = nullptr;
};

#endif // SINGLEINSTANCE_H
