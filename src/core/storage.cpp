#include "storage.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

Storage::Storage(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
}

Storage::~Storage()
{
    close();
}

bool Storage::initialize(const QString &databasePath)
{
    if (m_initialized) {
        return true;
    }
    
    // Use default path if none provided
    if (databasePath.isEmpty()) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(appDataPath);
        m_databasePath = appDataPath + "/notes.db";
    } else {
        m_databasePath = databasePath;
    }
    
    // Create database connection
    m_database = QSqlDatabase::addDatabase("QSQLITE", "NotesApp");
    m_database.setDatabaseName(m_databasePath);
    
    if (!m_database.open()) {
        emit databaseError("Failed to open database: " + m_database.lastError().text());
        return false;
    }
    
    // Create tables
    if (!createTables()) {
        emit databaseError("Failed to create database tables");
        return false;
    }
    
    // Run migrations
    if (!migrateDatabase()) {
        emit databaseError("Failed to migrate database");
        return false;
    }
    
    m_initialized = true;
    return true;
}

void Storage::close()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
    m_initialized = false;
}

bool Storage::isOpen() const
{
    return m_database.isOpen();
}

bool Storage::saveDocument(std::shared_ptr<Document> document)
{
    if (!m_initialized || !document) {
        return false;
    }
    
    beginTransaction();
    
    try {
        // Save document metadata
        QSqlQuery query = prepareQuery(
            "INSERT OR REPLACE INTO documents (id, title, description, created_date, modified_date, tags, data) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)"
        );
        
        query.addBindValue(document->id());
        query.addBindValue(document->title());
        query.addBindValue(document->description());
        query.addBindValue(document->createdDate());
        query.addBindValue(document->modifiedDate());
        query.addBindValue(QStringList(document->tags()).join(","));
        query.addBindValue(documentToBlob(document));
        
        if (!query.exec()) {
            rollbackTransaction();
            emit databaseError("Failed to save document: " + query.lastError().text());
            return false;
        }
        
        // Save pages
        for (const auto &page : document->pages()) {
            if (!savePage(document->id(), page)) {
                rollbackTransaction();
                return false;
            }
        }
        
        commitTransaction();
        emit documentSaved(document->id());
        return true;
        
    } catch (...) {
        rollbackTransaction();
        emit databaseError("Exception occurred while saving document");
        return false;
    }
}

std::shared_ptr<Document> Storage::loadDocument(const QString &documentId)
{
    if (!m_initialized || documentId.isEmpty()) {
        return nullptr;
    }
    
    QSqlQuery query = prepareQuery("SELECT data FROM documents WHERE id = ?");
    query.addBindValue(documentId);
    
    if (!query.exec() || !query.next()) {
        emit databaseError("Failed to load document: " + query.lastError().text());
        return nullptr;
    }
    
    QByteArray blob = query.value(0).toByteArray();
    return documentFromBlob(blob);
}

std::shared_ptr<Document> Storage::loadDocumentByTitle(const QString &title)
{
    if (!m_initialized || title.isEmpty()) {
        return nullptr;
    }
    
    QSqlQuery query = prepareQuery("SELECT data FROM documents WHERE title = ?");
    query.addBindValue(title);
    
    if (!query.exec() || !query.next()) {
        emit databaseError("Failed to load document by title: " + query.lastError().text());
        return nullptr;
    }
    
    QByteArray blob = query.value(0).toByteArray();
    return documentFromBlob(blob);
}

bool Storage::deleteDocument(const QString &documentId)
{
    if (!m_initialized || documentId.isEmpty()) {
        return false;
    }
    
    beginTransaction();
    
    // Delete pages first
    QSqlQuery deletePages = prepareQuery("DELETE FROM pages WHERE document_id = ?");
    deletePages.addBindValue(documentId);
    if (!deletePages.exec()) {
        rollbackTransaction();
        emit databaseError("Failed to delete document pages: " + deletePages.lastError().text());
        return false;
    }
    
    // Delete document
    QSqlQuery deleteDoc = prepareQuery("DELETE FROM documents WHERE id = ?");
    deleteDoc.addBindValue(documentId);
    if (!deleteDoc.exec()) {
        rollbackTransaction();
        emit databaseError("Failed to delete document: " + deleteDoc.lastError().text());
        return false;
    }
    
    commitTransaction();
    emit documentDeleted(documentId);
    return true;
}

QStringList Storage::listDocuments()
{
    QStringList documents;
    
    if (!m_initialized) {
        return documents;
    }
    
    QSqlQuery query = prepareQuery("SELECT id, title FROM documents ORDER BY modified_date DESC");
    if (query.exec()) {
        while (query.next()) {
            documents.append(query.value(0).toString());
        }
    } else {
        emit databaseError("Failed to list documents: " + query.lastError().text());
    }
    
    return documents;
}

