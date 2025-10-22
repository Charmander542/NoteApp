#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QPushButton>

/**
 * @brief Custom toolbar with drawing tools and properties
 * 
 * This toolbar provides quick access to drawing tools, colors, line widths,
 * and other object properties. It's designed to be context-sensitive and
 * update based on the currently selected tool or object.
 */
class Toolbar : public QToolBar
{
    Q_OBJECT

public:
    explicit Toolbar(QWidget *parent = nullptr);
    ~Toolbar() override;

    // Tool management
    enum Tool {
        SelectTool,
        TextTool,
        PenTool,
        HighlighterTool,
        EraserTool,
        ImageTool,
        PDFTool
    };
    
    Tool currentTool() const { return m_currentTool; }
    void setCurrentTool(Tool tool);
    
    // Drawing properties
    QColor penColor() const { return m_penColor; }
    void setPenColor(const QColor &color);
    
    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor &color);
    
    int penWidth() const { return m_penWidth; }
    void setPenWidth(int width);
    
    // Text properties
    QFont textFont() const { return m_textFont; }
    void setTextFont(const QFont &font);
    
    Qt::Alignment textAlignment() const { return m_textAlignment; }
    void setTextAlignment(Qt::Alignment alignment);
    
    // Actions
    QAction* selectAction() const { return m_selectAction; }
    QAction* textAction() const { return m_textAction; }
    QAction* penAction() const { return m_penAction; }
    QAction* highlighterAction() const { return m_highlighterAction; }
    QAction* eraserAction() const { return m_eraserAction; }
    QAction* imageAction() const { return m_imageAction; }
    QAction* pdfAction() const { return m_pdfAction; }

signals:
    void toolChanged(Tool tool);
    void penColorChanged(const QColor &color);
    void backgroundColorChanged(const QColor &color);
    void penWidthChanged(int width);
    void textFontChanged(const QFont &font);
    void textAlignmentChanged(Qt::Alignment alignment);
    void actionTriggered(QAction *action);

private slots:
    void onToolActionTriggered(QAction *action);
    void onPenColorChanged();
    void onBackgroundColorChanged();
    void onPenWidthChanged(int width);
    void onTextFontChanged();
    void onTextAlignmentChanged();

private:
    Tool m_currentTool;
    
    // Colors
    QColor m_penColor;
    QColor m_backgroundColor;
    
    // Drawing properties
    int m_penWidth;
    
    // Text properties
    QFont m_textFont;
    Qt::Alignment m_textAlignment;
    
    // Actions
    QAction *m_selectAction;
    QAction *m_textAction;
    QAction *m_penAction;
    QAction *m_highlighterAction;
    QAction *m_eraserAction;
    QAction *m_imageAction;
    QAction *m_pdfAction;
    
    // Action group
    QActionGroup *m_toolActionGroup;
    
    // UI widgets
    QPushButton *m_penColorButton;
    QPushButton *m_backgroundColorButton;
    QSpinBox *m_penWidthSpinBox;
    QComboBox *m_fontComboBox;
    QComboBox *m_fontSizeComboBox;
    QComboBox *m_alignmentComboBox;
    
    void setupActions();
    void setupUI();
    void updateToolIcons();
    void updateColorButtons();
    void updateFontComboBoxes();
};

#endif // TOOLBAR_H
