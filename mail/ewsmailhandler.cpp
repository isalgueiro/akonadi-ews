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

#include "ewsmailhandler.h"

#include <KMime/Message>
#include <Akonadi/KMime/MessageFlags>
#include <AkonadiCore/Item>

#include "ewsfetchmaildetailjob.h"
#include "ewsmodifymailjob.h"

using namespace Akonadi;

EwsMailHandler::EwsMailHandler()
{
}

EwsMailHandler::~EwsMailHandler()
{
}

EwsItemHandler *EwsMailHandler::factory()
{
    return new EwsMailHandler();
}

EwsFetchItemDetailJob *EwsMailHandler::fetchItemDetailJob(EwsClient &client, QObject *parent,
                                                          const Akonadi::Collection &collection)
{
    return new EwsFetchMailDetailJob(client, parent, collection);
}

void EwsMailHandler::setSeenFlag(Item &item, bool value)
{
    if (value) {
        item.setFlag(MessageFlags::Seen);
    }
    else {
        item.clearFlag(MessageFlags::Seen);
    }
}

QString EwsMailHandler::mimeType()
{
    return KMime::Message::mimeType();
}

bool EwsMailHandler::setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem)
{
    QString mimeContent = ewsItem[EwsItemFieldMimeContent].toString();
    if (mimeContent.isEmpty()) {
        qWarning() << QStringLiteral("MIME content is empty!");
        return false;
    }

    mimeContent.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));

    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(mimeContent.toLatin1());
    msg->parse();
    item.setPayload<KMime::Message::Ptr>(msg);
    return true;
}

EwsModifyItemJob *EwsMailHandler::modifyItemJob(EwsClient& client, const Akonadi::Item &item,
                                                const QSet<QByteArray> &parts, QObject *parent)
{
    return new EwsModifyMailJob(client, item, parts, parent);
}


EWS_DECLARE_ITEM_HANDLER(EwsMailHandler, EwsItemTypeMessage)