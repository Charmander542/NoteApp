#include "pagecanvas.h"
#include "../core/page.h"
#include "../core/object.h"
#include "../core/textobject.h"
#include "../core/drawingobject.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollArea>
#include <QApplication>
#include <QDebug>
#include <cmath>

PageCanvas::PageCanvas(QWidget *parent)
    : QWidget(parent)
    , m_zoomFactor(1.0)
    , m_viewportOffset(0, 0)
    , m_selecting(false)
    , m_mode(SelectMode)
    , m_showGrid(true)
    , m_gridSize(20)
    , m_snapToGrid(false)
    , m_dragging(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(400, 300);
}

PageCanvas::~PageCanvas()
{
}

void PageCanvas::setPage(std::shared_ptr<Page> page)
{
    if (m_page == page) return;
    
    m_page = page;
    update();
    emit pageChanged(m_page);
}

void PageCanvas::setZoomFactor(double factor)
{
    factor = qMax(0.1, qMin(5.0, factor)); // Clamp between 0.1 and 5.0
    
    if (m_zoomFactor != factor) {
        m_zoomFactor = factor;
        update();
        emit zoomChanged(m_zoomFactor);
    }
}

void PageCanvas::zoomIn()
{
    setZoomFactor(m_zoomFactor * 1.2);
}

void PageCanvas::zoomOut()
{
    setZoomFactor(m_zoomFactor / 1.2);
}

void PageCanvas::zoomFit()
{
    if (!m_page) return;
    
    QSize pageSize = m_page->size();
    QSize widgetSize = size();
    
    double scaleX = static_cast<double>(widgetSize.width()) / pageSize.width();
    double scaleY = static_cast<double>(widgetSize.height()) / pageSize.height();
    double scale = qMin(scaleX, scaleY) * 0.9; // 90% to leave some margin
    
    setZoomFactor(scale);
    centerOn(QRect(QPoint(0, 0), pageSize));
}

void PageCanvas::zoomActual()
{
    setZoomFactor(1.0);
}

void PageCanvas::setViewportOffset(const QPoint &offset)
{
    if (m_viewportOffset != offset) {
        m_viewportOffset = offset;
        update();
        emit viewportChanged(m_viewportOffset);
    }
}

void PageCanvas::centerOn(const QPoint &point)
{
    QPoint center = QPoint(width() / 2, height() / 2);
    QPoint offset = center - pageToScreen(point);
    setViewportOffset(offset);
}

void PageCanvas::centerOn(const QRect &rect)
{
    centerOn(rect.center());
}

void PageCanvas::clearSelection()
{
    if (m_page) {
        m_page->clearSelection();
    }
    m_selectionRect = QRect();
    update();
    emit selectionChanged();
}

void PageCanvas::selectAll()
{
    if (m_page) {
        m_page->selectAll();
        update();
        emit selectionChanged();
    }
}

void PageCanvas::setShowGrid(bool show)
{
    if (m_showGrid != show) {
        m_showGrid = show;
        update();
    }
}

void PageCanvas::setGridSize(int size)
{
    if (m_gridSize != size) {
        m_gridSize = qMax(5, size);
        update();
    }
}

void PageCanvas::setSnapToGrid(bool snap)
{
    m_snapToGrid = snap;
}

QPoint PageCanvas::snapToGrid(const QPoint &point) const
{
    if (!m_snapToGrid) return point;
    
    int gridSize = static_cast<int>(m_gridSize * m_zoomFactor);
    int x = ((point.x() + gridSize / 2) / gridSize) * gridSize;
    int y = ((point.y() + gridSize / 2) / gridSize) * gridSize;
    
    return QPoint(x, y);
}

void PageCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), QColor(240, 240, 240));
    
    if (!m_page) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "No page loaded");
        return;
    }
    
    painter.save();
    
    // Apply viewport transformation
    painter.translate(m_viewportOffset);
    painter.scale(m_zoomFactor, m_zoomFactor);
    
    // Draw grid
    if (m_showGrid) {
        drawGrid(painter);
    }
    
    // Draw page background
    QSize pageSize = m_page->size();
    painter.fillRect(QRect(QPoint(0, 0), pageSize), m_page->backgroundColor());
    
    // Draw page border
    painter.setPen(QPen(Qt::black, 1 / m_zoomFactor));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(QPoint(0, 0), pageSize));
    
    // Draw page content
    m_page->paint(painter, screenToPage(rect()));
    
    painter.restore();
    
    // Draw selection rectangle
    if (m_selecting && !m_selectionRect.isEmpty()) {
        drawSelection(painter);
    }
}

