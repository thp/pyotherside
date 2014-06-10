
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, 2014, Thomas Perl <m@thp.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **/

#ifndef PYOTHERSIDE_QVARIANT_CONVERTER_H
#define PYOTHERSIDE_QVARIANT_CONVERTER_H

#include "converter.h"

#include <QVariant>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QDebug>

class QVariantListBuilder : public ListBuilder<QVariant> {
    public:
        QVariantListBuilder() : list() {}
        virtual ~QVariantListBuilder() {}

        virtual void append(QVariant v) {
            list << v;
        }

        virtual QVariant value() {
            return QVariant(list);
        }

    private:
        QVariantList list;
};

class QVariantDictBuilder : public DictBuilder<QVariant> {
    public:
        QVariantDictBuilder() : dict() {}
        virtual ~QVariantDictBuilder() {}

        virtual void set(QVariant key, QVariant value) {
            dict[key.toString()] = value;
        }

        virtual QVariant value() {
            return QVariant(dict);
        }

    private:
        QMap<QString,QVariant> dict;
};

class QVariantListIterator : public ListIterator<QVariant> {
    public:
        QVariantListIterator(QVariant &v) : list(v.toList()), pos(0) {}
        virtual ~QVariantListIterator() {}

        virtual bool next(QVariant *v) {
            if (pos == list.size()) {
                return false;
            }

            *v = list[pos];

            pos++;
            return true;
        }

    private:
        QList<QVariant> list;
        int pos;
};

class QVariantDictIterator : public DictIterator<QVariant> {
    public:
        QVariantDictIterator(QVariant &v) : dict(v.toMap()), keys(dict.keys()), pos(0) {}
        virtual ~QVariantDictIterator() {}

        virtual bool next(QVariant *key, QVariant *value) {
            if (pos == keys.size()) {
                return false;
            }

            *key = keys[pos];
            *value = dict[keys[pos]];

            pos++;
            return true;
        }

    private:
        QMap<QString,QVariant> dict;
        QList<QString> keys;
        int pos;
};


class QVariantConverter : public Converter<QVariant> {
    public:
        QVariantConverter() : stringstorage() {}
        virtual ~QVariantConverter() {}

        virtual enum Type type(QVariant &v) {
            QVariant::Type t = v.type();
            switch (t) {
                case QVariant::Bool:
                    return BOOLEAN;
                case QVariant::Int:
                case QVariant::LongLong:
                case QVariant::UInt:
                case QVariant::ULongLong:
                    return INTEGER;
                case QVariant::Double:
                    return FLOATING;
                case QVariant::String:
                    return STRING;
                case QVariant::Date:
                    return DATE;
                case QVariant::Time:
                    return TIME;
                case QVariant::DateTime:
                    return DATETIME;
                case QVariant::List:
                case QVariant::StringList:
                    return LIST;
                case QVariant::Map:
                    return DICT;
                case QVariant::Invalid:
                    return NONE;
                default:
                    qDebug() << "Cannot convert:" << v;
                    return NONE;
            }
        }

        virtual ListIterator<QVariant> *list(QVariant &v) {
            return new QVariantListIterator(v);
        }

        virtual DictIterator<QVariant> *dict(QVariant &v) {
            return new QVariantDictIterator(v);
        }

        virtual long long integer(QVariant &v) {
            return v.toLongLong();
        }

        virtual double floating(QVariant &v) {
            return v.toDouble();
        }

        virtual bool boolean(QVariant &v) {
            return v.toBool();
        }

        virtual ConverterDate date(QVariant &v) {
            QDate d = v.toDate();
            return ConverterDate(d.year(), d.month(), d.day());
        }

        virtual ConverterTime time(QVariant &v) {
            QTime t = v.toTime();
            return ConverterTime(t.hour(), t.minute(), t.second(), t.msec());
        }

        virtual ConverterDateTime dateTime(QVariant &v) {
            QDateTime dt = v.toDateTime();
            QDate d = dt.date();
            QTime t = dt.time();
            return ConverterDateTime(d.year(), d.month(), d.day(),
                    t.hour(), t.minute(), t.second(), t.msec());
        }

        virtual const char *string(QVariant &v) {
            stringstorage = v.toString().toUtf8();
            return stringstorage.constData();
        }

        virtual ListBuilder<QVariant> *newList() {
            return new QVariantListBuilder;
        }

        virtual DictBuilder<QVariant> *newDict() {
            return new QVariantDictBuilder;
        }

        virtual QVariant fromInteger(long long v) { return QVariant(v); }
        virtual QVariant fromFloating(double v) { return QVariant(v); }
        virtual QVariant fromBoolean(bool v) { return QVariant(v); }
        virtual QVariant fromString(const char *v) { return QVariant(QString::fromUtf8(v)); }
        virtual QVariant fromDate(ConverterDate v) { return QVariant(QDate(v.y, v.m, v.d)); }
        virtual QVariant fromTime(ConverterTime v) { return QVariant(QTime(v.h, v.m, v.s, v.ms)); }
        virtual QVariant fromDateTime(ConverterDateTime v) {
            QDate d(v.y, v.m, v.d);
            QTime t(v.time.h, v.time.m, v.time.s, v.time.ms);
            return QVariant(QDateTime(d, t));
        }
        virtual QVariant none() { return QVariant(); };

    private:
        QByteArray stringstorage;
};

#endif /* PYOTHERSIDE_QVARIANT_CONVERTER_H */
