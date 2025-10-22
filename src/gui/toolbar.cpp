#include "toolbar.h"
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QPushButton>
#include <QFontComboBox>
#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

Toolbar::Toolbar(QWidget *parent)
    : QToolBar("Drawing Tools", parent)
    , m_currentTool(SelectTool)
    , m_penColor(Qt::black)
    , m_backgroundColor(Qt::white)
    , m_penWidth(2)
    , m_textFont(QApplication::font())
    , m_textAlignment(Qt::AlignLeft | Qt::AlignTop)
{
    setupActions();
    setupUI();
    updateToolIcons();
    updateColorButtons();
    updateFontComboBoxes();
}

Toolbar::~Toolbar()
{
}

void Toolbar::setCurrentTool(Tool tool)
{
    if (m_currentTool != tool) {
        m_currentTool = tool;
        
        // Update action states
        switch (tool) {
        case SelectTool:
            m_selectAction->setChecked(true);
            break;
        case TextTool:
            m_textAction->setChecked(true);
            break;
        case PenTool:
            m_penAction->setChecked(true);
            break;
        case HighlighterTool:
            m_highlighterAction->setChecked(true);
            break;
        case EraserTool:
            m_eraserAction->setChecked(true);
            break;
        case ImageTool:
            m_imageAction->setChecked(true);
            break;
        case PDFTool:
            m_pdfAction->setChecked(true);
            break;
        }
        
        emit toolChanged(tool);
    }
}

void Toolbar::setPenColor(const QColor &color)
{
    if (m_penColor != color) {
        m_penColor = color;
        updateColorButtons();
        emit penColorChanged(color);
    }
}

void Toolbar::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        updateColorButtons();
        emit backgroundColorChanged(color);
    }
}

void Toolbar::setPenWidth(int width)
{
    if (m_penWidth != width) {
        m_penWidth = qMax(1, qMin(50, width)); // Clamp between 1 and 50
        m_penWidthSpinBox->setValue(m_penWidth);
        emit penWidthChanged(m_penWidth);
    }
}

void Toolbar::setTextFont(const QFont &font)
{
    if (m_textFont != font) {
        m_textFont = font;
        updateFontComboBoxes();
        emit textFontChanged(font);
    }
}

void Toolbar::setTextAlignment(Qt::Alignment alignment)
{
    if (m_textAlignment != alignment) {
        m_textAlignment = alignment;
        
        int index = 0;
        if (alignment & Qt::AlignLeft) {
            index = 0;
        } else if (alignment & Qt::AlignHCenter) {
            index = 1;
        } else if (alignment & Qt::AlignRight) {
            index = 2;
        }
        
        m_alignmentComboBox->setCurrentIndex(index);
        emit textAlignmentChanged(alignment);
    }
}

void Toolbar::setupActions()
{
    // Create action group for tools
    m_toolActionGroup = new QActionGroup(this);
    m_toolActionGroup->setExclusive(true);
    
    // Select tool
    m_selectAction = new QAction("Select", this);
    m_selectAction->setCheckable(true);
    m_selectAction->setChecked(true);
    m_selectAction->setShortcut(QKeySequence("S"));
    m_selectAction->setStatusTip("Select and move objects");
    m_toolActionGroup->addAction(m_selectAction);
    
    // Text tool
    m_textAction = new QAction("Text", this);
    m_textAction->setCheckable(true);
    m_textAction->setShortcut(QKeySequence("T"));
    m_textAction->setStatusTip("Add text objects");
    m_toolActionGroup->addAction(m_textAction);
    
    // Pen tool
    m_penAction = new QAction("Pen", this);
    m_penAction->setCheckable(true);
    m_penAction->setShortcut(QKeySequence("P"));
    m_penAction->setStatusTip("Draw with pen");
    m_toolActionGroup->addAction(m_penAction);
    
    // Highlighter tool
    m_highlighterAction = new QAction("Highlighter", this);
    m_highlighterAction->setCheckable(true);
    m_highlighterAction->setShortcut(QKeySequence("H"));
    m_highlighterAction->setStatusTip("Highlight text");
    m_toolActionGroup->addAction(m_highlighterAction);
    
    // Eraser tool
    m_eraserAction = new QAction("Eraser", this);
    m_eraserAction->setCheckable(true);
    m_eraserAction->setShortcut(QKeySequence("E"));
    m_eraserAction->setStatusTip("Erase drawings");
    m_toolActionGroup->addAction(m_eraserAction);
    
    // Image tool
    m_imageAction = new QAction("Image", this);
    m_imageAction->setCheckable(true);
    m_imageAction->setShortcut(QKeySequence("I"));
    m_imageAction->setStatusTip("Add image objects");
    m_toolActionGroup->addAction(m_imageAction);
    
    // PDF tool
    m_pdfAction = new QAction("PDF", this);
    m_pdfAction->setCheckable(true);
    m_pdfAction->setShortcut(QKeySequence("F"));
    m_pdfAction->setStatusTip("Add PDF objects");
    m_toolActionGroup->addAction(m_pdfAction);
    
    // Connect actions
    connect(m_toolActionGroup, &QActionGroup::triggered, this, &Toolbar::onToolActionTriggered);
}

