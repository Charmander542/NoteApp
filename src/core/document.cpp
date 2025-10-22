#include "document.h"
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QRegularExpression>
#include <algorithm>

Document::Document(QObject *parent)
    : QObject(parent)
    , m_title("Untitled Document")
    , m_createdDate(QDateTime::currentDateTime())
    , m_modifiedDate(QDateTime::currentDateTime())
    , m_modified(false)
{
    generateId();
    // Create a default page
    createNewPage("Page 1");
}

Document::Document(const QString &title, QObject *parent)
    : QObject(parent)
    , m_title(title)
    , m_createdDate(QDateTime::currentDateTime())
    , m_modifiedDate(QDateTime::currentDateTime())
    , m_modified(false)
{
    generateId();
    // Create a default page
    createNewPage("Page 1");
}

Document::~Document()
{
    // Disconnect all page signals
    for (auto &page : m_pages) {
        disconnectPageSignals(page);
    }
}

void Document::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        markAsModified();
        emit titleChanged(m_title);
    }
}

void Document::setId(const QString &id)
{
    m_id = id;
}

void Document::setDescription(const QString &description)
{
    if (m_description != description) {
        m_description = description;
        markAsModified();
        emit descriptionChanged(m_description);
    }
}

void Document::addPage(std::shared_ptr<Page> page)
{
    if (!page) return;
    
    m_pages.append(page);
    connectPageSignals(page);
    
    if (!m_currentPage) {
        setCurrentPage(page);
    }
    
    markAsModified();
    emit pageAdded(page, m_pages.size() - 1);
}

void Document::insertPage(int index, std::shared_ptr<Page> page)
{
    if (!page) return;
    
    index = qBound(0, index, m_pages.size());
    m_pages.insert(index, page);
    connectPageSignals(page);
    
    if (!m_currentPage) {
        setCurrentPage(page);
    }
    
    markAsModified();
    emit pageAdded(page, index);
}

void Document::removePage(std::shared_ptr<Page> page)
{
    if (!page) return;
    
    int index = m_pages.indexOf(page);
    if (index >= 0) {
        disconnectPageSignals(page);
        m_pages.removeAt(index);
        
        if (m_currentPage == page) {
            if (m_pages.isEmpty()) {
                setCurrentPage(nullptr);
            } else {
                setCurrentPage(qMax(0, index - 1));
            }
        }
        
        markAsModified();
        emit pageRemoved(page, index);
    }
}

void Document::removePage(int index)
{
    if (index >= 0 && index < m_pages.size()) {
        auto page = m_pages[index];
        removePage(page);
    }
}

void Document::clearPages()
{
    for (auto &page : m_pages) {
        disconnectPageSignals(page);
    }
    m_pages.clear();
    setCurrentPage(nullptr);
    markAsModified();
}

std::shared_ptr<Page> Document::pageAt(int index) const
{
    if (index >= 0 && index < m_pages.size()) {
        return m_pages[index];
    }
    return nullptr;
}

std::shared_ptr<Page> Document::pageById(const QString &id) const
{
    for (const auto &page : m_pages) {
        if (page->id() == id) {
            return page;
        }
    }
    return nullptr;
}

int Document::pageIndex(std::shared_ptr<Page> page) const
{
    return m_pages.indexOf(page);
}

void Document::movePage(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < m_pages.size() && 
        toIndex >= 0 && toIndex < m_pages.size() && 
        fromIndex != toIndex) {
        
        auto page = m_pages[fromIndex];
        m_pages.move(fromIndex, toIndex);
        markAsModified();
        emit pageMoved(page, fromIndex, toIndex);
    }
}

void Document::duplicatePage(int index)
{
    if (index >= 0 && index < m_pages.size()) {
        auto originalPage = m_pages[index];
        auto clonedPage = originalPage->clone();
        clonedPage->setTitle(originalPage->title() + " (Copy)");
        
        insertPage(index + 1, std::shared_ptr<Page>(clonedPage.release()));
    }
}

std::shared_ptr<Page> Document::createNewPage(const QString &title)
{
    auto page = std::make_shared<Page>(title.isEmpty() ? "Untitled Page" : title);
    addPage(page);
    return page;
}

void Document::setCurrentPage(std::shared_ptr<Page> page)
{
    if (m_currentPage != page) {
        m_currentPage = page;
        emit currentPageChanged(m_currentPage);
    }
}

void Document::setCurrentPage(int index)
{
    setCurrentPage(pageAt(index));
}

void Document::setTags(const QStringList &tags)
{
    if (m_tags != tags) {
        m_tags = tags;
        markAsModified();
        emit tagsChanged(m_tags);
    }
}

void Document::addTag(const QString &tag)
{
    if (!m_tags.contains(tag)) {
        m_tags.append(tag);
        markAsModified();
        emit tagsChanged(m_tags);
    }
}

void Document::removeTag(const QString &tag)
{
    if (m_tags.removeAll(tag) > 0) {
        markAsModified();
        emit tagsChanged(m_tags);
    }
}

bool Document::hasTag(const QString &tag) const
{
    return m_tags.contains(tag);
}

QVector<std::shared_ptr<Page>> Document::searchPages(const QString &query) const
{
    QVector<std::shared_ptr<Page>> result;
    QRegularExpression regex(query, QRegularExpression::CaseInsensitiveOption);
    
    for (const auto &page : m_pages) {
        if (page->title().contains(regex)) {
            result.append(page);
        } else {
            // Search in page objects
            auto objects = page->findObjectsContaining(query);
            if (!objects.isEmpty()) {
                result.append(page);
            }
        }
    }
    
    return result;
}

