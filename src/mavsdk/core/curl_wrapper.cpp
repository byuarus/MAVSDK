#include "log.h"
#include "curl_wrapper.h"
#include "unused.h"
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>

namespace mavsdk {

static constexpr const char* kSpartaPrefix = "SPARTA - ";

// converts curl output to string
// taken from
// https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    reinterpret_cast<std::string*>(userp)->append(reinterpret_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

template<typename T>
bool setopt_with_logging(CURL* curl, CURLoption option, T value, const char* option_name)
{
    const auto result = curl_easy_setopt(curl, option, value);
    if (result != CURLcode::CURLE_OK) {
        LogErr() << kSpartaPrefix << "download_text: failed to set " << option_name << ": "
                 << curl_easy_strerror(result);
        return false;
    }

    LogDebug() << kSpartaPrefix << "download_text: set " << option_name;
    return true;
}

static int curl_debug_callback(
    CURL* handle, curl_infotype type, char* data, size_t size, void* userptr)
{
    UNUSED(handle);

    const auto* url = reinterpret_cast<const std::string*>(userptr);
    const std::string url_for_log = (url != nullptr) ? *url : "<unknown>";

    std::string type_string = "unknown";
    bool should_dump_payload = false;
    bool payload_is_binary = false;

    switch (type) {
        case CURLINFO_TEXT:
            type_string = "TEXT";
            should_dump_payload = true;
            break;
        case CURLINFO_HEADER_IN:
            type_string = "HEADER_IN";
            should_dump_payload = true;
            break;
        case CURLINFO_HEADER_OUT:
            type_string = "HEADER_OUT";
            should_dump_payload = true;
            break;
        case CURLINFO_DATA_IN:
            type_string = "DATA_IN";
            payload_is_binary = true;
            break;
        case CURLINFO_DATA_OUT:
            type_string = "DATA_OUT";
            payload_is_binary = true;
            break;
        case CURLINFO_SSL_DATA_IN:
            type_string = "SSL_DATA_IN";
            payload_is_binary = true;
            break;
        case CURLINFO_SSL_DATA_OUT:
            type_string = "SSL_DATA_OUT";
            payload_is_binary = true;
            break;
        default:
            break;
    }

    if (!should_dump_payload) {
        if (payload_is_binary) {
            LogDebug() << kSpartaPrefix << "download_text curl debug [" << type_string << "] " << size
                       << " bytes for URL: " << url_for_log;
        }
        return 0;
    }

    const size_t max_payload_size = 512;
    const size_t payload_size = std::min(size, max_payload_size);
    std::string payload(data, payload_size);
    if (size > max_payload_size) {
        payload += "...(truncated)";
    }

    LogDebug() << kSpartaPrefix << "download_text curl debug [" << type_string << "] for URL: " << url_for_log
               << ": " << payload;

    return 0;
}

static void log_transfer_metadata(CURL* curl, const std::string& url)
{
    long response_code = 0;
    long local_port = 0;
    long remote_port = 0;
    long num_connects = 0;
    long os_errno = 0;
    char* effective_url = nullptr;
    char* local_ip = nullptr;
    char* remote_ip = nullptr;
    double namelookup_time = 0.0;
    double connect_time = 0.0;
    double appconnect_time = 0.0;
    double pretransfer_time = 0.0;
    double starttransfer_time = 0.0;
    double total_time = 0.0;
    double redirect_time = 0.0;
    double downloaded_bytes = 0.0;
    double download_speed = 0.0;

    const auto getinfo_long = [&](CURLINFO info, long& value, const char* info_name) {
        const auto result = curl_easy_getinfo(curl, info, &value);
        if (result != CURLcode::CURLE_OK) {
            LogWarn() << kSpartaPrefix << "download_text: failed to read " << info_name << ": "
                      << curl_easy_strerror(result);
        }
    };
    const auto getinfo_double = [&](CURLINFO info, double& value, const char* info_name) {
        const auto result = curl_easy_getinfo(curl, info, &value);
        if (result != CURLcode::CURLE_OK) {
            LogWarn() << kSpartaPrefix << "download_text: failed to read " << info_name << ": "
                      << curl_easy_strerror(result);
        }
    };
    const auto getinfo_string = [&](CURLINFO info, char*& value, const char* info_name) {
        const auto result = curl_easy_getinfo(curl, info, &value);
        if (result != CURLcode::CURLE_OK) {
            LogWarn() << kSpartaPrefix << "download_text: failed to read " << info_name << ": "
                      << curl_easy_strerror(result);
        }
    };

    getinfo_long(CURLINFO_RESPONSE_CODE, response_code, "CURLINFO_RESPONSE_CODE");
    getinfo_string(CURLINFO_EFFECTIVE_URL, effective_url, "CURLINFO_EFFECTIVE_URL");
    getinfo_string(CURLINFO_LOCAL_IP, local_ip, "CURLINFO_LOCAL_IP");
    getinfo_long(CURLINFO_LOCAL_PORT, local_port, "CURLINFO_LOCAL_PORT");
    getinfo_string(CURLINFO_PRIMARY_IP, remote_ip, "CURLINFO_PRIMARY_IP");
    getinfo_long(CURLINFO_PRIMARY_PORT, remote_port, "CURLINFO_PRIMARY_PORT");
    getinfo_long(CURLINFO_NUM_CONNECTS, num_connects, "CURLINFO_NUM_CONNECTS");
    getinfo_long(CURLINFO_OS_ERRNO, os_errno, "CURLINFO_OS_ERRNO");
    getinfo_double(CURLINFO_NAMELOOKUP_TIME, namelookup_time, "CURLINFO_NAMELOOKUP_TIME");
    getinfo_double(CURLINFO_CONNECT_TIME, connect_time, "CURLINFO_CONNECT_TIME");
    getinfo_double(CURLINFO_APPCONNECT_TIME, appconnect_time, "CURLINFO_APPCONNECT_TIME");
    getinfo_double(CURLINFO_PRETRANSFER_TIME, pretransfer_time, "CURLINFO_PRETRANSFER_TIME");
    getinfo_double(CURLINFO_STARTTRANSFER_TIME, starttransfer_time, "CURLINFO_STARTTRANSFER_TIME");
    getinfo_double(CURLINFO_TOTAL_TIME, total_time, "CURLINFO_TOTAL_TIME");
    getinfo_double(CURLINFO_REDIRECT_TIME, redirect_time, "CURLINFO_REDIRECT_TIME");
    getinfo_double(CURLINFO_SIZE_DOWNLOAD, downloaded_bytes, "CURLINFO_SIZE_DOWNLOAD");
    getinfo_double(CURLINFO_SPEED_DOWNLOAD, download_speed, "CURLINFO_SPEED_DOWNLOAD");

    LogInfo() << kSpartaPrefix << "download_text metadata for URL: " << url
              << ", effective_url: " << (effective_url != nullptr ? effective_url : "<null>")
              << ", response_code: " << response_code
              << ", local_endpoint: " << (local_ip != nullptr ? local_ip : "<null>") << ":"
              << local_port
              << ", remote_endpoint: " << (remote_ip != nullptr ? remote_ip : "<null>") << ":"
              << remote_port << ", os_errno: " << os_errno << ", num_connects: " << num_connects
              << ", timings_s={dns:" << namelookup_time << ", connect:" << connect_time
              << ", tls:" << appconnect_time << ", pretransfer:" << pretransfer_time
              << ", first_byte:" << starttransfer_time << ", total:" << total_time
              << ", redirect:" << redirect_time << "}"
              << ", bytes_downloaded: " << downloaded_bytes
              << ", download_speed_Bps: " << download_speed;
}

bool CurlWrapper::download_text(const std::string& url, std::string& content)
{
    auto curl = std::shared_ptr<CURL>(curl_easy_init(), curl_easy_cleanup);
    std::string readBuffer;

    if (nullptr != curl) {
        CURLcode res;
        char error_buffer[CURL_ERROR_SIZE]{};

        bool options_ok = true;
        options_ok &= setopt_with_logging(
            curl.get(), CURLOPT_ERRORBUFFER, error_buffer, "CURLOPT_ERRORBUFFER");
        options_ok &=
            setopt_with_logging(curl.get(), CURLOPT_CONNECTTIMEOUT, 5L, "CURLOPT_CONNECTTIMEOUT");
        options_ok &= setopt_with_logging(curl.get(), CURLOPT_URL, url.c_str(), "CURLOPT_URL");
        options_ok &= setopt_with_logging(
            curl.get(), CURLOPT_WRITEFUNCTION, write_callback, "CURLOPT_WRITEFUNCTION");
        options_ok &=
            setopt_with_logging(curl.get(), CURLOPT_WRITEDATA, &readBuffer, "CURLOPT_WRITEDATA");
        options_ok &= setopt_with_logging(curl.get(), CURLOPT_VERBOSE, 1L, "CURLOPT_VERBOSE");
        options_ok &= setopt_with_logging(
            curl.get(), CURLOPT_DEBUGFUNCTION, curl_debug_callback, "CURLOPT_DEBUGFUNCTION");
        options_ok &= setopt_with_logging(curl.get(), CURLOPT_DEBUGDATA, &url, "CURLOPT_DEBUGDATA");

        if (!options_ok) {
            content.clear();
            LogErr() << kSpartaPrefix << "download_text: one or more CURLOPT setup calls failed for URL: " << url;
            return false;
        }

        LogInfo() << kSpartaPrefix << "download_text: starting transfer for URL: " << url;
        res = curl_easy_perform(curl.get());
        content = readBuffer;
        log_transfer_metadata(curl.get(), url);

        if (res == CURLcode::CURLE_OK) {
            LogInfo() << kSpartaPrefix << "download_text: transfer succeeded for URL: " << url
                      << ", bytes: " << content.size();
            return true;
        } else {
            LogErr() << kSpartaPrefix << "Error while downloading text for URL: " << url
                     << ", curl error code: " << curl_easy_strerror(res)
                     << ", curl error buffer: "
                     << (error_buffer[0] != '\0' ? error_buffer : "<empty>");
            return false;
        }
    } else {
        LogErr() << kSpartaPrefix << "Error: cannot start uploading because of curl initialization error. ";
        return false;
    }
}

static int
upload_progress_update(void* p, double dltotal, double dlnow, double ultotal, double ulnow)
{
    UNUSED(dltotal);
    UNUSED(dlnow);

    auto* myp = reinterpret_cast<struct dl_up_progress*>(p);

    if (myp->progress_callback == nullptr) {
        return 0;
    }

    if (ultotal == 0 || ulnow == 0) {
        return myp->progress_callback(0, Status::Idle, CURLcode::CURLE_OK);
    }

    int percentage = static_cast<int>(100.0 / ultotal * ulnow);

    if (percentage > myp->progress_in_percentage) {
        myp->progress_in_percentage = percentage;
        return myp->progress_callback(percentage, Status::Uploading, CURLcode::CURLE_OK);
    }

    return 0;
}

size_t get_file_size(const std::string& path)
{
    std::streampos begin, end;
    std::ifstream my_file(path.c_str(), std::ios::binary);
    begin = my_file.tellg();
    my_file.seekg(0, std::ios::end);
    end = my_file.tellg();
    my_file.close();
    return ((end - begin) > 0) ? (end - begin) : 0;
}

template<typename T> std::string to_string(T value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}

bool CurlWrapper::upload_file(
    const std::string& url, const std::string& path, const progress_callback_t& progress_callback)
{
    auto curl = std::shared_ptr<CURL>(curl_easy_init(), curl_easy_cleanup);
    CURLcode res;

    if (nullptr != curl) {
        struct dl_up_progress progress;
        progress.progress_callback = progress_callback;

        curl_httppost* post = nullptr;
        curl_httppost* last = nullptr;

        struct curl_slist* chunk = nullptr;

        // avoid sending 'Expect: 100-Continue' header, required by some server implementations
        chunk = curl_slist_append(chunk, "Expect:");

        // disable chunked upload
        chunk = curl_slist_append(chunk, "Content-Encoding: ");

        // to allow efficient file upload, we need to add the file size to the header
        std::string filesize_header = "File-Size: " + to_string(get_file_size(path));
        chunk = curl_slist_append(chunk, filesize_header.c_str());

        curl_formadd(
            &post, &last, CURLFORM_COPYNAME, "file", CURLFORM_FILE, path.c_str(), CURLFORM_END);

        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl.get(), CURLOPT_PROGRESSFUNCTION, upload_progress_update);
        curl_easy_setopt(curl.get(), CURLOPT_PROGRESSDATA, &progress);
        curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);

