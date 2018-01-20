#include "Utility/FileReader.h"
#include "Utility/Error.h"

#include <fstream>

FileReader::FileReader(std::string _path):
    path("Resources/" + _path)
{
    std::ifstream file(path.c_str());

    if (!file)
    {
        Error::add(ErrorType::FILE_NOT_FOUND, "FileReader::FileReader() -> File not found: " + path);
        return;
    }

    std::string line;
    std::istringstream* stream = nullptr;

    while (getline(file, line))
    {
        if (!line.size())
            continue;

        stream = new std::istringstream(line);
        *stream >> line;

        entries[line] = stream;
    }
}

FileReader::~FileReader()
{
    for (auto& entry: entries)
        delete entry.second;
}

/// Methods
std::istringstream& FileReader::operator[](std::string _entry)
{
    auto it = entries.find(_entry);

    if (it == entries.end())
        Error::add(ErrorType::USER_ERROR, "FileReader::operator[] -> No such entry: " + _entry);

    return *it->second;
}
