#include "object.h"
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>

Object::Object(QObject *parent)
    : QObject(parent)
    , m_bounds(0, 0, 100, 100)
    , m_selected(false)
    , m_layer(0)
    , m_visible(true)
{
    generateId();
}

void Object::setBounds(const QRect &bounds)
{
    if (m_bounds != bounds) {
        m_bounds = bounds;
        boundsChangedInternal();
        emit boundsChanged(m_bounds);
    }
}

void Object::setPosition(const QPoint &position)
{
    setBounds(QRect(position, m_bounds.size()));
}

void Object::setSize(const QSize &size)
{
    setBounds(QRect(m_bounds.topLeft(), size));
}

void Object::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        emit selectionChanged(m_selected);
    }
}

bool Object::contains(const QPoint &point) const
{
    return m_bounds.contains(point);
}

bool Object::intersects(const QRect &rect) const
{
    return m_bounds.intersects(rect);
}

void Object::setLayer(int layer)
{
    if (m_layer != layer) {
        m_layer = layer;
        emit layerChanged(m_layer);
    }
}

void Object::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        emit visibilityChanged(m_visible);
    }
}

void Object::paintSelection(QPainter &painter)
{
    if (!m_selected) return;
    
    painter.save();
    painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_bounds);
    
    // Draw resize handles
    const int handleSize = 8;
    QRect handles[] = {
        QRect(m_bounds.topLeft() - QPoint(handleSize/2, handleSize/2), QSize(handleSize, handleSize)),
        QRect(m_bounds.topRight() - QPoint(handleSize/2, handleSize/2), QSize(handleSize, handleSize)),
        QRect(m_bounds.bottomLeft() - QPoint(handleSize/2, handleSize/2), QSize(handleSize, handleSize)),
        QRect(m_bounds.bottomRight() - QPoint(handleSize/2, handleSize/2), QSize(handleSize, handleSize))
    };
    
    painter.setPen(Qt::blue);
    painter.setBrush(Qt::white);
    for (const QRect &handle : handles) {
        painter.drawRect(handle);
    }
    
    painter.restore();
}

QJsonObject Object::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["type"] = static_cast<int>(type());
    json["bounds"] = QJsonObject{
        {"x", m_bounds.x()},
        {"y", m_bounds.y()},
        {"width", m_bounds.width()},
        {"height", m_bounds.height()}
    };
    json["layer"] = m_layer;
    json["visible"] = m_visible;
    return json;
}

void Object::fromJson(const QJsonObject &json)
{
    m_id = json["id"].toString();
    
    QJsonObject boundsObj = json["bounds"].toObject();
    m_bounds = QRect(
        boundsObj["x"].toInt(),
        boundsObj["y"].toInt(),
        boundsObj["width"].toInt(),
        boundsObj["height"].toInt()
    );
    
    m_layer = json["layer"].toInt();
    m_visible = json["visible"].toBool();
}

void Object::moveBy(const QPoint &delta)
{
    setBounds(m_bounds.translated(delta));
}

void Object::scale(double factor)
{
    QPoint center = m_bounds.center();
    QSize newSize = QSize(
        static_cast<int>(m_bounds.width() * factor),
        static_cast<int>(m_bounds.height() * factor)
    );
    setBounds(QRect(center - QPoint(newSize.width()/2, newSize.height()/2), newSize));
}

QJsonObject Object::getState() const
{
    return toJson();
}

void Object::setState(const QJsonObject &state)
{
    fromJson(state);
}

void Object::generateId()
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Object::boundsChangedInternal()
{
    // Override in derived classes if needed
}
