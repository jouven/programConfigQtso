#include "programConfig.hpp"

#include "essentialQtso/essentialQt.hpp"
#include "translatorJSONQtso/translator.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QCommandLineParser>

void programConfig_c::readJSON_f(const QJsonObject& json_par_con)
{
    if (not json_par_con["translationConfigFile"].isUndefined())
    {
        translationConfigFileSet_pri = true;
        translationConfigFile_pri = json_par_con["translationConfigFile"].toString();
    }
    if (not json_par_con["logsDirectoryPath"].isUndefined())
    {
        logsDirectoryPathSet_pri = true;
        logsDirectoryPath_pri = json_par_con["logsDirectoryPath"].toString();
    }
    if (not json_par_con["loggingEnabled"].isUndefined())
    {
        loggingEnabled_pri = json_par_con["loggingEnabled"].toBool(loggingEnabled_pri);
    }
    derivedReadJSON_f(json_par_con);
}

void programConfig_c::writeJSONDocumented_f(QJsonObject& json_par) const
{
    json_par["translationConfigFile"] = translate_f("Path to translation file, the file can be empty or not exist (it will be created with placeholders)");
    json_par["logsDirectoryPath"] = translate_f("Directory path to save this program log files");
    json_par["loggingEnabled"] = translate_f("true, log extra information in the log file, it's not recommended to enable for regular use, it will fill the log file with information of each notification, false, no logging, default is false");
    derivedWriteJSONDocumented_f(json_par);
}

void programConfig_c::writeJSON_f(QJsonObject& json_par) const
{
    if (translationConfigFileSet_pri)
    {
        json_par["translationConfigFile"] = translationConfigFile_pri;
    }
    if (logsDirectoryPathSet_pri)
    {
        json_par["logsDirectoryPath"] = logsDirectoryPath_pri;
    }
    json_par["loggingEnabled"] = loggingEnabled_pri;
    derivedWriteJSON_f(json_par);
}

logDataHub_c* programConfig_c::logDataHub_f() const
{
    return logDataHub_pri;
}

translator_c* programConfig_c::translator_f() const
{
    return translator_pri;
}

void programConfig_c::quit_f()
{
    //signals can't be private but at least this can be done
    if (sender() == this)
    {
        derivedQuit_f();
        //quits the exec loop
        qApp->quit();
    }
}

bool programConfig_c::loggingEnabled_f() const
{
    return loggingEnabled_pri;
}

void programConfig_c::setLoggingEnabled_f(const bool enabled_par_con)
{
    loggingEnabled_pri = enabled_par_con;
    if (loggingInitialized_f())
    {
        logDataHub_pri->setLoggingEnabled_f(loggingEnabled_pri);
    }
}

bool programConfig_c::translationConfigLoaded_f() const
{
    bool resultTmp(false);
    while (true)
    {
        if (translator_pri == nullptr)
        {
            break;
        }

        if (translator_pri->isConfigSet_f())
        {
            break;
        }
        resultTmp = true;
        break;
    }
    return resultTmp;
    //return translator_pri not_eq nullptr and translator_pri->isConfigSet_f();
}

bool programConfig_c::loggingInitialized_f() const
{
    return logDataHub_pri not_eq nullptr;
}

QString programConfig_c::configFilePath_f() const
{
    return configFilePath_pri;
}

