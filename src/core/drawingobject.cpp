#include "drawingobject.h"
#include <QPainter>
#include <QPainterPath>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>

DrawingObject::DrawingObject(QObject *parent)
    : Object(parent)
    , m_currentMode(PenMode)
    , m_drawing(false)
{
    setupDefaultPen();
    setupDefaultBrush();
}

DrawingObject::~DrawingObject()
{
}

void DrawingObject::setCurrentMode(DrawingMode mode)
{
    if (m_currentMode != mode) {
        m_currentMode = mode;
        setupDefaultPen();
        emit drawingModeChanged(mode);
    }
}

void DrawingObject::setCurrentPen(const QPen &pen)
{
    m_currentPen = pen;
}

void DrawingObject::setCurrentBrush(const QBrush &brush)
{
    m_currentBrush = brush;
}

void DrawingObject::addStroke(const Stroke &stroke)
{
    m_strokes.append(stroke);
    emit strokeAdded(m_strokes.size() - 1);
}

void DrawingObject::removeStroke(int index)
{
    if (index >= 0 && index < m_strokes.size()) {
        m_strokes.removeAt(index);
        m_selectedStrokes.removeAll(index);
        emit strokeRemoved(index);
    }
}

void DrawingObject::clearStrokes()
{
    m_strokes.clear();
    m_selectedStrokes.clear();
    emit strokeSelectionChanged();
}

void DrawingObject::startStroke(const QPoint &point)
{
    if (m_drawing) return;
    
    m_drawing = true;
    m_currentStroke = Stroke();
    m_currentStroke.mode = m_currentMode;
    m_currentStroke.pen = m_currentPen;
    m_currentStroke.brush = m_currentBrush;
    m_currentStroke.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    m_smoothPoints.clear();
    m_smoothPoints.append(point);
    
    m_currentStroke.path.moveTo(point);
}

void DrawingObject::addPointToStroke(const QPoint &point)
{
    if (!m_drawing) return;
    
    QPoint smoothedPoint = smoothPoint(point);
    m_smoothPoints.append(smoothedPoint);
    
    if (m_currentMode == EraserMode) {
        // For eraser, we need to handle erasing logic
        // This is a simplified implementation
        return;
    }
    
    m_currentStroke.path.lineTo(smoothedPoint);
}

void DrawingObject::finishStroke()
{
    if (!m_drawing) return;
    
    m_drawing = false;
    
    if (m_currentStroke.path.length() > 0) {
        addStroke(m_currentStroke);
    }
    
    m_currentStroke = Stroke();
    m_smoothPoints.clear();
}

void DrawingObject::cancelStroke()
{
    if (!m_drawing) return;
    
    m_drawing = false;
    m_currentStroke = Stroke();
    m_smoothPoints.clear();
}

int DrawingObject::getStrokeAt(const QPoint &point) const
{
    for (int i = m_strokes.size() - 1; i >= 0; --i) {
        if (m_strokes[i].path.boundingRect().contains(point)) {
            return i;
        }
    }
    return -1;
}

void DrawingObject::selectStroke(int index)
{
    if (index >= 0 && index < m_strokes.size() && !m_selectedStrokes.contains(index)) {
        m_selectedStrokes.append(index);
        emit strokeSelectionChanged();
    }
}

void DrawingObject::deselectStroke(int index)
{
    if (m_selectedStrokes.removeAll(index) > 0) {
        emit strokeSelectionChanged();
    }
}

void DrawingObject::clearStrokeSelection()
{
    if (!m_selectedStrokes.isEmpty()) {
        m_selectedStrokes.clear();
        emit strokeSelectionChanged();
    }
}

void DrawingObject::moveSelectedStrokes(const QPoint &delta)
{
    for (int index : m_selectedStrokes) {
        if (index >= 0 && index < m_strokes.size()) {
            m_strokes[index].path.translate(delta);
        }
    }
}

void DrawingObject::deleteSelectedStrokes()
{
    // Sort in descending order to avoid index shifting issues
    QVector<int> sortedSelection = m_selectedStrokes;
    std::sort(sortedSelection.begin(), sortedSelection.end(), std::greater<int>());
    
    for (int index : sortedSelection) {
        removeStroke(index);
    }
}

void DrawingObject::duplicateSelectedStrokes()
{
    QVector<Stroke> newStrokes;
    for (int index : m_selectedStrokes) {
        if (index >= 0 && index < m_strokes.size()) {
            newStrokes.append(m_strokes[index]);
        }
    }
    
    for (const Stroke &stroke : newStrokes) {
        addStroke(stroke);
    }
}

void DrawingObject::paint(QPainter &painter, const QRect &viewport)
{
    if (!m_visible) return;
    
    painter.save();
    
    // Set up clipping
    QRect drawRect = m_bounds.intersected(viewport);
    if (drawRect.isEmpty()) {
        painter.restore();
        return;
    }
    painter.setClipRect(drawRect);
    
    // Draw all strokes
    for (int i = 0; i < m_strokes.size(); ++i) {
        const Stroke &stroke = m_strokes[i];
        
        painter.save();
        
        // Set pen and brush
        painter.setPen(stroke.pen);
        painter.setBrush(stroke.brush);
        
        // Draw stroke
        renderStroke(painter, stroke);
        
        // Draw selection highlight
        if (m_selectedStrokes.contains(i)) {
            painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(stroke.path.boundingRect());
        }
        
        painter.restore();
    }
    
    // Draw current stroke being drawn
    if (m_drawing && !m_currentStroke.path.isEmpty()) {
        painter.save();
        painter.setPen(m_currentStroke.pen);
        painter.setBrush(m_currentStroke.brush);
        renderStroke(painter, m_currentStroke);
        painter.restore();
    }
    
    painter.restore();
    
    // Draw object selection handles
    paintSelection(painter);
}

