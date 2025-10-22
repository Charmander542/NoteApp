#include "note.h"
#include <QTimer>
#include <QDebug>

Note::Note(QObject *parent)
    : QObject(parent)
    , m_storage(std::make_unique<Storage>(this))
    , m_autoSaveTimer(new QTimer(this))
    , m_autoSaveEnabled(false)
    , m_autoSaveInterval(30) // 30 seconds default
    , m_modified(false)
{
    setupAutoSave();
}

Note::~Note()
{
    closeStorage();
}

void Note::setCurrentDocument(std::shared_ptr<Document> document)
{
    if (m_currentDocument == document) {
        return;
    }
    
    // Save current document if modified
    if (m_currentDocument && m_modified) {
        saveCurrentDocument();
    }
    
    // Disconnect old document signals
    if (m_currentDocument) {
        disconnectDocumentSignals(m_currentDocument);
    }
    
    m_currentDocument = document;
    
    // Connect new document signals
    if (m_currentDocument) {
        connectDocumentSignals(m_currentDocument);
        m_modified = m_currentDocument->isModified();
    } else {
        m_modified = false;
    }
    
    emit currentDocumentChanged(m_currentDocument);
    emit modifiedChanged(m_modified);
}

std::shared_ptr<Document> Note::createNewDocument(const QString &title)
{
    auto document = std::make_shared<Document>(title.isEmpty() ? "Untitled Document" : title);
    setCurrentDocument(document);
    return document;
}

bool Note::loadDocument(const QString &documentId)
{
    if (!m_storage || !m_storage->isOpen()) {
        emit storageError("Storage not initialized");
        return false;
    }
    
    auto document = m_storage->loadDocument(documentId);
    if (!document) {
        emit storageError("Failed to load document");
        return false;
    }
    
    setCurrentDocument(document);
    emit documentLoaded(documentId);
    return true;
}

bool Note::saveCurrentDocument()
{
    if (!m_currentDocument || !m_storage || !m_storage->isOpen()) {
        return false;
    }
    
    bool success = m_storage->saveDocument(m_currentDocument);
    if (success) {
        m_currentDocument->setModified(false);
        m_modified = false;
        emit documentSaved(m_currentDocument->id());
        emit modifiedChanged(false);
    }
    
    return success;
}

bool Note::saveDocumentAs(const QString &title)
{
    if (!m_currentDocument) {
        return false;
    }
    
    m_currentDocument->setTitle(title);
    return saveCurrentDocument();
}

void Note::closeCurrentDocument()
{
    if (m_currentDocument) {
        // Save if modified
        if (m_modified) {
            saveCurrentDocument();
        }
        
        disconnectDocumentSignals(m_currentDocument);
        m_currentDocument.reset();
        m_modified = false;
        
        emit documentClosed();
        emit modifiedChanged(false);
    }
}

QStringList Note::listDocuments()
{
    if (!m_storage || !m_storage->isOpen()) {
        return QStringList();
    }
    
    return m_storage->listDocuments();
}

bool Note::deleteDocument(const QString &documentId)
{
    if (!m_storage || !m_storage->isOpen()) {
        return false;
    }
    
    // If we're deleting the current document, close it first
    if (m_currentDocument && m_currentDocument->id() == documentId) {
        closeCurrentDocument();
    }
    
    return m_storage->deleteDocument(documentId);
}

bool Note::duplicateDocument(const QString &documentId)
{
    if (!m_storage || !m_storage->isOpen()) {
        return false;
    }
    
    auto originalDocument = m_storage->loadDocument(documentId);
    if (!originalDocument) {
        return false;
    }
    
    auto clonedDocument = originalDocument->clone();
    clonedDocument->setTitle(originalDocument->title() + " (Copy)");
    
    setCurrentDocument(std::shared_ptr<Document>(clonedDocument.release()));
    return saveCurrentDocument();
}

bool Note::initializeStorage(const QString &databasePath)
{
    if (!m_storage) {
        m_storage = std::make_unique<Storage>(this);
    }
    
    bool success = m_storage->initialize(databasePath);
    if (!success) {
        emit storageError("Failed to initialize storage");
    }
    
    return success;
}

void Note::closeStorage()
{
    if (m_storage) {
        m_storage->close();
    }
}

bool Note::isStorageOpen() const
{
    return m_storage && m_storage->isOpen();
}

void Note::enableAutoSave(bool enable)
{
    m_autoSaveEnabled = enable;
    
    if (enable) {
        m_autoSaveTimer->start(m_autoSaveInterval * 1000);
    } else {
        m_autoSaveTimer->stop();
    }
}

void Note::setAutoSaveInterval(int seconds)
{
    m_autoSaveInterval = qMax(5, seconds); // Minimum 5 seconds
    
    if (m_autoSaveEnabled) {
        m_autoSaveTimer->start(m_autoSaveInterval * 1000);
    }
}

void Note::triggerAutoSave()
{
    if (m_modified && m_currentDocument) {
        saveCurrentDocument();
        emit autoSaveTriggered();
    }
}

QStringList Note::searchDocuments(const QString &query)
{
    if (!m_storage || !m_storage->isOpen()) {
        return QStringList();
    }
    
    return m_storage->searchDocuments(query);
}

QStringList Note::findDocumentsByTag(const QString &tag)
{
    if (!m_storage || !m_storage->isOpen()) {
        return QStringList();
    }
    
    return m_storage->findDocumentsByTag(tag);
}

QVector<QJsonObject> Note::getRecentDocuments(int limit)
{
    if (!m_storage || !m_storage->isOpen()) {
        return QVector<QJsonObject>();
    }
    
    return m_storage->getRecentDocuments(limit);
}

bool Note::createBackup(const QString &backupPath)
{
    if (!m_storage || !m_storage->isOpen()) {
        return false;
    }
    
    return m_storage->createBackup(backupPath);
}

bool Note::restoreFromBackup(const QString &backupPath)
{
    if (!m_storage) {
        return false;
    }
    
    // Close current document
    closeCurrentDocument();
    
    return m_storage->restoreFromBackup(backupPath);
}

bool Note::isModified() const
{
    return m_modified;
}

void Note::markAsModified()
{
    if (!m_modified) {
        m_modified = true;
        emit modifiedChanged(true);
    }
}

void Note::clearModified()
{
    if (m_modified) {
        m_modified = false;
        emit modifiedChanged(false);
    }
}

void Note::setupAutoSave()
{
    connect(m_autoSaveTimer, &QTimer::timeout, this, &Note::onAutoSaveTimeout);
    
    if (m_storage) {
        connect(m_storage.get(), &Storage::databaseError, this, &Note::onStorageError);
    }
}

void Note::connectDocumentSignals(std::shared_ptr<Document> document)
{
    connect(document.get(), &Document::modifiedChanged, this, &Note::onDocumentModifiedChanged);
}

void Note::disconnectDocumentSignals(std::shared_ptr<Document> document)
{
    disconnect(document.get(), &Document::modifiedChanged, this, &Note::onDocumentModifiedChanged);
}

void Note::onAutoSaveTimeout()
{
    triggerAutoSave();
}

void Note::onDocumentModifiedChanged(bool modified)
{
    if (modified) {
        markAsModified();
    } else {
        clearModified();
    }
}

void Note::onStorageError(const QString &error)
{
    emit storageError(error);
}
