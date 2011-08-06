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

#ifndef AQSIS_EQSL_H_INCLUDED
#define AQSIS_EQSL_H_INCLUDED

#include <QtGui/QMainWindow>
#include <QtGui/QTextEdit>
#include <QtGui/QProgressBar>
#include <string>

class EqslMainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        EqslMainWindow();

    protected:
        void keyReleaseEvent(QKeyEvent* event);

        QSize sizeHint() const;

    signals:
        void sendNewStdout(const QString& str);

    private slots:
        void aboutDialog();

        void renderFile();
        void compileShader();
        void openFramebuffer();

        void clearCommandHistory();
        void clearConsoleHistory();

        void displayCommandStdout(const QString& input);

    private:
        void collectCommandStdout(const char* str);

        QTextEdit* m_commandHistory;
        QTextEdit* m_consoleHistory;
        QProgressBar* m_progressBar;

        std::string m_currentDirectory;
};


#endif // AQSIS_EQSL_H_INCLUDED