bool Storage::savePage(const QString &documentId, std::shared_ptr<Page> page)
{
    if (!m_initialized || !page) {
        return false;
    }
    
    QSqlQuery query = prepareQuery(
        "INSERT OR REPLACE INTO pages (id, document_id, title, data) VALUES (?, ?, ?, ?)"
    );
    
    query.addBindValue(page->id());
    query.addBindValue(documentId);
    query.addBindValue(page->title());
    query.addBindValue(pageToBlob(page));
    
    if (!query.exec()) {
        emit databaseError("Failed to save page: " + query.lastError().text());
        return false;
    }
    
    return true;
}

std::shared_ptr<Page> Storage::loadPage(const QString &pageId)
{
    if (!m_initialized || pageId.isEmpty()) {
        return nullptr;
    }
    
    QSqlQuery query = prepareQuery("SELECT data FROM pages WHERE id = ?");
    query.addBindValue(pageId);
    
    if (!query.exec() || !query.next()) {
        emit databaseError("Failed to load page: " + query.lastError().text());
        return nullptr;
    }
    
    QByteArray blob = query.value(0).toByteArray();
    return pageFromBlob(blob);
}

bool Storage::deletePage(const QString &pageId)
{
    if (!m_initialized || pageId.isEmpty()) {
        return false;
    }
    
    QSqlQuery query = prepareQuery("DELETE FROM pages WHERE id = ?");
    query.addBindValue(pageId);
    
    if (!query.exec()) {
        emit databaseError("Failed to delete page: " + query.lastError().text());
        return false;
    }
    
    return true;
}

QStringList Storage::searchDocuments(const QString &query)
{
    QStringList results;
    
    if (!m_initialized || query.isEmpty()) {
        return results;
    }
    
    QSqlQuery sqlQuery = prepareQuery(
        "SELECT id FROM documents WHERE title LIKE ? OR description LIKE ? OR tags LIKE ?"
    );
    
    QString searchPattern = "%" + query + "%";
    sqlQuery.addBindValue(searchPattern);
    sqlQuery.addBindValue(searchPattern);
    sqlQuery.addBindValue(searchPattern);
    
    if (sqlQuery.exec()) {
        while (sqlQuery.next()) {
            results.append(sqlQuery.value(0).toString());
        }
    } else {
        emit databaseError("Failed to search documents: " + sqlQuery.lastError().text());
    }
    
    return results;
}

QStringList Storage::findDocumentsByTag(const QString &tag)
{
    QStringList results;
    
    if (!m_initialized || tag.isEmpty()) {
        return results;
    }
    
    QSqlQuery query = prepareQuery("SELECT id FROM documents WHERE tags LIKE ?");
    query.addBindValue("%" + tag + "%");
    
    if (query.exec()) {
        while (query.next()) {
            results.append(query.value(0).toString());
        }
    } else {
        emit databaseError("Failed to find documents by tag: " + query.lastError().text());
    }
    
    return results;
}

QVector<QJsonObject> Storage::getRecentDocuments(int limit)
{
    QVector<QJsonObject> results;
    
    if (!m_initialized) {
        return results;
    }
    
    QSqlQuery query = prepareQuery(
        "SELECT id, title, description, modified_date FROM documents "
        "ORDER BY modified_date DESC LIMIT ?"
    );
    query.addBindValue(limit);
    
    if (query.exec()) {
        while (query.next()) {
            QJsonObject doc;
            doc["id"] = query.value(0).toString();
            doc["title"] = query.value(1).toString();
            doc["description"] = query.value(2).toString();
            doc["modifiedDate"] = query.value(3).toString();
            results.append(doc);
        }
    } else {
        emit databaseError("Failed to get recent documents: " + query.lastError().text());
    }
    
    return results;
}

bool Storage::createBackup(const QString &backupPath)
{
    if (!m_initialized) {
        return false;
    }
    
    // Simple file copy backup
    if (QFile::exists(m_databasePath)) {
        return QFile::copy(m_databasePath, backupPath);
    }
    
    return false;
}

bool Storage::restoreFromBackup(const QString &backupPath)
{
    if (!QFile::exists(backupPath)) {
        return false;
    }
    
    close();
    
    if (QFile::exists(m_databasePath)) {
        QFile::remove(m_databasePath);
    }
    
    bool success = QFile::copy(backupPath, m_databasePath);
    
    if (success) {
        initialize(m_databasePath);
    }
    
    return success;
}

bool Storage::updateDocumentMetadata(const QString &documentId, const QJsonObject &metadata)
{
    if (!m_initialized || documentId.isEmpty()) {
        return false;
    }
    
    QSqlQuery query = prepareQuery(
        "INSERT OR REPLACE INTO metadata (document_id, key, value) VALUES (?, ?, ?)"
    );
    
    for (auto it = metadata.begin(); it != metadata.end(); ++it) {
        query.addBindValue(documentId);
        query.addBindValue(it.key());
        query.addBindValue(it.value().toString());
        
        if (!query.exec()) {
            emit databaseError("Failed to update metadata: " + query.lastError().text());
            return false;
        }
    }
    
    return true;
}

