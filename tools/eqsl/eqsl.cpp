// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

#include "eqsl.h"

#include <aqsis/aqsis.h>

#include <cstdlib>
#include <functional>
#if !defined AQSIS_SYSTEM_WIN32
#   include <signal.h>
#   include <sys/wait.h>
#   include <sys/types.h>
#endif
#ifdef AQSIS_SYSTEM_MACOSX
#   include <Carbon/Carbon.h>
#endif
#ifdef AQSIS_SYSTEM_WIN32
#define NOMINMAX
#   include <windows.h>
#endif

#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMenuBar>
#include <QtGui/QMouseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>

#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include <aqsis/version.h>
#include <aqsis/util/execute.h>


EqslMainWindow::EqslMainWindow()
    : m_commandHistory(0),
    m_consoleHistory(0),
    m_progressBar(0),
    m_currentDirectory(".")
{
    setWindowTitle("eqsl");

    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* quitAct = fileMenu->addAction(tr("&Quit"));
    quitAct->setStatusTip(tr("Quit eqsl"));
    quitAct->setShortcuts(QKeySequence::Quit);
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    // Tools menu
    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    QAction* renderAct = toolsMenu->addAction(tr("&Render File"));
    connect(renderAct, SIGNAL(triggered()), this, SLOT(renderFile()));
    QAction* compileAct = toolsMenu->addAction(tr("&Compile Shaders"));
    connect(compileAct, SIGNAL(triggered()), this, SLOT(compileShader()));
    QAction* frameBufAct = toolsMenu->addAction(tr("Open &Framebuffer"));
    connect(frameBufAct, SIGNAL(triggered()), this, SLOT(openFramebuffer()));
    toolsMenu->addSeparator();
    QAction* clearHistAct = toolsMenu->addAction(tr("Clear History"));
    connect(clearHistAct, SIGNAL(triggered()), this, SLOT(clearCommandHistory()));
    QAction* clearOutputAct = toolsMenu->addAction(tr("Clear Console"));
    connect(clearOutputAct, SIGNAL(triggered()), this, SLOT(clearConsoleHistory()));


    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* aboutAct = helpMenu->addAction(tr("&About"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    // Status bar with progress indicator
    m_progressBar = new QProgressBar();
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    statusBar()->addPermanentWidget(m_progressBar, 1);

    // Text viewers
    QTabWidget* tabs = new QTabWidget(this);
    setCentralWidget(tabs);
    m_commandHistory = new QTextEdit();
    m_commandHistory->setReadOnly(true);
    m_commandHistory->setPlainText("Command history:");
    tabs->addTab(m_commandHistory, tr("History"));
    m_consoleHistory = new QTextEdit();
    m_consoleHistory->setReadOnly(true);
    tabs->addTab(m_consoleHistory, tr("Console"));

    // Connect up signals which get emitted in other threads to the
    // appropriate slots on the GUI thread:
    connect(this, SIGNAL(sendNewStdout(const QString&)),
            this, SLOT(displayCommandStdout(const QString&)));
}


void EqslMainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Escape)
        close();
    else
        event->ignore();
}


QSize EqslMainWindow::sizeHint() const
{
    // Size hint, mainly for getting the initial window size right.
    // setMinimumSize() also sort of works for this, but doesn't allow
    // the user to later make the window smaller.
    return QSize(640,480);
}


void EqslMainWindow::aboutDialog()
{
    QString message = tr("Aqsis GUI frontend for command line tools\n"
                            "version %1.").arg(AQSIS_VERSION_STR_FULL);
    QMessageBox::information(this, tr("About eqsl"), message);
}


