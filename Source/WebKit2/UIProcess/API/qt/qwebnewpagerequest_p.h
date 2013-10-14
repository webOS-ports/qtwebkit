/*
    Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)

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

#ifndef qwebnewpagerequest_p_h
#define qwebnewpagerequest_p_h

#include "qquickwebview_p.h"
#include "qwebkitglobal.h"

#include <QtCore/QObject>
#include <QtCore/QUrl>

class QWebNewPageRequestPrivate;

class QWEBKIT_EXPORT QWebNewPageRequest : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)
    Q_PROPERTY(QVariantMap windowFeatures READ windowFeatures CONSTANT FINAL)
    Q_PROPERTY(int mouseButton READ mouseButton CONSTANT FINAL)
    Q_PROPERTY(int keyboardModifiers READ keyboardModifiers CONSTANT FINAL)
    Q_PROPERTY(QQuickWebView *webView READ webView WRITE setWebView NOTIFY webViewChanged);

public:
    QWebNewPageRequest(const QUrl& url, const QVariantMap& windowFeature,
                       Qt::KeyboardModifiers modifiers, Qt::MouseButton mouseButton,
                       QObject* parent = 0);
    ~QWebNewPageRequest();

    QUrl url() const;
    int mouseButton() const;
    int keyboardModifiers() const;
    QVariantMap windowFeatures() const;

    void setWebView(QQuickWebView *webView);
    QQuickWebView *webView() const;

Q_SIGNALS:
    void webViewChanged();

private:
    QWebNewPageRequestPrivate* d;
};

#endif // qwebnewpagerequest_h