//don't log anything here because log should be initialized always after translations
void programConfig_c::tryLoadTranslations_f()
{
    if (translator_pri not_eq nullptr)
    {
        return;
    }

    //check json field else try default path
    textCompilation_c messagesTmp(true);
    if (translationConfigFile_pri.isEmpty())
    {
        translationConfigFile_pri = fileTypePath_f(fileTypes_ec::translationConfig);
    }
    else
    {
        if (not QFile::exists(translationConfigFile_pri))
        {
            messagesTmp.append_f({"Translation config file path {0} doesn't exist", translationConfigFile_pri});
        }
    }

    if (QFile::exists(translationConfigFile_pri) or translationCreatePlacerholders_f())
    {
        translator_pri = new translator_c;
        translator_pri->setParent(this);
        if (QFile::exists(translationConfigFile_pri))
        {
            translator_pri->readConfigJSONFile_f(translationConfigFile_pri);
        }

        translator_pri->setAddNotFoundKeys_f(translationCreatePlacerholders_f());
        if (not translator_pri->isConfigSet_f())
        {
            if (translator_pri->addNotFoundKeys_f() )
            {
                //minimum to make translator_pri->isConfigSet_f() = true and "allow translations"
                translator_pri->addEmptyLanguageLink_f("hard-coded", "english");
                translator_pri->setTranslateFromLanguage_f("hard-coded");
                translator_pri->setTranslateToLanguageChain_f({"english"});

                messageUser_f(R"(Translation/s not loaded, adding "empty", "hard-coded"-"english" translation)", messageType_ec::debug);
                //MACRO_ADDMESSAGE((*logDataHub_pri), R"(Translation/s not loaded, adding "empty", "hard-coded"-"english" translation)", messageType_ec::information);
            }
            else
            {
                messagesTmp.append_f("Translation config not set, there will be no translation/s or placeholder creation");
            }
        }
    }
    else
    {
        messagesTmp.append_f({"Translation config file path {0} doesn't exist", translationConfigFile_pri});
    }

    if (not messagesTmp.empty_f())
    {
        messageUser_f(messagesTmp, messageType_ec::warning);
    }
}

//
void programConfig_c::loadLogging_f()
{
    if (logDataHub_pri not_eq nullptr)
    {
        return;
    }

    //log can always happen, the thing that can go wrong is saving/loading from a log file
    logDataHub_pri = new logDataHub_c;
    logDataHub_pri->setParent(this);

    if (loggingSaveLogFileRequired_f())
    {
        //try to set the log path from the config file field
        logDataHub_pri->setLogSaveDirectoryPath_f(logsDirectoryPath_pri);
        if (logDataHub_pri->isValidLogPathBaseName_f())
        {
            logDataHub_pri->setSaveLogFiles_f(true);
        }
        //no need for this since the class will return the actual error
//        else
//        {
//            messagesTmp.append_f(text_c("Log custom path couldn't be set to: {0}", logsDirectoryPath_pri));
//        }
        if (logDataHub_pri->anyError_f())
        {
            messageUser_f(logDataHub_pri->getErrors_f(), messageType_ec::warning);
        }
//        else
//        {
//            messagesTmp.append_f(text_c("Using log path: {0}", logDataHub_pri->logSaveDirectoryPath_f()));
//        }

        //try loading any previous log file from the log path from the config file field else try it from the default path
        if (logDataHub_pri->isValidLogPathBaseName_f() and logsDirectoryPathSet_pri)
        {
            logDataHub_pri->loadLogFiles_f(logsDirectoryPath_pri, logFilter_c(), true, true);
        }
        else
        {
            //this will set, if it's not set, the log path to the default one
            logDataHub_pri->loadLogFiles_f(QString(), logFilter_c(), true, true);
        }
        if (logDataHub_pri->anyError_f())
        {
            messageUser_f(logDataHub_pri->getErrors_f(), messageType_ec::error);
        }
    }

    logDataHub_pri->setLoggingEnabled_f(loggingEnabled_pri);

    //log some pending stuff
    if (translator_pri not_eq nullptr and translator_pri->isConfigSet_f())
    {
        MACRO_ADDMESSAGE((*logDataHub_pri), "Translation/s loaded successful", messageType_ec::information);
    }

    if (configLoaded_pri)
    {
        MACRO_ADDMESSAGE((*logDataHub_pri), "App config loaded successfully", messageType_ec::information);
    }
}

