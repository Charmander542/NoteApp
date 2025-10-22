#ifndef DRAWINGOBJECT_H
#define DRAWINGOBJECT_H

#include "object.h"
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QPoint>
#include <QVector>
#include <QColor>

/**
 * @brief Drawing object that supports freeform pen input and drawing
 * 
 * This class provides a drawing object that can capture pen strokes,
 * support different drawing modes (pen, highlighter, eraser), and
 * allow editing of individual strokes.
 */
class DrawingObject : public Object
{
    Q_OBJECT

public:
    enum DrawingMode {
        PenMode,
        HighlighterMode,
        EraserMode
    };

    struct Stroke {
        QPainterPath path;
        QPen pen;
        QBrush brush;
        DrawingMode mode;
        qint64 timestamp;
        
        Stroke() : mode(PenMode), timestamp(0) {}
    };

    explicit DrawingObject(QObject *parent = nullptr);
    ~DrawingObject() override;

    // Object interface
    Type type() const override { return DrawingObject; }
    QString typeName() const override { return "Drawing"; }
    
    // Drawing properties
    DrawingMode currentMode() const { return m_currentMode; }
    void setCurrentMode(DrawingMode mode);
    
    QPen currentPen() const { return m_currentPen; }
    void setCurrentPen(const QPen &pen);
    
    QBrush currentBrush() const { return m_currentBrush; }
    void setCurrentBrush(const QBrush &brush);
    
    // Stroke management
    const QVector<Stroke> &strokes() const { return m_strokes; }
    void addStroke(const Stroke &stroke);
    void removeStroke(int index);
    void clearStrokes();
    
    // Drawing operations
    void startStroke(const QPoint &point);
    void addPointToStroke(const QPoint &point);
    void finishStroke();
    void cancelStroke();
    
    // Stroke editing
    int getStrokeAt(const QPoint &point) const;
    void selectStroke(int index);
    void deselectStroke(int index);
    void clearStrokeSelection();
    const QVector<int> &selectedStrokes() const { return m_selectedStrokes; }
    
    // Stroke manipulation
    void moveSelectedStrokes(const QPoint &delta);
    void deleteSelectedStrokes();
    void duplicateSelectedStrokes();
    
    // Rendering
    void paint(QPainter &painter, const QRect &viewport) override;
    
    // Serialization
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;
    
    // Operations
    std::unique_ptr<Object> clone() const override;
    
    // Undo/Redo
    QJsonObject getState() const override;
    void setState(const QJsonObject &state) override;

signals:
    void strokeAdded(int index);
    void strokeRemoved(int index);
    void strokeSelectionChanged();
    void drawingModeChanged(DrawingMode mode);

private:
    DrawingMode m_currentMode;
    QPen m_currentPen;
    QBrush m_currentBrush;
    QVector<Stroke> m_strokes;
    QVector<int> m_selectedStrokes;
    
    // Current stroke being drawn
    Stroke m_currentStroke;
    bool m_drawing;
    
    // Stroke smoothing
    QVector<QPoint> m_smoothPoints;
    static const int SMOOTH_WINDOW_SIZE = 3;
    
    void setupDefaultPen();
    void setupDefaultBrush();
    QPoint smoothPoint(const QPoint &point);
    void renderStroke(QPainter &painter, const Stroke &stroke);
    QJsonObject strokeToJson(const Stroke &stroke) const;
    Stroke strokeFromJson(const QJsonObject &json) const;
};

#endif // DRAWINGOBJECT_H
