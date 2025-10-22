#ifndef STORAGE_H
#define STORAGE_H

#include "document.h"
#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QStringList>
#include <memory>

/**
 * @brief Storage manager for persisting documents and managing the database
 * 
 * This class handles all database operations including saving/loading documents,
 * managing the SQLite database schema, and providing backup/restore functionality.
 */
class Storage : public QObject
{
    Q_OBJECT

public:
    explicit Storage(QObject *parent = nullptr);
    ~Storage() override;

    // Database management
    bool initialize(const QString &databasePath = QString());
    void close();
    bool isOpen() const;
    
    // Document operations
    bool saveDocument(std::shared_ptr<Document> document);
    std::shared_ptr<Document> loadDocument(const QString &documentId);
    std::shared_ptr<Document> loadDocumentByTitle(const QString &title);
    bool deleteDocument(const QString &documentId);
    QStringList listDocuments();
    
    // Page operations
    bool savePage(const QString &documentId, std::shared_ptr<Page> page);
    std::shared_ptr<Page> loadPage(const QString &pageId);
    bool deletePage(const QString &pageId);
    
    // Search and queries
    QStringList searchDocuments(const QString &query);
    QStringList findDocumentsByTag(const QString &tag);
    QVector<QJsonObject> getRecentDocuments(int limit = 10);
    
    // Backup and restore
    bool createBackup(const QString &backupPath);
    bool restoreFromBackup(const QString &backupPath);
    
    // Metadata operations
    bool updateDocumentMetadata(const QString &documentId, const QJsonObject &metadata);
    QJsonObject getDocumentMetadata(const QString &documentId);
    
    // Statistics
    int getDocumentCount() const;
    int getPageCount() const;
    qint64 getDatabaseSize() const;

signals:
    void documentSaved(const QString &documentId);
    void documentDeleted(const QString &documentId);
    void databaseError(const QString &error);

private:
    QSqlDatabase m_database;
    QString m_databasePath;
    bool m_initialized;
    
    // Database schema management
    bool createTables();
    bool createDocumentTable();
    bool createPageTable();
    bool createObjectTable();
    bool createMetadataTable();
    bool createLinksTable();
    
    // Helper methods
    bool executeQuery(const QString &query, const QVariantList &params = QVariantList());
    QSqlQuery prepareQuery(const QString &query);
    QString getLastError() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // JSON serialization helpers
    QByteArray documentToBlob(std::shared_ptr<Document> document);
    std::shared_ptr<Document> documentFromBlob(const QByteArray &blob);
    QByteArray pageToBlob(std::shared_ptr<Page> page);
    std::shared_ptr<Page> pageFromBlob(const QByteArray &blob);
    
    // Migration support
    bool migrateDatabase();
    int getCurrentVersion();
    bool setCurrentVersion(int version);
};

#endif // STORAGE_H