void programConfig_c::locateConfigFilePath_f(
        const QCommandLineParser& commandLineParser_par_con)
{
    constexpr fileTypes_ec fileTypeTmp_constexpr(fileTypes_ec::config);

    textCompilation_c messagesTmp(true);
    //first argument case
    if (firstPositionalArgumentCanBeConfig_f())
    {
        const QStringList parsedPositionalArgs(commandLineParser_par_con.positionalArguments());
        if (parsedPositionalArgs.size() > 0)
        {
            const QString& configjsonAlternativePathStr(parsedPositionalArgs.at(0));
            while (true)
            {
                if (configjsonAlternativePathStr.isEmpty())
                {
                    messagesTmp.append_f("Config file path is empty");
                    break;
                }

                if (not QFile::exists(configjsonAlternativePathStr))
                {
                    messagesTmp.append_f(text_c("Config file path {0} doesn't exist (1)", configjsonAlternativePathStr));
                    break;
                }
                configFilePath_pri = configjsonAlternativePathStr;
                configFilePathSet_pri = true;
                break;
            }
        }
    }

    //--configFile="somePath" case
    if (not configFilePathSet_pri and commandLineParser_par_con.isSet("configFile"))
    {
        QString configjsonAlternativePathStr(commandLineParser_par_con.value("configFile"));
        while (true)
        {
            if (configjsonAlternativePathStr.isEmpty())
            {
                messagesTmp.append_f("Config file path is empty");
                break;
            }

            if (not QFile::exists(configjsonAlternativePathStr))
            {
                messagesTmp.append_f({"Config file path {0} doesn't exist (2)", configjsonAlternativePathStr});
                break;
            }
            configFilePath_pri = configjsonAlternativePathStr;
            configFilePathSet_pri = true;
            break;
        }
    }

    //check default path case
    if (not configFilePathSet_pri)
    {
        if (not QFile::exists(fileTypePath_f(fileTypeTmp_constexpr)))
        {
            messagesTmp.append_f({"Config file path {0} doesn't exist (3)", fileTypePath_f(fileTypeTmp_constexpr)});
        }
        else
        {
            configFilePath_pri = fileTypePath_f(fileTypeTmp_constexpr);
            configFilePathSet_pri = true;
        }
    }

    if (not configFilePathSet_pri)
    {
        if (not commandLineParser_par_con.isSet("configFile"))
        {
            messagesTmp.append_f("No --configFile argument provided");
        }

        if (not messagesTmp.empty_f())
        {
            messageUser_f(messagesTmp, messageType_ec::error);
        }
    }
}

bool programConfig_c::hideDebugMessageToUser_f() const
{
    return hideDebugMessageToUser_pri;
}

void programConfig_c::setHideDebugMessageToUser_f(const bool hideDebugMessageToUser_par_con)
{
    hideDebugMessageToUser_pri = hideDebugMessageToUser_par_con;
}

void programConfig_c::setLoggingDataHubObject_f(logDataHub_c* logDataHub_par)
{
    logDataHub_pri = logDataHub_par;
}

void programConfig_c::setTranslatorObject_f(translator_c* translator_par)
{
    translator_pri = translator_par;
}

programConfig_c::programConfig_c(QObject* parent_par)
    : QObject(parent_par)
{
    QObject::connect(this, &programConfig_c::quit_signal, this, &programConfig_c::quit_f, Qt::QueuedConnection);
    QTimer::singleShot(0, this, &programConfig_c::start_f);
}

