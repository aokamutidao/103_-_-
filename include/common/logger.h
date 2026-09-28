/**
 * @file logger.h
 * @brief 日志系统
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <mutex>

/**
 * @brief 日志级别
 */
enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
};

/**
 * @brief 日志类
 */
class Logger {
public:
    /**
     * @brief 获取单例实例
     */
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief 设置日志级别
     */
    void setLevel(LogLevel level) {
        level_ = level;
    }

    /**
     * @brief 设置是否输出到文件
     */
    void enableFileOutput(const std::string& filename) {
        file_output_ = true;
        log_file_.open(filename, std::ios::app);
    }

    /**
     * @brief 禁用文件输出
     */
    void disableFileOutput() {
        file_output_ = false;
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    /**
     * @brief 输出调试日志
     */
    void debug(const std::string& message) {
        log(LOG_DEBUG, "DEBUG", message);
    }

    /**
     * @brief 输出信息日志
     */
    void info(const std::string& message) {
        log(LOG_INFO, "INFO", message);
    }

    /**
     * @brief 输出警告日志
     */
    void warning(const std::string& message) {
        log(LOG_WARNING, "WARN", message);
    }

    /**
     * @brief 输出错误日志
     */
    void error(const std::string& message) {
        log(LOG_ERROR, "ERROR", message);
    }

private:
    Logger() : level_(LOG_INFO), file_output_(false) {}
    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 输出日志
     */
    void log(LogLevel level, const std::string& level_str, const std::string& message) {
        if (level < level_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        // 获取时间戳
        time_t now = time(nullptr);
        char time_str[32];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

        // 格式化日志消息
        std::stringstream ss;
        ss << "[" << time_str << "] [" << level_str << "] " << message;
        std::string log_message = ss.str();

        // 输出到控制台
        std::cout << log_message << std::endl;

        // 输出到文件
        if (file_output_ && log_file_.is_open()) {
            log_file_ << log_message << std::endl;
        }
    }

    LogLevel level_;
    bool file_output_;
    std::ofstream log_file_;
    std::mutex mutex_;
};

// 便捷宏
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)

#endif // LOGGER_H