        res = curl_easy_perform(curl.get());

        curl_slist_free_all(chunk);
        curl_formfree(post);

        if (res == CURLcode::CURLE_OK) {
            if (nullptr != progress_callback) {
                progress_callback(100, Status::Finished, CURLcode::CURLE_OK);
            }
            return true;
        } else {
            if (nullptr != progress_callback) {
                progress_callback(0, Status::Error, res);
            }
            LogErr() << "Error while uploading file, curl error code: " << curl_easy_strerror(res);
            return false;
        }
    } else {
        LogErr() << "Error: cannot start uploading because of curl initialization error.";
        return false;
    }
}

static int
download_progress_update(void* p, double dltotal, double dlnow, double ultotal, double ulnow)
{
    UNUSED(ultotal);
    UNUSED(ulnow);

    auto* myp = reinterpret_cast<struct dl_up_progress*>(p);

    if (myp->progress_callback == nullptr) {
        return 0;
    }

    if (dltotal == 0 || dlnow == 0) {
        return myp->progress_callback(0, Status::Idle, CURLcode::CURLE_OK);
    }

    int percentage = static_cast<int>(100 / dltotal * dlnow);

    if (percentage > myp->progress_in_percentage) {
        myp->progress_in_percentage = percentage;
        return myp->progress_callback(percentage, Status::Downloading, CURLcode::CURLE_OK);
    }

    return 0;
}

bool CurlWrapper::download_file_to_path(
    const std::string& url, const std::string& path, const progress_callback_t& progress_callback)
{
    auto curl = std::shared_ptr<CURL>(curl_easy_init(), curl_easy_cleanup);
    FILE* fp;

    if (nullptr != curl) {
        CURLcode res;
        struct dl_up_progress progress;
        progress.progress_callback = progress_callback;

        fp = fopen(path.c_str(), "wb");
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl.get(), CURLOPT_PROGRESSFUNCTION, download_progress_update);
        curl_easy_setopt(curl.get(), CURLOPT_PROGRESSDATA, &progress);
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
        res = curl_easy_perform(curl.get());
        fclose(fp);

        if (res == CURLcode::CURLE_OK) {
            if (nullptr != progress_callback) {
                progress_callback(100, Status::Finished, res);
            }
            return true;
        } else {
            if (nullptr != progress_callback) {
                progress_callback(0, Status::Error, res);
            }
            remove(path.c_str());
            LogErr() << "Error while downloading file, curl error code: "
                     << curl_easy_strerror(res);
            return false;
        }
    } else {
        LogErr() << "Error: cannot start downloading file because of curl initialization error. ";
        return false;
    }
}

} // namespace mavsdk