void programConfig_c::loadConfigFile_f()
{
    if (loadConfigFileCalled_pri)
    {
        return;
    }

    {
        QCommandLineParser parser;
        QCommandLineOption configFilePathOption("configFile", "Config file path", "configFilePath", "config.json");
        parser.addOption(configFilePathOption);
        parser.addHelpOption();
        //.process doesn't allow parsing more than once
        //.parse allows parsing more than once plus doesn't do error handling (see qt doc)
        parser.parse(qApp->arguments());

        locateConfigFilePath_f(parser);
    }

    //(try) load config
    while (configFilePathSet_pri)
    {
        QFile configFileLoad(configFilePath_pri);
        QByteArray jsonByteArray;
        if (configFileLoad.open(QIODevice::ReadOnly))
        {
            jsonByteArray = configFileLoad.readAll();
        }
        else
        {
            messageUser_f({"Could not open config file {0}", configFilePath_pri}, messageType_ec::error);
            break;
        }

        QJsonDocument jsonDocObj(QJsonDocument::fromJson(jsonByteArray));
        if (jsonDocObj.isNull())
        {
            messageUser_f({"Config file {0} json is null", configFilePath_pri}, messageType_ec::error);
            break;
        }

        readJSON_f(jsonDocObj.object());
        loadChecksum_pri = QCryptographicHash::hash(jsonByteArray, QCryptographicHash::Md5);
        configLoaded_pri = true;
        break;
    }

    {
        QCommandLineParser parser;
        if (translator_pri not_eq nullptr and translator_pri->isConfigSet_f())
        {
            if (not programDescription_f().empty_f())
            {
                parser.setApplicationDescription(translateAndReplace_f(programDescription_f()));
            }
            QCommandLineOption configFilePathOption("configFile", translate_f("Config file path"), "configFilePath", "config.json");
            parser.addOption(configFilePathOption);
            if (requiresJSONDocumented_f())
            {
                QCommandLineOption generateConfigFileDocumentationOption("g", translate_f("Generate config file/s documentation"));
                parser.addOption(generateConfigFileDocumentationOption);
            }
        }
        else
        {
            if (not programDescription_f().empty_f())
            {
                parser.setApplicationDescription(programDescription_f().rawReplace_f());
            }
            QCommandLineOption configFilePathOption("configFile", "Config file path", "configFilePath", "config.json");
            parser.addOption(configFilePathOption);
            if (requiresJSONDocumented_f())
            {
                QCommandLineOption generateConfigFileDocumentationOption("g", "Generate config file/s documentation");
                parser.addOption(generateConfigFileDocumentationOption);
            }
        }

        derivedConfigureCommandLineParser_f(parser);

        parser.addVersionOption();
        parser.addHelpOption();
        parser.process(qApp->arguments());

        if (requiresJSONDocumented_f())
        {
            generateJSONDocumentation_pri = parser.isSet("g");
        }

        derivedCheckCommandLineParser_f(parser);
    }
    loadConfigFileCalled_pri = true;
}

void programConfig_c::reloadConfigFile_f(const QString& configFilePath_par_con)
{
    QString fileToReloadTmp;
    if (configFilePath_par_con.isEmpty())
    {
        //reload current
        fileToReloadTmp = configFilePath_pri;
    }
    else
    {
        fileToReloadTmp = configFilePath_par_con;
    }

    //(try) load config
    while (true)
    {
        QFileInfo configFilePathFileInfo(fileToReloadTmp);
        if (configFilePathFileInfo.exists())
        {
            //exists
        }
        else
        {
            messageUser_f({"Config file to reload doesn't exist {0} (1)", fileToReloadTmp}, messageType_ec::error);
            break;
        }

        if (configFilePathFileInfo.isFile())
        {
            //it's a file
        }
        else
        {
            messageUser_f({"Config file to reload is not a file {0} (2)", fileToReloadTmp}, messageType_ec::error);
            break;
        }

        QFile configFileLoad(fileToReloadTmp);
        QByteArray jsonByteArray;
        if (configFileLoad.open(QIODevice::ReadOnly))
        {
            jsonByteArray = configFileLoad.readAll();
        }
        else
        {
            messageUser_f({"Could not open config file {0}", fileToReloadTmp}, messageType_ec::error);
            break;
        }

        QJsonDocument jsonDocObj(QJsonDocument::fromJson(jsonByteArray));
        if (jsonDocObj.isNull())
        {
            messageUser_f({"Config file {0} json is null", fileToReloadTmp}, messageType_ec::error);
            break;
        }

        readJSON_f(jsonDocObj.object());
        loadChecksum_pri = QCryptographicHash::hash(jsonByteArray, QCryptographicHash::Md5);
        configLoaded_pri = true;
        break;
    }
}

