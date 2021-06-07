
#ifndef PROGRAMCONFIGQTSO_PROGRAMCONFIGCONSOLE_HPP
#define PROGRAMCONFIGQTSO_PROGRAMCONFIGCONSOLE_HPP

#include "crossPlatformMacros.hpp"
#include "programConfig.hpp"

#include "textQtso/text.hpp"

#include <QObject>
#include <QString>

class EXPIMP_PROGRAMCONFIGQTSO programConfigConsole_c : public programConfig_c
{
    Q_OBJECT

    virtual void derivedMessageUser_f(const text_c& message_par_con, const messageType_ec messageType_par_con) const override;
    virtual void derivedMessageUser_f(const textCompilation_c& message_par_con, const messageType_ec messageType_par_con) const override;
public:
    programConfigConsole_c(QObject* parent_par);

    virtual QString messageTypeStartSeparator_f() const
    {
        return "[";
    }
    virtual QString messageTypeEndSeparator_f() const
    {
        //I don't want to add another gimmick to make the message and the separator have a whitespace inbetween
        return "] ";
    }

    virtual bool addMessageTypeInMessage_f() const = 0;
};

#endif // PROGRAMCONFIGQTSO_PROGRAMCONFIGCONSOLE_HPP
