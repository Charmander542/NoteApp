#include "objectselector.h"
#include "../core/page.h"
#include "../core/object.h"
#include "../core/textobject.h"
#include "../core/drawingobject.h"
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
#include <QFontComboBox>
#include <QColorDialog>
#include <QMessageBox>
#include <QApplication>

ObjectSelector::ObjectSelector(QWidget *parent)
    : QDockWidget("Object Properties", parent)
{
    setupUI();
    setupConnections();
}

ObjectSelector::~ObjectSelector()
{
}

void ObjectSelector::setPage(std::shared_ptr<Page> page)
{
    if (m_page == page) return;
    
    m_page = page;
    m_selectedObject.reset();
    
    updateObjectTree();
    updatePropertyEditors();
    updateButtons();
}

void ObjectSelector::setSelectedObject(std::shared_ptr<Object> object)
{
    if (m_selectedObject == object) return;
    
    m_selectedObject = object;
    
    // Update tree selection
    if (object) {
        for (int i = 0; i < m_objectTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = m_objectTree->topLevelItem(i);
            if (item->data(0, Qt::UserRole).value<std::shared_ptr<Object>>() == object) {
                m_objectTree->setCurrentItem(item);
                break;
            }
        }
    } else {
        m_objectTree->clearSelection();
    }
    
    updatePropertyEditors();
    updateButtons();
}

void ObjectSelector::clearSelection()
{
    setSelectedObject(nullptr);
}

void ObjectSelector::addObject(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    QTreeWidgetItem *item = createObjectItem(object);
    m_objectTree->addTopLevelItem(item);
    
    // Connect object signals
    connect(object.get(), &Object::boundsChanged, this, [this, object](const QRect &bounds) {
        Q_UNUSED(bounds)
        updateObject(object);
    });
    connect(object.get(), &Object::selectionChanged, this, [this, object](bool selected) {
        if (selected) {
            setSelectedObject(object);
        }
    });
    connect(object.get(), &Object::layerChanged, this, [this, object](int layer) {
        Q_UNUSED(layer)
        updateObject(object);
    });
    connect(object.get(), &Object::visibilityChanged, this, [this, object](bool visible) {
        Q_UNUSED(visible)
        updateObject(object);
    });
}

void ObjectSelector::removeObject(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    // Find and remove the item
    for (int i = 0; i < m_objectTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_objectTree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).value<std::shared_ptr<Object>>() == object) {
            delete item;
            break;
        }
    }
    
    // Clear selection if this was the selected object
    if (m_selectedObject == object) {
        clearSelection();
    }
}

void ObjectSelector::updateObject(std::shared_ptr<Object> object)
{
    if (!object) return;
    
    // Find and update the item
    for (int i = 0; i < m_objectTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_objectTree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).value<std::shared_ptr<Object>>() == object) {
            updateObjectItem(item, object);
            break;
        }
    }
    
    // Update property editors if this is the selected object
    if (m_selectedObject == object) {
        updatePropertyEditors();
    }
}

void ObjectSelector::setupUI()
{
    QWidget *mainWidget = new QWidget();
    setWidget(mainWidget);
    
    m_mainLayout = new QVBoxLayout(mainWidget);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    
    setupObjectTree();
    setupPropertyEditors();
}

void ObjectSelector::setupObjectTree()
{
    m_objectTree = new QTreeWidget();
    m_objectTree->setHeaderLabels({"Object", "Type"});
    m_objectTree->setRootIsDecorated(false);
    m_objectTree->setAlternatingRowColors(true);
    m_mainLayout->addWidget(m_objectTree);
    
    // Object operation buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_deleteButton = new QPushButton("Delete");
    m_bringToFrontButton = new QPushButton("Front");
    m_sendToBackButton = new QPushButton("Back");
    m_bringForwardButton = new QPushButton("Forward");
    m_sendBackwardButton = new QPushButton("Backward");
    m_duplicateButton = new QPushButton("Duplicate");
    
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_bringToFrontButton);
    buttonLayout->addWidget(m_sendToBackButton);
    buttonLayout->addWidget(m_bringForwardButton);
    buttonLayout->addWidget(m_sendBackwardButton);
    buttonLayout->addWidget(m_duplicateButton);
    
    m_mainLayout->addLayout(buttonLayout);
}