QJsonObject DrawingObject::toJson() const
{
    QJsonObject json = Object::toJson();
    
    QJsonArray strokesArray;
    for (const Stroke &stroke : m_strokes) {
        strokesArray.append(strokeToJson(stroke));
    }
    json["strokes"] = strokesArray;
    
    json["currentMode"] = static_cast<int>(m_currentMode);
    json["currentPen"] = QJsonObject{
        {"color", m_currentPen.color().name()},
        {"width", m_currentPen.widthF()},
        {"style", static_cast<int>(m_currentPen.style())},
        {"capStyle", static_cast<int>(m_currentPen.capStyle())},
        {"joinStyle", static_cast<int>(m_currentPen.joinStyle())}
    };
    
    return json;
}

void DrawingObject::fromJson(const QJsonObject &json)
{
    Object::fromJson(json);
    
    m_strokes.clear();
    QJsonArray strokesArray = json["strokes"].toArray();
    for (const QJsonValue &value : strokesArray) {
        m_strokes.append(strokeFromJson(value.toObject()));
    }
    
    m_currentMode = static_cast<DrawingMode>(json["currentMode"].toInt());
    
    QJsonObject penObj = json["currentPen"].toObject();
    m_currentPen = QPen(
        QColor(penObj["color"].toString()),
        penObj["width"].toDouble(),
        static_cast<Qt::PenStyle>(penObj["style"].toInt()),
        static_cast<Qt::PenCapStyle>(penObj["capStyle"].toInt()),
        static_cast<Qt::PenJoinStyle>(penObj["joinStyle"].toInt())
    );
}

std::unique_ptr<Object> DrawingObject::clone() const
{
    auto clone = std::make_unique<DrawingObject>();
    clone->fromJson(this->toJson());
    return clone;
}

QJsonObject DrawingObject::getState() const
{
    return toJson();
}

void DrawingObject::setState(const QJsonObject &state)
{
    fromJson(state);
}

void DrawingObject::setupDefaultPen()
{
    switch (m_currentMode) {
    case PenMode:
        m_currentPen = QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        break;
    case HighlighterMode:
        m_currentPen = QPen(QColor(255, 255, 0, 128), 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        break;
    case EraserMode:
        m_currentPen = QPen(Qt::white, 20, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        break;
    }
}

void DrawingObject::setupDefaultBrush()
{
    m_currentBrush = QBrush(Qt::NoBrush);
}

QPoint DrawingObject::smoothPoint(const QPoint &point)
{
    if (m_smoothPoints.size() < SMOOTH_WINDOW_SIZE) {
        return point;
    }
    
    // Simple moving average smoothing
    QPoint sum(0, 0);
    int count = 0;
    
    for (int i = qMax(0, m_smoothPoints.size() - SMOOTH_WINDOW_SIZE); i < m_smoothPoints.size(); ++i) {
        sum += m_smoothPoints[i];
        count++;
    }
    
    return QPoint(sum.x() / count, sum.y() / count);
}

void DrawingObject::renderStroke(QPainter &painter, const Stroke &stroke)
{
    switch (stroke.mode) {
    case PenMode:
        painter.drawPath(stroke.path);
        break;
    case HighlighterMode:
        painter.setCompositionMode(QPainter::CompositionMode_Multiply);
        painter.drawPath(stroke.path);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        break;
    case EraserMode:
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.drawPath(stroke.path);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        break;
    }
}

QJsonObject DrawingObject::strokeToJson(const Stroke &stroke) const
{
    QJsonObject json;
    json["mode"] = static_cast<int>(stroke.mode);
    json["timestamp"] = stroke.timestamp;
    
    // Convert path to JSON (simplified - store as SVG path)
    json["path"] = stroke.path.toSvgPath();
    
    // Convert pen to JSON
    json["pen"] = QJsonObject{
        {"color", stroke.pen.color().name()},
        {"width", stroke.pen.widthF()},
        {"style", static_cast<int>(stroke.pen.style())},
        {"capStyle", static_cast<int>(stroke.pen.capStyle())},
        {"joinStyle", static_cast<int>(stroke.pen.joinStyle())}
    };
    
    return json;
}

DrawingObject::Stroke DrawingObject::strokeFromJson(const QJsonObject &json) const
{
    Stroke stroke;
    stroke.mode = static_cast<DrawingMode>(json["mode"].toInt());
    stroke.timestamp = json["timestamp"].toVariant().toLongLong();
    
    // Convert SVG path back to QPainterPath
    QString svgPath = json["path"].toString();
    stroke.path = QPainterPath::fromSvgPath(svgPath);
    
    // Convert pen from JSON
    QJsonObject penObj = json["pen"].toObject();
    stroke.pen = QPen(
        QColor(penObj["color"].toString()),
        penObj["width"].toDouble(),
        static_cast<Qt::PenStyle>(penObj["style"].toInt()),
        static_cast<Qt::PenCapStyle>(penObj["capStyle"].toInt()),
        static_cast<Qt::PenJoinStyle>(penObj["joinStyle"].toInt())
    );
    
    return stroke;
}