void programConfig_c::messageUser_f(const text_c& message_par_con, const messageType_ec messageType_par_con) const
{
    if (hideDebugMessageToUser_pri and messageType_par_con == messageType_ec::debug)
    {
        //hide it from the user but add it to the logs
        if (logDataHub_pri not_eq nullptr)
        {
            addLogMessage_f(message_par_con, QString(), messageType_par_con, MACRO_FILEMETA);
        }
    }
    else
    {
        derivedMessageUser_f(message_par_con ,messageType_par_con);
    }
}

void programConfig_c::messageUser_f(const textCompilation_c& message_par_con, const messageType_ec messageType_par_con) const
{
    if (hideDebugMessageToUser_pri and messageType_par_con == messageType_ec::debug)
    {
        //hide it from the user but add it to the logs
        if (logDataHub_pri not_eq nullptr)
        {
            addLogMessage_f(message_par_con, QString(), messageType_par_con, MACRO_FILEMETA);
        }
    }
    else
    {
        derivedMessageUser_f(message_par_con ,messageType_par_con);
    }
}

bool programConfig_c::saveConfigFile_f()
{
    bool configSavedTmp(false);
    while (true)
    {
        QString configFileStr;
        if (configFilePathSet_pri)
        {
            configFileStr = configFilePath_pri;
        }
        else
        {
            configFileStr = fileTypePath_f(fileTypes_ec::config);
        }

        QJsonObject jsonObjectTmp;
        writeJSON_f(jsonObjectTmp);
        QJsonDocument jsonDocumentTmp(jsonObjectTmp);
        QByteArray jsonByteArray(jsonDocumentTmp.toJson(QJsonDocument::Indented));

        if (QCryptographicHash::hash(jsonByteArray, QCryptographicHash::Md5) not_eq loadChecksum_pri)
        {
            QFile configFileSaveTmp(configFileStr);
            if (configFileSaveTmp.open(QIODevice::WriteOnly))
            {
                if (configFileSaveTmp.write(jsonByteArray) == -1)
                {
                    messageUser_f({"Error when writing file {0}", configFileStr}, messageType_ec::error);
                    break;
                }
                else
                {
                    configSavedTmp = true;
                }
            }
            else
            {
                messageUser_f({"Error when opening file {0} during save", configFileStr}, messageType_ec::error);
                break;
            }
        }

        break;
    }

    return configSavedTmp;
}

bool programConfig_c::saveTranslationFile_f()
{
    bool resultTmp(false);
    //translator_pri->setAddNotFoundKeys_f(translationCreatePlacerholders_f());
    if (translationConfigLoaded_f())
    {
        QString errorTmp;
        resultTmp = translator_pri->writeConfigJSONFile_f(fileTypePath_f(fileTypes_ec::translationConfig), true, std::addressof(errorTmp));
        if (not errorTmp.isEmpty())
        {
            messageUser_f(errorTmp, messageType_ec::error);
        }
    }
    return resultTmp;
}

