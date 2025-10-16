#include    "includes.hpp"


bool    createUploadDir(const std::string &dir) {

    if (mkdir(dir.c_str(), 0755) == -1) {
        if (errno != EEXIST) {
            std::cerr << "mkdir failed: " << strerror(errno) << std::endl;
            return false;
        }
    }
    return true;
}

// 중복 이름 방지 함수 (앞서 제공한 함수와 유사)
std::string getUniqueFileName(const std::string& dir, const std::string& filename) {
    std::string fullPath = dir + "/" + filename;
    if (access(fullPath.c_str(), F_OK) != 0) {
        // 파일 없으면 원래 이름 반환
        return filename;
    }

    std::string name = filename;
    std::string ext;
    size_t pos = filename.find_last_of('.');
    if (pos != std::string::npos) {
        name = filename.substr(0, pos);
        ext = filename.substr(pos);
    }

    int counter = 2;
    std::string newName;
    do {
        std::stringstream ss;
        ss << counter;
        newName = name + "(" + ss.str() + ")" + ext;
        fullPath = dir + "/" + newName;
        ++counter;
    } while (access(fullPath.c_str(), F_OK) == 0);

    return newName;
}

// 수정된 saveFile 함수
bool saveFile(const std::string &dir, const std::string &filename, const std::string &content) {
    if(!createUploadDir(dir))
        return false;

    // 중복 이름 방지용으로 파일명 갱신
    std::string uniqueFilename = getUniqueFileName(dir, filename);
    std::string path = dir + "/" + uniqueFilename;

    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "open failed: " << strerror(errno) << std::endl;
        return false;
    }
    if (write(fd, content.c_str(), content.size()) == -1) {
        std::cerr << "write failed: " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }
    close(fd);
    return true;
}


// bool saveFile(const std::string &dir, const std::string &filename, const std::string &content) {
//     if(!createUploadDir(dir))
//         return false;
//     std::string path = dir + "/" + filename;
//     int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
//     if (fd == -1) {
//         std::cerr << "open failed: " << strerror(errno) << std::endl;
//         return false;
//     }
//     if (write(fd, content.c_str(), content.size()) == -1) {
//         std::cerr << "write failed: " << strerror(errno) << std::endl;
//         close(fd);
//         return false;
//     }
//     close(fd);
//     return true;
// }

// #include <sstream>
// #include <dirent.h>
// #include <string>

// // C++98 호환 JS 문자열 이스케이프 함수
// std::string jsEscape(const std::string &s) {
//     std::string result;
//     for (size_t i = 0; i < s.size(); ++i) {
//         char c = s[i];
//         if (c == '\'' || c == '"' || c == '\\')
//             result += '\\';
//         result += c;
//     }
//     return result;
// }

// std::string generateUploadPage(const std::string &uploadDir, int message) {
//     std::ostringstream oss;

//     oss << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
//         << "<title>Upload Page</title>"
//         << "<style>"
//         << "body { background-color: #FFFFFF; font-family: Arial, sans-serif; color: #333; }"
//         << "h1 { text-align: center; margin-top: 150px; color: #007acc; font-size: 60px; margin-bottom: 50px; }"
//         << "h2 { text-align: center; color: #333; }"
//         << ".container { max-width: 700px; margin: 0 auto; }"
//         << ".file-list { background-color: #f9f9f9; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 40px; }"
//         << ".file-list ul { list-style: none; padding: 0; margin: 0; }"
//         << ".file-list li { padding: 10px; border-bottom: 1px solid #ddd; text-align: left; }"
//         << ".file-list li:last-child { border-bottom: none; }"
//         << ".file-list a { font-size: 15px; margin-left: 10px; color: #007acc; text-decoration: none; }"
//         << ".file-list a:hover { text-decoration: none; color: #005f99; }"
//         << ".upload-form { background-color: #f9f9f9; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); text-align: center; }"
//         << ".upload-form h2 { color: #333; font-size: 24px; margin-bottom: 20px; }"
//         << "input[type='file'] { margin: 20px 0; }"
//         << "input[type='submit'] { background-color: #007acc; color: white; border: none; padding: 10px 20px; font-size: 16px; border-radius: 5px; cursor: pointer; }"
//         << "input[type='submit']:hover { background-color: #005f99; }"
//         << ".nav { text-align: center; margin-top: 20px; }"
//         << ".nav a { margin: 0 10px; color: #007acc; font-weight: bold; text-decoration: none; }"
//         << ".nav a:hover { color: #005f99; }"
//         << ".message { font-size: 18px; margin-bottom: 30px; text-align: center; }"
//         << ".success { color: #28a745; }"
//         << ".error { color: #cc0000; }"
//         << "</style></head><body>";

