#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include "object.h"
#include <QTextDocument>
#include <QTextEdit>
#include <QFont>
#include <QColor>

/**
 * @brief Text object that supports Markdown rendering and editing
 * 
 * This class provides a text object that can display and edit Markdown content
 * with WYSIWYG capabilities. It supports rich text formatting, links, images,
 * and other Markdown features.
 */
class TextObject : public Object
{
    Q_OBJECT

public:
    explicit TextObject(QObject *parent = nullptr);
    ~TextObject() override;

    // Object interface
    Type type() const override { return TextObject; }
    QString typeName() const override { return "Text"; }
    
    // Text content
    QString content() const { return m_content; }
    void setContent(const QString &content);
    
    // Markdown support
    QString markdownContent() const;
    void setMarkdownContent(const QString &markdown);
    bool isMarkdownMode() const { return m_markdownMode; }
    void setMarkdownMode(bool markdownMode);
    
    // Formatting
    QFont font() const { return m_font; }
    void setFont(const QFont &font);
    QColor textColor() const { return m_textColor; }
    void setTextColor(const QColor &color);
    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor &color);
    
    // Layout
    Qt::Alignment alignment() const { return m_alignment; }
    void setAlignment(Qt::Alignment alignment);
    int lineSpacing() const { return m_lineSpacing; }
    void setLineSpacing(int spacing);
    
    // Editing
    bool isEditing() const { return m_editing; }
    void startEditing();
    void stopEditing();
    void commitChanges();
    
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
    void contentChanged(const QString &newContent);
    void editingStateChanged(bool editing);

private slots:
    void onTextChanged();
    void onDocumentSizeChanged();

private:
    QString m_content;
    bool m_markdownMode;
    QFont m_font;
    QColor m_textColor;
    QColor m_backgroundColor;
    Qt::Alignment m_alignment;
    int m_lineSpacing;
    bool m_editing;
    
    std::unique_ptr<QTextDocument> m_document;
    std::unique_ptr<QTextEdit> m_textEdit;
    
    void setupDocument();
    void updateDocumentSize();
    void renderMarkdown();
    void renderPlainText();
    QRect getTextRect() const;
};

#endif // TEXTOBJECT_H
