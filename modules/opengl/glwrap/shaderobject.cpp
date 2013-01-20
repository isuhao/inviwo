#include "shaderobject.h"
#include <stdio.h>
#include <fstream>

#ifdef WIN32
#define OPEN_FILE(a,b,c) fopen_s(&a, b, c);
#else
#define OPEN_FILE(a,b,c) a = fopen(b, c);
#endif

namespace inviwo {

const std::string ShaderObject::logSource_ = "ShaderObject";

ShaderObject::ShaderObject(GLenum shaderType, std::string fileName) :
    fileName_(fileName),
    shaderType_(shaderType)
{
    id_ = glCreateShader(shaderType);
}

ShaderObject::~ShaderObject() {
    //free(source_);
}

std::string ShaderObject::embeddDefines(std::string source) {
    std::ostringstream result;
    for (size_t i=0; i<shaderDefines_.size(); i++) {
        std::pair<std::string, std::string> curDefine = shaderDefines_[i];
        result << "#define" << curDefine.first << " " << curDefine.second << "\n";
    }
    std::string curLine;
    std::istringstream shaderSource(source);
    while (std::getline(shaderSource, curLine))
        result << curLine << "\n";

    return result.str();
}

std::string ShaderObject::embeddIncludes(std::string source, std::string fileName) {
    std::ostringstream result;
    std::string curLine;
    std::istringstream shaderSource(source);
    unsigned int localLineNumber = 0;
    while (std::getline(shaderSource, curLine)) {
        std::string::size_type posInclude = curLine.find("#include");
        std::string::size_type posComment = curLine.find("//");
        if (posInclude!=std::string::npos && (posComment==std::string::npos || posComment>posInclude)) {
            std::string::size_type pathBegin = curLine.find("\"", posInclude+1);
            std::string::size_type pathEnd = curLine.find("\"", pathBegin+1);
            std::string includeFileName(curLine, pathBegin+1, pathEnd-pathBegin-1);

            // TODO: remove absolute path
            std::ifstream includeFileStream(std::string(IVW_DIR+"modules/opengl/glsl/"+includeFileName).c_str());
            std::stringstream buffer;
            buffer << includeFileStream.rdbuf();
            std::string includeSource = buffer.str();
            
            if (!includeSource.empty())
                result << embeddIncludes(includeSource, includeFileName) << "\n"; 
        }
        else
            result << curLine << "\n";
        lineNumberResolver_.push_back(std::pair<std::string, unsigned int>(fileName, localLineNumber));
        localLineNumber++;
    }
    return result.str();
}


bool ShaderObject::initialize() {
    loadSource(fileName_);
    std::string sourceStr = std::string(source_);
    sourceStr = embeddDefines(sourceStr);
    lineNumberResolver_.clear();
    sourceStr = embeddIncludes(sourceStr, fileName_);
    source_ = sourceStr.c_str();
    upload();
    return compile();
}

void ShaderObject::deinitialize() {}

void ShaderObject::loadSource(std::string fileName) {
    FILE* file;
    char* fileContent = NULL;
    long len;

    if (fileName.length() > 0) {
        OPEN_FILE(file, fileName.c_str(), "rb");
        if (file != NULL) {
            fseek(file, 0L, SEEK_END);
            len = ftell(file);
            fseek(file, 0L, SEEK_SET);
            fileContent = (char*)malloc(sizeof(char)*(len));
            if(fileContent != NULL){
                fread(fileContent, sizeof(char), len, file);
                fileContent[len] = '\0';
            }
            fclose(file);
        }
    }
    source_ = fileContent;
}

void ShaderObject::upload() {
    glShaderSource(id_, 1, &source_, 0);
    LGL_ERROR;
}

std::string ShaderObject::getCompileLog() {
    GLint maxLogLength;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH , &maxLogLength);
    LGL_ERROR;
    if (maxLogLength > 1) {
        GLchar* compileLog = new GLchar[maxLogLength];
        ivwAssert(compileLog!=0, "could not allocate memory for compiler log");
        GLsizei logLength;
        glGetShaderInfoLog(id_, maxLogLength, &logLength, compileLog);
        std::istringstream compileLogStr(compileLog);
        delete[] compileLog;
        return compileLogStr.str();
    } else return "";
}

int ShaderObject::getLogLineNumber(const std::string& compileLogLine) {
    // TODO: adapt to ATI compile log syntax
    int result = 0;
    std::istringstream input(compileLogLine);
    int num;
    if (input>>num) {
        char c;
        if (input>>c && c=='(') {
            if (input>>result) {
                return result;
            }
        }
    }
    return result;
}

std::string ShaderObject::reformatCompileLog(const std::string compileLog) {
    std::ostringstream result;
    std::string curLine;
    std::istringstream origCompileLog(compileLog);
    while (std::getline(origCompileLog, curLine)) {
        unsigned int origLineNumber = getLogLineNumber(curLine);
        unsigned int lineNumber = lineNumberResolver_[origLineNumber].second;
        std::string fileName = lineNumberResolver_[origLineNumber].first;
        // TODO: adapt substr call to ATI compile log syntax
        result << "\n" << fileName << " (" << lineNumber << "): " << curLine.substr(curLine.find(":")+1);
    }
    return result.str();
}

bool ShaderObject::compile() {
    glCompileShader(id_);
    GLint compiledOk = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &compiledOk);
    if (!compiledOk) {
        std::string compilerLog = getCompileLog();
        compilerLog = reformatCompileLog(compilerLog);
        LogError(compilerLog);
        return false;
    }
    return true;
}

} // namespace