QJsonObject Storage::getDocumentMetadata(const QString &documentId)
{
    QJsonObject metadata;
    
    if (!m_initialized || documentId.isEmpty()) {
        return metadata;
    }
    
    QSqlQuery query = prepareQuery("SELECT key, value FROM metadata WHERE document_id = ?");
    query.addBindValue(documentId);
    
    if (query.exec()) {
        while (query.next()) {
            metadata[query.value(0).toString()] = query.value(1).toString();
        }
    } else {
        emit databaseError("Failed to get metadata: " + query.lastError().text());
    }
    
    return metadata;
}

int Storage::getDocumentCount() const
{
    if (!m_initialized) {
        return 0;
    }
    
    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM documents");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

int Storage::getPageCount() const
{
    if (!m_initialized) {
        return 0;
    }
    
    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM pages");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

qint64 Storage::getDatabaseSize() const
{
    QFileInfo fileInfo(m_databasePath);
    return fileInfo.size();
}

bool Storage::createTables()
{
    return createDocumentTable() && 
           createPageTable() && 
           createObjectTable() && 
           createMetadataTable() && 
           createLinksTable();
}

bool Storage::createDocumentTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS documents (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT,
            created_date TEXT NOT NULL,
            modified_date TEXT NOT NULL,
            tags TEXT,
            data BLOB NOT NULL
        )
    )";
    
    return executeQuery(query);
}

bool Storage::createPageTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS pages (
            id TEXT PRIMARY KEY,
            document_id TEXT NOT NULL,
            title TEXT NOT NULL,
            data BLOB NOT NULL,
            FOREIGN KEY (document_id) REFERENCES documents (id) ON DELETE CASCADE
        )
    )";
    
    return executeQuery(query);
}

bool Storage::createObjectTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS objects (
            id TEXT PRIMARY KEY,
            page_id TEXT NOT NULL,
            type INTEGER NOT NULL,
            data BLOB NOT NULL,
            FOREIGN KEY (page_id) REFERENCES pages (id) ON DELETE CASCADE
        )
    )";
    
    return executeQuery(query);
}

bool Storage::createMetadataTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS metadata (
            document_id TEXT NOT NULL,
            key TEXT NOT NULL,
            value TEXT NOT NULL,
            PRIMARY KEY (document_id, key),
            FOREIGN KEY (document_id) REFERENCES documents (id) ON DELETE CASCADE
        )
    )";
    
    return executeQuery(query);
}

bool Storage::createLinksTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS links (
            from_page_id TEXT NOT NULL,
            to_page_id TEXT NOT NULL,
            PRIMARY KEY (from_page_id, to_page_id),
            FOREIGN KEY (from_page_id) REFERENCES pages (id) ON DELETE CASCADE,
            FOREIGN KEY (to_page_id) REFERENCES pages (id) ON DELETE CASCADE
        )
    )";
    
    return executeQuery(query);
}

bool Storage::executeQuery(const QString &query, const QVariantList &params)
{
    QSqlQuery sqlQuery = prepareQuery(query);
    
    for (const QVariant &param : params) {
        sqlQuery.addBindValue(param);
    }
    
    bool success = sqlQuery.exec();
    if (!success) {
        emit databaseError("Query failed: " + sqlQuery.lastError().text());
    }
    
    return success;
}

QSqlQuery Storage::prepareQuery(const QString &query)
{
    QSqlQuery sqlQuery(m_database);
    sqlQuery.prepare(query);
    return sqlQuery;
}

QString Storage::getLastError() const
{
    return m_database.lastError().text();
}

bool Storage::beginTransaction()
{
    return m_database.transaction();
}

bool Storage::commitTransaction()
{
    return m_database.commit();
}

bool Storage::rollbackTransaction()
{
    return m_database.rollback();
}

QByteArray Storage::documentToBlob(std::shared_ptr<Document> document)
{
    QJsonDocument doc(document->toJson());
    return doc.toJson(QJsonDocument::Compact);
}

std::shared_ptr<Document> Storage::documentFromBlob(const QByteArray &blob)
{
    QJsonDocument doc = QJsonDocument::fromJson(blob);
    if (doc.isNull()) {
        return nullptr;
    }
    
    auto document = std::make_shared<Document>();
    document->fromJson(doc.object());
    return document;
}

QByteArray Storage::pageToBlob(std::shared_ptr<Page> page)
{
    QJsonDocument doc(page->toJson());
    return doc.toJson(QJsonDocument::Compact);
}

std::shared_ptr<Page> Storage::pageFromBlob(const QByteArray &blob)
{
    QJsonDocument doc = QJsonDocument::fromJson(blob);
    if (doc.isNull()) {
        return nullptr;
    }
    
    auto page = std::make_shared<Page>();
    page->fromJson(doc.object());
    return page;
}

bool Storage::migrateDatabase()
{
    int currentVersion = getCurrentVersion();
    int targetVersion = 1; // Current schema version
    
    if (currentVersion < targetVersion) {
        // Run migrations here as the schema evolves
        // For now, just set the version
        return setCurrentVersion(targetVersion);
    }
    
    return true;
}

int Storage::getCurrentVersion()
{
    QSqlQuery query = prepareQuery("PRAGMA user_version");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool Storage::setCurrentVersion(int version)
{
    QString query = QString("PRAGMA user_version = %1").arg(version);
    return executeQuery(query);
}