void Toolbar::setupUI()
{
    // Add tool actions
    addAction(m_selectAction);
    addAction(m_textAction);
    addAction(m_penAction);
    addAction(m_highlighterAction);
    addAction(m_eraserAction);
    addAction(m_imageAction);
    addAction(m_pdfAction);
    
    addSeparator();
    
    // Color controls
    m_penColorButton = new QPushButton();
    m_penColorButton->setFixedSize(24, 24);
    m_penColorButton->setToolTip("Pen Color");
    addWidget(m_penColorButton);
    connect(m_penColorButton, &QPushButton::clicked, this, &Toolbar::onPenColorChanged);
    
    m_backgroundColorButton = new QPushButton();
    m_backgroundColorButton->setFixedSize(24, 24);
    m_backgroundColorButton->setToolTip("Background Color");
    addWidget(m_backgroundColorButton);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &Toolbar::onBackgroundColorChanged);
    
    addSeparator();
    
    // Pen width
    addWidget(new QLabel("Width:"));
    m_penWidthSpinBox = new QSpinBox();
    m_penWidthSpinBox->setRange(1, 50);
    m_penWidthSpinBox->setValue(m_penWidth);
    m_penWidthSpinBox->setSuffix(" px");
    m_penWidthSpinBox->setToolTip("Pen Width");
    addWidget(m_penWidthSpinBox);
    connect(m_penWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &Toolbar::onPenWidthChanged);
    
    addSeparator();
    
    // Font controls
    addWidget(new QLabel("Font:"));
    m_fontComboBox = new QFontComboBox();
    m_fontComboBox->setCurrentFont(m_textFont);
    m_fontComboBox->setToolTip("Font Family");
    addWidget(m_fontComboBox);
    connect(m_fontComboBox, &QFontComboBox::currentFontChanged, 
            this, &Toolbar::onTextFontChanged);
    
    m_fontSizeComboBox = new QComboBox();
    m_fontSizeComboBox->setEditable(true);
    m_fontSizeComboBox->setToolTip("Font Size");
    
    // Add common font sizes
    QStringList sizes = {"8", "9", "10", "11", "12", "14", "16", "18", "20", "24", "28", "32", "36", "48", "72"};
    m_fontSizeComboBox->addItems(sizes);
    m_fontSizeComboBox->setCurrentText(QString::number(m_textFont.pointSize()));
    addWidget(m_fontSizeComboBox);
    connect(m_fontSizeComboBox, &QComboBox::currentTextChanged, 
            this, &Toolbar::onTextFontChanged);
    
    addSeparator();
    
    // Text alignment
    addWidget(new QLabel("Align:"));
    m_alignmentComboBox = new QComboBox();
    m_alignmentComboBox->addItems({"Left", "Center", "Right"});
    m_alignmentComboBox->setCurrentIndex(0);
    m_alignmentComboBox->setToolTip("Text Alignment");
    addWidget(m_alignmentComboBox);
    connect(m_alignmentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &Toolbar::onTextAlignmentChanged);
}

