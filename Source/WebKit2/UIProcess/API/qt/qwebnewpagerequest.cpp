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

#include "config.h"
#include "qwebnewpagerequest_p.h"
#include "qquickwebview_p.h"

class QWebNewPageRequestPrivate {
public:
    QWebNewPageRequestPrivate(const QUrl& url, const QVariantMap& windowFeatures, Qt::KeyboardModifiers keyboardModifiers, Qt::MouseButton mouseButton)
        : url(url)
        , mouseButton(mouseButton)
        , keyboardModifiers(keyboardModifiers)
        , windowFeatures(windowFeatures)
        , webView(0)
    {
    }

    ~QWebNewPageRequestPrivate()
    {
    }

    QUrl url;
    Qt::MouseButton mouseButton;
    Qt::KeyboardModifiers keyboardModifiers;
    QVariantMap windowFeatures;
    QQuickWebView *webView;
};

QWebNewPageRequest::QWebNewPageRequest(const QUrl& url, const QVariantMap& windowFeatures, Qt::KeyboardModifiers keyboardModifiers, Qt::MouseButton mouseButton, QObject* parent)
    : QObject(parent)
    , d(new QWebNewPageRequestPrivate(url, windowFeatures, keyboardModifiers, mouseButton))
{
}

QWebNewPageRequest::~QWebNewPageRequest()
{
    delete d;
}

void QWebNewPageRequest::setWebView(QQuickWebView *webView)
{
    if (d->webView == webView)
        return;

    d->webView = webView;
    emit webViewChanged();
}

QUrl QWebNewPageRequest::url() const
{
    return d->url;
}

QVariantMap QWebNewPageRequest::windowFeatures() const
{
    return d->windowFeatures;
}

int QWebNewPageRequest::mouseButton() const
{
    return int(d->mouseButton);
}

int QWebNewPageRequest::keyboardModifiers() const
{
    return int(d->keyboardModifiers);
}

QQuickWebView* QWebNewPageRequest::webView() const
{
    return d->webView;
}
