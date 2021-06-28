
#ifndef PROGRAMCONFIGQTSO_PROGRAMCONFIG_HPP
#define PROGRAMCONFIGQTSO_PROGRAMCONFIG_HPP

#include "crossPlatformMacros.hpp"

#include "logsinJSONQtso/logDataHub.hpp"
#include "essentialQtso/enumClass.hpp"

#include <QObject>
#include <QByteArray>

class QJsonObject;
class translator_c;
class QCommandLineParser;

//don't call this macro inside the appConfig_c class (like calling from the ctor and hanging the process... without errors...)
//inside use MACRO_ADDMESSAGE
//MACRO_FILENAME is from the log library
#define MACRO_FILEMETA MACRO_FILENAME, __func__, __LINE__
#define MACRO_ADDLOG(...) appConfig_ptr_ext->addLogMessage_f(__VA_ARGS__, MACRO_FILEMETA)

//this class initialization rquires a QCoreApplication (or subclass) object initialized first
class EXPIMP_PROGRAMCONFIGQTSO programConfig_c : public QObject
{
    Q_OBJECT

    bool configLoaded_pri = false;
    QString configFilePath_pri;
    bool configFilePathSet_pri = false;

    QByteArray loadChecksum_pri;

    QString translationConfigFile_pri;
    bool translationConfigFileSet_pri = false;
    translator_c* translator_pri = nullptr;

    QString logsDirectoryPath_pri;
    bool logsDirectoryPathSet_pri = false;
    logDataHub_c* logDataHub_pri = nullptr;
    bool loggingEnabled_pri = false;

    void locateConfigFilePath_f(const QCommandLineParser& commandLineParser_par_con);

    bool hideDebugMessageToUser_pri = true;

    //automatically started when the exec loop starts
    void start_f();
protected:
    virtual void readJSON_f(const QJsonObject &json_par_con);
    virtual void writeJSON_f(QJsonObject &json_par) const;
    virtual void writeJSONDocumented_f(QJsonObject &json_par) const;

    virtual void derivedReadJSON_f(const QJsonObject &json_par) = 0;
    virtual void derivedWriteJSON_f(QJsonObject &json_par) const = 0;
    virtual bool requiresJSONDocumented_f() const = 0;
    //if requiresJSONDocumented_f is false this function just needs to be implemented empty
    virtual void derivedWriteJSONDocumented_f(QJsonObject &json_par) const = 0;

    bool generateJSONDocumentation_pri = false;

    virtual void derivedMessageUser_f(const text_c& message_par_con, const messageType_ec messageType_par_con) const = 0;
    virtual void derivedMessageUser_f(const textCompilation_c& message_par_con, const messageType_ec messageType_par_con) const = 0;

    //these 6 functions below are called inside loadConfigFiles_f
    virtual bool firstPositionalArgumentCanBeConfig_f() const = 0;
    //call this before parse to add options
    virtual void derivedConfigureCommandLineParser_f(QCommandLineParser& commandLineParser_par) const = 0;
    //call this after parse to check the options values
    virtual void derivedCheckCommandLineParser_f(QCommandLineParser& commandLineParser_par) = 0;
    virtual bool translationCreatePlacerholders_f() const = 0;
    //doesn't mean it's enabled by default
    virtual bool loggingSaveLogFileRequired_f() const = 0;

    virtual void derivedStart_f() = 0;
    virtual void derivedQuit_f() = 0;

    //these 3 should be loaded in order 1st loadConfigFile_f 2nd tryLoadTranslations_f / loadLogging_f (doesn't matter what goes second)
    void loadConfigFile_f();
    bool loadConfigFileCalled_pri = false;
    //this will reload from the current json config file or reload from another one
    //won't reload translation or log configuration
    //logging or translation reloads can be done by clearing the ptr using the set functions, however remember to delete the previous object,
    //still that should be rare because all? config changes for translation and logging can be achieved without reinitializing the class objects
    void reloadConfigFile_f(const QString& configFilePath_par_con = QString());
    void tryLoadTranslations_f();
    void loadLogging_f();
    //saving order doesn't matter
    bool saveConfigFile_f();
    //will only try to save if translationConfigLoaded_f is true
    bool saveTranslationFile_f();

    logDataHub_c* logDataHub_f() const;
    translator_c* translator_f() const;
    //if the derived classes want to do whatever, this doesn't parent the objects to this
    //also doesn't "clean up" if some other objects were set previously
    void setLoggingDataHubObject_f(logDataHub_c* logDataHub_par);
    void setTranslatorObject_f(translator_c* translator_par);
public:
    programConfig_c() = delete;
    programConfig_c(QObject* parent_par);

    //pure virtual forces every program using this class to implement them
    //also allows each program to decide if the user can modify any of them or not
    //why not just inherit regular class variables? because it allows lazyness, then again one might use copy+paste to overcome this
    virtual QString programName_f() const = 0;
    virtual text_c programDescription_f() const = 0;
    virtual QString programVersion_f() const = 0;

    //both translate if the translation object is set and configured
    void messageUser_f(const text_c& message_par_con, const messageType_ec messageType_par_con) const;
    void messageUser_f(const textCompilation_c& message_par_con, const messageType_ec messageType_par_con) const;

    //this is specific for the program config, logging will always be able to load and translation has translationConfigLoaded_f
    bool configLoaded_f() const;
    //if translator_pri is initialized in any way and a minimum config is set
    bool translationConfigLoaded_f() const;
    bool loggingInitialized_f() const;

    QString configFilePath_f() const;

    //these translate functions can/should be used in the whole program
    QString translate_f(const QString& key_par_con) const;
    QString translateAndReplace_f(const text_c& text_par_con) const;
    QString translateAndReplace_f(const textCompilation_c& textCompilation_par_con) const;

    //FUTURE allow to change language using translationFromToPairs_f (the "to" part)
    //Making a program like "translationParserAvidcalm" (which is another program made by me, that parses the source js/cpp files for translations)
    //but to generate skeleton json language files
    //(from language will always be hardcoded because the initial source-language is always harcoded)

    //although enabling/disabling logging can be done directly from logDataHub_f, using the set function saves the state in the config file
    bool loggingEnabled_f() const;
    void setLoggingEnabled_f(const bool enabled_par_con);

    //this log function can/should be used in the whole program
    bool addLogMessage_f(
            const text_c& message_par_con
            , const QString& reference_par_con
            , const messageType_ec logType_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const int_fast32_t line_par_con
    ) const;
    //this log function can/should be used in the whole program
    std::vector<bool> addLogMessage_f(
            const textCompilation_c& message_par_con
            , const QString& reference_par_con
            , const messageType_ec logType_par_con
            , const QString& sourceFile_par_con
            , const QString& sourceFunction_par_con
            , const int_fast32_t line_par_con
    ) const;

    std::vector<std::pair<const logItem_c* const, const QDateTime* const>> getLogs_f(const logFilter_c& logFilter_par_con = logFilter_c()) const;

    bool hideDebugMessageToUser_f() const;
    void setHideDebugMessageToUser_f(const bool hideDebugMessageToUser_par_con);

Q_SIGNALS:
    void quit_signal();
    void started_signal();
protected Q_SLOTS:
    //first ctor line QObject::connect(this, &appConfig_c::quit_signal, this, &appConfig_c::quit_f, Qt::QueuedConnection);
    void quit_f();
public Q_SLOTS:

};

#endif // PROGRAMCONFIGQTSO_PROGRAMCONFIG_HPP