void ObjectSelector::setupPropertyEditors()
{
    m_propertiesLayout = new QVBoxLayout();
    m_mainLayout->addLayout(m_propertiesLayout);
    
    // Position group
    m_positionGroup = new QGroupBox("Position & Size");
    QGridLayout *positionLayout = new QGridLayout(m_positionGroup);
    
    positionLayout->addWidget(new QLabel("X:"), 0, 0);
    m_xSpinBox = new QSpinBox();
    m_xSpinBox->setRange(-10000, 10000);
    positionLayout->addWidget(m_xSpinBox, 0, 1);
    
    positionLayout->addWidget(new QLabel("Y:"), 0, 2);
    m_ySpinBox = new QSpinBox();
    m_ySpinBox->setRange(-10000, 10000);
    positionLayout->addWidget(m_ySpinBox, 0, 3);
    
    positionLayout->addWidget(new QLabel("Width:"), 1, 0);
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setRange(1, 10000);
    positionLayout->addWidget(m_widthSpinBox, 1, 1);
    
    positionLayout->addWidget(new QLabel("Height:"), 1, 2);
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setRange(1, 10000);
    positionLayout->addWidget(m_heightSpinBox, 1, 3);
    
    m_propertiesLayout->addWidget(m_positionGroup);
    
    // Layer group
    m_layerGroup = new QGroupBox("Layer");
    QHBoxLayout *layerLayout = new QHBoxLayout(m_layerGroup);
    
    layerLayout->addWidget(new QLabel("Layer:"));
    m_layerSpinBox = new QSpinBox();
    m_layerSpinBox->setRange(0, 100);
    layerLayout->addWidget(m_layerSpinBox);
    
    m_visibleCheckBox = new QCheckBox("Visible");
    m_visibleCheckBox->setChecked(true);
    layerLayout->addWidget(m_visibleCheckBox);
    
    m_propertiesLayout->addWidget(m_layerGroup);
    
    // Text group
    m_textGroup = new QGroupBox("Text Properties");
    QGridLayout *textLayout = new QGridLayout(m_textGroup);
    
    textLayout->addWidget(new QLabel("Font:"), 0, 0);
    m_fontComboBox = new QFontComboBox();
    textLayout->addWidget(m_fontComboBox, 0, 1, 1, 2);
    
    textLayout->addWidget(new QLabel("Size:"), 1, 0);
    m_fontSizeSpinBox = new QSpinBox();
    m_fontSizeSpinBox->setRange(6, 72);
    m_fontSizeSpinBox->setValue(12);
    textLayout->addWidget(m_fontSizeSpinBox, 1, 1);
    
    textLayout->addWidget(new QLabel("Color:"), 1, 2);
    m_textColorButton = new QPushButton();
    m_textColorButton->setFixedSize(24, 24);
    textLayout->addWidget(m_textColorButton, 1, 3);
    
    textLayout->addWidget(new QLabel("Background:"), 2, 0);
    m_backgroundColorButton = new QPushButton();
    m_backgroundColorButton->setFixedSize(24, 24);
    textLayout->addWidget(m_backgroundColorButton, 2, 1);
    
    textLayout->addWidget(new QLabel("Align:"), 2, 2);
    m_alignmentComboBox = new QComboBox();
    m_alignmentComboBox->addItems({"Left", "Center", "Right"});
    textLayout->addWidget(m_alignmentComboBox, 2, 3);
    
    m_propertiesLayout->addWidget(m_textGroup);
    
    // Drawing group
    m_drawingGroup = new QGroupBox("Drawing Properties");
    QGridLayout *drawingLayout = new QGridLayout(m_drawingGroup);
    
    drawingLayout->addWidget(new QLabel("Color:"), 0, 0);
    m_penColorButton = new QPushButton();
    m_penColorButton->setFixedSize(24, 24);
    drawingLayout->addWidget(m_penColorButton, 0, 1);
    
    drawingLayout->addWidget(new QLabel("Width:"), 0, 2);
    m_penWidthSpinBox = new QSpinBox();
    m_penWidthSpinBox->setRange(1, 50);
    m_penWidthSpinBox->setValue(2);
    drawingLayout->addWidget(m_penWidthSpinBox, 0, 3);
    
    drawingLayout->addWidget(new QLabel("Mode:"), 1, 0);
    m_drawingModeComboBox = new QComboBox();
    m_drawingModeComboBox->addItems({"Pen", "Highlighter", "Eraser"});
    drawingLayout->addWidget(m_drawingModeComboBox, 1, 1, 1, 2);
    
    m_propertiesLayout->addWidget(m_drawingGroup);
    
    // Initially hide property groups
    m_textGroup->setVisible(false);
    m_drawingGroup->setVisible(false);
}