//     oss << "<div class='container'>";
//     oss << "<h1>File Upload</h1>";

//     // 상태 메시지
//     if (message == 2)
//         oss << "<div class='message success'>✅ File uploaded successfully!</div>";
//     else if (message == 3)
//         oss << "<div class='message error'>❌ File upload failed.</div>";
//     else if (message == 4)
//         oss << "<div class='message error'>❌ File upload failed. Invalid file extension.</div>";

//     // 업로드된 파일 목록
//     oss << "<div class='file-list'><h2>Uploaded Files</h2><ul>";
//     DIR *dir = opendir(uploadDir.c_str());
//     if (dir) {
//         struct dirent *entry;
//         bool hasFiles = false;
//         while ((entry = readdir(dir)) != NULL) {
//             if (entry->d_name[0] == '.') continue;
//             hasFiles = true;
//             oss << "<li>"
//                 << entry->d_name
//                 << " <a href=\"/uploads/" << entry->d_name << "\">[View]</a>"
//                 << " <button type='button' onclick=\"deleteFile('" << jsEscape(entry->d_name) << "')\" style='background:none; border:none; color:#cc0000; cursor:pointer;'>[Delete]</button>"
//                 << "</li>";
//         }
//         if (!hasFiles)
//             oss << "<li><em>No files uploaded yet.</em></li>";
//         closedir(dir);
//     } else {
//         oss << "<li><em>Unable to open upload directory.</em></li>";
//     }
//     oss << "</ul></div>";

//     // 업로드 폼
//     oss << "<div class='upload-form'>"
//         << "<h2>Upload a New File</h2>"
//         << "<form action='/upload' method='POST' enctype='multipart/form-data'>"
//         << "<input type='file' name='file' required><br>"
//         << "<input type='submit' value='Upload'>"
//         << "</form></div>";

//     // 네비게이션
//     oss << "<div class='nav'><p><a href='/'>Go back to Home</a></p></div>";

//     // JavaScript DELETE 함수
//     oss << "<script>"
//         << "function deleteFile(filename) {"
//         << "  if (confirm('Are you sure you want to delete ' + filename + '?')) {"
//         << "    var xhr = new XMLHttpRequest();"
//         << "    xhr.open('DELETE', '/upload/' + encodeURIComponent(filename), true);"
//         << "    xhr.onreadystatechange = function() {"
//         << "      if (xhr.readyState == 4) {"
//         << "        if (xhr.status == 200) { alert('File deleted successfully!'); location.reload(); }"
//         << "        else { alert('Delete failed.'); }"
//         << "      }"
//         << "    };"
//         << "    xhr.send(null);"
//         << "  }"
//         << "}"
//         << "</script>";

//     oss << "</div></body></html>";

//     return oss.str();
// }