void EqslMainWindow::renderFile()
{
    QFileDialog fileDialog(this, tr("Select file to render"),
                           QString::fromStdString(m_currentDirectory),
                    tr("RenderMan Geometry Files (*.rib *.ribz *.rib.gz)"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    if(!fileDialog.exec())
        return;
    QStringList selectedFiles = fileDialog.selectedFiles();
    if(selectedFiles.empty())
        return;
    m_currentDirectory = fileDialog.directory().absolutePath().toStdString();
    std::string renderEngineFile = selectedFiles[0].toStdString();

    std::vector<std::string> renderEngineArgsBundle;
    renderEngineArgsBundle.push_back("-Progress");
    renderEngineArgsBundle.push_back("-nocolor");

// Call relevant commandline
#if defined AQSIS_SYSTEM_WIN32 && !defined AQSIS_COMPILER_GCC
    char acPath[256];
    char root[256];
    if( GetModuleFileName( NULL, acPath, 256 ) != 0)
    {
            // guaranteed file name of at least one character after path
            *( strrchr( acPath, '\\' ) ) = '\0';
            std::string      stracPath(acPath);
            _fullpath(root,&stracPath[0],256);
    }
    std::string program = root;
    program.append("\\");
#elif defined(AQSIS_SYSTEM_MACOSX)
    CFURLRef pluginRef = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    CFURLRef contentRef = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, pluginRef);
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    CFStringRef contentPath = CFURLCopyFileSystemPath(contentRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    const char *contentPtr = CFStringGetCStringPtr(contentPath, CFStringGetSystemEncoding());
    std::string program = pathPtr;
    program.append("/");
    renderEngineArgsBundle.push_back(std::string("-displays=") + contentPtr + "/lib");
    renderEngineArgsBundle.push_back(std::string("-shaders=") + ".:" + contentPtr + "/share/aqsis/shaders/displacement:" + contentPtr + "/share/aqsis/shaders/imager:" + contentPtr + "/share/aqsis/shaders/light:" + contentPtr + "/share/aqsis/shaders/surface:" + contentPtr + "/share/aqsis/shaders/volume");
#else
    std::string program;
#endif
    std::string renderEngine = "aqsis";
    std::string commandLine = renderEngine + " ";
    program.append(renderEngine);
    std::vector<std::string> args;
    for(std::vector<std::string>::iterator arg = renderEngineArgsBundle.begin(); arg != renderEngineArgsBundle.end(); ++arg)
    {
            args.push_back(*arg);
            commandLine += *arg + " ";
    }
    args.push_back(renderEngineFile);
    // TODO: Things would be nicer if we used QProcess here...
    Aqsis::CqExecute tool(program, args, m_currentDirectory);
    Aqsis::CqExecute::TqCallback outputStdOut = std::bind1st(
            std::mem_fun(&EqslMainWindow::collectCommandStdout), this);
    tool.setStdOutCallback(outputStdOut);
    boost::thread thread(tool);

    // Log commandline event
    commandLine += " \"" + renderEngineFile + "\"";
    m_commandHistory->append(commandLine.c_str());
}


void EqslMainWindow::compileShader()
{
    QFileDialog fileDialog(this, tr("Select shader to compile"),
                           QString::fromStdString(m_currentDirectory),
                           tr("RenderMan Shader Files (*.sl)"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    if(!fileDialog.exec())
        return;
    QStringList selectedFiles = fileDialog.selectedFiles();
    if(selectedFiles.empty())
        return;
    m_currentDirectory = fileDialog.directory().absolutePath().toStdString();
    std::string shaderCompilerFile = selectedFiles[0].toStdString();

// Call relevant commandline
#if defined AQSIS_SYSTEM_WIN32 && !defined AQSIS_COMPILER_GCC
    char acPath[256];
    char root[256];
    if( GetModuleFileName( NULL, acPath, 256 ) != 0)
    {
        // guaranteed file name of at least one character after path
        *( strrchr( acPath, '\\' ) ) = '\0';
        std::string stracPath(acPath);
        _fullpath(root,&stracPath[0],256);
    }
    std::string program = root;
    program.append("\\");
#elif defined(AQSIS_SYSTEM_MACOSX)
    CFURLRef pluginRef = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    std::string program = pathPtr;
    program.append("/");
#else
    std::string program;
#endif
    std::string shaderCompiler = "aqsl";
    std::string commandLine = shaderCompiler + " ";
    program.append(shaderCompiler);
    std::vector<std::string> args;
    std::string shaderCompilerArgs[] = {"-nocolor"};
    for(TqInt arg = 0; arg < static_cast<TqInt>(sizeof(shaderCompilerArgs)/sizeof(shaderCompilerArgs[0])); arg++)
    {
        args.push_back(shaderCompilerArgs[arg]);
        commandLine += shaderCompilerArgs[arg] + " ";
    }
    args.push_back(shaderCompilerFile);
    Aqsis::CqExecute tool(program, args, m_currentDirectory);
    Aqsis::CqExecute::TqCallback outputStdOut = std::bind1st(
            std::mem_fun(&EqslMainWindow::collectCommandStdout), this);
    tool.setStdOutCallback(outputStdOut);
    boost::thread thread(tool);
    commandLine += " \"" + shaderCompilerFile + "\"";

    // Log commandline event
    m_commandHistory->append(commandLine.c_str());
}


void EqslMainWindow::openFramebuffer()
{
    // Call relevant commandline
#if defined AQSIS_SYSTEM_WIN32 && !defined AQSIS_COMPILER_GCC
    char acPath[256];
    char root[256];
    if( GetModuleFileName( NULL, acPath, 256 ) != 0)
    {
            // guaranteed file name of at least one character after path
            *( strrchr( acPath, '\\' ) ) = '\0';
            std::string      stracPath(acPath);
            _fullpath(root,&stracPath[0],256);
    }
    std::string program = root;
    program.append("\\");

#elif defined(AQSIS_SYSTEM_MACOSX)
    CFURLRef pluginRef = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    std::string program = pathPtr;
    program.append("/");
#else
    std::string program;
#endif

    std::string renderViewer = "piqsl";
    std::string commandLine = renderViewer + " ";
    program.append(renderViewer);
#ifdef AQSIS_COMPILER_GCC
    std::vector<std::string> args;
    Aqsis::CqExecute tool(program, args, m_currentDirectory);
    Aqsis::CqExecute::TqCallback outputStdOut = std::bind1st(
            std::mem_fun(&EqslMainWindow::collectCommandStdout), this);
    tool.setStdOutCallback(outputStdOut);
    boost::thread thread(tool);
#else
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bFuncRetn = FALSE;

    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO);
    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

    char *command = "piqsl -i 127.0.0.1";
    bFuncRetn = CreateProcess(NULL,
                        command,       // command line
                        NULL,          // process security attributes
                        NULL,          // primary thread security attributes
                        TRUE,          // handles are inherited
                        IDLE_PRIORITY_CLASS,             // creation flags
                        NULL,          // use parent's environment
                        NULL,          // use parent's current directory
                        &siStartInfo,  // STARTUPINFO pointer
                        &piProcInfo);  // receives PROCESS_INFORMATION
#endif

    // Log commandline event
    m_commandHistory->append(program.c_str());
}


void EqslMainWindow::clearCommandHistory()
{
    m_commandHistory->setPlainText("Command History:");
}


void EqslMainWindow::clearConsoleHistory()
{
    m_consoleHistory->clear();
}


void EqslMainWindow::displayCommandStdout(const QString& inputQs)
{
    // NB: Call only from GUI thread!
    static boost::regex aqsisProgressExpression =
        boost::regex("^R90000[[:space:]]*([0-9.]*)%.*");
    static std::string line;
    // Buffer input until we have a line.
    std::string input = inputQs.toStdString();
    std::string::size_type index = 0, end = 0;
    while(1)
    {
        if(( end = input.find_first_of('\n', index)) != std::string::npos)
        {
            // Get line, without trailing newline
            line.append(input, index, end - index);
            index = end+1;
            boost::cmatch match;
            // Match for progress expression, if matched, elide from the
            // visible output & set progress bar.
            if(boost::regex_match(line.c_str(), match, aqsisProgressExpression))
            {
                std::string percentage;
                percentage.assign(match[1].first, match[1].second);
                try
                {
                    float fp = boost::lexical_cast<float>(percentage);
                    m_progressBar->setValue(int(fp));
                }
                catch(boost::bad_lexical_cast &)
                {
                }
            }
            else
            {
                m_consoleHistory->append(line.c_str());
            }
            line.clear();
        }
        else
        {
            line.append(input.substr(index));
            break;
        }
    }
}


void EqslMainWindow::collectCommandStdout(const char* str)
{
    QString string(str);
    emit sendNewStdout(string);
}


//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    EqslMainWindow window;
    window.show();

    return app.exec();
}
