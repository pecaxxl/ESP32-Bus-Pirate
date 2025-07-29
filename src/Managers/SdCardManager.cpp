#include "SdCardManager.h"

SdCardManager::SdCardManager(SdService& sdService, ITerminalView& view, IInput& input, ArgTransformer& argTransformer)
    : sd(sdService), terminalView(view), terminalInput(input), currentDir("/"), argTransformer(argTransformer) {}

void SdCardManager::runShell() {
    terminalView.println("- SD Shell: Type 'help' for commands. Type 'exit' to quit.");

    while (true) {
        terminalView.print(currentDir + " $ ");
        std::string input = readLine();

        if (input.empty()) continue;
        if (input == "exit") break;

        executeCommand(input);
    }

    terminalView.println("- Exiting SD shell.\n");
}

void SdCardManager::executeCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    if (cmd == "ls") cmdLs();
    else if (cmd == "cd") cmdCd(iss);
    else if (cmd == "mkdir") cmdMkdir(iss);
    else if (cmd == "touch") cmdTouch(iss);
    else if (cmd == "rm") cmdRm(iss);
    else if (cmd == "cat") cmdCat(iss);
    else if (cmd == "echo") cmdEcho(iss);
    else if (cmd == "help") cmdHelp();
    else terminalView.println("Unknown command: " + cmd);
}

void SdCardManager::cmdLs() {
    auto files = sd.listElementsCached(currentDir);

    for (const auto& f : files) {
        std::string fullPath = currentDir;
        if (fullPath.back() != '/') fullPath += '/';
        fullPath += f;

        if (sd.isDirectory(fullPath)) {
            terminalView.println(" 📁 " + f);
        } else {
            std::string ext = sd.getFileExt(f);
            std::string icon;

            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (ext == "txt" || ext == "md" || ext == "log" || ext == "csv" || ext == "pdf") {
                icon = " 📝";
            } else if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif" || ext == "webp") {
                icon = " 🖼️ ";
            } else if (ext == "mp3" || ext == "wav" || ext == "ogg" || ext == "flac" || ext == "m4a") {
                icon = " 🎵";
            } else if (ext == "mp4" || ext == "avi" || ext == "mov" || ext == "mkv" || ext == "webm") {
                icon = " 🎞️ ";
            } else if (ext == "zip" || ext == "rar" || ext == "7z" || ext == "tar" || ext == "gz") {
                icon = " 📦";
            } else if (ext == "ino" || ext == "cpp" || ext == "c" || ext == "h" ||
                       ext == "py" || ext == "js" || ext == "ts" || ext == "html" ||
                       ext == "css" || ext == "json" || ext == "xml" || ext == "sh") {
                icon = " 💻";
            } else if (ext == "bin") {
                icon = " 🧾";
            } else {
                icon = " 📄"; // Default
            }

            terminalView.println(icon + " " + f);
        }
    }
}

void SdCardManager::cmdCd(std::istringstream& iss) {
    std::string arg;
    iss >> arg;

    if (arg.empty()) {
        currentDir = "/";
        return;
    }

    std::string newPath;

    if (arg[0] == '/') {
        newPath = normalizePath(arg);
    } else {
        newPath = resolveRelativePath(currentDir, arg); //  ../.., images/ etc.
    }

    if (sd.isDirectory(newPath)) {
        currentDir = newPath;
    } else {
        terminalView.println("Directory not found: " + newPath);
    }
}

void SdCardManager::cmdMkdir(std::istringstream& iss) {
    std::string name;
    iss >> name;
    if (name.empty()) {
        terminalView.println("Usage: mkdir <dirname>");
        return;
    }
    std::string fullPath = currentDir + "/" + name;
    if (sd.ensureDirectory(fullPath)) {
        terminalView.println("Directory created: " + name);
        sd.removeCachedPath(currentDir); // changed, remove to reload
    }
    else terminalView.println("Failed to create directory.");
}

void SdCardManager::cmdTouch(std::istringstream& iss) {
    std::string name;
    iss >> name;
    if (name.empty()) {
        terminalView.println("Usage: touch <filename>");
        return;
    }
    std::string fullPath = currentDir + "/" + name;
    if (sd.writeFile(fullPath, "")) {
        terminalView.println("File created: " + name);
        sd.removeCachedPath(currentDir); // changed, remove to reload
    }
    else terminalView.println("Failed to create file.");
}

void SdCardManager::cmdRm(std::istringstream& iss) {
    std::string name;
    iss >> name;
    if (name.empty()) {
        terminalView.println("Usage: rm <file_or_dir>");
        return;
    }
    std::string fullPath = currentDir + "/" + name;
    if (sd.isFile(fullPath)) {
        if (sd.deleteFile(fullPath)) {
            terminalView.println("File deleted.");
            sd.removeCachedPath(currentDir); // changed, remove to reload
        }
        else terminalView.println("Failed to delete file.");

    } else if (sd.isDirectory(fullPath)) {
        if (sd.deleteDirectory(fullPath)) {
            terminalView.println("Folder deleted.");
            sd.removeCachedPath(currentDir); // changed, remove to reload
        }
    } else {
        terminalView.println("Path not found.");
    }
}