std::string generateUploadPage(const std::string &uploadDir, int message) {
    std::ostringstream oss;

    oss << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
        << "<title>Upload Page</title>"
        << "<style>"
        << "body { background-color: #FFFFFF; font-family: Arial, sans-serif; color: #333; }"
        << "h1 { text-align: center; margin-top: 150px; color: #007acc; font-size: 60px; margin-bottom: 50px; }"
        << "h2 { text-align: center; color: #333; }"
        << ".container { max-width: 700px; margin: 0 auto; }"
        << ".file-list { background-color: #f9f9f9; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 40px; }"
        << ".file-list ul { list-style: none; padding: 0; margin: 0; }"
        << ".file-list li { padding: 10px; border-bottom: 1px solid #ddd; text-align: left; }"
        << ".file-list li:last-child { border-bottom: none; }"
        << ".file-list a { font-size: 15px; margin-left: 10px; color: #007acc; text-decoration: none; }"
        << ".file-list a:hover { text-decoration: none; color: #005f99; }"
        << ".upload-form { background-color: #f9f9f9; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); text-align: center; }"
        << ".upload-form h2 { color: #333; font-size: 24px; margin-bottom: 20px; }"
        << "input[type='file'] { margin: 20px 0; }"
        << "input[type='submit'] { background-color: #007acc; color: white; border: none; padding: 10px 20px; font-size: 16px; border-radius: 5px; cursor: pointer; }"
        << "input[type='submit']:hover { background-color: #005f99; }"
        << ".nav { text-align: center; margin-top: 20px; }"
        << ".nav a { margin: 0 10px; color: #007acc; font-weight: bold; text-decoration: none; }"
        << ".nav a:hover { color: #005f99; }"
        << ".message { font-size: 18px; margin-bottom: 30px; text-align: center; }"
        << ".success { color: #28a745; }"
        << ".error { color: #cc0000; }"
        << "</style></head><body>";

    oss << "<div class='container'>";
    oss << "<h1>File Upload</h1>";

    // ✅ 상태 메시지 출력 (1: 없음, 2: 성공, 3: 실패, 4: 확장자 오류)
    if (message == 2) {
        oss << "<div class='message success'>✅ File uploaded successfully!</div>";
    } else if (message == 3) {
        oss << "<div class='message error'>❌ File upload failed.</div>";
    }
    else if (message == 4) {
        oss << "<div class='message error'>❌ File upload failed. Invalid file extension.</div>";
    }

    // 📂 업로드된 파일 목록
    oss << "<div class='file-list'><h2>Uploaded Files</h2><ul>";
    DIR *dir = opendir(uploadDir.c_str());
    if (dir) {
        struct dirent *entry;
        bool hasFiles = false;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            hasFiles = true;
            oss << "<li>"
                << entry->d_name
                << " <a href=\"/upload/" << entry->d_name << "\">[View]</a>"
                << "<button type=\"button\" onclick=\"deleteFile('" << entry->d_name << "')\" style=\"background:none; border:none; color:#cc0000; cursor:pointer;\">[Delete]</button>"
                << "</li>";
        }
        if (!hasFiles)
            oss << "<li><em>No files uploaded yet.</em></li>";
        closedir(dir);
    } else {
        oss << "<li><em>Unable to open upload directory.</em></li>";
    }
    oss << "</ul></div>";

    // 📤 업로드 폼
    oss << "<div class='upload-form'>"
        << "<h2>Upload a New File</h2>"
        << "<form action=\"/upload\" method=\"POST\" enctype=\"multipart/form-data\">"
        << "<input type=\"file\" name=\"file\" required><br>"
        << "<input type=\"submit\" value=\"Upload\">"
        << "</form></div>";

    // 🧭 하단 네비게이션
    oss << "<div class='nav'>"
        << "<p><a href=\"/\">Go back to Home</a></p>"
        << "</div>";


    // ⭐ JavaScript 코드 추가 (</body> 바로 앞에)
    oss << "<script>"
        << "function deleteFile(filename) {"
        << "  if (confirm('Are you sure you want to delete ' + filename + '?')) {"
        << "    fetch('/upload/' + encodeURIComponent(filename), { method: 'DELETE' })"
        << "    .then(response => {"
        << "      if (response.ok) {"
        << "        alert('File deleted successfully!');"
        << "        location.reload();"  // 페이지 새로고침
        << "      } else {"
        << "        alert('Delete failed. Please try again.');"
        << "      }"
        << "    })"
        << "    .catch(error => {"
        << "      console.error('Error:', error);"
        << "      alert('Error occurred while deleting file.');"
        << "    });"
        << "  }"
        << "}"
        << "</script>";

    oss << "</div></body></html>";

    

    return oss.str();
}

