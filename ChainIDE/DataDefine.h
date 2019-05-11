#ifndef DATADEFINE_H
#define DATADEFINE_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <QString>
#include <QStringList>
namespace DataDefine
{
    //主题类型
    enum ThemeStyle{Black_Theme,White_Theme};
    //语言类型
    enum Language{SimChinese,English};
    //链类型--种类
    enum BlockChainClass{UB,HX,CTC};
    //链类型--测试、正式
    enum ChainType{NONE = 0,TEST = 1 << 0,FORMAL = 1 << 1};
    typedef int ChainTypes;

    //输出类型
    enum BackStageMessageType{NONE_TYPE = 0,
                              NODE_ERROR_TEST_TYPE = 1<<0,NODE_OUT_TEST_TYPE = 1<<1,
                              NODE_ERROR_FORMAL_TYPE = 1<<2,NODE_OUT_FORMAL_TYPE = 1<<3,
                              CLIENT_ERROR_TEST_TYPE = 1<<4,CLIENT_OUT_TEST_TYPE = 1<<5,
                              CLIENT_ERROR_FORMAL_TYPE = 1<<6,CLIENT_OUT_FORMAL_TYPE = 1<<7};

    //codemirror路径
    static const QString CODEMIRROR_BLACK = "codemirror/editor_black.html";
    static const QString CODEMIRROR_WHITE = "codemirror/editor_white.html";

    //帮助网页路径
    static const QString CONTRACT_HELP_PATH = "help/contract_html/index.html";
    static const QString EDITOR_HELP_PATH_CH = "help/editor_html/index_ch.html";
    static const QString EDITOR_HELP_PATH_ENG = "help/editor_html/index_eng.html";

    //合约文件后缀
    static const QString META_SUFFIX = "meta.json";
    static const QString BYTE_OUT_SUFFIX = "out";
    static const QString CONTRACT_SUFFIX = "gpc";
    static const QString GLUA_SUFFIX = "uvlua";
    static const QString JAVA_SUFFIX = "java";
    static const QString CSHARP_SUFFIX = "cs";
    static const QString KOTLIN_SUFFIX = "kt";

    //合约存放路径
    static const QString CONTRACT_DIR = "contracts";

    static const QString GLUA_DIR = CONTRACT_DIR+"/uvlua";
    static const QString JAVA_DIR = CONTRACT_DIR+"/java";
    static const QString CSHARP_DIR = CONTRACT_DIR+"/csharp";
    static const QString KOTLIN_DIR = CONTRACT_DIR+"/kotlin";

    static const QString LOCAL_CONTRACT_TEST_PATH = CONTRACT_DIR+"/contracts_test.contract";
    static const QString LOCAL_CONTRACT_FORMAL_PATH = CONTRACT_DIR+"/contracts_formal.contract";

    //链后台程序路径
    static const QString BACKSTAGE_DIR = "backstage";
#ifdef  WIN32
    static const QString UBCD_NODE_EXE = BACKSTAGE_DIR+"/ubcd.exe";
    static const QString UBCD_CLIENT_EXE = BACKSTAGE_DIR+"/ubc-cli.exe";

    static const QString LINK_NODE_EXE = BACKSTAGE_DIR+"/hx_node.exe";
    static const QString LINK_CLIENT_EXE = BACKSTAGE_DIR+"/hx_client.exe";
    static const QString LINK_NODE_TEST_EXE = BACKSTAGE_DIR+"/hx_node_test.exe";
    static const QString LINK_CLIENT_TEST_EXE = BACKSTAGE_DIR+"/hx_client_test.exe";
    static const QString LINK_TEST_CONFIG_PATH = BACKSTAGE_DIR+"/hx_test_config.ini";

    static const QString CTC_NODE_TEST_EXE = BACKSTAGE_DIR+"/ctc_test.exe";
    static const QString CTC_TEST_SCRIPT = BACKSTAGE_DIR+"/ctc_test_import_wif_keys.json";
    static const QString CTC_NODE_FORMAL_EXE = BACKSTAGE_DIR+"/ctc_formal.exe";
#else
    static const QString UBCD_NODE_EXE = BACKSTAGE_DIR+"/ubcd";
    static const QString UBCD_CLIENT_EXE = BACKSTAGE_DIR+"/ubc-cli";

    static const QString LINK_NODE_EXE = BACKSTAGE_DIR+"/hx_node";
    static const QString LINK_CLIENT_EXE = BACKSTAGE_DIR+"/hx_client";
    static const QString LINK_NODE_TEST_EXE = BACKSTAGE_DIR+"/hx_node_test";
    static const QString LINK_CLIENT_TEST_EXE = BACKSTAGE_DIR+"/hx_client_test";
    static const QString LINK_TEST_CONFIG_PATH = BACKSTAGE_DIR+"/hx_test_config.ini";

    static const QString CTC_NODE_TEST_EXE = BACKSTAGE_DIR+"/ctc_test";
    static const QString CTC_TEST_SCRIPT = BACKSTAGE_DIR+"/ctc_test_import_wif_keys.json";
    static const QString CTC_NODE_FORMAL_EXE = BACKSTAGE_DIR+"/ctc_formal";
#endif

