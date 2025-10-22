#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "page.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QStringList>
#include <memory>

/**
 * @brief A document that contains multiple pages and manages the overall structure
 * 
 * This class represents a complete document/notebook containing multiple pages.
 * It manages page organization, metadata, tags, and provides search functionality.
 */
class Document : public QObject
{
    Q_OBJECT

public:
    explicit Document(QObject *parent = nullptr);
    explicit Document(const QString &title, QObject *parent = nullptr);
    ~Document() override;

    // Basic properties
    QString title() const { return m_title; }
    void setTitle(const QString &title);
    
    QString id() const { return m_id; }
    void setId(const QString &id);
    
    QString description() const { return m_description; }
    void setDescription(const QString &description);
    
    QDateTime createdDate() const { return m_createdDate; }
    QDateTime modifiedDate() const { return m_modifiedDate; }
    
    // Page management
    const QVector<std::shared_ptr<Page>> &pages() const { return m_pages; }
    void addPage(std::shared_ptr<Page> page);
    void insertPage(int index, std::shared_ptr<Page> page);
    void removePage(std::shared_ptr<Page> page);
    void removePage(int index);
    void clearPages();
    
    std::shared_ptr<Page> pageAt(int index) const;
    std::shared_ptr<Page> pageById(const QString &id) const;
    int pageIndex(std::shared_ptr<Page> page) const;
    
    // Page operations
    void movePage(int fromIndex, int toIndex);
    void duplicatePage(int index);
    std::shared_ptr<Page> createNewPage(const QString &title = QString());
    
    // Current page
    std::shared_ptr<Page> currentPage() const { return m_currentPage; }
    void setCurrentPage(std::shared_ptr<Page> page);
    void setCurrentPage(int index);
    
    // Tags and metadata
    QStringList tags() const { return m_tags; }
    void setTags(const QStringList &tags);
    void addTag(const QString &tag);
    void removeTag(const QString &tag);
    bool hasTag(const QString &tag) const;
    
    // Search functionality
    QVector<std::shared_ptr<Page>> searchPages(const QString &query) const;
    QVector<std::shared_ptr<Object>> searchObjects(const QString &query) const;
    QVector<std::shared_ptr<Page>> findPagesByTag(const QString &tag) const;
    
    // Links and references
    QStringList getBacklinks(const QString &pageId) const;
    void addLink(const QString &fromPageId, const QString &toPageId);
    void removeLink(const QString &fromPageId, const QString &toPageId);
    
    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
    
    // Operations
    std::unique_ptr<Document> clone() const;
    
    // Undo/Redo support
    QJsonObject getState() const;
    void setState(const QJsonObject &state);
    
    // File operations
    bool isModified() const { return m_modified; }
    void setModified(bool modified);
    void markAsModified();

signals:
    void titleChanged(const QString &newTitle);
    void descriptionChanged(const QString &newDescription);
    void pageAdded(std::shared_ptr<Page> page, int index);
    void pageRemoved(std::shared_ptr<Page> page, int index);
    void pageMoved(std::shared_ptr<Page> page, int fromIndex, int toIndex);
    void currentPageChanged(std::shared_ptr<Page> newPage);
    void tagsChanged(const QStringList &newTags);
    void modifiedChanged(bool modified);

private:
    QString m_title;
    QString m_id;
    QString m_description;
    QDateTime m_createdDate;
    QDateTime m_modifiedDate;
    QVector<std::shared_ptr<Page>> m_pages;
    std::shared_ptr<Page> m_currentPage;
    QStringList m_tags;
    QMap<QString, QStringList> m_links; // from page ID to list of linked page IDs
    bool m_modified;
    
    void generateId();
    void connectPageSignals(std::shared_ptr<Page> page);
    void disconnectPageSignals(std::shared_ptr<Page> page);
    void updateModifiedDate();

private slots:
    void onPageTitleChanged(const QString &newTitle);
    void onPageObjectAdded(std::shared_ptr<Object> object);
    void onPageObjectRemoved(std::shared_ptr<Object> object);
};

#endif // DOCUMENT_H
