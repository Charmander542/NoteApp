#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QRect>
#include <QPoint>
#include <QPainter>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

/**
 * @brief Base class for all objects that can be placed on a page
 * 
 * This class provides the fundamental interface for all objects that can be
 * placed, moved, resized, and rendered on a page. It includes support for
 * serialization, selection, and basic manipulation operations.
 */
class Object : public QObject
{
    Q_OBJECT

public:
    enum Type {
        TextObject,
        DrawingObject,
        ImageObject,
        PDFObject
    };

    explicit Object(QObject *parent = nullptr);
    virtual ~Object() = default;

    // Core properties
    virtual Type type() const = 0;
    virtual QString typeName() const = 0;
    
    // Geometry
    QRect bounds() const { return m_bounds; }
    void setBounds(const QRect &bounds);
    QPoint position() const { return m_bounds.topLeft(); }
    void setPosition(const QPoint &position);
    QSize size() const { return m_bounds.size(); }
    void setSize(const QSize &size);
    
    // Selection and interaction
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected);
    bool contains(const QPoint &point) const;
    bool intersects(const QRect &rect) const;
    
    // Layer management
    int layer() const { return m_layer; }
    void setLayer(int layer);
    
    // Visibility
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible);
    
    // Rendering
    virtual void paint(QPainter &painter, const QRect &viewport) = 0;
    virtual void paintSelection(QPainter &painter);
    
    // Serialization
    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject &json);
    
    // Operations
    virtual void moveBy(const QPoint &delta);
    virtual void scale(double factor);
    virtual std::unique_ptr<Object> clone() const = 0;
    
    // Undo/Redo support
    virtual QJsonObject getState() const;
    virtual void setState(const QJsonObject &state);

signals:
    void boundsChanged(const QRect &newBounds);
    void selectionChanged(bool selected);
    void layerChanged(int newLayer);
    void visibilityChanged(bool visible);

protected:
    QRect m_bounds;
    bool m_selected;
    int m_layer;
    bool m_visible;
    QString m_id;
    
    void generateId();
    virtual void boundsChangedInternal();
};

#endif // OBJECT_H
