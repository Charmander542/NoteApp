#include "textobject.h"
#include <QTextDocument>
#include <QTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
#include <QClipboard>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QTextCharFormat>

TextObject::TextObject(QObject *parent)
    : Object(parent)
    , m_content("")
    , m_markdownMode(true)
    , m_font(QApplication::font())
    , m_textColor(Qt::black)
    , m_backgroundColor(Qt::transparent)
    , m_alignment(Qt::AlignLeft | Qt::AlignTop)
    , m_lineSpacing(0)
    , m_editing(false)
{
    setupDocument();
}

TextObject::~TextObject()
{
    if (m_textEdit) {
        m_textEdit->deleteLater();
    }
}

void TextObject::setContent(const QString &content)
{
    if (m_content != content) {
        m_content = content;
        setupDocument();
        emit contentChanged(m_content);
    }
}

QString TextObject::markdownContent() const
{
    if (m_markdownMode) {
        return m_content;
    }
    // Convert rich text to markdown (simplified)
    return m_content;
}

void TextObject::setMarkdownContent(const QString &markdown)
{
    if (m_markdownMode) {
        setContent(markdown);
    } else {
        // Convert markdown to rich text (simplified)
        setContent(markdown);
    }
}

void TextObject::setMarkdownMode(bool markdownMode)
{
    if (m_markdownMode != markdownMode) {
        m_markdownMode = markdownMode;
        setupDocument();
    }
}

void TextObject::setFont(const QFont &font)
{
    if (m_font != font) {
        m_font = font;
        setupDocument();
    }
}

void TextObject::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        setupDocument();
    }
}

void TextObject::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        setupDocument();
    }
}

void TextObject::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment != alignment) {
        m_alignment = alignment;
        setupDocument();
    }
}

void TextObject::setLineSpacing(int spacing)
{
    if (m_lineSpacing != spacing) {
        m_lineSpacing = spacing;
        setupDocument();
    }
}

void TextObject::startEditing()
{
    if (m_editing) return;
    
    m_editing = true;
    
    if (!m_textEdit) {
        m_textEdit = std::make_unique<QTextEdit>();
        m_textEdit->setParent(nullptr); // No parent to avoid layout issues
        m_textEdit->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
        connect(m_textEdit.get(), &QTextEdit::textChanged, this, &TextObject::onTextChanged);
    }
    
    m_textEdit->setPlainText(m_content);
    m_textEdit->setFont(m_font);
    m_textEdit->setGeometry(m_bounds);
    m_textEdit->show();
    m_textEdit->setFocus();
    
    emit editingStateChanged(true);
}

void TextObject::stopEditing()
{
    if (!m_editing) return;
    
    m_editing = false;
    
    if (m_textEdit) {
        m_textEdit->hide();
    }
    
    emit editingStateChanged(false);
}

void TextObject::commitChanges()
{
    if (m_textEdit) {
        setContent(m_textEdit->toPlainText());
    }
    stopEditing();
}

void TextObject::paint(QPainter &painter, const QRect &viewport)
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
    
    // Draw background
    if (m_backgroundColor.alpha() > 0) {
        painter.fillRect(m_bounds, m_backgroundColor);
    }
    
    // Draw text
    if (m_document) {
        painter.translate(m_bounds.topLeft());
        m_document->drawContents(&painter);
    }
    
    painter.restore();
    
    // Draw selection handles
    paintSelection(painter);
}

QJsonObject TextObject::toJson() const
{
    QJsonObject json = Object::toJson();
    json["content"] = m_content;
    json["markdownMode"] = m_markdownMode;
    json["font"] = QJsonObject{
        {"family", m_font.family()},
        {"size", m_font.pointSize()},
        {"bold", m_font.bold()},
        {"italic", m_font.italic()}
    };
    json["textColor"] = m_textColor.name();
    json["backgroundColor"] = m_backgroundColor.name();
    json["alignment"] = static_cast<int>(m_alignment);
    json["lineSpacing"] = m_lineSpacing;
    return json;
}

void TextObject::fromJson(const QJsonObject &json)
{
    Object::fromJson(json);
    
    m_content = json["content"].toString();
    m_markdownMode = json["markdownMode"].toBool();
    
    QJsonObject fontObj = json["font"].toObject();
    m_font = QFont(
        fontObj["family"].toString(),
        fontObj["size"].toInt(),
        fontObj["bold"].toBool() ? QFont::Bold : QFont::Normal
    );
    m_font.setItalic(fontObj["italic"].toBool());
    
    m_textColor = QColor(json["textColor"].toString());
    m_backgroundColor = QColor(json["backgroundColor"].toString());
    m_alignment = static_cast<Qt::Alignment>(json["alignment"].toInt());
    m_lineSpacing = json["lineSpacing"].toInt();
    
    setupDocument();
}

std::unique_ptr<Object> TextObject::clone() const
{
    auto clone = std::make_unique<TextObject>();
    clone->fromJson(this->toJson());
    return clone;
}

QJsonObject TextObject::getState() const
{
    return toJson();
}

void TextObject::setState(const QJsonObject &state)
{
    fromJson(state);
}

void TextObject::setupDocument()
{
    m_document = std::make_unique<QTextDocument>();
    m_document->setDefaultFont(m_font);
    m_document->setDefaultStyleSheet(QString(
        "body { color: %1; background-color: %2; }"
    ).arg(m_textColor.name(), m_backgroundColor.name()));
    
    if (m_markdownMode) {
        renderMarkdown();
    } else {
        renderPlainText();
    }
    
    updateDocumentSize();
}

void TextObject::updateDocumentSize()
{
    if (!m_document) return;
    
    // Set document size to match object bounds
    m_document->setTextWidth(m_bounds.width());
    
    // Adjust object height to fit content if needed
    int contentHeight = static_cast<int>(m_document->size().height());
    if (contentHeight != m_bounds.height()) {
        setSize(QSize(m_bounds.width(), contentHeight));
    }
}

void TextObject::renderMarkdown()
{
    if (!m_document) return;
    
    // Simple markdown rendering (can be enhanced with a proper markdown library)
    QString html = m_content;
    
    // Convert markdown to HTML (basic implementation)
    html.replace(QRegularExpression("^# (.+)$", QRegularExpression::MultilineOption), "<h1>\\1</h1>");
    html.replace(QRegularExpression("^## (.+)$", QRegularExpression::MultilineOption), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^### (.+)$", QRegularExpression::MultilineOption), "<h3>\\1</h3>");
    html.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
    html.replace(QRegularExpression("\\*(.+?)\\*"), "<i>\\1</i>");
    html.replace(QRegularExpression("`(.+?)`"), "<code>\\1</code>");
    html.replace(QRegularExpression("\\n"), "<br>");
    
    m_document->setHtml(html);
}

void TextObject::renderPlainText()
{
    if (!m_document) return;
    m_document->setPlainText(m_content);
}

QRect TextObject::getTextRect() const
{
    return m_bounds.adjusted(5, 5, -5, -5); // Add padding
}

void TextObject::onTextChanged()
{
    if (m_textEdit) {
        setContent(m_textEdit->toPlainText());
    }
}

void TextObject::onDocumentSizeChanged()
{
    updateDocumentSize();
}