void PageCanvas::mousePressEvent(QMouseEvent *event)
{
    if (!m_page) return;
    
    QPoint pagePoint = screenToPage(event->pos());
    m_lastMousePos = event->pos();
    
    if (event->button() == Qt::LeftButton) {
        std::shared_ptr<Object> object = objectAt(pagePoint);
        
        if (object) {
            // Clicked on an object
            if (!object->isSelected()) {
                if (!(event->modifiers() & Qt::ControlModifier)) {
                    m_page->clearSelection();
                }
                m_page->selectObject(object);
                emit objectSelected(object);
            }
            
            // Start dragging
            startDrag(object, event->pos());
        } else {
            // Clicked on empty space
            if (!(event->modifiers() & Qt::ControlModifier)) {
                m_page->clearSelection();
            }
            
            // Start selection rectangle
            startSelection(event->pos());
        }
    } else if (event->button() == Qt::RightButton) {
        // Context menu or pan mode
        m_mode = PanMode;
        m_dragging = true;
    }
    
    update();
}

void PageCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_page) return;
    
    QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    
    if (m_dragging && m_draggedObject) {
        updateDrag(event->pos());
    } else if (m_selecting) {
        updateSelection(event->pos());
    } else if (m_mode == PanMode && m_dragging) {
        setViewportOffset(m_viewportOffset + delta);
    }
    
    update();
}

void PageCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_page) return;
    
    if (event->button() == Qt::LeftButton) {
        if (m_dragging && m_draggedObject) {
            finishDrag();
        } else if (m_selecting) {
            finishSelection();
        }
    } else if (event->button() == Qt::RightButton) {
        if (m_mode == PanMode) {
            m_mode = SelectMode;
            m_dragging = false;
        }
    }
    
    update();
}

void PageCanvas::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom
        QPoint mousePos = event->pos();
        QPoint pagePoint = screenToPage(mousePos);
        
        double scaleFactor = event->angleDelta().y() > 0 ? 1.2 : 1.0 / 1.2;
        double newZoom = m_zoomFactor * scaleFactor;
        
        setZoomFactor(newZoom);
        
        // Keep the point under the mouse in the same place
        QPoint newScreenPoint = pageToScreen(pagePoint);
        QPoint offset = mousePos - newScreenPoint;
        setViewportOffset(m_viewportOffset + offset);
    } else {
        // Pan
        QPoint delta = event->angleDelta() / 8;
        setViewportOffset(m_viewportOffset - delta);
    }
    
    event->accept();
}