void ObjectSelector::setupConnections()
{
    // Object tree connections
    connect(m_objectTree, &QTreeWidget::itemClicked, this, &ObjectSelector::onObjectTreeItemClicked);
    connect(m_objectTree, &QTreeWidget::itemChanged, this, &ObjectSelector::onObjectTreeItemChanged);
    
    // Button connections
    connect(m_deleteButton, &QPushButton::clicked, this, &ObjectSelector::onDeleteObjectClicked);
    connect(m_bringToFrontButton, &QPushButton::clicked, this, &ObjectSelector::onBringToFrontClicked);
    connect(m_sendToBackButton, &QPushButton::clicked, this, &ObjectSelector::onSendToBackClicked);
    connect(m_bringForwardButton, &QPushButton::clicked, this, &ObjectSelector::onBringForwardClicked);
    connect(m_sendBackwardButton, &QPushButton::clicked, this, &ObjectSelector::onSendBackwardClicked);
    connect(m_duplicateButton, &QPushButton::clicked, this, &ObjectSelector::onDuplicateObjectClicked);
    
    // Property connections
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectSelector::onPositionChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectSelector::onPositionChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectSelector::onSizeChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectSelector::onSizeChanged);
    connect(m_layerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectSelector::onLayerChanged);
    connect(m_visibleCheckBox, &QCheckBox::toggled, this, &ObjectSelector::onVisibilityChanged);
    
    connect(m_fontComboBox, &QFontComboBox::currentFontChanged, this, &ObjectSelector::onTextFontChanged);
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectSelector::onTextFontChanged);
    connect(m_textColorButton, &QPushButton::clicked, this, &ObjectSelector::onTextColorChanged);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &ObjectSelector::onBackgroundColorChanged);
    connect(m_alignmentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ObjectSelector::onTextAlignmentChanged);
    
    connect(m_penColorButton, &QPushButton::clicked, this, &ObjectSelector::onPenColorChanged);
    connect(m_penWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectSelector::onPenWidthChanged);
    connect(m_drawingModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ObjectSelector::onDrawingModeChanged);
}

void ObjectSelector::updateObjectTree()
{
    m_objectTree->clear();
    
    if (!m_page) return;
    
    for (const auto &object : m_page->objects()) {
        addObject(object);
    }
}

