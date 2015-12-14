/*  This file is part of Akonadi EWS Resource
    Copyright (C) 2015 Krzysztof Nowicki <krissn@op.pl>

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

#ifndef EWSPROPERTYFIELD_H
#define EWSPROPERTYFIELD_H

#include <QtCore/QXmlStreamWriter>
#include <QtCore/QSharedDataPointer>

#include "ewstypes.h"

class EwsPropertyFieldPrivate;

class EwsPropertyField
{
public:
    EwsPropertyField();
    EwsPropertyField(QString uri);  // FieldURI
    EwsPropertyField(QString uri, int index);   // IndexedFieldURI
    EwsPropertyField(EwsDistinguishedPropSetId psid, int id, EwsPropertyType type);
    EwsPropertyField(EwsDistinguishedPropSetId psid, QString name, EwsPropertyType type);
    EwsPropertyField(QString psid, int id, EwsPropertyType type);
    EwsPropertyField(QString psid, QString name, EwsPropertyType type);
    EwsPropertyField(int tag, EwsPropertyType type);
    EwsPropertyField(const EwsPropertyField &other);
    ~EwsPropertyField();

    EwsPropertyField& operator=(const EwsPropertyField &other);
    bool operator==(const EwsPropertyField &other);

#ifdef Q_COMPILER_RVALUE_REFS
    EwsPropertyField(EwsPropertyField &&other);
    EwsPropertyField& operator=(EwsPropertyField &&other);
#endif

    void write(QXmlStreamWriter &writer) const;
private:
    QSharedDataPointer<EwsPropertyFieldPrivate> d;
};

#endif