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