QVector<std::shared_ptr<Object>> Document::searchObjects(const QString &query) const
{
    QVector<std::shared_ptr<Object>> result;
    
    for (const auto &page : m_pages) {
        auto objects = page->findObjectsContaining(query);
        result.append(objects);
    }
    
    return result;
}

QVector<std::shared_ptr<Page>> Document::findPagesByTag(const QString &tag) const
{
    QVector<std::shared_ptr<Page>> result;
    
    for (const auto &page : m_pages) {
        // For now, we'll search in page titles and content
        // In a full implementation, pages could have their own tags
        if (page->title().contains(tag, Qt::CaseInsensitive)) {
            result.append(page);
        }
    }
    
    return result;
}

QStringList Document::getBacklinks(const QString &pageId) const
{
    QStringList result;
    
    for (auto it = m_links.begin(); it != m_links.end(); ++it) {
        if (it.value().contains(pageId)) {
            result.append(it.key());
        }
    }
    
    return result;
}

void Document::addLink(const QString &fromPageId, const QString &toPageId)
{
    if (!m_links[fromPageId].contains(toPageId)) {
        m_links[fromPageId].append(toPageId);
        markAsModified();
    }
}

void Document::removeLink(const QString &fromPageId, const QString &toPageId)
{
    if (m_links[fromPageId].removeAll(toPageId) > 0) {
        markAsModified();
    }
}

QJsonObject Document::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["title"] = m_title;
    json["description"] = m_description;
    json["createdDate"] = m_createdDate.toString(Qt::ISODate);
    json["modifiedDate"] = m_modifiedDate.toString(Qt::ISODate);
    json["tags"] = QJsonArray::fromStringList(m_tags);
    
    QJsonArray pagesArray;
    for (const auto &page : m_pages) {
        pagesArray.append(page->toJson());
    }
    json["pages"] = pagesArray;
    
    // Save links
    QJsonObject linksObj;
    for (auto it = m_links.begin(); it != m_links.end(); ++it) {
        linksObj[it.key()] = QJsonArray::fromStringList(it.value());
    }
    json["links"] = linksObj;
    
    return json;
}

void Document::fromJson(const QJsonObject &json)
{
    m_id = json["id"].toString();
    m_title = json["title"].toString();
    m_description = json["description"].toString();
    m_createdDate = QDateTime::fromString(json["createdDate"].toString(), Qt::ISODate);
    m_modifiedDate = QDateTime::fromString(json["modifiedDate"].toString(), Qt::ISODate);
    
    // Load tags
    QJsonArray tagsArray = json["tags"].toArray();
    m_tags.clear();
    for (const QJsonValue &value : tagsArray) {
        m_tags.append(value.toString());
    }
    
    // Clear existing pages
    clearPages();
    
    // Load pages
    QJsonArray pagesArray = json["pages"].toArray();
    for (const QJsonValue &value : pagesArray) {
        auto page = std::make_shared<Page>();
        page->fromJson(value.toObject());
        addPage(page);
    }
    
    // Load links
    m_links.clear();
    QJsonObject linksObj = json["links"].toObject();
    for (auto it = linksObj.begin(); it != linksObj.end(); ++it) {
        QStringList links;
        QJsonArray linksArray = it.value().toArray();
        for (const QJsonValue &value : linksArray) {
            links.append(value.toString());
        }
        m_links[it.key()] = links;
    }
    
    m_modified = false;
}

std::unique_ptr<Document> Document::clone() const
{
    auto clone = std::make_unique<Document>();
    clone->fromJson(this->toJson());
    return clone;
}

QJsonObject Document::getState() const
{
    return toJson();
}

void Document::setState(const QJsonObject &state)
{
    fromJson(state);
}

void Document::setModified(bool modified)
{
    if (m_modified != modified) {
        m_modified = modified;
        emit modifiedChanged(m_modified);
    }
}

void Document::markAsModified()
{
    updateModifiedDate();
    setModified(true);
}

void Document::generateId()
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Document::connectPageSignals(std::shared_ptr<Page> page)
{
    connect(page.get(), &Page::titleChanged, this, &Document::onPageTitleChanged);
    connect(page.get(), &Page::objectAdded, this, &Document::onPageObjectAdded);
    connect(page.get(), &Page::objectRemoved, this, &Document::onPageObjectRemoved);
}

void Document::disconnectPageSignals(std::shared_ptr<Page> page)
{
    disconnect(page.get(), &Page::titleChanged, this, &Document::onPageTitleChanged);
    disconnect(page.get(), &Page::objectAdded, this, &Document::onPageObjectAdded);
    disconnect(page.get(), &Page::objectRemoved, this, &Document::onPageObjectRemoved);
}

void Document::updateModifiedDate()
{
    m_modifiedDate = QDateTime::currentDateTime();
}

void Document::onPageTitleChanged(const QString &newTitle)
{
    Q_UNUSED(newTitle)
    markAsModified();
}

void Document::onPageObjectAdded(std::shared_ptr<Object> object)
{
    Q_UNUSED(object)
    markAsModified();
}

void Document::onPageObjectRemoved(std::shared_ptr<Object> object)
{
    Q_UNUSED(object)
    markAsModified();
}