void PageCanvas::keyPressEvent(QKeyEvent *event)
{
    if (!m_page) return;
    
    switch (event->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        m_page->deleteSelectedObjects();
        update();
        break;
        
    case Qt::Key_Escape:
        clearSelection();
        break;
        
    case Qt::Key_A:
        if (event->modifiers() & Qt::ControlModifier) {
            selectAll();
        }
        break;
        
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void PageCanvas::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateViewport();
}

QPoint PageCanvas::screenToPage(const QPoint &screenPoint) const
{
    QPoint point = screenPoint - m_viewportOffset;
    return QPoint(static_cast<int>(point.x() / m_zoomFactor), 
                  static_cast<int>(point.y() / m_zoomFactor));
}

QPoint PageCanvas::pageToScreen(const QPoint &pagePoint) const
{
    QPoint point = QPoint(static_cast<int>(pagePoint.x() * m_zoomFactor),
                          static_cast<int>(pagePoint.y() * m_zoomFactor));
    return point + m_viewportOffset;
}

QRect PageCanvas::screenToPage(const QRect &screenRect) const
{
    QPoint topLeft = screenToPage(screenRect.topLeft());
    QPoint bottomRight = screenToPage(screenRect.bottomRight());
    return QRect(topLeft, bottomRight);
}

QRect PageCanvas::pageToScreen(const QRect &pageRect) const
{
    QPoint topLeft = pageToScreen(pageRect.topLeft());
    QPoint bottomRight = pageToScreen(pageRect.bottomRight());
    return QRect(topLeft, bottomRight);
}

void PageCanvas::drawGrid(QPainter &painter)
{
    if (!m_page) return;
    
    QSize pageSize = m_page->size();
    int gridSize = static_cast<int>(m_gridSize * m_zoomFactor);
    
    if (gridSize < 5) return; // Don't draw grid if too small
    
    painter.setPen(QPen(QColor(200, 200, 200), 1 / m_zoomFactor));
    
    // Draw vertical lines
    for (int x = 0; x <= pageSize.width(); x += m_gridSize) {
        int screenX = static_cast<int>(x * m_zoomFactor);
        painter.drawLine(screenX, 0, screenX, static_cast<int>(pageSize.height() * m_zoomFactor));
    }
    
    // Draw horizontal lines
    for (int y = 0; y <= pageSize.height(); y += m_gridSize) {
        int screenY = static_cast<int>(y * m_zoomFactor);
        painter.drawLine(0, screenY, static_cast<int>(pageSize.width() * m_zoomFactor), screenY);
    }
}

void PageCanvas::drawSelection(QPainter &painter)
{
    if (m_selectionRect.isEmpty()) return;
    
    painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
    painter.setBrush(QColor(0, 0, 255, 30));
    painter.drawRect(m_selectionRect);
}

void PageCanvas::drawViewport(QPainter &painter)
{
    // This method can be used to draw viewport indicators
    // Currently not implemented
    Q_UNUSED(painter)
}

std::shared_ptr<Object> PageCanvas::objectAt(const QPoint &point) const
{
    if (!m_page) return nullptr;
    
    return m_page->objectAt(point);
}

QVector<std::shared_ptr<Object>> PageCanvas::objectsInRect(const QRect &rect) const
{
    if (!m_page) return QVector<std::shared_ptr<Object>>();
    
    return m_page->objectsInRect(rect);
}

void PageCanvas::startSelection(const QPoint &point)
{
    m_selecting = true;
    m_selectionStart = point;
    m_selectionEnd = point;
    m_selectionRect = QRect();
}

void PageCanvas::updateSelection(const QPoint &point)
{
    if (!m_selecting) return;
    
    m_selectionEnd = point;
    m_selectionRect = QRect(m_selectionStart, m_selectionEnd).normalized();
}

void PageCanvas::finishSelection()
{
    if (!m_selecting) return;
    
    m_selecting = false;
    
    if (!m_selectionRect.isEmpty()) {
        QRect pageRect = screenToPage(m_selectionRect);
        m_page->selectObjectsInRect(pageRect);
        emit selectionChanged();
    }
    
    m_selectionRect = QRect();
}

void PageCanvas::cancelSelection()
{
    m_selecting = false;
    m_selectionRect = QRect();
}

void PageCanvas::startDrag(std::shared_ptr<Object> object, const QPoint &point)
{
    m_draggedObject = object;
    m_dragStartPos = point;
    m_dragging = true;
}

void PageCanvas::updateDrag(const QPoint &point)
{
    if (!m_draggedObject || !m_dragging) return;
    
    QPoint delta = point - m_dragStartPos;
    QPoint pageDelta = screenToPage(delta) - screenToPage(QPoint(0, 0));
    
    if (m_snapToGrid) {
        pageDelta = snapToGrid(pageDelta) - snapToGrid(QPoint(0, 0));
    }
    
    m_page->moveSelectedObjects(pageDelta);
    m_dragStartPos = point;
}

void PageCanvas::finishDrag()
{
    m_dragging = false;
    m_draggedObject.reset();
}

void PageCanvas::cancelDrag()
{
    m_dragging = false;
    m_draggedObject.reset();
}

void PageCanvas::updateViewport()
{
    // Ensure the page is visible
    if (m_page) {
        QSize pageSize = m_page->size();
        QSize screenSize = pageToScreen(pageSize).size();
        
        if (screenSize.width() < width() || screenSize.height() < height()) {
            centerOn(QRect(QPoint(0, 0), pageSize));
        }
    }
}

void PageCanvas::ensureVisible(const QRect &rect)
{
    QRect screenRect = pageToScreen(rect);
    QRect widgetRect = this->rect();
    
    if (!widgetRect.contains(screenRect)) {
        centerOn(rect);
    }
}
