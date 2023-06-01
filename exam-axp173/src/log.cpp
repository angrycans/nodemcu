
#include "log.h"

char *mdCheckLevel(const int level)
{ //返回对应的日志等级字符串
    if (level == LOG_DEBUG)
    {
        return "DEBUG";
    }
    else if (level == LOG_INFO)
    {
        return "INFO";
    }
    else if (level == LOG_WARING)
    {
        return "WARING";
    }
    else if (level == LOG_ERROR)
    {
        return "ERROR";
    }
    return "UNKNOW";
}

void logPrintf(const int level, const char *fun, const int line, const char *date, const char *time, const char *fmt, ...)
{
    //"..." :该格式支持传入多个参数，传入时需要用逗号隔开
    va_list arg;                                //声明"..."传入值的存储列表
    va_start(arg, fmt);                         //绑定传入的值与传入的输出格式fmt
    char buf[1 + vsnprintf(NULL, 0, fmt, arg)]; // buf大小由vsnprintf()函数返回，由于 "\n" 结束符存在，因此需要 "+1"
    vsnprintf(buf, sizeof(buf), fmt, arg);      //缓冲区打印（打印到指定缓冲区buf中）
    va_end(arg);                                //结束绑定

    if (level >= LOG_LEVEL)
    {                                                                            //判断输出的日志等级
        Serial.printf("\n[%s][%s %d]: %s", mdCheckLevel(level), fun, line, buf); //打印到串口
    }
}
