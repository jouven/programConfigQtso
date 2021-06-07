#include "programConfigConsole.hpp"

#include "essentialQtso/essentialQt.hpp"

void programConfigConsole_c::derivedMessageUser_f(const text_c& message_par_con, const messageType_ec messageType_par_con) const
{
    QString strTmp;
    strTmp = translateAndReplace_f(message_par_con);
    if (addMessageTypeInMessage_f())
    {
        strTmp = messageTypeStartSeparator_f() + messageTypeToString_f(messageType_par_con) + messageTypeEndSeparator_f() + strTmp;
    }
    qtOutLine_f(strTmp);
}

void programConfigConsole_c::derivedMessageUser_f(const textCompilation_c& message_par_con, const messageType_ec messageType_par_con) const
{
    QString strTmp;
    strTmp = translateAndReplace_f(message_par_con);
    if (addMessageTypeInMessage_f())
    {
        strTmp = messageTypeStartSeparator_f() + messageTypeToString_f(messageType_par_con) + messageTypeEndSeparator_f() + strTmp;
    }
    qtOutLine_f(strTmp);
}

programConfigConsole_c::programConfigConsole_c(QObject* parent_par)
    : programConfig_c(parent_par)
{}
