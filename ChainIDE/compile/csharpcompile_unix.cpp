#include "csharpcompile_unix.h"

#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

#include "IDEUtil.h"
#include "DataDefine.h"

class csharpCompile_unix::DataPrivate
{
public:
    DataPrivate()
    {

    }
public:
};

csharpCompile_unix::csharpCompile_unix(QObject *parent)
    :BaseCompile(parent)
    ,_p(new DataPrivate())
{

}

csharpCompile_unix::~csharpCompile_unix()
{
    delete _p;
    _p = nullptr;
}

void csharpCompile_unix::initConfig(const QString &sourceFilePath)
{
    setTempDir( QCoreApplication::applicationDirPath()+QDir::separator()+
                DataDefine::CSHARP_COMPILE_TEMP_DIR + QDir::separator() + QFileInfo(sourceFilePath).baseName()
               );

    setSourceDir( IDEUtil::getNextDir(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::CSHARP_DIR,
                                        sourceFilePath)
                 );

    //清空临时目录
    IDEUtil::deleteDir(getTempDir());

    readyBuild();
}

void csharpCompile_unix::startCompileFile(const QString &sourceFilePath)
{
    initConfig(sourceFilePath);

    //设置控制台路径为当前路径
    getCompileProcess()->setWorkingDirectory(getTempDir());

    emit CompileOutput(QString("start Compile %1").arg(getSourceDir()));
    generateProject();
}

void csharpCompile_unix::finishCompile(int exitcode, QProcess::ExitStatus exitStatus)
{
    if(QProcess::NormalExit != exitStatus)
    {
        //删除之前的文件
        QFile::remove(getDstByteFilePath());
        QFile::remove(getDstMetaFilePath());
        QFile::remove(getDstOutFilePath());

        //删除临时目录
        IDEUtil::deleteDir(getTempDir());

        emit CompileOutput(QString("compile error:stage %1").arg(getCompileStage()));
        emit errorCompileFile(getSourceDir());
        return;
    }

    switch (getCompileStage()) {
    case BaseCompile::StageOne:
        emit CompileOutput(QString("generate build.project finish."));
        generateDllFile();
        break;
    case BaseCompile::StageTwo:
        emit CompileOutput(QString("generate build.dll finish."));
        generateUVMFile();
        break;
    case BaseCompile::StageThree:
        emit CompileOutput(QString("generate build.uvms && build.meta.json finish."));
        generateOutFile();
        break;
    case BaseCompile::StageFour:
        emit CompileOutput(QString("generate build.out finish."));
        generateContractFile();
        break;
    case BaseCompile::StageFive:
        dealFinishOperate();
        break;

    default:
        break;
    }

}

void csharpCompile_unix::onReadStandardOutput()
{
    emit CompileOutput(QString::fromLocal8Bit(getCompileProcess()->readAll()));
}

void csharpCompile_unix::onReadStandardError()
{
    emit CompileOutput(QString::fromLocal8Bit(getCompileProcess()->readAllStandardError()));
}

void csharpCompile_unix::generateProject()
{
    setCompileStage(BaseCompile::StageOne);

    //先将源码拷贝到临时目录
    QStringList fileList;
    IDEUtil::GetAllFile(getSourceDir(),fileList,QStringList()<<DataDefine::CSHARP_SUFFIX);
    foreach (QString path, fileList) {
        QFile::copy(path,getTempDir()+"/"+QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix());
    }
    //dotnet运行生成build目录
    QStringList params;
    params<<"new"<<"classlib"<<"-n"<<"build";
    qDebug()<<"c#-compiler-generate build.csproj file:"<<DataDefine::CSHARP_COMPILER_EXE_PATH<<params;
    getCompileProcess()->start(DataDefine::CSHARP_COMPILER_EXE_PATH,params);

}

void csharpCompile_unix::generateDllFile()
{
    setCompileStage(BaseCompile::StageTwo);
    //替换已有的build.csproj,删除class1.cs
    QString buildProj = getTempDir()+"/build/build.csproj";
    if(QFileInfo(buildProj).exists())
    {
        QFile::remove(getTempDir()+"/build/Class1.cs");
        QFile::remove(buildProj);
        QFile::copy(QCoreApplication::applicationDirPath()+"/"+DataDefine::CSHARP_PROJECT_TEMPLATE_PATH,buildProj);
    }
    else
    {
        emit CompileOutput(QString("cannot find file %1").arg("build.csproj"));
        emit errorCompileFile(getSourceDir());
        return;
    }
    //复制uvmlib到build目录，复制源码到build目录
    QFile::copy(QCoreApplication::applicationDirPath()+"/"+DataDefine::CSHARP_UVMCORE_LIB_PATH,getTempDir()+"/build/uvmcorelib.dll");

    //先将源码拷贝到build目录
    QStringList fileList;
    IDEUtil::GetAllFile(getSourceDir(),fileList,QStringList()<<DataDefine::CSHARP_SUFFIX);
    foreach (QString path, fileList) {
        QFile::copy(path,getTempDir()+"/build/"+QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix());
    }
    //开始编译生成build.dll
    getCompileProcess()->setWorkingDirectory(getTempDir()+"/build");
    qDebug()<<"c#-compiler-generate build.dll file:"<<DataDefine::CSHARP_COMPILER_EXE_PATH<<"build";
    getCompileProcess()->start(DataDefine::CSHARP_COMPILER_EXE_PATH,QStringList()<<"build");
}

