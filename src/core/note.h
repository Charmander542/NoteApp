#ifndef NOTE_H
#define NOTE_H

#include "document.h"
#include "storage.h"
#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

/**
 * @brief Main application class that manages documents and storage
 * 
 * This class serves as the central coordinator for the note-taking application,
 * managing documents, storage operations, and providing the main interface
 * for the GUI components.
 */
class Note : public QObject
{
    Q_OBJECT

public:
    explicit Note(QObject *parent = nullptr);
    ~Note() override;

    // Document management
    std::shared_ptr<Document> currentDocument() const { return m_currentDocument; }
    void setCurrentDocument(std::shared_ptr<Document> document);
    std::shared_ptr<Document> createNewDocument(const QString &title = QString());
    bool loadDocument(const QString &documentId);
    bool saveCurrentDocument();
    bool saveDocumentAs(const QString &title);
    void closeCurrentDocument();
    
    // Document operations
    QStringList listDocuments();
    bool deleteDocument(const QString &documentId);
    bool duplicateDocument(const QString &documentId);
    
    // Storage management
    bool initializeStorage(const QString &databasePath = QString());
    void closeStorage();
    bool isStorageOpen() const;
    
    // Auto-save functionality
    void enableAutoSave(bool enable = true);
    void setAutoSaveInterval(int seconds);
    void triggerAutoSave();
    
    // Search functionality
    QStringList searchDocuments(const QString &query);
    QStringList findDocumentsByTag(const QString &tag);
    
    // Recent documents
    QVector<QJsonObject> getRecentDocuments(int limit = 10);
    
    // Backup and restore
    bool createBackup(const QString &backupPath);
    bool restoreFromBackup(const QString &backupPath);
    
    // Application state
    bool isModified() const;
    void markAsModified();
    void clearModified();

signals:
    void currentDocumentChanged(std::shared_ptr<Document> newDocument);
    void documentSaved(const QString &documentId);
    void documentLoaded(const QString &documentId);
    void documentClosed();
    void modifiedChanged(bool modified);
    void autoSaveTriggered();
    void storageError(const QString &error);

private:
    std::shared_ptr<Document> m_currentDocument;
    std::unique_ptr<Storage> m_storage;
    QTimer *m_autoSaveTimer;
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;
    bool m_modified;
    
    void setupAutoSave();
    void connectDocumentSignals(std::shared_ptr<Document> document);
    void disconnectDocumentSignals(std::shared_ptr<Document> document);

private slots:
    void onAutoSaveTimeout();
    void onDocumentModifiedChanged(bool modified);
    void onStorageError(const QString &error);
};

#endif // NOTE_H