void SdCardManager::cmdHelp() {
    terminalView.println(" Available commands:");
    terminalView.println("  ls                : List files in the directory");
    terminalView.println("  cd <dir>          : Change directory");
    terminalView.println("  cat <file>        : Show content of a text file");
    terminalView.println("  echo TEXT > file  : Overwrite a file with TEXT");
    terminalView.println("  echo TEXT >> file : Append TEXT to the file");
    terminalView.println("  mkdir <dir>       : Create a new directory");
    terminalView.println("  touch <file>      : Create an empty file");
    terminalView.println("  rm <file/dir>     : Delete a file or directory");
    terminalView.println("  help              : Show this help message");
    terminalView.println("  exit              : Exit SD shell");
}

void SdCardManager::cmdCat(std::istringstream& iss) {
    constexpr size_t MAX_DISPLAY_CHARS = 4096;

    std::string filename;
    iss >> filename;
    if (filename.empty()) {
        terminalView.println("Usage: cat <filename>");
        return;
    }

    std::string fullPath = currentDir;
    if (fullPath.back() != '/') fullPath += '/';
    fullPath += filename;

    if (!sd.isFile(fullPath)) {
        terminalView.println("File not found: " + filename);
        return;
    }

    std::string content = sd.readFileChunk(fullPath, 0, MAX_DISPLAY_CHARS);
    terminalView.println(content);
    if (content.length() == MAX_DISPLAY_CHARS) {
        terminalView.println("\n... (file too long)");
    }
}

void SdCardManager::cmdEcho(std::istringstream& iss) {
    std::vector<std::string> tokens;
    std::string word;

    while (iss >> word) {
        tokens.push_back(word);
    }

    if (tokens.size() < 3) {
        terminalView.println("Usage: echo <text> > <filename>  or  >> <filename>");
        return;
    }

    // Find > or >> position
    size_t redirPos = tokens.size();
    std::string redir;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == ">" || tokens[i] == ">>") {
            redir = tokens[i];
            redirPos = i;
        }
    }

    if (redir.empty() || redirPos == tokens.size() - 1) {
        terminalView.println("Usage: echo <text> > <filename>  or  >> <filename>");
        return;
    }

    // Text
    std::string text;
    for (size_t i = 0; i < redirPos; ++i) {
        if (!text.empty()) text += " ";
        text += tokens[i];
    }

    // Path
    std::string filename = tokens.back();
    std::string fullPath = currentDir;
    if (fullPath.back() != '/') fullPath += '/';
    fullPath += filename;

    // Decode escape, eg \\n
    auto decodedText = argTransformer.decodeEscapes(text);
    
    // Write or append to file
    bool append = (redir == ">>");
    if (sd.writeFile(fullPath, decodedText, append)) {
        terminalView.println((append ? "Appended to " : "Wrote to ") + filename);
        sd.removeCachedPath(currentDir);
    } else {
        terminalView.println("Failed to write to: " + filename);
    }
}

std::string SdCardManager::readLine() {
    std::string line;
    size_t cursorIndex = 0;

    while (true) {
        char c = terminalInput.readChar();
        if (c == KEY_NONE) continue;

        // ENTER
        if (c == '\n' || c == '\r') {
            terminalView.println("");
            commandHistoryManager.add(line);
            return line;
        }

        // BACKSPACE
        if (c == '\b' || c == 127) {
            if (cursorIndex == 0) continue;
            cursorIndex--;
            line.erase(cursorIndex, 1);

            terminalView.print("\r" + currentDir + " $ " + line + " \033[K");
            size_t moveBack = line.length() - cursorIndex;
            for (size_t i = 0; i <= moveBack; ++i) {
                terminalView.print("\x1B[D");
            }
            continue;
        }

        // ESCAPE SEQUENCE
        if (c == '\x1B') {
            if (terminalInput.readChar() == '[') {
                char next = terminalInput.readChar();
                if (next == 'A') {
                    line = commandHistoryManager.up();
                    cursorIndex = line.length();
                    terminalView.print("\r" + currentDir + " $ " + line + "\033[K");
                } else if (next == 'B') {
                    line = commandHistoryManager.down();
                    cursorIndex = line.length();
                    terminalView.print("\r" + currentDir + " $ " + line + "\033[K");
                } else if (next == 'C') {  // RIGHT
                    if (cursorIndex < line.length()) {
                        cursorIndex++;
                        terminalView.print("\x1B[C");
                    }
                } else if (next == 'D') {  // LEFT
                    if (cursorIndex > 0) {
                        cursorIndex--;
                        terminalView.print("\x1B[D");
                    }
                }
            }
            continue;
        }

        // Printable
        if (isprint(c)) {
            line.insert(cursorIndex, 1, c);
            cursorIndex++;

            terminalView.print("\r" + currentDir + " $ " + line + "\033[K");
            size_t moveBack = line.length() - cursorIndex;
            for (size_t i = 0; i < moveBack; ++i) {
                terminalView.print("\x1B[D");
            }
        }
    }
}

std::string SdCardManager::normalizePath(const std::string& path) {
    std::vector<std::string> parts;
    std::istringstream ss(path);
    std::string token;

    while (std::getline(ss, token, '/')) {
        if (token.empty() || token == ".") continue;
        if (token == "..") {
            if (!parts.empty()) parts.pop_back();
        } else {
            parts.push_back(token);
        }
    }

    std::string result = "/";
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1) result += '/';
    }

    return result;
}

std::string SdCardManager::resolveRelativePath(const std::string& base, const std::string& arg) {
    std::string combined = base;
    if (!combined.empty() && combined.back() != '/') {
        combined += '/';
    }
    combined += arg;
    return normalizePath(combined);
}
