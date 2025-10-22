#include "page.h"
#include "textobject.h"
#include "drawingobject.h"
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QApplication>
#include <algorithm>

Page::Page(QObject *parent)
    : QObject(parent)
    , m_title("Untitled Page")
    , m_size(800, 600)
    , m_backgroundColor(Qt::white)
{
    generateId();
}

Page::Page(const QString &title, QObject *parent)
    : QObject(parent)
    , m_title(title)
    , m_size(800, 600)
    , m_backgroundColor(Qt::white)
{
    generateId();
}

Page::~Page()
{
    // Disconnect all object signals
    for (auto &object : m_objects) {
        disconnectObjectSignals(object);
    }
}

void Page::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged(m_title);
    }
}

void Page::setId(const QString &id)
{
    m_id = id;
}

void Page::setSize(const QSize &size)
{
    if (m_size != size) {
        m_size = size;
        emit sizeChanged(m_size);
    }
}

void Page::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged(m_backgroundColor);
    }
}

void Page::addObject(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    m_objects.append(object);
    connectObjectSignals(object);
    sortObjectsByLayer();
    emit objectAdded(object);
}

void Page::removeObject(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    int index = m_objects.indexOf(object);
    if (index >= 0) {
        disconnectObjectSignals(object);
        m_objects.removeAt(index);
        emit objectRemoved(object);
    }
}

void Page::removeObject(int index)
{
    if (index >= 0 && index < m_objects.size()) {
        auto object = m_objects[index];
        disconnectObjectSignals(object);
        m_objects.removeAt(index);
        emit objectRemoved(object);
    }
}

void Page::clearObjects()
{
    for (auto &object : m_objects) {
        disconnectObjectSignals(object);
    }
    m_objects.clear();
    emit objectSelectionChanged();
}

std::shared_ptr<Object> Page::objectAt(const QPoint &point) const
{
    // Search from top to bottom (reverse order due to layer sorting)
    for (int i = m_objects.size() - 1; i >= 0; --i) {
        if (m_objects[i]->isVisible() && m_objects[i]->contains(point)) {
            return m_objects[i];
        }
    }
    return nullptr;
}

QVector<std::shared_ptr<Object>> Page::objectsInRect(const QRect &rect) const
{
    QVector<std::shared_ptr<Object>> result;
    for (const auto &object : m_objects) {
        if (object->isVisible() && object->intersects(rect)) {
            result.append(object);
        }
    }
    return result;
}

QVector<std::shared_ptr<Object>> Page::selectedObjects() const
{
    QVector<std::shared_ptr<Object>> result;
    for (const auto &object : m_objects) {
        if (object->isSelected()) {
            result.append(object);
        }
    }
    return result;
}

void Page::selectObject(std::shared_ptr<Object> object)
{
    if (object) {
        object->setSelected(true);
    }
}

void Page::deselectObject(std::shared_ptr<Object> object)
{
    if (object) {
        object->setSelected(false);
    }
}

void Page::selectObjectsInRect(const QRect &rect)
{
    for (const auto &object : m_objects) {
        if (object->isVisible() && object->intersects(rect)) {
            object->setSelected(true);
        }
    }
}

void Page::clearSelection()
{
    for (const auto &object : m_objects) {
        object->setSelected(false);
    }
}

void Page::selectAll()
{
    for (const auto &object : m_objects) {
        if (object->isVisible()) {
            object->setSelected(true);
        }
    }
}

void Page::moveSelectedObjects(const QPoint &delta)
{
    for (const auto &object : m_objects) {
        if (object->isSelected()) {
            object->moveBy(delta);
        }
    }
}

void Page::deleteSelectedObjects()
{
    // Remove selected objects (iterate backwards to avoid index issues)
    for (int i = m_objects.size() - 1; i >= 0; --i) {
        if (m_objects[i]->isSelected()) {
            removeObject(i);
        }
    }
}

void Page::duplicateSelectedObjects()
{
    QVector<std::shared_ptr<Object>> objectsToDuplicate;
    
    // Collect selected objects
    for (const auto &object : m_objects) {
        if (object->isSelected()) {
            objectsToDuplicate.append(object);
        }
    }
    
    // Clear selection and add duplicates
    clearSelection();
    for (const auto &object : objectsToDuplicate) {
        auto clone = object->clone();
        clone->moveBy(QPoint(20, 20)); // Offset duplicates
        clone->setSelected(true);
        addObject(std::shared_ptr<Object>(clone.release()));
    }
}

void Page::bringToFront(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    int index = m_objects.indexOf(object);
    if (index >= 0) {
        m_objects.move(index, m_objects.size() - 1);
        object->setLayer(m_objects.size() - 1);
    }
}

void Page::sendToBack(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    int index = m_objects.indexOf(object);
    if (index >= 0) {
        m_objects.move(index, 0);
        object->setLayer(0);
    }
}

void Page::bringForward(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    int index = m_objects.indexOf(object);
    if (index >= 0 && index < m_objects.size() - 1) {
        m_objects.move(index, index + 1);
        object->setLayer(index + 1);
    }
}

