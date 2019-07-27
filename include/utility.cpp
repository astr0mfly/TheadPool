#include "utility.h"
#include "stdio.h"
#include <string>
#include <vector>
#include <algorithm>

#include "debug.h"

void printMemory(const char *buf, int len) {
	int i, j, k;
	char binstr[80];
    int addr = (int)buf;
	for (i = 0; i < len; i++) {
		if (0 == (i % 16)) {
			sprintf(binstr, "%08x -", i + addr);
			sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
		}
		else if (15 == (i % 16)) {
			sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
			sprintf(binstr, "%s  ", binstr);
			for (j = i - 15; j <= i; j++) {
				sprintf(binstr, "%s%c", binstr, ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
			}
			printf("%s\n", binstr);
		}
		else {
			sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
		}
	}
	if (0 != (i % 16)) {
		k = 16 - (i % 16);
		for (j = 0; j < k; j++) {
			sprintf(binstr, "%s   ", binstr);
		}
		sprintf(binstr, "%s  ", binstr);
		k = 16 - k;
		for (j = i - k; j < i; j++) {
			sprintf(binstr, "%s%c", binstr, ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
		}
		printf("%s\n", binstr);
	}
}

using namespace std;
using namespace utility;
const vector<string> StringSplit(const string& str, string delim)
{
    vector<string> result;

    size_t last_seen = 0;
    size_t next = 0;
    for (; (next = str.find(delim, last_seen)) != string::npos; last_seen = next + 1) {
        result.push_back(str.substr(last_seen, next - last_seen));
    }
    result.push_back(str.substr(last_seen));

    return result;
}

string PadRight(string const& str, size_t size)
{
    if (str.size() < size) {
        return str + string(size - str.size(), ' ');
    }
    else {
        return str;
    }
}


string PadLeft(string const& str, size_t size)
{
    if (str.size() < size) {
        return string(size - str.size(), ' ') + str;
    }
    else {
        return str;
    }
}

void LeftTrim(string& str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
        }));
}

// trim from end (in place)
void RightTrim(string& str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
        }).base(), str.end());
}

// trim from both ends (in place)
void Trim(std::string& str)
{
    LeftTrim(str);
    RightTrim(str);
}


bool SyscallErrorInfo(bool syscall_success, const char* error_message_prefix)
{
    if (!syscall_success) {
        cwarn("Error: %s, %s (%d)", error_message_prefix, strerror(errno), errno);
    }
    return syscall_success;
}


bool PersistentFileUpdate(const char* filename, const void* new_contents, int new_contents_len)
{
    // assume old file is safe
    std::string tmp_filename = "tmp_" + std::string(filename);
    cdebug("tmp name: %s", tmp_filename.c_str());
    FILE* tmp_file = fopen(tmp_filename.c_str(), "wb");
    bool opened = SyscallErrorInfo(tmp_file != NULL,
        "fopen of temp file as 'wb' failed"); //TODO: include filename
    if (!tmp_file) {
        return false;
    }
    bool written = SyscallErrorInfo(new_contents_len == fwrite(new_contents, 1, new_contents_len, tmp_file),
        "fwrite of new_contents to tmp_file failed"); //TODO include more info
    bool flushed = SyscallErrorInfo(0 == fflush(tmp_file),
        "Error: fflush failed to push all writes to disk, ");
    bool closed = SyscallErrorInfo(0 == fclose(tmp_file), "fclose of tmp_file failed"); //TODO include filename
    if (opened && written && closed && flushed) {
        return SyscallErrorInfo(0 == rename(tmp_filename.c_str(), filename),
            "rename of tmp_filename to filename failed"); //TODO: include more info
    }
    cwarn("File Update Failed %s, %s, %d", filename, new_contents, new_contents_len);
    return false;
}