void ObjectSelector::updatePropertyEditors()
{
    if (!m_selectedObject) {
        // Disable all property editors
        m_positionGroup->setEnabled(false);
        m_layerGroup->setEnabled(false);
        m_textGroup->setEnabled(false);
        m_drawingGroup->setEnabled(false);
        return;
    }
    
    // Enable position and layer groups
    m_positionGroup->setEnabled(true);
    m_layerGroup->setEnabled(true);
    
    // Update position and size
    QRect bounds = m_selectedObject->bounds();
    m_xSpinBox->setValue(bounds.x());
    m_ySpinBox->setValue(bounds.y());
    m_widthSpinBox->setValue(bounds.width());
    m_heightSpinBox->setValue(bounds.height());
    
    // Update layer
    m_layerSpinBox->setValue(m_selectedObject->layer());
    m_visibleCheckBox->setChecked(m_selectedObject->isVisible());
    
    // Update type-specific properties
    updatePropertyGroupVisibility();
    
    if (m_selectedObject->type() == Object::TextObject) {
        auto textObject = std::dynamic_pointer_cast<TextObject>(m_selectedObject);
        if (textObject) {
            m_fontComboBox->setCurrentFont(textObject->font());
            m_fontSizeSpinBox->setValue(textObject->font().pointSize());
            setButtonColor(m_textColorButton, textObject->textColor());
            setButtonColor(m_backgroundColorButton, textObject->backgroundColor());
            
            Qt::Alignment alignment = textObject->alignment();
            if (alignment & Qt::AlignLeft) {
                m_alignmentComboBox->setCurrentIndex(0);
            } else if (alignment & Qt::AlignHCenter) {
                m_alignmentComboBox->setCurrentIndex(1);
            } else if (alignment & Qt::AlignRight) {
                m_alignmentComboBox->setCurrentIndex(2);
            }
        }
    } else if (m_selectedObject->type() == Object::DrawingObject) {
        auto drawingObject = std::dynamic_pointer_cast<DrawingObject>(m_selectedObject);
        if (drawingObject) {
            setButtonColor(m_penColorButton, drawingObject->currentPen().color());
            m_penWidthSpinBox->setValue(drawingObject->currentPen().width());
            m_drawingModeComboBox->setCurrentIndex(static_cast<int>(drawingObject->currentMode()));
        }
    }
}

void ObjectSelector::updateButtons()
{
    bool hasSelection = m_selectedObject != nullptr;
    
    m_deleteButton->setEnabled(hasSelection);
    m_bringToFrontButton->setEnabled(hasSelection);
    m_sendToBackButton->setEnabled(hasSelection);
    m_bringForwardButton->setEnabled(hasSelection);
    m_sendBackwardButton->setEnabled(hasSelection);
    m_duplicateButton->setEnabled(hasSelection);
}

QTreeWidgetItem* ObjectSelector::createObjectItem(std::shared_ptr<Object> object)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setData(0, Qt::UserRole, QVariant::fromValue(object));
    updateObjectItem(item, object);
    return item;
}

void ObjectSelector::updateObjectItem(QTreeWidgetItem *item, std::shared_ptr<Object> object)
{
    item->setText(0, object->typeName());
    item->setText(1, QString("Layer %1").arg(object->layer()));
    
    // Set item appearance based on selection and visibility
    if (object->isSelected()) {
        item->setBackground(0, QColor(200, 200, 255));
        item->setBackground(1, QColor(200, 200, 255));
    } else {
        item->setBackground(0, QColor());
        item->setBackground(1, QColor());
    }
    
    if (!object->isVisible()) {
        item->setForeground(0, QColor(150, 150, 150));
        item->setForeground(1, QColor(150, 150, 150));
    } else {
        item->setForeground(0, QColor());
        item->setForeground(1, QColor());
    }
}

void ObjectSelector::showPropertyGroup(QGroupBox *group, bool show)
{
    group->setVisible(show);
    group->setEnabled(show);
}

void ObjectSelector::updatePropertyGroupVisibility()
{
    if (!m_selectedObject) {
        m_textGroup->setVisible(false);
        m_drawingGroup->setVisible(false);
        return;
    }
    
    switch (m_selectedObject->type()) {
    case Object::TextObject:
        m_textGroup->setVisible(true);
        m_drawingGroup->setVisible(false);
        break;
    case Object::DrawingObject:
        m_textGroup->setVisible(false);
        m_drawingGroup->setVisible(true);
        break;
    default:
        m_textGroup->setVisible(false);
        m_drawingGroup->setVisible(false);
        break;
    }
}

