#include "command_line.hpp"
#include "file_sys.hpp"
#include <sstream>
#include <cstdlib>
#include <cstring>
#include "dEntry.hpp"
#include "security.hpp"

// 构造函数
CommandLine::CommandLine()
{
    command_map["exit"] = [this](const std::vector<std::string> &args)
    { return cmdExit(args); };
    command_map["dir"] = [this](const std::vector<std::string> &args)
    { return cmdDir(args); };
    command_map["ls"] = [this](const std::vector<std::string> &args)
    { return cmdDir(args); };
    command_map["mkdir"] = [this](const std::vector<std::string> &args)
    { return cmdMkdir(args); };
    command_map["cd"] = [this](const std::vector<std::string> &args)
    { return cmdCd(args); };
    command_map["mkfile"] = [this](const std::vector<std::string> &args)
    { return cmdMkfile(args); };
    command_map["del"] = [this](const std::vector<std::string> &args)
    { return cmdDel(args); };
    command_map["write"] = [this](const std::vector<std::string> &args)
    { return cmdWrite(args); };
    command_map["read"] = [this](const std::vector<std::string> &args)
    { return cmdRead(args); };
}

// 输入解析
std::vector<std::string> CommandLine::parseInput(const std::string &input)
{
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}

int CommandLine::execute(const std::string &input)
{
    std::vector<std::string> args = parseInput(input);
    if (args.empty())
    {
        return SUCC_RETURN;
    }

    auto it = command_map.find(args[0]);
    if (it != command_map.end())
    {
        return it->second(args);
    }
    else
    {
        std::cerr << "未知命令: " << args[0] << std::endl;
        return SUCC_RETURN;
    }
}

// dir 命令
int CommandLine::cmdDir(const std::vector<std::string> &args)
{
    _dir();
    return SUCC_RETURN;
}

// mkdir 命令
int CommandLine::cmdMkdir(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "mkdir 命令的正确格式为: mkdir <dirname>" << std::endl;
        return SUCC_RETURN;
    }
    mkdir(args[1].c_str());
    return SUCC_RETURN;
}

// cd 命令
int CommandLine::cmdCd(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "cd 命令的正确格式为: cd <dirname>" << std::endl;
        return SUCC_RETURN;
    }
    chdir(args[1].c_str());
    return SUCC_RETURN;
}

// mkfile 命令
int CommandLine::cmdMkfile(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "mkfile 命令的正确格式为: mkfile <filename> [mode]" << std::endl;
        return SUCC_RETURN;
    }
    std::string filename = args[1];
    uint16_t mode = DEFAULTMODE;
    if (args.size() > 2)
    {
        sscanf(args[2].c_str(), "%ho", &mode);
    }
    mode |= DIFILE | 0700;
    int fd = creat(0, filename.c_str(), mode);
    if (fd == -1)
    {
        std::cerr << "创建文件失败！" << std::endl;
        return SUCC_RETURN;
    }
    close(0, fd);
    return SUCC_RETURN;
}

// del 命令
int CommandLine::cmdDel(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "del 命令的正确格式为: del <filename>" << std::endl;
        return SUCC_RETURN;
    }
    removeFile(args[1].c_str());
    return SUCC_RETURN;
}

// write 命令
int CommandLine::cmdWrite(const std::vector<std::string> &args)
{
    if (args.size() < 3)
    {
        std::cerr << "用法: write <filename> <data>" << std::endl;
        return SUCC_RETURN;
    }

    std::string filename = args[1];
    std::string data = args[2]; // 要写入的内容

    // 打开文件
    short mode = FWRITE; // 写模式
    int file_id = open(0, filename.c_str(), mode);
    if (file_id == -1)
    {
        std::cerr << "错误: 无法打开文件 '" << filename << "'，请检查路径和权限。" << std::endl;
        return SUCC_RETURN;
    }

    // 调用 write 函数写入数据
    uint32_t size = data.size();
    char *buf = (char *)malloc(size + 1);
    strcpy(buf, data.c_str());
    uint32_t written_size = write(file_id, buf, size);

    std::cout << written_size << " 字节数据已写入文件 '" << filename << "'。" << std::endl;

    // 关闭文件
    close(0, file_id);
    return SUCC_RETURN;
}

// read 命令
int CommandLine::cmdRead(const std::vector<std::string> &args)
{
    if (args.size() < 3)
    {
        std::cerr << "read 命令的正确格式为: read <filename> <bytes>" << std::endl;
        return SUCC_RETURN;
    }
    std::string filename = args[1];
    uint32_t size = std::stoi(args[2]);

    int fd = open(0, filename.c_str(), READ);
    if (fd == -1)
    {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return SUCC_RETURN;
    }
    char *buf = (char *)malloc(size + 1);
    size = read(fd, buf, size);
    std::cout << size << " bytes 已从文件 " << filename << " 读取到缓冲区." << std::endl;
    free(buf);
    close(0, fd);
    return SUCC_RETURN;
}

// exit 命令
int CommandLine::cmdExit(const std::vector<std::string> &args)
{
    return EXIT_RETURN;
}
