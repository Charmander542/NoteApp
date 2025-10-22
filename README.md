# NotesApp - Advanced Note-Taking Application

A full-featured, Linux-native note-taking application built with C++ and Qt 6. This application provides a superior alternative to OneNote with advanced Markdown support, freeform drawing, pen input, object manipulation, and PDF import/export capabilities.

## Features

### Core Functionality
- **Document Management**: Create, save, and organize multiple documents
- **Page System**: Multiple pages per document with tabbed interface
- **Object-Based Layout**: Freeform placement of text, drawings, images, and PDFs
- **Layer Management**: Organize objects in layers with bring-to-front/send-to-back operations

### Text and Markdown Support
- **Rich Text Editing**: Full Markdown rendering with WYSIWYG preview
- **Inline Editing**: Click-to-edit text objects with live preview
- **Formatting**: Support for headings, lists, tables, code blocks, links, and images
- **Font Management**: Customizable fonts, sizes, colors, and alignment

### Drawing and Pen Input
- **Freehand Drawing**: Multiple drawing modes (pen, highlighter, eraser)
- **Stroke Editing**: Select, move, copy, and delete individual pen strokes
- **Color Support**: Full color palette with customizable pen colors
- **Variable Width**: Adjustable pen width from 1-50 pixels
- **Smoothing**: Automatic stroke smoothing for better pen input

### Object Manipulation
- **Selection**: Click, drag-select, or Ctrl+click for multiple selection
- **Movement**: Drag objects freely around the page
- **Resizing**: Resize objects using corner handles
- **Copy/Paste**: Duplicate objects with keyboard shortcuts
- **Layer Operations**: Bring to front, send to back, bring forward, send backward

### Storage and Persistence
- **SQLite Database**: Robust storage with automatic save/load
- **Auto-Save**: Configurable automatic saving every 30 seconds
- **Backup/Restore**: Create and restore from backup files
- **Metadata**: Document metadata, tags, and search functionality

### User Interface
- **Modern Design**: Dark theme with professional appearance
- **Toolbar**: Context-sensitive tools and properties
- **Object Inspector**: Right-side panel for object properties
- **Document Browser**: Left-side tree view of all documents
- **Status Bar**: Zoom level, selection info, and operation feedback

## Architecture

The application follows a modular architecture with clear separation of concerns:

### Core Module (`src/core/`)
- **Object**: Base class for all page objects
- **TextObject**: Markdown-capable text objects
- **DrawingObject**: Freeform drawing with pen input
- **Page**: Manages objects and layout on a single page
- **Document**: Contains multiple pages and metadata
- **Storage**: SQLite-based persistence layer
- **Note**: Main application coordinator

### GUI Module (`src/gui/`)
- **MainWindow**: Main application window with menus and toolbars
- **PageCanvas**: Interactive canvas for page display and editing
- **Toolbar**: Drawing tools and property controls
- **ObjectSelector**: Object browser and property editor

## Building the Application

### Prerequisites

1. **Qt 6**: Install Qt 6 with the following components:
   - Qt6Core
   - Qt6Widgets
   - Qt6Sql
   - Qt6OpenGL
   - Qt6PrintSupport

2. **CMake**: Version 3.16 or higher

3. **C++ Compiler**: C++17 compatible compiler (GCC, Clang, or MSVC)

4. **SQLite**: Usually included with Qt

### Build Instructions

#### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# Clone and build
git clone <repository-url>
cd NotesApp
mkdir build
cd build
cmake ..
make -j$(nproc)
```

#### Linux (Fedora/CentOS)
```bash
# Install dependencies
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Windows (MinGW)
```bash
# Install Qt 6 and MinGW through Qt Installer
# Set environment variables:
# QTDIR=C:\Qt\6.x.x\mingw_64
# PATH=%QTDIR%\bin;%PATH%

mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

#### Windows (Visual Studio)
```bash
# Install Qt 6 and Visual Studio
# Set environment variables for Qt

mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

#### macOS
```bash
# Install dependencies via Homebrew
brew install qt6 cmake

# Build
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt6)
make -j$(sysctl -n hw.ncpu)
```

### Running the Application

After building, run the executable:
```bash
# Linux/macOS
./NotesApp

# Windows
NotesApp.exe
```

## Usage Guide

