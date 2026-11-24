// You should not need to edit this file

#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <variant>
#include <vector>

/// An error type, which is really just a wrapped string
struct Error {
  std::string msg; // The error message

  /// Construct an Error from a string
  ///
  /// @param _msg The error message to wrap
  ///
  /// @return An Error with the given message
  Error(std::string _msg) : msg(_msg) {}
};

/// Load a file and return its contents
///
/// @param filename The name of the file to open
///
/// @return A vector with the file contents.  On error, returns an empty
///         vector
std::variant<std::vector<uint8_t>, Error>
load_entire_file(const char *filename) {
  using namespace std;

  // make sure file exists, then open it.  stat also lets us get its size later
  //
  // NB: There's a TOCTOU bug here, but we're OK with it.
  struct stat f_info;
  if (stat(filename, &f_info) != 0)
    return Error("File not found");

  // Use a unique_ptr so the FILE* closes when it goes out of scope
  auto closer = [](FILE *stream) { fclose(stream); };
  unique_ptr<FILE, decltype(closer)> f(fopen(filename, "r"), closer);
  if (!f)
    return Error("File  not found");

  // reserve space in vec, based on stat() from before open() (TOCTOU risk)
  vector<uint8_t> res(f_info.st_size);

  // NB: Reading one extra byte should mean we get an EOF and only size bytes.
  //     Also, since we know it's a true file, we don't need to worry about
  //     short counts and EINTR.
  unsigned recd = fread(res.data(), sizeof(char), f_info.st_size + 1, f.get());
  if (recd != res.size() || !feof(f.get()))
    return Error("Incorrect number of bytes read from file");
  return res;
}

/// trim a string in-place, from both ends
///
/// @param s The string to trim
///
/// @return None
void trim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

/// Get a line of text from stdin, and trim its whitespace
///
/// @param prompt A prompt to display, so the user knows to enter something
///
/// @return A string that is expected to be some Scheme code, possibly `""`
std::string getFromStdin(std::string prompt) {
  // string for holding user input that we send to the server
  std::string data;

  // Get the data.  We are using C++ streams (cin) instead of scanf, because
  // it's easier and more portable than doing it in C
  //
  // NB: this assumes that stdin hasn't been redirected from a socket
  std::cout << prompt;
  getline(std::cin, data);
  if (std::cin.eof())
    return "";
  trim(data);
  return data;
}

/// Read the entire contents of a file as a string
///
/// @param fileName The name of the file to open
///
/// @return An error message or a string representation of the file contents
std::variant<std::string, Error> getFile(const char *fileName) {
  auto v_e = load_entire_file(fileName);
  if (std::holds_alternative<Error>(v_e))
    return std::get<Error>(v_e);
  auto contents = std::get<std::vector<uint8_t>>(v_e);
  return std::string(contents.begin(), contents.end());
}

/// Escape a string for outputting it to XML as an attribute
///
/// @param s The string to escape
///
/// @return A new string with special characters escaped
std::string escape(std::string_view s) {
  std::string res = "";
  for (char c : s) {
    if (c == '\\')
      res += ("\\\\");
    else if (c == '\t')
      res += ("\\t");
    else if (c == '\n')
      res += ("\\n");
    else if (c == '\'')
      res += ("\\'");
    else if (c == '&')
      res += ("&amp;");
    else if (c == '<')
      res += ("&lt;");
    else if (c == '>')
      res += ("&gt;");
    else if (c == '"')
      res += ("&quot;");
    else
      res += c;
  }
  return res;
}

/// Remove escape characters from a string when reading from XML
///
/// @param s The string to unescape
///
/// @return A new string with escape sequences replaced by their characters
std::string unescape(std::string_view s) {
  // We need to go char by char to get it right
  std::string ans;
  bool inEscape = false;
  for (auto i : s) {
    if (!inEscape) {
      if (i != '\\')
        ans += i;
      else
        inEscape = true;
    } else {
      if (i == '\\') {
        ans += '\\';
        inEscape = false;
      } else if (i == 't') {
        ans += '\t';
        inEscape = false;
      } else if (i == 'n') {
        ans += '\n';
        inEscape = false;
      } else if (i == '\'') {
        ans += "'";
        inEscape = false;
      }
      if (inEscape)
        throw "Invalid string?!?";
    }
  }
  return ans;
}
