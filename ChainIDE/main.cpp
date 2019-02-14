#ifdef WIN32
#ifdef _DEBUG
#include "VisualLeakDetector/include/vld.h"
#endif
#endif

#include <QApplication>

#include "IDEUtil.h"
#include "LocalPeer.h"
#include "ChainIDE.h"
#include "mainwindow.h"

//定义windows、release下生成dump文件
#ifdef WIN32
#ifdef QT_NO_DEBUG
#include <Windows.h>
#include <DbgHelp.h>
#include <tchar.h>
#define DUMP_CORE_FILE
LONG WINAPI TopLevelExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
    HANDLE hFile = CreateFile(  _T("project.dmp"),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    MINIDUMP_EXCEPTION_INFORMATION stExceptionParam;
    stExceptionParam.ThreadId    = GetCurrentThreadId();
    stExceptionParam.ExceptionPointers = pExceptionInfo;
    stExceptionParam.ClientPointers    = FALSE;
    MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile,MiniDumpWithFullMemory,&stExceptionParam,nullptr,nullptr);
    CloseHandle(hFile);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
#endif

#ifdef WIN32
#ifdef QT_NO_DEBUG
#define LOG_DEBUG_TO_FILE
#endif
#endif

int main(int argc, char *argv[])
{
    //判断是否是第一次启动
    {
        QApplication a(argc, argv);
        LocalPeer loc;
        if(!loc.IsAlreadyRunning())
        {
#ifdef LOG_DEBUG_TO_FILE
            //安装debug导出函数
            qInstallMessageHandler(IDEUtil::myMessageOutput);
#endif
#ifdef DUMP_CORE_FILE
            //安装dumpfile
            SetUnhandledExceptionFilter(TopLevelExceptionFilter);
#endif
            //初始化单例配置等
            ChainIDE::getInstance();
            //主窗口
            MainWindow w;
            QObject::connect(&loc,&LocalPeer::newConnection,&w,&MainWindow::activateWindow);
            return a.exec();
        }
    }
    return 0;
}
