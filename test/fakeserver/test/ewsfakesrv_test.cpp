/*  This file is part of Akonadi EWS Resource
    Copyright (C) 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtTest/QtTest>
#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "fakeewsserver.h"

class UtEwsFakeSrvTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void emptyDialog();
    void invalidURL();
    void invalidMethod();
    void emptyRequest();
    void defaultCallback();
    void simpleResponse();
    void callbackResponse();
    void regexpResponse();
    void conditionalResponse();
    void emptyResponse();
private:
    QPair<QString, ushort> synchronousHttpReq(const QString &content);
};

void UtEwsFakeSrvTest::emptyDialog()
{
    const FakeEwsServer::DialogEntry::List emptyDialog;

    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(emptyDialog, this));

    QNetworkAccessManager nam(this);
    QUrl url(QStringLiteral("http://127.0.0.1:13548/EWS/Exchange.asmx"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
    QNetworkReply *reply = nam.post(req, "test");

    QEventLoop loop;
    QString resp;
    ushort respCode = 0;

    connect(reply, &QNetworkReply::readyRead, this, [reply, &resp]() {
        resp += QString::fromUtf8(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, this, [&loop, reply]() {
        loop.exit(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt());
    });
    respCode = loop.exec();

    QCOMPARE(respCode, static_cast<ushort>(500));
}

void UtEwsFakeSrvTest::invalidURL()
{
    const FakeEwsServer::DialogEntry::List emptyDialog;

    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(emptyDialog, this));

    QNetworkAccessManager nam(this);
    QUrl url(QStringLiteral("http://127.0.0.1:13548/some/url"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
    QNetworkReply *reply = nam.post(req, "test");

    QEventLoop loop;
    QString resp;
    ushort respCode = 0;

    connect(reply, &QNetworkReply::readyRead, this, [reply, &resp]() {
        resp += QString::fromUtf8(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, this, [&loop, reply]() {
        loop.exit(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt());
    });
    respCode = loop.exec();

    QCOMPARE(respCode, static_cast<ushort>(500));
}

void UtEwsFakeSrvTest::invalidMethod()
{
    const FakeEwsServer::DialogEntry::List emptyDialog;

    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(emptyDialog, this));

    QNetworkAccessManager nam(this);
    QUrl url(QStringLiteral("http://127.0.0.1:13548/some/url"));
    QNetworkRequest req(url);
    QNetworkReply *reply = nam.get(req);

    QEventLoop loop;
    QString resp;
    ushort respCode = 0;

    connect(reply, &QNetworkReply::readyRead, this, [reply, &resp]() {
        resp += QString::fromUtf8(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, this, [&loop, reply]() {
        loop.exit(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt());
    });
    respCode = loop.exec();

    QCOMPARE(respCode, static_cast<ushort>(500));
}

void UtEwsFakeSrvTest::emptyRequest()
{
    const FakeEwsServer::DialogEntry::List emptyDialog;

    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(emptyDialog, this));

    auto resp = synchronousHttpReq(QStringLiteral(""));
    QCOMPARE(resp.second, static_cast<ushort>(500));
}

void UtEwsFakeSrvTest::defaultCallback()
{
    const FakeEwsServer::DialogEntry::List emptyDialog;

    QString receivedReq;
    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(emptyDialog, this));
    srv->setDefaultReplyCallback([&receivedReq](const QString &req) {
        receivedReq = req;
        return FakeEwsServer::DialogEntry::HttpResponse(QStringLiteral("testresp"), 200);
    });

    auto resp = synchronousHttpReq(QStringLiteral("testreq"));
    QCOMPARE(receivedReq, QStringLiteral("testreq"));
    QCOMPARE(resp.first, QStringLiteral("testresp"));
    QCOMPARE(resp.second, static_cast<ushort>(200));
}

void UtEwsFakeSrvTest::simpleResponse()
{
    const FakeEwsServer::DialogEntry::List dialog = {
        {
            QStringLiteral("samplereq1"),
            QRegularExpression(),
            {QStringLiteral("sampleresp1"), 200},
            FakeEwsServer::DialogEntry::ReplyCallback(),
            QStringLiteral("Sample request 1")
        }
    };

    QString receivedReq;
    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(dialog, this));

    auto resp = synchronousHttpReq(QStringLiteral("samplereq1"));
    QCOMPARE(resp.first, QStringLiteral("sampleresp1"));
    QCOMPARE(resp.second, static_cast<ushort>(200));
}

void UtEwsFakeSrvTest::callbackResponse()
{
    const FakeEwsServer::DialogEntry::List dialog = {
        {
            QStringLiteral("samplereq1"),
            QRegularExpression(),
            {},
            [](const QString &) {
                return FakeEwsServer::DialogEntry::HttpResponse("sampleresp2", 200);
            },
            QStringLiteral("Sample request 1")
        }
    };

    QString receivedReq;
    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(dialog, this));

    auto resp = synchronousHttpReq(QStringLiteral("samplereq1"));
    QCOMPARE(resp.first, QStringLiteral("sampleresp2"));
    QCOMPARE(resp.second, static_cast<ushort>(200));
}

void UtEwsFakeSrvTest::regexpResponse()
{
    const FakeEwsServer::DialogEntry::List dialog = {
        {
            QString(),
            QRegularExpression(QStringLiteral("samplereq[24]")),
            {QStringLiteral("sampleresp1"), 200},
            FakeEwsServer::DialogEntry::ReplyCallback(),
            QStringLiteral("Sample request 1")
        }
    };

    QString receivedReq;
    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(dialog, this));

    auto resp = synchronousHttpReq(QStringLiteral("samplereq1"));
    QCOMPARE(resp.second, static_cast<ushort>(500));

    resp = synchronousHttpReq(QStringLiteral("samplereq2"));
    QCOMPARE(resp.second, static_cast<ushort>(200));
    QCOMPARE(resp.first, QStringLiteral("sampleresp1"));

    resp = synchronousHttpReq(QStringLiteral("samplereq3"));
    QCOMPARE(resp.second, static_cast<ushort>(500));

    resp = synchronousHttpReq(QStringLiteral("samplereq4"));
    QCOMPARE(resp.second, static_cast<ushort>(200));
    QCOMPARE(resp.first, QStringLiteral("sampleresp1"));
}

void UtEwsFakeSrvTest::conditionalResponse()
{
    const FakeEwsServer::DialogEntry::List dialog = {
        {
            QString(),
            QRegularExpression("samplereq[0-9]"),
            FakeEwsServer::EmptyResponse,
            [](const QString& req) {
                if (req[9] == '1' || req[9] == '3')
                    return FakeEwsServer::DialogEntry::HttpResponse(QStringLiteral("sampleresp1"), 200);
                else
                    return FakeEwsServer::EmptyResponse;
            },
            QStringLiteral("Sample request 1")
        },
        {
            QString(),
            QRegularExpression("samplereq[0-9]"),
            FakeEwsServer::EmptyResponse,
            [](const QString& req) {
            if (req[9] == '2' || req[9] == '3')
                return FakeEwsServer::DialogEntry::HttpResponse(QStringLiteral("sampleresp2"), 200);
            else
                return FakeEwsServer::EmptyResponse;
            },
            QStringLiteral("Sample request 2")
        }
    };

    QString receivedReq;
    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(dialog, this));

    auto resp = synchronousHttpReq(QStringLiteral("samplereq1"));
    QCOMPARE(resp.first, QStringLiteral("sampleresp1"));
    QCOMPARE(resp.second, static_cast<ushort>(200));

    resp = synchronousHttpReq(QStringLiteral("samplereq2"));
    QCOMPARE(resp.first, QStringLiteral("sampleresp2"));
    QCOMPARE(resp.second, static_cast<ushort>(200));

    resp = synchronousHttpReq(QStringLiteral("samplereq3"));
    QCOMPARE(resp.first, QStringLiteral("sampleresp1"));
    QCOMPARE(resp.second, static_cast<ushort>(200));

    resp = synchronousHttpReq(QStringLiteral("samplereq4"));
    QCOMPARE(resp.second, static_cast<ushort>(500));
}

void UtEwsFakeSrvTest::emptyResponse()
{
    bool callbackCalled = false;
    const FakeEwsServer::DialogEntry::List dialog = {
        {
            QStringLiteral("samplereq1"),
            QRegularExpression(),
            FakeEwsServer::EmptyResponse,
            [&callbackCalled](const QString &) {
                callbackCalled = true;
                return FakeEwsServer::EmptyResponse;
            },
            QStringLiteral("Sample request 1")
        }
    };

    QString receivedReq;
    QScopedPointer<FakeEwsServer> srv(new FakeEwsServer(dialog, this));

    auto resp = synchronousHttpReq(QStringLiteral("samplereq1"));
    QCOMPARE(resp.second, static_cast<ushort>(500));
    QCOMPARE(callbackCalled, true);
}

QPair<QString, ushort> UtEwsFakeSrvTest::synchronousHttpReq(const QString &content)
{
    QNetworkAccessManager nam(this);
    QUrl url(QStringLiteral("http://127.0.0.1:13548/EWS/Exchange.asmx"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
    QNetworkReply *reply = nam.post(req, content.toUtf8());

    QEventLoop loop;
    QString resp;
    ushort respCode = 0;

    connect(reply, &QNetworkReply::readyRead, this, [reply, &resp]() {
        resp += QString::fromUtf8(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, this, [&loop, reply]() {
        loop.exit(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt());
    });
    respCode = loop.exec();

    return {resp, respCode};
}

QTEST_MAIN(UtEwsFakeSrvTest)

#include "ewsfakesrv_test.moc"