QColor ObjectSelector::getButtonColor(QPushButton *button) const
{
    QPixmap pixmap = button->icon().pixmap(16, 16);
    if (pixmap.isNull()) return Qt::black;
    
    QImage image = pixmap.toImage();
    return image.pixel(8, 8);
}

void ObjectSelector::setButtonColor(QPushButton *button, const QColor &color)
{
    QPixmap pixmap(22, 22);
    pixmap.fill(color);
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 21, 21);
    button->setIcon(QIcon(pixmap));
}

// Slot implementations
void ObjectSelector::onObjectTreeItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    
    if (!item) return;
    
    std::shared_ptr<Object> object = item->data(0, Qt::UserRole).value<std::shared_ptr<Object>>();
    if (object) {
        setSelectedObject(object);
        emit objectSelected(object);
    }
}

void ObjectSelector::onObjectTreeItemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(item)
    Q_UNUSED(column)
    // Handle item text changes if needed
}

void ObjectSelector::onDeleteObjectClicked()
{
    if (!m_selectedObject || !m_page) return;
    
    int ret = QMessageBox::question(this, "Delete Object", 
        "Are you sure you want to delete this object?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        emit objectDeleted(m_selectedObject);
        m_page->removeObject(m_selectedObject);
    }
}

void ObjectSelector::onBringToFrontClicked()
{
    if (!m_selectedObject || !m_page) return;
    
    m_page->bringToFront(m_selectedObject);
    updateObjectTree();
}

void ObjectSelector::onSendToBackClicked()
{
    if (!m_selectedObject || !m_page) return;
    
    m_page->sendToBack(m_selectedObject);
    updateObjectTree();
}

void ObjectSelector::onBringForwardClicked()
{
    if (!m_selectedObject || !m_page) return;
    
    m_page->bringForward(m_selectedObject);
    updateObjectTree();
}

void ObjectSelector::onSendBackwardClicked()
{
    if (!m_selectedObject || !m_page) return;
    
    m_page->sendBackward(m_selectedObject);
    updateObjectTree();
}

void ObjectSelector::onDuplicateObjectClicked()
{
    if (!m_selectedObject || !m_page) return;
    
    auto clone = m_selectedObject->clone();
    clone->moveBy(QPoint(20, 20)); // Offset the duplicate
    m_page->addObject(std::shared_ptr<Object>(clone.release()));
}

void ObjectSelector::onPositionChanged()
{
    if (!m_selectedObject) return;
    
    QPoint position(m_xSpinBox->value(), m_ySpinBox->value());
    m_selectedObject->setPosition(position);
    emit objectPropertyChanged(m_selectedObject, "position", position);
}

void ObjectSelector::onSizeChanged()
{
    if (!m_selectedObject) return;
    
    QSize size(m_widthSpinBox->value(), m_heightSpinBox->value());
    m_selectedObject->setSize(size);
    emit objectPropertyChanged(m_selectedObject, "size", size);
}

void ObjectSelector::onLayerChanged(int layer)
{
    if (!m_selectedObject) return;
    
    m_selectedObject->setLayer(layer);
    emit objectPropertyChanged(m_selectedObject, "layer", layer);
    updateObjectTree();
}

void ObjectSelector::onVisibilityChanged(bool visible)
{
    if (!m_selectedObject) return;
    
    m_selectedObject->setVisible(visible);
    emit objectPropertyChanged(m_selectedObject, "visible", visible);
    updateObjectTree();
}

void ObjectSelector::onTextContentChanged()
{
    // This would be connected to a text editor widget
    // For now, it's a placeholder
}

void ObjectSelector::onTextFontChanged()
{
    if (!m_selectedObject || m_selectedObject->type() != Object::TextObject) return;
    
    auto textObject = std::dynamic_pointer_cast<TextObject>(m_selectedObject);
    if (textObject) {
        QFont font = m_fontComboBox->currentFont();
        font.setPointSize(m_fontSizeSpinBox->value());
        textObject->setFont(font);
        emit objectPropertyChanged(m_selectedObject, "font", font);
    }
}