    //编译临时路径(文件夹)
    static const QString GLUA_COMPILE_TEMP_DIR = "gluaTemp";
    static const QString JAVA_COMPILE_TEMP_DIR = "javaTemp";
    static const QString CSHARP_COMPILE_TEMP_DIR = "csharpTemp";
    static const QString KOTLIN_COMPILE_TEMP_DIR = "kotlinTemp";

    //编译工具路径
    static const QString COMPILE_DIR="compile";
#ifdef WIN32
    static const QString GLUA_COMPILE_PATH = COMPILE_DIR+"/glua/glua_compiler.exe";

    static const QString JAVA_CORE_PATH = COMPILE_DIR+"/java/gjavac-core-1.0.2-dev.jar";
    static const QString JAVA_COMPILE_PATH = COMPILE_DIR+"/java/gjavac-compiler-1.0.2-dev-jar-with-dependencies.jar";
    static const QString JAVA_UVM_ASS_PATH = COMPILE_DIR+"/java/uvm_ass.exe";
    static const QString JAVA_PACKAGE_GPC_PATH = COMPILE_DIR+"/java/package_gpc.exe";

    static const QString CSHARP_COMPILER_EXE_PATH = COMPILE_DIR+"/csharp/win/Roslyn/csc.exe";//本地csc.exe路径
    static const QString CSHARP_COMPILE_DIR = COMPILE_DIR+"/csharp/win/";
    static const QString CSHARP_JSON_DLL_PATH = COMPILE_DIR+"/csharp/win/Newtonsoft.Json.dll";
    static const QString CSHARP_CORE_DLL_PATH = COMPILE_DIR+"/csharp/win/GluaCoreLib.dll";
    static const QString CSHARP_COMPILE_PATH = COMPILE_DIR+"/csharp/win/gsharpccore.exe";

    static const QString KOTLIN_COMPILE_PATH = COMPILE_DIR+"/kotlin/kotlinc/bin/kotlinc-jvm.bat";
#else
    static const QString GLUA_COMPILE_PATH = COMPILE_DIR+"/glua/glua_compiler";

    static const QString JAVA_CORE_PATH = COMPILE_DIR+"/java/gjavac-core-1.0.2-dev.jar";
    static const QString JAVA_COMPILE_PATH = COMPILE_DIR+"/java/gjavac-compiler-1.0.2-dev-jar-with-dependencies.jar";
    static const QString JAVA_UVM_ASS_PATH = COMPILE_DIR+"/java/uvm_ass";
    static const QString JAVA_PACKAGE_GPC_PATH = COMPILE_DIR+"/java/package_gpc";

    static const QString CSHARP_COMPILER_EXE_PATH = "/usr/local/share/dotnet/dotnet";//本地csc.exe路径
    static const QString CSHARP_UVMCORE_LIB_PATH = COMPILE_DIR+"/csharp/unix/uvmcorelib.dll";
    static const QString CSHARP_PROJECT_TEMPLATE_PATH = COMPILE_DIR+"/csharp/unix/build.csproj";
    static const QString CSHARP_GSHARPCORE_DLL_PATH = COMPILE_DIR+"/csharp/unix/gsharpccore.dll";
    static const QString CSHARP_UVMASS_PATH = COMPILE_DIR+"/csharp/unix/uvm_ass";
    static const QString CSHARP_PACKAGE_PATH = COMPILE_DIR+"/csharp/unix/package_gpc";

    static const QString KOTLIN_COMPILE_PATH = COMPILE_DIR+"/kotlin/kotlinc/bin/kotlinc-jvm";

#endif

//调试器
    static const QString DEBUG_TEST_ADDRESS = "testaddress";
    static const QString DEBUG_TEST_PUBKEY = "testpubkey";
    //调试器路径
    static const QString STORAGE_DATA_FILE_NAME = "uvm_storage_demo.json";
#ifdef WIN32
    static const QString DEBUGGER_UVM_DIR = "debug/win";
    static const QString DEBUGGER_UVM_NAME = "uvm_single.exe";
#else
    static const QString DEBUGGER_UVM_DIR = "debug/unix";
    static const QString DEBUGGER_UVM_NAME = "uvm_single";
#endif
//合约api类
    class ApiEvent
    {
    public:
        ApiEvent(){

        }
        void addApi(const QString &name){
            std::lock_guard<std::mutex> lock(lockMutex);
            if(!apis.contains(name))
            {
                apis<<name;
            }
        }

        void addOfflineApi(const QString &name){
            std::lock_guard<std::mutex> lock(lockMutex);
            if(!offlineApis.contains(name))
            {
                offlineApis<<name;
            }
        }

        void addEvent(const QString &name){
            std::lock_guard<std::mutex> lock(lockMutex);
            if(!events.contains(name))
            {
                events<<name;
            }
        }

        const QStringList &getAllApiName()const{
            return apis;
        }

        const QStringList &getAllOfflineApiName()const{
            return offlineApis;
        }

        const QStringList &getAllEventName()const{
            return events;
        }
        void clear(){
            std::lock_guard<std::mutex> lock(lockMutex);
            apis.clear();
            offlineApis.clear();
            events.clear();
        }

    private:
        QStringList apis;
        QStringList offlineApis;
        QStringList events;
        std::mutex lockMutex;
    };
    typedef std::shared_ptr<DataDefine::ApiEvent> ApiEventPtr;

}

#endif // DATADEFINE_H
