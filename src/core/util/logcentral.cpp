/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stacktrace.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

void Logger::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                          std::string msg, const char* file, const char* function, int line) {
    log("Processor " + processor->getIdentifier(), level, audience, file, function, line, msg);
}

void Logger::logNetwork(LogLevel level, LogAudience audience, std::string msg, const char* file,
                        const char* function, int line) {
    log("ProcessorNetwork", level, audience, file, function, line, msg);
}

ConsoleLogger::ConsoleLogger() = default;
ConsoleLogger::~ConsoleLogger() = default;

void ConsoleLogger::log(std::string logSource, LogLevel logLevel, LogAudience audience,
                        const char* fileName, const char* functionName, int lineNumber,
                        std::string logMsg) {
    IVW_UNUSED_PARAM(fileName);
    IVW_UNUSED_PARAM(logLevel);
    IVW_UNUSED_PARAM(functionName);
    IVW_UNUSED_PARAM(lineNumber);
    if (logLevel == LogLevel::Error) {
        std::cerr << "(" << logSource << ") " << logMsg << std::endl;
    } else {
        std::cout << "(" << logSource << ") " << logMsg << std::endl;
    }
}

FileLogger::FileLogger(std::string logPath) : Logger() {
    if (filesystem::getFileExtension(logPath) != "") {
        fileStream_ = new std::ofstream(logPath.c_str());
    } else {
        fileStream_ = new std::ofstream(logPath.append("/inviwo-log.html").c_str());
    }

    (*fileStream_) << "<style>" << std::endl
        << ".warn{" << std::endl
        << "    color: orange;" << std::endl
        << "}" << std::endl
        << ".error{" << std::endl
        << "    color: red;" << std::endl
        << "}" << std::endl
        << ".level{" << std::endl
        << "    font-weight: bold;" << std::endl
        << "}" << std::endl
        << "</style>" << std::endl;


    (*fileStream_) << "<div class ='info'>Inviwo (V " << IVW_VERSION << ") Log File</div>"
        << std::endl;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char formatedString[21];
    strftime(formatedString, sizeof(formatedString), "%Y-%m-%d %H:%M:%S ", &tm);
    (*fileStream_) << "<div class ='info'>Created at: " << formatedString << "</div>" << std::endl;
}

FileLogger::~FileLogger() {
    fileStream_->close();
    delete fileStream_;
}

void FileLogger::log(std::string logSource, LogLevel logLevel, LogAudience audience,
                     const char* fileName, const char* functionName, int lineNumber,
                     std::string logMsg) {
    IVW_UNUSED_PARAM(fileName);
    IVW_UNUSED_PARAM(logLevel);
    IVW_UNUSED_PARAM(functionName);
    IVW_UNUSED_PARAM(lineNumber);

    switch (logLevel) {
        case LogLevel::Info:
            (*fileStream_) << "<div class ='info'><span class='level'>Info: </span>";
            break;

        case LogLevel::Warn:
            (*fileStream_) << "<div class ='warn'><span class='level'>Warn: </span>";
            break;

        case LogLevel::Error:
            (*fileStream_) << "<div class ='error'><span class='level'>Error: </span>";
            break;
    }
    
    logMsg = htmlEncode(logMsg);
    replaceInString(logMsg, " ", "&nbsp;");
    replaceInString(logMsg, "\n", "<br/>");
    replaceInString(logMsg, "\t", "&nbsp;&nbsp;&nbsp;&nbsp;");

    (*fileStream_) << "(" << htmlEncode(logSource) << ":" << lineNumber << ") " << logMsg;
    (*fileStream_) << "</div>" << std::endl;
}

LogCentral::LogCentral() : logLevel_(LogLevel::Info), logStacktrace_(false) {}

void LogCentral::registerLogger(std::weak_ptr<Logger> logger) { loggers_.push_back(logger); }

void LogCentral::log(std::string source, LogLevel level, LogAudience audience, const char* file,
                     const char* function, int line, std::string msg) {
    if (logStacktrace_ && level == LogLevel::Error && audience == LogAudience::Developer) {
        std::stringstream ss;
        ss << msg;

        std::vector<std::string> stacktrace = getStackTrace();
        // start at i == 3 to remove log and getStacktrace from stackgrace
        for (size_t i = 3; i < stacktrace.size(); ++i) {
            ss << std::endl << stacktrace[i];
        }
        // append an extra line break to easier separate several stacktraces in a row
        ss << std::endl;

        msg = ss.str();
    }

    if (level >= logLevel_) {
        // use remove if here to remove expired weak pointers while calling the loggers.
        util::erase_remove_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
            if (auto l = logger.lock()) {
                l->log(source, level, audience, file, function, line, msg);
                return false;
            } else {
                return true;
            }
        });
    }
}

void LogCentral::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function, int line) {
    if (level >= logLevel_) {
        // use remove if here to remove expired weak pointers while calling the loggers.
        util::erase_remove_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
            if (auto l = logger.lock()) {
                l->logProcessor(processor, level, audience, msg, file, function, line);
                return false;
            } else {
                return true;
            }
        });
    }
}

void LogCentral::logNetwork(LogLevel level, LogAudience audience, std::string msg, const char* file,
                            const char* function, int line) {
    if (level >= logLevel_) {
        // use remove if here to remove expired weak pointers while calling the loggers.
        util::erase_remove_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
            if (auto l = logger.lock()) {
                l->logNetwork(level, audience, msg, file, function, line);
                return false;
            } else {
                return true;
            }
        });
    }
}

void LogCentral::setLogStacktrace(const bool& logStacktrace) { logStacktrace_ = logStacktrace; }

bool LogCentral::getLogStacktrace() const { return logStacktrace_; }

void util::log(ExceptionContext context, std::string message, LogLevel level,
               LogAudience audience) {
    LogCentral::getPtr()->log(context.getCaller(), level, audience, context.getFile().c_str(),
                              context.getFunction().c_str(), context.getLine(), message);
}

}  // namespace