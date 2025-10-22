#ifndef OBJECTSELECTOR_H
#define OBJECTSELECTOR_H

#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <memory>

// Forward declarations
class Page;
class Object;

/**
 * @brief Dock widget for object selection and property editing
 * 
 * This widget provides a hierarchical view of objects on the current page
 * and allows editing of object properties. It's designed to be docked
 * on the right side of the main window.
 */
class ObjectSelector : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectSelector(QWidget *parent = nullptr);
    ~ObjectSelector() override;

    // Page management
    void setPage(std::shared_ptr<Page> page);
    std::shared_ptr<Page> page() const { return m_page; }
    
    // Selection management
    void setSelectedObject(std::shared_ptr<Object> object);
    std::shared_ptr<Object> selectedObject() const { return m_selectedObject; }
    void clearSelection();
    
    // Object operations
    void addObject(std::shared_ptr<Object> object);
    void removeObject(std::shared_ptr<Object> object);
    void updateObject(std::shared_ptr<Object> object);

signals:
    void objectSelected(std::shared_ptr<Object> object);
    void objectPropertyChanged(std::shared_ptr<Object> object, const QString &property, const QVariant &value);
    void objectDeleted(std::shared_ptr<Object> object);

private slots:
    void onObjectTreeItemClicked(QTreeWidgetItem *item, int column);
    void onObjectTreeItemChanged(QTreeWidgetItem *item, int column);
    void onDeleteObjectClicked();
    void onBringToFrontClicked();
    void onSendToBackClicked();
    void onBringForwardClicked();
    void onSendBackwardClicked();
    void onDuplicateObjectClicked();
    
    // Property change slots
    void onPositionChanged();
    void onSizeChanged();
    void onLayerChanged(int layer);
    void onVisibilityChanged(bool visible);
    void onTextContentChanged();
    void onTextFontChanged();
    void onTextColorChanged();
    void onBackgroundColorChanged();
    void onTextAlignmentChanged();
    void onPenColorChanged();
    void onPenWidthChanged(int width);
    void onDrawingModeChanged();

private:
    std::shared_ptr<Page> m_page;
    std::shared_ptr<Object> m_selectedObject;
    
    // UI components
    QTreeWidget *m_objectTree;
    QPushButton *m_deleteButton;
    QPushButton *m_bringToFrontButton;
    QPushButton *m_sendToBackButton;
    QPushButton *m_bringForwardButton;
    QPushButton *m_sendBackwardButton;
    QPushButton *m_duplicateButton;
    
    // Property editors
    QGroupBox *m_positionGroup;
    QSpinBox *m_xSpinBox;
    QSpinBox *m_ySpinBox;
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
    
    QGroupBox *m_layerGroup;
    QSpinBox *m_layerSpinBox;
    QCheckBox *m_visibleCheckBox;
    
    QGroupBox *m_textGroup;
    QComboBox *m_fontComboBox;
    QSpinBox *m_fontSizeSpinBox;
    QPushButton *m_textColorButton;
    QPushButton *m_backgroundColorButton;
    QComboBox *m_alignmentComboBox;
    
    QGroupBox *m_drawingGroup;
    QPushButton *m_penColorButton;
    QSpinBox *m_penWidthSpinBox;
    QComboBox *m_drawingModeComboBox;
    
    // Layout
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_propertiesLayout;
    
    void setupUI();
    void setupObjectTree();
    void setupPropertyEditors();
    void setupConnections();
    
    void updateObjectTree();
    void updatePropertyEditors();
    void updateButtons();
    
    QTreeWidgetItem* createObjectItem(std::shared_ptr<Object> object);
    void updateObjectItem(QTreeWidgetItem *item, std::shared_ptr<Object> object);
    
    void showPropertyGroup(QGroupBox *group, bool show);
    void updatePropertyGroupVisibility();
    
    QColor getButtonColor(QPushButton *button) const;
    void setButtonColor(QPushButton *button, const QColor &color);
};

#endif // OBJECTSELECTOR_H