void csharpCompile_unix::generateUVMFile()
{
    setCompileStage(BaseCompile::StageThree);
    //找到build.dll
    QString buildDll = "";
    QStringList fileList;
    IDEUtil::GetAllFile(getTempDir()+"/build",fileList,QStringList()<<"dll");
    foreach (QString path, fileList) {
        if(QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix() == "build.dll")
        {
            buildDll = path;
            break;
        }
    }
    if(buildDll.isEmpty())
    {
        emit CompileOutput(QString("cannot find file %1").arg("build.dll"));
        emit errorCompileFile(getSourceDir());
        return;
    }

    //编译build.dll，生成build.uvms

    QStringList params;
    params<<QCoreApplication::applicationDirPath()+"/"+DataDefine::CSHARP_GSHARPCORE_DLL_PATH
          <<"-c"<<buildDll;
    qDebug()<<"c#-compiler-generate build.uvms file:"<<DataDefine::CSHARP_COMPILER_EXE_PATH<<params;
    getCompileProcess()->start(DataDefine::CSHARP_COMPILER_EXE_PATH,params);

}

void csharpCompile_unix::generateOutFile()
{
    setCompileStage(BaseCompile::StageFour);
    //确定build.uvms位置
    QString buildUVM = "";
    QStringList fileList;
    IDEUtil::GetAllFile(getTempDir()+"/build",fileList,QStringList()<<"dll");
    foreach (QString path, fileList) {
        if(QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix() == "build.uvms")
        {
            buildUVM = path;
            break;
        }
    }
    if(buildUVM.isEmpty())
    {
        emit CompileOutput(QString("cannot find file %1").arg("build.uvms"));
        emit errorCompileFile(getSourceDir());
        return;
    }
    //调用uvm_ass
    QStringList params;
    params<<buildUVM;
    qDebug()<<"c#-compiler-generate build.out file:"<<DataDefine::CSHARP_UVMASS_PATH<<params;
    getCompileProcess()->start(DataDefine::CSHARP_UVMASS_PATH,params);

}

void csharpCompile_unix::generateContractFile()
{
    setCompileStage(BaseCompile::StageFive);
    //确定.out .meta.json位置
    QString OutPath = "";
    QString MetaPath = "";
    QStringList fileList;
    IDEUtil::GetAllFile(getTempDir()+"/build",fileList,QStringList()<<"out"<<"meta.json");
    foreach (QString path, fileList) {
        if(QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix() == "build.out")
        {
            OutPath = path;
        }
        else if(QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix() == "build.meta.json")
        {
            MetaPath = path;
        }
    }
    if(OutPath.isEmpty() || MetaPath.isEmpty())
    {
        emit CompileOutput(QString("cannot find file %1 or %2").arg("build.out").arg("build.meta.json"));
        emit errorCompileFile(getSourceDir());
        return;
    }
    //编译build.out 和 build.meta.json,生成build.gpc
    QStringList params;
    params<<OutPath<<MetaPath;
    qDebug()<<"c#-compiler-generate build.gpc file:"<<DataDefine::CSHARP_PACKAGE_PATH<<params;
    getCompileProcess()->start(DataDefine::CSHARP_PACKAGE_PATH,params);

}

void csharpCompile_unix::dealFinishOperate()
{
    //获取gpc\out\meta.json文件
    QString OutPath = "";
    QString MetaPath = "";
    QString GpcPath = "";

    QStringList fileList;

    IDEUtil::GetAllFile(getTempDir()+"/build",fileList,QStringList()<<"gpc"<<"out"<<"meta.json");
    foreach (QString path, fileList) {
        if(QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix() == "build.gpc")
        {
            GpcPath = path;
        }
        else if(QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix() == "build.out")
        {
            OutPath = path;
        }
        else if(QFileInfo(path).baseName()+"."+QFileInfo(path).completeSuffix() == "build.meta.json")
        {
            MetaPath = path;
        }
    }

    //复制gpc meta.json文件到源目录
    QFile::copy(GpcPath,getDstByteFilePath());
    QFile::copy(MetaPath,getDstMetaFilePath());
    QFile::copy(OutPath,getDstOutFilePath());

    //删除临时目录
    IDEUtil::deleteDir(getTempDir());

    if(QFile(getDstByteFilePath()).exists())
    {
        emit CompileOutput(QString("compile finish,see %1").arg(getDstByteFilePath()));
        emit finishCompileFile(getDstByteFilePath());
    }
    else
    {
        emit CompileOutput(QString("compile error,cann't find :%1").arg(getDstByteFilePath()));
        emit errorCompileFile(getSourceDir());
    }
}