void Toolbar::updateToolIcons()
{
    // Create simple icons for tools
    QPixmap selectIcon(16, 16);
    selectIcon.fill(Qt::transparent);
    QPainter painter(&selectIcon);
    painter.setPen(Qt::black);
    painter.drawRect(2, 2, 12, 12);
    m_selectAction->setIcon(QIcon(selectIcon));
    
    QPixmap textIcon(16, 16);
    textIcon.fill(Qt::transparent);
    painter.begin(&textIcon);
    painter.setPen(Qt::black);
    painter.drawText(2, 2, 12, 12, Qt::AlignCenter, "T");
    painter.end();
    m_textAction->setIcon(QIcon(textIcon));
    
    QPixmap penIcon(16, 16);
    penIcon.fill(Qt::transparent);
    painter.begin(&penIcon);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(2, 14, 14, 2);
    painter.end();
    m_penAction->setIcon(QIcon(penIcon));
    
    QPixmap highlighterIcon(16, 16);
    highlighterIcon.fill(Qt::transparent);
    painter.begin(&highlighterIcon);
    painter.setPen(QPen(QColor(255, 255, 0, 128), 4));
    painter.drawLine(2, 14, 14, 2);
    painter.end();
    m_highlighterAction->setIcon(QIcon(highlighterIcon));
    
    QPixmap eraserIcon(16, 16);
    eraserIcon.fill(Qt::transparent);
    painter.begin(&eraserIcon);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawRect(2, 2, 12, 12);
    painter.end();
    m_eraserAction->setIcon(QIcon(eraserIcon));
    
    QPixmap imageIcon(16, 16);
    imageIcon.fill(Qt::transparent);
    painter.begin(&imageIcon);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::lightGray);
    painter.drawRect(2, 2, 12, 12);
    painter.end();
    m_imageAction->setIcon(QIcon(imageIcon));
    
    QPixmap pdfIcon(16, 16);
    pdfIcon.fill(Qt::transparent);
    painter.begin(&pdfIcon);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::red);
    painter.drawRect(2, 2, 12, 12);
    painter.end();
    m_pdfAction->setIcon(QIcon(pdfIcon));
}

void Toolbar::updateColorButtons()
{
    // Update pen color button
    QPixmap penColorPixmap(22, 22);
    penColorPixmap.fill(m_penColor);
    QPainter painter(&penColorPixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 21, 21);
    m_penColorButton->setIcon(QIcon(penColorPixmap));
    
    // Update background color button
    QPixmap bgColorPixmap(22, 22);
    bgColorPixmap.fill(m_backgroundColor);
    painter.begin(&bgColorPixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 21, 21);
    painter.end();
    m_backgroundColorButton->setIcon(QIcon(bgColorPixmap));
}

void Toolbar::updateFontComboBoxes()
{
    m_fontComboBox->setCurrentFont(m_textFont);
    m_fontSizeComboBox->setCurrentText(QString::number(m_textFont.pointSize()));
}

void Toolbar::onToolActionTriggered(QAction *action)
{
    if (action == m_selectAction) {
        setCurrentTool(SelectTool);
    } else if (action == m_textAction) {
        setCurrentTool(TextTool);
    } else if (action == m_penAction) {
        setCurrentTool(PenTool);
    } else if (action == m_highlighterAction) {
        setCurrentTool(HighlighterTool);
    } else if (action == m_eraserAction) {
        setCurrentTool(EraserTool);
    } else if (action == m_imageAction) {
        setCurrentTool(ImageTool);
    } else if (action == m_pdfAction) {
        setCurrentTool(PDFTool);
    }
    
    emit actionTriggered(action);
}

void Toolbar::onPenColorChanged()
{
    QColor color = QColorDialog::getColor(m_penColor, this, "Select Pen Color");
    if (color.isValid()) {
        setPenColor(color);
    }
}

void Toolbar::onBackgroundColorChanged()
{
    QColor color = QColorDialog::getColor(m_backgroundColor, this, "Select Background Color");
    if (color.isValid()) {
        setBackgroundColor(color);
    }
}

void Toolbar::onPenWidthChanged(int width)
{
    setPenWidth(width);
}

void Toolbar::onTextFontChanged()
{
    QFont font = m_fontComboBox->currentFont();
    font.setPointSize(m_fontSizeComboBox->currentText().toInt());
    setTextFont(font);
}

void Toolbar::onTextAlignmentChanged()
{
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
    
    setTextAlignment(alignment);
}