void Page::sendBackward(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    int index = m_objects.indexOf(object);
    if (index > 0) {
        m_objects.move(index, index - 1);
        object->setLayer(index - 1);
    }
}

void Page::setObjectLayer(std::shared_ptr<Object> object, int layer)
{
    if (!object) return;
    
    object->setLayer(layer);
    sortObjectsByLayer();
}

void Page::reorderObjectsByLayer()
{
    sortObjectsByLayer();
}

void Page::paint(QPainter &painter, const QRect &viewport)
{
    painter.save();
    
    // Draw background
    painter.fillRect(QRect(QPoint(0, 0), m_size), m_backgroundColor);
    
    // Draw all objects in layer order
    for (const auto &object : m_objects) {
        if (object->isVisible()) {
            object->paint(painter, viewport);
        }
    }
    
    painter.restore();
}

QJsonObject Page::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["title"] = m_title;
    json["size"] = QJsonObject{
        {"width", m_size.width()},
        {"height", m_size.height()}
    };
    json["backgroundColor"] = m_backgroundColor.name();
    
    QJsonArray objectsArray;
    for (const auto &object : m_objects) {
        objectsArray.append(object->toJson());
    }
    json["objects"] = objectsArray;
    
    return json;
}

void Page::fromJson(const QJsonObject &json)
{
    m_id = json["id"].toString();
    m_title = json["title"].toString();
    
    QJsonObject sizeObj = json["size"].toObject();
    m_size = QSize(sizeObj["width"].toInt(), sizeObj["height"].toInt());
    
    m_backgroundColor = QColor(json["backgroundColor"].toString());
    
    // Clear existing objects
    clearObjects();
    
    // Load objects
    QJsonArray objectsArray = json["objects"].toArray();
    for (const QJsonValue &value : objectsArray) {
        QJsonObject objJson = value.toObject();
        Object::Type type = static_cast<Object::Type>(objJson["type"].toInt());
        
        std::shared_ptr<Object> object;
        switch (type) {
        case Object::TextObject:
            object = std::make_shared<TextObject>();
            break;
        case Object::DrawingObject:
            object = std::make_shared<DrawingObject>();
            break;
        case Object::ImageObject:
        case Object::PDFObject:
            // TODO: Implement these object types
            continue;
        }
        
        if (object) {
            object->fromJson(objJson);
            addObject(object);
        }
    }
}

std::unique_ptr<Page> Page::clone() const
{
    auto clone = std::make_unique<Page>();
    clone->fromJson(this->toJson());
    return clone;
}

QJsonObject Page::getState() const
{
    return toJson();
}

void Page::setState(const QJsonObject &state)
{
    fromJson(state);
}

QVector<std::shared_ptr<Object>> Page::findObjectsByType(Object::Type type) const
{
    QVector<std::shared_ptr<Object>> result;
    for (const auto &object : m_objects) {
        if (object->type() == type) {
            result.append(object);
        }
    }
    return result;
}

QVector<std::shared_ptr<Object>> Page::findObjectsContaining(const QString &text) const
{
    QVector<std::shared_ptr<Object>> result;
    for (const auto &object : m_objects) {
        if (object->type() == Object::TextObject) {
            auto textObject = std::dynamic_pointer_cast<TextObject>(object);
            if (textObject && textObject->content().contains(text, Qt::CaseInsensitive)) {
                result.append(object);
            }
        }
    }
    return result;
}

void Page::generateId()
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Page::connectObjectSignals(std::shared_ptr<Object> object)
{
    connect(object.get(), &Object::boundsChanged, this, &Page::onObjectBoundsChanged);
    connect(object.get(), &Object::selectionChanged, this, &Page::onObjectSelectionChanged);
    connect(object.get(), &Object::layerChanged, this, &Page::onObjectLayerChanged);
    connect(object.get(), &Object::visibilityChanged, this, &Page::onObjectVisibilityChanged);
}

void Page::disconnectObjectSignals(std::shared_ptr<Object> object)
{
    disconnect(object.get(), &Object::boundsChanged, this, &Page::onObjectBoundsChanged);
    disconnect(object.get(), &Object::selectionChanged, this, &Page::onObjectSelectionChanged);
    disconnect(object.get(), &Object::layerChanged, this, &Page::onObjectLayerChanged);
    disconnect(object.get(), &Object::visibilityChanged, this, &Page::onObjectVisibilityChanged);
}

void Page::sortObjectsByLayer()
{
    std::sort(m_objects.begin(), m_objects.end(), 
        [](const std::shared_ptr<Object> &a, const std::shared_ptr<Object> &b) {
            return a->layer() < b->layer();
        });
}

void Page::onObjectBoundsChanged(const QRect &newBounds)
{
    // Handle object bounds changes if needed
    Q_UNUSED(newBounds)
}

void Page::onObjectSelectionChanged(bool selected)
{
    Q_UNUSED(selected)
    emit objectSelectionChanged();
}

void Page::onObjectLayerChanged(int newLayer)
{
    Q_UNUSED(newLayer)
    sortObjectsByLayer();
}

void Page::onObjectVisibilityChanged(bool visible)
{
    Q_UNUSED(visible)
    emit objectSelectionChanged();
}
