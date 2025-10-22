#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "../core/note.h"
#include "../core/document.h"
#include "../core/page.h"
#include "../core/object.h"
#include "pagecanvas.h"
#include "toolbar.h"
#include "objectselector.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QTreeWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QActionGroup>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QCloseEvent>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_initialized(false)
    , m_zoomFactor(1.0)
{
    ui->setupUi(this);
    initializeApplication();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeApplication()
{
    // Initialize core components
    m_note = std::make_unique<Note>();
    
    // Initialize storage
    if (!m_note->initializeStorage()) {
        showErrorMessage("Storage Error", "Failed to initialize storage system");
        return;
    }
    
    // Setup UI
    setupUI();
    setupActions();
    setupMenus();
    setupToolbars();
    setupStatusBar();
    setupConnections();
    
    // Create initial document
    newDocument();
    
    m_initialized = true;
}

void MainWindow::setupUI()
{
    // Create main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);
    
    // Create document tree (left panel)
    m_documentTree = new QTreeWidget();
    m_documentTree->setHeaderLabel("Documents");
    m_documentTree->setMaximumWidth(250);
    m_documentTree->setMinimumWidth(200);
    m_mainSplitter->addWidget(m_documentTree);
    
    // Create main content area
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create page tabs
    m_pageTabs = new QTabWidget();
    m_pageTabs->setTabsClosable(true);
    contentLayout->addWidget(m_pageTabs);
    
    // Create page canvas
    m_pageCanvas = new PageCanvas();
    m_pageTabs->addTab(m_pageCanvas, "Page 1");
    
    m_mainSplitter->addWidget(contentWidget);
    
    // Set splitter proportions
    m_mainSplitter->setSizes({200, 600});
    
    // Create toolbar
    m_toolbar = new Toolbar();
    addToolBar(Qt::TopToolBarArea, m_toolbar);
    
    // Create object selector
    m_objectSelector = new ObjectSelector();
    addDockWidget(Qt::RightDockWidgetArea, m_objectSelector);
}

void MainWindow::setupActions()
{
    // Document actions
    m_newDocumentAction = new QAction("&New Document", this);
    m_newDocumentAction->setShortcut(QKeySequence::New);
    m_newDocumentAction->setStatusTip("Create a new document");
    m_newDocumentAction->setIcon(QIcon(":/icons/new.png"));
    
    m_openDocumentAction = new QAction("&Open Document", this);
    m_openDocumentAction->setShortcut(QKeySequence::Open);
    m_openDocumentAction->setStatusTip("Open an existing document");
    m_openDocumentAction->setIcon(QIcon(":/icons/open.png"));
    
    m_saveDocumentAction = new QAction("&Save Document", this);
    m_saveDocumentAction->setShortcut(QKeySequence::Save);
    m_saveDocumentAction->setStatusTip("Save the current document");
    m_saveDocumentAction->setIcon(QIcon(":/icons/save.png"));
    
    m_saveDocumentAsAction = new QAction("Save Document &As...", this);
    m_saveDocumentAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveDocumentAsAction->setStatusTip("Save the current document with a new name");
    
    m_closeDocumentAction = new QAction("&Close Document", this);
    m_closeDocumentAction->setShortcut(QKeySequence::Close);
    m_closeDocumentAction->setStatusTip("Close the current document");
    
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("Exit the application");
    
    // Page actions
    m_newPageAction = new QAction("&New Page", this);
    m_newPageAction->setShortcut(QKeySequence("Ctrl+Shift+N"));
    m_newPageAction->setStatusTip("Create a new page");
    m_newPageAction->setIcon(QIcon(":/icons/new_page.png"));
    
    m_deletePageAction = new QAction("&Delete Page", this);
    m_deletePageAction->setShortcut(QKeySequence("Ctrl+Shift+D"));
    m_deletePageAction->setStatusTip("Delete the current page");
    m_deletePageAction->setIcon(QIcon(":/icons/delete.png"));
    
    m_duplicatePageAction = new QAction("&Duplicate Page", this);
    m_duplicatePageAction->setShortcut(QKeySequence("Ctrl+Shift+U"));
    m_duplicatePageAction->setStatusTip("Duplicate the current page");
    m_duplicatePageAction->setIcon(QIcon(":/icons/duplicate.png"));
    
    // Object actions
    m_addTextAction = new QAction("Add &Text", this);
    m_addTextAction->setShortcut(QKeySequence("Ctrl+T"));
    m_addTextAction->setStatusTip("Add a text object");
    m_addTextAction->setIcon(QIcon(":/icons/text.png"));
    m_addTextAction->setCheckable(true);
    
    m_addDrawingAction = new QAction("Add &Drawing", this);
    m_addDrawingAction->setShortcut(QKeySequence("Ctrl+D"));
    m_addDrawingAction->setStatusTip("Add a drawing object");
    m_addDrawingAction->setIcon(QIcon(":/icons/drawing.png"));
    m_addDrawingAction->setCheckable(true);
    
    m_addImageAction = new QAction("Add &Image", this);
    m_addImageAction->setShortcut(QKeySequence("Ctrl+I"));
    m_addImageAction->setStatusTip("Add an image object");
    m_addImageAction->setIcon(QIcon(":/icons/image.png"));
    m_addImageAction->setCheckable(true);
    
    m_addPDFAction = new QAction("Add &PDF", this);
    m_addPDFAction->setShortcut(QKeySequence("Ctrl+P"));
    m_addPDFAction->setStatusTip("Add a PDF object");
    m_addPDFAction->setIcon(QIcon(":/icons/pdf.png"));
    m_addPDFAction->setCheckable(true);
    
    // Edit actions
    m_undoAction = new QAction("&Undo", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setStatusTip("Undo the last action");
    m_undoAction->setIcon(QIcon(":/icons/undo.png"));
    
    m_redoAction = new QAction("&Redo", this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setStatusTip("Redo the last undone action");
    m_redoAction->setIcon(QIcon(":/icons/redo.png"));
    
    m_cutAction = new QAction("Cu&t", this);
    m_cutAction->setShortcut(QKeySequence::Cut);
    m_cutAction->setStatusTip("Cut selected objects");
    m_cutAction->setIcon(QIcon(":/icons/cut.png"));
    
    m_copyAction = new QAction("&Copy", this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setStatusTip("Copy selected objects");
    m_copyAction->setIcon(QIcon(":/icons/copy.png"));
    
    m_pasteAction = new QAction("&Paste", this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setStatusTip("Paste objects from clipboard");
    m_pasteAction->setIcon(QIcon(":/icons/paste.png"));
    
    m_deleteAction = new QAction("&Delete", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setStatusTip("Delete selected objects");
    m_deleteAction->setIcon(QIcon(":/icons/delete.png"));
    
    m_selectAllAction = new QAction("Select &All", this);
    m_selectAllAction->setShortcut(QKeySequence::SelectAll);
    m_selectAllAction->setStatusTip("Select all objects");
    m_selectAllAction->setIcon(QIcon(":/icons/select_all.png"));
    
    // View actions
    m_zoomInAction = new QAction("Zoom &In", this);
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    m_zoomInAction->setStatusTip("Zoom in");
    m_zoomInAction->setIcon(QIcon(":/icons/zoom_in.png"));
    
    m_zoomOutAction = new QAction("Zoom &Out", this);
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    m_zoomOutAction->setStatusTip("Zoom out");
    m_zoomOutAction->setIcon(QIcon(":/icons/zoom_out.png"));
    
    m_zoomFitAction = new QAction("Zoom &Fit", this);
    m_zoomFitAction->setShortcut(QKeySequence("Ctrl+0"));
    m_zoomFitAction->setStatusTip("Fit to window");
    m_zoomFitAction->setIcon(QIcon(":/icons/zoom_fit.png"));
    
    m_zoomActualAction = new QAction("Zoom &Actual Size", this);
    m_zoomActualAction->setShortcut(QKeySequence("Ctrl+1"));
    m_zoomActualAction->setStatusTip("Actual size");
    m_zoomActualAction->setIcon(QIcon(":/icons/zoom_actual.png"));
    
    // Search actions
    m_searchAction = new QAction("&Search", this);
    m_searchAction->setShortcut(QKeySequence::Find);
    m_searchAction->setStatusTip("Search in documents");
    m_searchAction->setIcon(QIcon(":/icons/search.png"));
    
    m_tagManagerAction = new QAction("&Tag Manager", this);
    m_tagManagerAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    m_tagManagerAction->setStatusTip("Manage tags");
    m_tagManagerAction->setIcon(QIcon(":/icons/tags.png"));
    
    m_recentDocumentsAction = new QAction("&Recent Documents", this);
    m_recentDocumentsAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    m_recentDocumentsAction->setStatusTip("Show recent documents");
    m_recentDocumentsAction->setIcon(QIcon(":/icons/recent.png"));
    
    // Create action groups
    m_toolActionGroup = new QActionGroup(this);
    m_toolActionGroup->addAction(m_addTextAction);
    m_toolActionGroup->addAction(m_addDrawingAction);
    m_toolActionGroup->addAction(m_addImageAction);
    m_toolActionGroup->addAction(m_addPDFAction);
    m_toolActionGroup->setExclusive(true);
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(m_newDocumentAction);
    fileMenu->addAction(m_openDocumentAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_saveDocumentAction);
    fileMenu->addAction(m_saveDocumentAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_closeDocumentAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);
    
    // Edit menu
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editMenu->addSeparator();
    editMenu->addAction(m_cutAction);
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(m_deleteAction);
    editMenu->addSeparator();
    editMenu->addAction(m_selectAllAction);
    
    // Page menu
    QMenu *pageMenu = menuBar()->addMenu("&Page");
    pageMenu->addAction(m_newPageAction);
    pageMenu->addAction(m_deletePageAction);
    pageMenu->addAction(m_duplicatePageAction);
    
    // Insert menu
    QMenu *insertMenu = menuBar()->addMenu("&Insert");
    insertMenu->addAction(m_addTextAction);
    insertMenu->addAction(m_addDrawingAction);
    insertMenu->addAction(m_addImageAction);
    insertMenu->addAction(m_addPDFAction);
    
    // View menu
    QMenu *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(m_zoomInAction);
    viewMenu->addAction(m_zoomOutAction);
    viewMenu->addAction(m_zoomFitAction);
    viewMenu->addAction(m_zoomActualAction);
    
    // Tools menu
    QMenu *toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction(m_searchAction);
    toolsMenu->addAction(m_tagManagerAction);
    toolsMenu->addAction(m_recentDocumentsAction);
}

void MainWindow::setupToolbars()
{
    // Main toolbar
    QToolBar *mainToolbar = addToolBar("Main");
    mainToolbar->addAction(m_newDocumentAction);
    mainToolbar->addAction(m_openDocumentAction);
    mainToolbar->addAction(m_saveDocumentAction);
    mainToolbar->addSeparator();
    mainToolbar->addAction(m_undoAction);
    mainToolbar->addAction(m_redoAction);
    mainToolbar->addSeparator();
    mainToolbar->addAction(m_cutAction);
    mainToolbar->addAction(m_copyAction);
    mainToolbar->addAction(m_pasteAction);
    mainToolbar->addSeparator();
    mainToolbar->addAction(m_deleteAction);
    
    // Object toolbar
    QToolBar *objectToolbar = addToolBar("Objects");
    objectToolbar->addAction(m_addTextAction);
    objectToolbar->addAction(m_addDrawingAction);
    objectToolbar->addAction(m_addImageAction);
    objectToolbar->addAction(m_addPDFAction);
    
    // View toolbar
    QToolBar *viewToolbar = addToolBar("View");
    viewToolbar->addAction(m_zoomInAction);
    viewToolbar->addAction(m_zoomOutAction);
    viewToolbar->addAction(m_zoomFitAction);
    viewToolbar->addAction(m_zoomActualAction);
}

void MainWindow::setupStatusBar()
{
    // Create status bar widgets
    QLabel *statusLabel = new QLabel("Ready");
    statusBar()->addWidget(statusLabel);
    
    QLabel *zoomLabel = new QLabel("100%");
    statusBar()->addPermanentWidget(zoomLabel);
    
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setVisible(false);
    statusBar()->addPermanentWidget(progressBar);
}

void MainWindow::setupConnections()
{
    // Document connections
    connect(m_note.get(), &Note::currentDocumentChanged, this, &MainWindow::onDocumentChanged);
    connect(m_note.get(), &Note::documentSaved, this, &MainWindow::onDocumentSaved);
    connect(m_note.get(), &Note::documentLoaded, this, &MainWindow::onDocumentLoaded);
    connect(m_note.get(), &Note::documentClosed, this, &MainWindow::onDocumentClosed);
    connect(m_note.get(), &Note::modifiedChanged, this, &MainWindow::onModifiedChanged);
    connect(m_note.get(), &Note::autoSaveTriggered, this, &MainWindow::onAutoSaveTriggered);
    connect(m_note.get(), &Note::storageError, this, &MainWindow::onStorageError);
    
    // Document actions
    connect(m_newDocumentAction, &QAction::triggered, this, &MainWindow::newDocument);
    connect(m_openDocumentAction, &QAction::triggered, this, &MainWindow::openDocument);
    connect(m_saveDocumentAction, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(m_saveDocumentAsAction, &QAction::triggered, this, &MainWindow::saveDocumentAs);
    connect(m_closeDocumentAction, &QAction::triggered, this, &MainWindow::closeDocument);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Page actions
    connect(m_newPageAction, &QAction::triggered, this, &MainWindow::newPage);
    connect(m_deletePageAction, &QAction::triggered, this, &MainWindow::deletePage);
    connect(m_duplicatePageAction, &QAction::triggered, this, &MainWindow::duplicatePage);
    
    // Object actions
    connect(m_addTextAction, &QAction::triggered, this, &MainWindow::addTextObject);
    connect(m_addDrawingAction, &QAction::triggered, this, &MainWindow::addDrawingObject);
    connect(m_addImageAction, &QAction::triggered, this, &MainWindow::addImageObject);
    connect(m_addPDFAction, &QAction::triggered, this, &MainWindow::addPDFObject);
    
    // Edit actions
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::undo);
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::redo);
    connect(m_cutAction, &QAction::triggered, this, &MainWindow::cut);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copy);
    connect(m_pasteAction, &QAction::triggered, this, &MainWindow::paste);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::deleteSelected);
    connect(m_selectAllAction, &QAction::triggered, this, &MainWindow::selectAll);
    
    // View actions
    connect(m_zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(m_zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(m_zoomFitAction, &QAction::triggered, this, &MainWindow::zoomFit);
    connect(m_zoomActualAction, &QAction::triggered, this, &MainWindow::zoomActual);
    
    // Search actions
    connect(m_searchAction, &QAction::triggered, this, &MainWindow::showSearchDialog);
    connect(m_tagManagerAction, &QAction::triggered, this, &MainWindow::showTagManager);
    connect(m_recentDocumentsAction, &QAction::triggered, this, &MainWindow::showRecentDocuments);
    
    // UI connections
    connect(m_documentTree, &QTreeWidget::itemClicked, this, &MainWindow::onDocumentTreeItemClicked);
    connect(m_pageTabs, &QTabWidget::currentChanged, this, &MainWindow::onPageTabChanged);
    connect(m_toolbar, &Toolbar::actionTriggered, this, &MainWindow::onToolbarActionTriggered);
}

// Document management implementations
void MainWindow::newDocument()
{
    if (!confirmClose()) return;
    
    m_note->createNewDocument("Untitled Document");
    updateWindowTitle();
    updateDocumentTree();
    updatePageTabs();
}

void MainWindow::openDocument()
{
    if (!confirmClose()) return;
    
    QStringList documentIds = m_note->listDocuments();
    if (documentIds.isEmpty()) {
        showInfoMessage("No Documents", "No documents found. Create a new document first.");
        return;
    }
    
    bool ok;
    QString documentId = QInputDialog::getItem(this, "Open Document", "Select document:", documentIds, 0, false, &ok);
    if (ok && !documentId.isEmpty()) {
        m_note->loadDocument(documentId);
    }
}

void MainWindow::saveDocument()
{
    if (m_note->saveCurrentDocument()) {
        showInfoMessage("Success", "Document saved successfully.");
    } else {
        showErrorMessage("Save Error", "Failed to save document.");
    }
}

void MainWindow::saveDocumentAs()
{
    bool ok;
    QString title = QInputDialog::getText(this, "Save Document As", "Document title:", QLineEdit::Normal, m_currentDocument ? m_currentDocument->title() : "", &ok);
    if (ok && !title.isEmpty()) {
        if (m_note->saveDocumentAs(title)) {
            showInfoMessage("Success", "Document saved successfully.");
        } else {
            showErrorMessage("Save Error", "Failed to save document.");
        }
    }
}

void MainWindow::closeDocument()
{
    if (confirmClose()) {
        m_note->closeCurrentDocument();
    }
}

// Page management implementations
void MainWindow::newPage()
{
    if (!m_currentDocument) return;
    
    bool ok;
    QString title = QInputDialog::getText(this, "New Page", "Page title:", QLineEdit::Normal, "Untitled Page", &ok);
    if (ok && !title.isEmpty()) {
        m_currentDocument->createNewPage(title);
    }
}

void MainWindow::deletePage()
{
    if (!m_currentPage) return;
    
    int ret = QMessageBox::question(this, "Delete Page", "Are you sure you want to delete this page?", QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_currentDocument->removePage(m_currentPage);
    }
}

void MainWindow::duplicatePage()
{
    if (!m_currentPage) return;
    
    int index = m_currentDocument->pageIndex(m_currentPage);
    if (index >= 0) {
        m_currentDocument->duplicatePage(index);
    }
}

// Object management implementations
void MainWindow::addTextObject()
{
    if (!m_currentPage) return;
    
    auto textObject = std::make_shared<TextObject>();
    textObject->setPosition(QPoint(100, 100));
    textObject->setSize(QSize(200, 100));
    textObject->setContent("Click to edit text...");
    
    m_currentPage->addObject(textObject);
}

void MainWindow::addDrawingObject()
{
    if (!m_currentPage) return;
    
    auto drawingObject = std::make_shared<DrawingObject>();
    drawingObject->setPosition(QPoint(100, 100));
    drawingObject->setSize(QSize(300, 200));
    
    m_currentPage->addObject(drawingObject);
}

void MainWindow::addImageObject()
{
    // TODO: Implement image object
    showInfoMessage("Not Implemented", "Image objects are not yet implemented.");
}

void MainWindow::addPDFObject()
{
    // TODO: Implement PDF object
    showInfoMessage("Not Implemented", "PDF objects are not yet implemented.");
}

// Edit operations implementations
void MainWindow::undo()
{
    // TODO: Implement undo
    showInfoMessage("Not Implemented", "Undo is not yet implemented.");
}

void MainWindow::redo()
{
    // TODO: Implement redo
    showInfoMessage("Not Implemented", "Redo is not yet implemented.");
}

void MainWindow::cut()
{
    if (!m_currentPage) return;
    
    copy();
    m_currentPage->deleteSelectedObjects();
}

void MainWindow::copy()
{
    if (!m_currentPage) return;
    
    // TODO: Implement copy to clipboard
    showInfoMessage("Not Implemented", "Copy is not yet implemented.");
}

void MainWindow::paste()
{
    if (!m_currentPage) return;
    
    // TODO: Implement paste from clipboard
    showInfoMessage("Not Implemented", "Paste is not yet implemented.");
}

void MainWindow::deleteSelected()
{
    if (!m_currentPage) return;
    
    m_currentPage->deleteSelectedObjects();
}

void MainWindow::selectAll()
{
    if (!m_currentPage) return;
    
    m_currentPage->selectAll();
}

// View operations implementations
void MainWindow::zoomIn()
{
    m_zoomFactor *= 1.2;
    m_pageCanvas->setZoomFactor(m_zoomFactor);
    updateStatusBar();
}

void MainWindow::zoomOut()
{
    m_zoomFactor /= 1.2;
    m_pageCanvas->setZoomFactor(m_zoomFactor);
    updateStatusBar();
}

void MainWindow::zoomFit()
{
    m_zoomFactor = 1.0;
    m_pageCanvas->setZoomFactor(m_zoomFactor);
    updateStatusBar();
}

void MainWindow::zoomActual()
{
    m_zoomFactor = 1.0;
    m_pageCanvas->setZoomFactor(m_zoomFactor);
    updateStatusBar();
}

// Search and navigation implementations
void MainWindow::showSearchDialog()
{
    // TODO: Implement search dialog
    showInfoMessage("Not Implemented", "Search dialog is not yet implemented.");
}

void MainWindow::showTagManager()
{
    // TODO: Implement tag manager
    showInfoMessage("Not Implemented", "Tag manager is not yet implemented.");
}

void MainWindow::showRecentDocuments()
{
    // TODO: Implement recent documents dialog
    showInfoMessage("Not Implemented", "Recent documents dialog is not yet implemented.");
}

// Event handlers
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmClose()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Handle global shortcuts
    if (event->key() == Qt::Key_Escape) {
        // Clear selection or exit current mode
        if (m_currentPage) {
            m_currentPage->clearSelection();
        }
    }
    
    QMainWindow::keyPressEvent(event);
}

// Slot implementations
void MainWindow::onDocumentChanged(std::shared_ptr<Document> document)
{
    m_currentDocument = document;
    updateWindowTitle();
    updateDocumentTree();
    updatePageTabs();
    updateActions();
}

void MainWindow::onDocumentSaved(const QString &documentId)
{
    Q_UNUSED(documentId)
    updateWindowTitle();
    updateDocumentTree();
}

void MainWindow::onDocumentLoaded(const QString &documentId)
{
    Q_UNUSED(documentId)
    updateWindowTitle();
    updateDocumentTree();
    updatePageTabs();
}

void MainWindow::onDocumentClosed()
{
    m_currentDocument.reset();
    m_currentPage.reset();
    updateWindowTitle();
    updateDocumentTree();
    updatePageTabs();
    updateActions();
}

void MainWindow::onModifiedChanged(bool modified)
{
    updateWindowTitle();
    updateActions();
}

void MainWindow::onPageChanged(std::shared_ptr<Page> page)
{
    m_currentPage = page;
    updateActions();
}

void MainWindow::onPageAdded(std::shared_ptr<Page> page, int index)
{
    Q_UNUSED(page)
    Q_UNUSED(index)
    updatePageTabs();
}

void MainWindow::onPageRemoved(std::shared_ptr<Page> page, int index)
{
    Q_UNUSED(page)
    Q_UNUSED(index)
    updatePageTabs();
}

void MainWindow::onObjectSelectionChanged()
{
    updateActions();
}

void MainWindow::onObjectAdded(std::shared_ptr<Object> object)
{
    Q_UNUSED(object)
    updateActions();
}

void MainWindow::onObjectRemoved(std::shared_ptr<Page> object)
{
    Q_UNUSED(object)
    updateActions();
}

void MainWindow::onDocumentTreeItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if (item && item->data(0, Qt::UserRole).isValid()) {
        QString documentId = item->data(0, Qt::UserRole).toString();
        m_note->loadDocument(documentId);
    }
}

void MainWindow::onPageTabChanged(int index)
{
    Q_UNUSED(index)
    // TODO: Switch to the selected page
}

void MainWindow::onToolbarActionTriggered(QAction *action)
{
    Q_UNUSED(action)
    // TODO: Handle toolbar actions
}

void MainWindow::onAutoSaveTriggered()
{
    statusBar()->showMessage("Auto-saved", 2000);
}

void MainWindow::onStorageError(const QString &error)
{
    showErrorMessage("Storage Error", error);
}

// Update methods
void MainWindow::updateWindowTitle()
{
    QString title = "NotesApp";
    if (m_currentDocument) {
        title = m_currentDocument->title() + " - " + title;
        if (m_note->isModified()) {
            title += " *";
        }
    }
    setWindowTitle(title);
}

void MainWindow::updateDocumentTree()
{
    m_documentTree->clear();
    
    QStringList documentIds = m_note->listDocuments();
    for (const QString &documentId : documentIds) {
        auto document = m_note->m_storage->loadDocument(documentId);
        if (document) {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_documentTree);
            item->setText(0, document->title());
            item->setData(0, Qt::UserRole, documentId);
            
            if (m_currentDocument && documentId == m_currentDocument->id()) {
                item->setSelected(true);
            }
        }
    }
}

void MainWindow::updatePageTabs()
{
    m_pageTabs->clear();
    
    if (!m_currentDocument) return;
    
    for (const auto &page : m_currentDocument->pages()) {
        m_pageTabs->addTab(m_pageCanvas, page->title());
    }
}

void MainWindow::updateActions()
{
    bool hasDocument = m_currentDocument != nullptr;
    bool hasPage = m_currentPage != nullptr;
    bool hasSelection = hasPage && !m_currentPage->selectedObjects().isEmpty();
    bool isModified = m_note->isModified();
    
    // Document actions
    m_saveDocumentAction->setEnabled(hasDocument && isModified);
    m_saveDocumentAsAction->setEnabled(hasDocument);
    m_closeDocumentAction->setEnabled(hasDocument);
    
    // Page actions
    m_newPageAction->setEnabled(hasDocument);
    m_deletePageAction->setEnabled(hasPage);
    m_duplicatePageAction->setEnabled(hasPage);
    
    // Object actions
    m_addTextAction->setEnabled(hasPage);
    m_addDrawingAction->setEnabled(hasPage);
    m_addImageAction->setEnabled(hasPage);
    m_addPDFAction->setEnabled(hasPage);
    
    // Edit actions
    m_cutAction->setEnabled(hasSelection);
    m_copyAction->setEnabled(hasSelection);
    m_deleteAction->setEnabled(hasSelection);
    m_selectAllAction->setEnabled(hasPage);
}

void MainWindow::updateStatusBar()
{
    // Update zoom label
    QLabel *zoomLabel = qobject_cast<QLabel*>(statusBar()->children().last());
    if (zoomLabel) {
        zoomLabel->setText(QString("%1%").arg(static_cast<int>(m_zoomFactor * 100)));
    }
}

// Helper methods
void MainWindow::showErrorMessage(const QString &title, const QString &message)
{
    QMessageBox::critical(this, title, message);
}

void MainWindow::showInfoMessage(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message);
}

bool MainWindow::confirmClose()
{
    if (m_note->isModified()) {
        int ret = QMessageBox::question(this, "Unsaved Changes", 
            "The document has unsaved changes. Do you want to save before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            return m_note->saveCurrentDocument();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::setupDocumentTree()
{
    // Document tree is already set up in setupUI()
}

void MainWindow::setupPageTabs()
{
    // Page tabs are already set up in setupUI()
}
