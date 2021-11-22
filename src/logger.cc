#include "logger.h"

Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}
Logger::Logger()
{
    //启动专门的写日志线程
    std::thread writeLogTask([&]()
    {
        while (true)
        {
            //获取当天的日期，然后取日志信息，
            //写入响应的文件当中
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);
            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);
            /*
            a+ 以附加方式打开可读写的文件。
            若文件不存在，则会建立该文件，
            如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。
            */
            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr)
            {
                std::cout << "logger file:" << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckQue.pop();
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s] ", nowtm->tm_hour,
                    nowtm->tm_min,
                    nowtm->tm_sec,
                    m_loglevel == INFO ? "info" : "error");
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    //设置分离线程，守护线程
    writeLogTask.detach();
}

//设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}
//写日志,把日志信息写入lockqueue缓冲区当中
void Logger::Log(std::string msg)
{
    m_lckQue.push(msg);
}
