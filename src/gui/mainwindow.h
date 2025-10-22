#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QTreeWidget>
#include <QTabWidget>
#include <QAction>
#include <QActionGroup>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Forward declarations
class Note;
class Document;
class Page;
class PageCanvas;
class Toolbar;
class ObjectSelector;

/**
 * @brief Main application window with comprehensive UI layout
 * 
 * This class provides the main window interface with menus, toolbars,
 * document browser, page canvas, and status bar. It coordinates all
 * GUI components and handles user interactions.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Document management
    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();
    void closeDocument();
    
    // Page management
    void newPage();
    void deletePage();
    void duplicatePage();
    
    // Object management
    void addTextObject();
    void addDrawingObject();
    void addImageObject();
    void addPDFObject();
    
    // Edit operations
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void deleteSelected();
    void selectAll();
    
    // View operations
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomActual();
    
    // Search and navigation
    void showSearchDialog();
    void showTagManager();
    void showRecentDocuments();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // Document slots
    void onDocumentChanged(std::shared_ptr<Document> document);
    void onDocumentSaved(const QString &documentId);
    void onDocumentLoaded(const QString &documentId);
    void onDocumentClosed();
    void onModifiedChanged(bool modified);
    
    // Page slots
    void onPageChanged(std::shared_ptr<Page> page);
    void onPageAdded(std::shared_ptr<Page> page, int index);
    void onPageRemoved(std::shared_ptr<Page> page, int index);
    
    // Object slots
    void onObjectSelectionChanged();
    void onObjectAdded(std::shared_ptr<Object> object);
    void onObjectRemoved(std::shared_ptr<Page> object);
    
    // UI slots
    void onDocumentTreeItemClicked(QTreeWidgetItem *item, int column);
    void onPageTabChanged(int index);
    void onToolbarActionTriggered(QAction *action);
    
    // Auto-save
    void onAutoSaveTriggered();
    
    // Error handling
    void onStorageError(const QString &error);

private:
    Ui::MainWindow *ui;
    
    // Core components
    std::unique_ptr<Note> m_note;
    std::shared_ptr<Document> m_currentDocument;
    std::shared_ptr<Page> m_currentPage;
    
    // UI components
    QSplitter *m_mainSplitter;
    QTreeWidget *m_documentTree;
    QTabWidget *m_pageTabs;
    PageCanvas *m_pageCanvas;
    Toolbar *m_toolbar;
    ObjectSelector *m_objectSelector;
    
    // Actions
    QAction *m_newDocumentAction;
    QAction *m_openDocumentAction;
    QAction *m_saveDocumentAction;
    QAction *m_saveDocumentAsAction;
    QAction *m_closeDocumentAction;
    QAction *m_exitAction;
    
    QAction *m_newPageAction;
    QAction *m_deletePageAction;
    QAction *m_duplicatePageAction;
    
    QAction *m_addTextAction;
    QAction *m_addDrawingAction;
    QAction *m_addImageAction;
    QAction *m_addPDFAction;
    
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_deleteAction;
    QAction *m_selectAllAction;
    
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_zoomFitAction;
    QAction *m_zoomActualAction;
    
    QAction *m_searchAction;
    QAction *m_tagManagerAction;
    QAction *m_recentDocumentsAction;
    
    // Action groups
    QActionGroup *m_toolActionGroup;
    
    // State
    bool m_initialized;
    double m_zoomFactor;
    
    // Setup methods
    void setupUI();
    void setupMenus();
    void setupToolbars();
    void setupStatusBar();
    void setupActions();
    void setupConnections();
    
    // UI update methods
    void updateWindowTitle();
    void updateDocumentTree();
    void updatePageTabs();
    void updateActions();
    void updateStatusBar();
    
    // Helper methods
    void initializeApplication();
    void showErrorMessage(const QString &title, const QString &message);
    void showInfoMessage(const QString &title, const QString &message);
    bool confirmClose();
    void setupDocumentTree();
    void setupPageTabs();
};

#endif // MAINWINDOW_H
