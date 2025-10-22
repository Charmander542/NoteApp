#ifndef PAGE_H
#define PAGE_H

#include "object.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSize>
#include <QColor>
#include <memory>

/**
 * @brief A page that contains multiple objects and manages their layout
 * 
 * This class represents a single page in a note, containing various objects
 * like text, drawings, images, and PDFs. It manages object layering, selection,
 * and provides operations for object manipulation.
 */
class Page : public QObject
{
    Q_OBJECT

public:
    explicit Page(QObject *parent = nullptr);
    explicit Page(const QString &title, QObject *parent = nullptr);
    ~Page() override;

    // Basic properties
    QString title() const { return m_title; }
    void setTitle(const QString &title);
    
    QString id() const { return m_id; }
    void setId(const QString &id);
    
    QSize size() const { return m_size; }
    void setSize(const QSize &size);
    
    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor &color);
    
    // Object management
    const QVector<std::shared_ptr<Object>> &objects() const { return m_objects; }
    void addObject(std::shared_ptr<Object> object);
    void removeObject(std::shared_ptr<Object> object);
    void removeObject(int index);
    void clearObjects();
    
    // Object queries
    std::shared_ptr<Object> objectAt(const QPoint &point) const;
    QVector<std::shared_ptr<Object>> objectsInRect(const QRect &rect) const;
    QVector<std::shared_ptr<Object>> selectedObjects() const;
    
    // Selection management
    void selectObject(std::shared_ptr<Object> object);
    void deselectObject(std::shared_ptr<Object> object);
    void selectObjectsInRect(const QRect &rect);
    void clearSelection();
    void selectAll();
    
    // Object manipulation
    void moveSelectedObjects(const QPoint &delta);
    void deleteSelectedObjects();
    void duplicateSelectedObjects();
    void bringToFront(std::shared_ptr<Object> object);
    void sendToBack(std::shared_ptr<Object> object);
    void bringForward(std::shared_ptr<Object> object);
    void sendBackward(std::shared_ptr<Object> object);
    
    // Layer management
    void setObjectLayer(std::shared_ptr<Object> object, int layer);
    void reorderObjectsByLayer();
    
    // Rendering
    void paint(QPainter &painter, const QRect &viewport);
    
    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
    
    // Operations
    std::unique_ptr<Page> clone() const;
    
    // Undo/Redo support
    QJsonObject getState() const;
    void setState(const QJsonObject &state);
    
    // Search and filtering
    QVector<std::shared_ptr<Object>> findObjectsByType(Object::Type type) const;
    QVector<std::shared_ptr<Object>> findObjectsContaining(const QString &text) const;

signals:
    void titleChanged(const QString &newTitle);
    void sizeChanged(const QSize &newSize);
    void backgroundColorChanged(const QColor &newColor);
    void objectAdded(std::shared_ptr<Object> object);
    void objectRemoved(std::shared_ptr<Object> object);
    void objectSelectionChanged();
    void objectLayerChanged(std::shared_ptr<Object> object, int newLayer);

private:
    QString m_title;
    QString m_id;
    QSize m_size;
    QColor m_backgroundColor;
    QVector<std::shared_ptr<Object>> m_objects;
    
    void generateId();
    void connectObjectSignals(std::shared_ptr<Object> object);
    void disconnectObjectSignals(std::shared_ptr<Object> object);
    void sortObjectsByLayer();

private slots:
    void onObjectBoundsChanged(const QRect &newBounds);
    void onObjectSelectionChanged(bool selected);
    void onObjectLayerChanged(int newLayer);
    void onObjectVisibilityChanged(bool visible);
};

#endif // PAGE_H
