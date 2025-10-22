#ifndef PAGECANVAS_H
#define PAGECANVAS_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollArea>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <memory>

// Forward declarations
class Page;
class Object;

/**
 * @brief Canvas widget for displaying and interacting with page content
 * 
 * This widget provides the main drawing area where users can view and interact
 * with page objects. It handles mouse input for object selection, manipulation,
 * and drawing operations.
 */
class PageCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit PageCanvas(QWidget *parent = nullptr);
    ~PageCanvas() override;

    // Page management
    void setPage(std::shared_ptr<Page> page);
    std::shared_ptr<Page> page() const { return m_page; }
    
    // Zoom and view
    double zoomFactor() const { return m_zoomFactor; }
    void setZoomFactor(double factor);
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomActual();
    
    // Viewport management
    QPoint viewportOffset() const { return m_viewportOffset; }
    void setViewportOffset(const QPoint &offset);
    void centerOn(const QPoint &point);
    void centerOn(const QRect &rect);
    
    // Selection
    void clearSelection();
    void selectAll();
    QRect selectionRect() const { return m_selectionRect; }
    
    // Grid and guides
    bool showGrid() const { return m_showGrid; }
    void setShowGrid(bool show);
    int gridSize() const { return m_gridSize; }
    void setGridSize(int size);
    
    // Snap to grid
    bool snapToGrid() const { return m_snapToGrid; }
    void setSnapToGrid(bool snap);
    QPoint snapToGrid(const QPoint &point) const;

signals:
    void pageChanged(std::shared_ptr<Page> page);
    void objectSelected(std::shared_ptr<Object> object);
    void objectDeselected(std::shared_ptr<Object> object);
    void selectionChanged();
    void zoomChanged(double factor);
    void viewportChanged(const QPoint &offset);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    std::shared_ptr<Page> m_page;
    double m_zoomFactor;
    QPoint m_viewportOffset;
    
    // Selection state
    QRect m_selectionRect;
    bool m_selecting;
    QPoint m_selectionStart;
    QPoint m_selectionEnd;
    
    // Interaction state
    enum InteractionMode {
        SelectMode,
        DrawMode,
        PanMode
    };
    InteractionMode m_mode;
    
    // Grid and guides
    bool m_showGrid;
    int m_gridSize;
    bool m_snapToGrid;
    
    // Mouse state
    QPoint m_lastMousePos;
    bool m_dragging;
    std::shared_ptr<Object> m_draggedObject;
    QPoint m_dragStartPos;
    
    // Helper methods
    QPoint screenToPage(const QPoint &screenPoint) const;
    QPoint pageToScreen(const QPoint &pagePoint) const;
    QRect screenToPage(const QRect &screenRect) const;
    QRect pageToScreen(const QRect &pageRect) const;
    
    void drawGrid(QPainter &painter);
    void drawSelection(QPainter &painter);
    void drawViewport(QPainter &painter);
    
    std::shared_ptr<Object> objectAt(const QPoint &point) const;
    QVector<std::shared_ptr<Object>> objectsInRect(const QRect &rect) const;
    
    void startSelection(const QPoint &point);
    void updateSelection(const QPoint &point);
    void finishSelection();
    void cancelSelection();
    
    void startDrag(std::shared_ptr<Object> object, const QPoint &point);
    void updateDrag(const QPoint &point);
    void finishDrag();
    void cancelDrag();
    
    void updateViewport();
    void ensureVisible(const QRect &rect);
};

#endif // PAGECANVAS_H
