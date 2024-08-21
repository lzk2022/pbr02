#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>

// 宏函数，调用日志函数进行输出
#define LOG_INFO(format, ...) Log::Write(Level::Info,Log::Format(format,__VA_ARGS__));
#define LOG_WARN(format, ...) Log::Write(Level::Warm,Log::Format(format,__VA_ARGS__),__LINE__,__FUNCTION__,__FILE__);
#define LOG_ERROR(format, ...) Log::Write(Level::Error,Log::Format(format,__VA_ARGS__),__LINE__,__FUNCTION__,__FILE__);
#define LOG_ASSERT_TRUE(condition,format,...) if(condition) Log::Write(Level::Assert,Log::Format(format,__VA_ARGS__),__LINE__,__FUNCTION__,__FILE__);
#define LOG_ASSERT(condition,format,...) if(!(condition)) Log::Write(Level::Assert,Log::Format(format,__VA_ARGS__),__LINE__,__FUNCTION__,__FILE__);
#define LOG_ASSERT_FILE(...) Log::AssertFile(__LINE__,__FUNCTION__,__FILE__,__VA_ARGS__);
#define LOG_EXCEPTION_TRUE(condition,format,...) if(condition) Log::Exception(Log::Format(format,__VA_ARGS__),__LINE__,__FUNCTION__,__FILE__);
#define LOG_EXCEPTION(condition,format,...) if(!(condition)) Log::Exception(Log::Format(format,__VA_ARGS__),__LINE__,__FUNCTION__,__FILE__);

//#define LOG_TRACK Log::Track(__LINE__,__FUNCTION__,__FILE__);
#define LOG_TRACK\
    static std::time_t sPreTime = 0;\
    auto curTime = std::time(nullptr);\
    double interval = std::difftime(curTime, sPreTime); \
    if (sPreTime ==0 || interval > 0.1) Log::Track(__LINE__,__FUNCTION__,__FILE__);\
    sPreTime = curTime;

namespace utils {
    enum class Level
    {
        Info,
        Warm,
        Error,
        Assert,
        Exception,
        Track,
        Reset,
    };
}
using Level = utils::Level;

class Log {
public:
    using cstring = const std::string&;
    
    template<typename... Args>
    static void AssertFile(int line, std::string func, std::string file, Args... args) 
    {
        std::string codePos = Format("File:{0} Func:{1} Line:{2}", file, GetFunc(func), GetFormatString(line));
        (HandleFile(codePos, args), ...);
    };

    static void Write(Level level, cstring message, int line, cstring func, cstring file)
    {
        std::string filename = file.substr(file.find_last_of("\\") + 1);
        std::string log = Format("{0} {1} [{2}] > {3} \t({4} {5} {6}) {7} ", GetCurrTime(), GetLogColor(level),
            GetLogType(level), message,GetFormatString(line), GetFunc(func), filename, GetLogColor(Level::Reset));
        std::cout << log << std::endl;

    }
    static void Write(Level level,cstring message)
    {
        std::string log = Format("{0} {1} [{2}] > {3} {4} ", GetCurrTime(), GetLogColor(level), 
            GetLogType(level), message, GetLogColor(Level::Reset));
        std::cout << log << std::endl;
    }

    static void Exception(cstring message,int line, cstring func, cstring file)
    {
        std::string filename = file.substr(file.find_last_of("\\") + 1);
        std::string log = Format("{0} {1} [{2}] > {3} \t({4} {5} {6}) {7} ", GetCurrTime(), GetLogColor(Level::Exception),
            GetLogType(Level::Exception), message, GetFormatString(line), GetFunc(func), filename, GetLogColor(Level::Exception));
        std::cout << log << std::endl;
        std::string error = Format("File:{0} Func:{1} Line:{2}", file, GetFunc(func), GetFormatString(line));
        throw std::runtime_error(error);
    }

    static void Track(int line, cstring func, cstring file)
    {
        std::string filename = file.substr(file.find_last_of("\\") + 1);
        std::string log = Format("{0} {1} [{2}] > {3} {4} {5}", GetCurrTime(), GetLogColor(Level::Track), 
            GetLogType(Level::Track), GetFormatString(line), GetFunc(func), filename, GetLogColor(Level::Reset));
        std::cout << log << std::endl;
    }

    template<typename... Args>
    static std::string Format(cstring format, Args... args)
    {
        std::string formatMessage = format;
        size_t index = 0;
        // 使用初始化列表展开参数包，并替换占位符
        (void)std::initializer_list<int>{(formatMessage = ReplacePlaceholder(formatMessage, index++, args), 0)...};
        return formatMessage;
    }
private:
    static void HandleFile(cstring codePos, cstring arg) {
        if (std::filesystem::exists(arg)) return;
        Write(Level::Assert, Format("文件不存在：{0}", arg));
        throw std::runtime_error(codePos);
    }

    static std::string GetLogType(Level level) {
        switch (level){
        case Level::Info:       return "I";
        case Level::Track:      return "T";
        case Level::Warm:       return "W";
        case Level::Assert:     return "A";
        case Level::Error:      return "E";
        case Level::Exception:  return "E";
        case Level::Reset:      return " ";
        }
    }
    static std::string GetFormatString(int number)
    {
        std::ostringstream ss;
        ss << std::setw(3) << std::setfill('0') << number;
        return ss.str();
    }
    static std::string GetFunc(const std::string func) {
        return func.substr(func.find_last_of("::") + 1);
    }
    static std::string GetLogColor(Level level) {
        switch (level) {
        case Level::Info:       return "\033[32m";
        case Level::Track:      return "\033[37m";
        case Level::Warm:       return "\033[33m";
        case Level::Assert:     return "\033[35m";
        case Level::Error:      return "\033[91m";
        case Level::Exception:  return "\033[33m";
        case Level::Reset:      return "\033[0m";
        }
    }

    template<typename T>
    static std::string ReplacePlaceholder(const std::string& format, size_t index, const T& arg) {
        std::ostringstream oss;
        oss << arg;
        std::string result = format;
        std::string placeholder = "{" + std::to_string(index) + "}";
        size_t pos = result.find(placeholder);
        if (pos != std::string::npos) {
            result.replace(pos, placeholder.length(), oss.str());
        }
        return result;
    }
    static std::string GetCurrTime()
    {
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    
};