void programConfig_c::start_f()
{
    if (not programName_f().isEmpty())
    {
        QCoreApplication::setApplicationName(programName_f());
    }
    if (not programVersion_f().isEmpty())
    {
        QCoreApplication::setApplicationVersion(programVersion_f());
    }

    while (generateJSONDocumentation_pri)
    {
        QString exampleFilenameStr(fileTypeBasePath_f(fileTypes_ec::config) + "_example" + fileTypeToExtension_f(fileTypes_ec::config));

        if (QFile::exists(exampleFilenameStr))
        {
            messageUser_f({"An example file {0} already exists", exampleFilenameStr}, messageType_ec::information);
            break;
        }

        QFile exampleFile(exampleFilenameStr);
        if (exampleFile.open(QIODevice::WriteOnly))
        {
            QJsonObject tmp;
            writeJSONDocumented_f(tmp);
            QJsonDocument jsonDocTmp(tmp);
            QByteArray jsonByteArray(jsonDocTmp.toJson(QJsonDocument::JsonFormat::Indented));
            if (exampleFile.write(jsonByteArray) not_eq -1)
            {
                messageUser_f({"Example file {0} generated successfully", exampleFilenameStr}, messageType_ec::information);
            }
            else
            {
                messageUser_f({"Error writing example file {0}", exampleFilenameStr}, messageType_ec::error);
            }
        }
        else
        {
            messageUser_f({"Can't create/write example file {0}", exampleFilenameStr}, messageType_ec::error);
        }

        break;
    }

    derivedStart_f();

    Q_EMIT started_signal();
}

bool programConfig_c::configLoaded_f() const
{
    return configLoaded_pri;
}

QString programConfig_c::translate_f(const QString& key_par_con) const
{
    QString resultTmp;
    bool foundTmp(false);
    if (translationConfigLoaded_f())
    {
        resultTmp = translator_pri->translate_f(key_par_con, std::addressof(foundTmp));
    }
    //this is already done when addNotFoundKeys in the translator_c object is true
    //it is done like this because otherwise a lot of texts/labels will be empty
    if (not foundTmp)
    {
        resultTmp = key_par_con;
    }
    return resultTmp;
}

QString programConfig_c::translateAndReplace_f(const text_c& text_par_con) const
{
    QString resultTmp;
    bool foundTmp(false);
    if (translationConfigLoaded_f())
    {
        resultTmp = translator_pri->translateAndReplace_f(text_par_con, std::addressof(foundTmp));
    }
    //this is already done when addNotFoundKeys in the translator_c object is true
    //it is done like this because otherwise a lot of texts/labels will be empty
    if (not foundTmp)
    {
        resultTmp = text_par_con.rawReplace_f();
    }
    return resultTmp;
}

QString programConfig_c::translateAndReplace_f(const textCompilation_c& textCompilation_par_con) const
{
    QString resultTmp;
    if (translationConfigLoaded_f())
    {
        resultTmp = translator_pri->translateAndReplaceToString_f(textCompilation_par_con);
    }
    else
    {
        resultTmp = textCompilation_par_con.toRawReplace_f();
    }
    return resultTmp;
}

bool programConfig_c::addLogMessage_f(
        const text_c& message_par_con
        , const QString& reference_par_con
        , const messageType_ec logType_par_con
        , const QString& sourceFile_par_con
        , const QString& sourceFunction_par_con
        , const int_fast32_t line_par_con
) const
{
    bool resultTmp(false);
    if (loggingInitialized_f())
    {
        resultTmp = logDataHub_pri->addMessage_f(message_par_con, reference_par_con, logType_par_con, sourceFile_par_con, sourceFunction_par_con, line_par_con);
    }
    return resultTmp;
}

std::vector<bool> programConfig_c::addLogMessage_f(
        const textCompilation_c& message_par_con
        , const QString& reference_par_con
        , const messageType_ec logType_par_con
        , const QString& sourceFile_par_con
        , const QString& sourceFunction_par_con
        , const int_fast32_t line_par_con) const
{
    std::vector<bool> resultTmp;
    if (loggingInitialized_f())
    {
        resultTmp = logDataHub_pri->addMessage_f(message_par_con, reference_par_con, logType_par_con, sourceFile_par_con, sourceFunction_par_con, line_par_con);
    }
    return resultTmp;
}

std::vector<std::pair<const logItem_c* const, const QDateTime* const>> programConfig_c::getLogs_f(const logFilter_c& logFilter_par_con) const
{
    std::vector<std::pair<const logItem_c* const, const QDateTime* const>> resultTmp;
    if (loggingInitialized_f())
    {
        resultTmp = logDataHub_pri->filter_f(logFilter_par_con);
    }
    return resultTmp;
}