void ObjectSelector::onTextColorChanged()
{
    if (!m_selectedObject || m_selectedObject->type() != Object::TextObject) return;
    
    auto textObject = std::dynamic_pointer_cast<TextObject>(m_selectedObject);
    if (textObject) {
        QColor color = QColorDialog::getColor(textObject->textColor(), this, "Select Text Color");
        if (color.isValid()) {
            textObject->setTextColor(color);
            setButtonColor(m_textColorButton, color);
            emit objectPropertyChanged(m_selectedObject, "textColor", color);
        }
    }
}

void ObjectSelector::onBackgroundColorChanged()
{
    if (!m_selectedObject || m_selectedObject->type() != Object::TextObject) return;
    
    auto textObject = std::dynamic_pointer_cast<TextObject>(m_selectedObject);
    if (textObject) {
        QColor color = QColorDialog::getColor(textObject->backgroundColor(), this, "Select Background Color");
        if (color.isValid()) {
            textObject->setBackgroundColor(color);
            setButtonColor(m_backgroundColorButton, color);
            emit objectPropertyChanged(m_selectedObject, "backgroundColor", color);
        }
    }
}

void ObjectSelector::onTextAlignmentChanged()
{
    if (!m_selectedObject || m_selectedObject->type() != Object::TextObject) return;
    
    auto textObject = std::dynamic_pointer_cast<TextObject>(m_selectedObject);
    if (textObject) {
        Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignTop;
        
        switch (m_alignmentComboBox->currentIndex()) {
        case 0: // Left
            alignment = Qt::AlignLeft | Qt::AlignTop;
            break;
        case 1: // Center
            alignment = Qt::AlignHCenter | Qt::AlignTop;
            break;
        case 2: // Right
            alignment = Qt::AlignRight | Qt::AlignTop;
            break;
        }
        
        textObject->setAlignment(alignment);
        emit objectPropertyChanged(m_selectedObject, "alignment", static_cast<int>(alignment));
    }
}

void ObjectSelector::onPenColorChanged()
{
    if (!m_selectedObject || m_selectedObject->type() != Object::DrawingObject) return;
    
    auto drawingObject = std::dynamic_pointer_cast<DrawingObject>(m_selectedObject);
    if (drawingObject) {
        QColor color = QColorDialog::getColor(drawingObject->currentPen().color(), this, "Select Pen Color");
        if (color.isValid()) {
            QPen pen = drawingObject->currentPen();
            pen.setColor(color);
            drawingObject->setCurrentPen(pen);
            setButtonColor(m_penColorButton, color);
            emit objectPropertyChanged(m_selectedObject, "penColor", color);
        }
    }
}

void ObjectSelector::onPenWidthChanged(int width)
{
    if (!m_selectedObject || m_selectedObject->type() != Object::DrawingObject) return;
    
    auto drawingObject = std::dynamic_pointer_cast<DrawingObject>(m_selectedObject);
    if (drawingObject) {
        QPen pen = drawingObject->currentPen();
        pen.setWidth(width);
        drawingObject->setCurrentPen(pen);
        emit objectPropertyChanged(m_selectedObject, "penWidth", width);
    }
}

void ObjectSelector::onDrawingModeChanged()
{
    if (!m_selectedObject || m_selectedObject->type() != Object::DrawingObject) return;
    
    auto drawingObject = std::dynamic_pointer_cast<DrawingObject>(m_selectedObject);
    if (drawingObject) {
        DrawingObject::DrawingMode mode = static_cast<DrawingObject::DrawingMode>(m_drawingModeComboBox->currentIndex());
        drawingObject->setCurrentMode(mode);
        emit objectPropertyChanged(m_selectedObject, "drawingMode", static_cast<int>(mode));
    }
}