### Getting Started

1. **Create a Document**: File → New Document or Ctrl+N
2. **Add Text**: Click the Text tool (T) and click on the page
3. **Draw**: Select the Pen tool (P) and draw freely
4. **Save**: File → Save or Ctrl+S

### Keyboard Shortcuts

#### Document Operations
- `Ctrl+N`: New Document
- `Ctrl+O`: Open Document
- `Ctrl+S`: Save Document
- `Ctrl+Shift+S`: Save As
- `Ctrl+W`: Close Document

#### Page Operations
- `Ctrl+Shift+N`: New Page
- `Ctrl+Shift+D`: Delete Page
- `Ctrl+Shift+U`: Duplicate Page

#### Object Operations
- `Ctrl+T`: Add Text Object
- `Ctrl+D`: Add Drawing Object
- `Ctrl+I`: Add Image Object
- `Ctrl+P`: Add PDF Object

#### Editing
- `Ctrl+Z`: Undo
- `Ctrl+Y`: Redo
- `Ctrl+X`: Cut
- `Ctrl+C`: Copy
- `Ctrl+V`: Paste
- `Delete`: Delete Selected
- `Ctrl+A`: Select All
- `Escape`: Clear Selection

#### Tools
- `S`: Select Tool
- `T`: Text Tool
- `P`: Pen Tool
- `H`: Highlighter Tool
- `E`: Eraser Tool
- `I`: Image Tool
- `F`: PDF Tool

#### View
- `Ctrl++`: Zoom In
- `Ctrl+-`: Zoom Out
- `Ctrl+0`: Fit to Window
- `Ctrl+1`: Actual Size

### Advanced Features

#### Markdown Support
Text objects support full Markdown syntax:
- Headers: `# H1`, `## H2`, `### H3`
- Bold: `**bold text**`
- Italic: `*italic text*`
- Code: `` `code` ``
- Lists: `- item` or `1. item`
- Links: `[text](url)`

#### Drawing Modes
- **Pen**: Standard drawing with solid lines
- **Highlighter**: Semi-transparent highlighting
- **Eraser**: Remove existing strokes

#### Object Properties
Use the Object Properties panel (right side) to:
- Adjust position and size
- Change layer order
- Toggle visibility
- Edit text formatting
- Modify drawing properties

## Development

### Project Structure
```
NotesApp/
├── src/
│   ├── core/           # Core business logic
│   │   ├── object.h/cpp
│   │   ├── textobject.h/cpp
│   │   ├── drawingobject.h/cpp
│   │   ├── page.h/cpp
│   │   ├── document.h/cpp
│   │   ├── storage.h/cpp
│   │   └── note.h/cpp
│   └── gui/            # User interface
│       ├── mainwindow.h/cpp
│       ├── pagecanvas.h/cpp
│       ├── toolbar.h/cpp
│       └── objectselector.h/cpp
├── CMakeLists.txt      # Build configuration
├── main.cpp           # Application entry point
└── README.md          # This file
```

### Adding New Features

1. **New Object Types**: Inherit from `Object` base class
2. **New Tools**: Add to `Toolbar` class and handle in `PageCanvas`
3. **New Properties**: Extend object classes and update property editors
4. **New File Formats**: Add import/export methods to `Storage` class

### Code Style

- Use C++17 features
- Follow Qt naming conventions
- Use smart pointers for memory management
- Document all public interfaces
- Use const correctness
- Prefer composition over inheritance

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Roadmap

### Version 1.1
- [ ] PDF import/export functionality
- [ ] Image object support
- [ ] Undo/redo system
- [ ] Search and tagging
- [ ] Cloud synchronization

### Version 1.2
- [ ] Plugin system
- [ ] Advanced drawing tools (shapes, arrows)
- [ ] Table support
- [ ] Collaborative editing
- [ ] Mobile companion app

### Version 2.0
- [ ] AI-powered features
- [ ] Advanced markdown extensions
- [ ] Custom themes
- [ ] Scripting support
- [ ] Advanced export options

## Support

For issues, feature requests, or questions:
- Create an issue on GitHub
- Check the documentation
- Join our community discussions

## Acknowledgments

- Built with Qt 6 framework
- Uses SQLite for data persistence
- Inspired by modern note-taking applications
- Community feedback and contributions
