/*
 * Compile as:
 *
 * g++ tsticonv.cc `pkg-config --cflags --libs glibmm-2.4`
 *
 */

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <glibmm/convert.h>
#include <cerrno>
#include <cstring>
//#include <iconv.h>

int main()
{
  setlocale(LC_ALL, "");

  char buf[256];

  size_t inbytes = 0;
  for (int i = 1; i < 256; ++i)
    buf[inbytes++] = i;

  Glib::IConv iconv("UTF8", "ISO8859-1");
#if 0
  iconv_t cd = iconv_open("UTF8", "ISO8859-1");
  if (cd == (iconv_t)-1)
  {
    perror("iconv_open");
    exit(1);
  }
#endif

  char buf2[512];
  size_t outbytes = sizeof(buf2);
  char* inbuf = buf;
  char* outbuf = buf2;

  while(inbytes > 0)
  {
    size_t res = iconv.iconv(&inbuf, &inbytes, &outbuf, &outbytes);
    if (res == static_cast<size_t>(-1))
    {
      std::cerr << "iconv: " << std::strerror(errno) << std::endl;
      exit(1);
    }
    else
      std::cout << "res = " << res << "; inbytes = " << inbytes << "; outbytes = " << outbytes << '\n';
  }

  //iconv_close(cd);

  std::cout << "outbytes = " << outbytes << '\n';
  for (int i = 0; i < sizeof(buf2) - outbytes; ++i)
  {
    unsigned int data = (unsigned int)(unsigned char)buf2[i];
    std::cout << i << " : " << data;
    if (data == 194 || data == 195)
    {
      ++i;
      std::cout << " " << (unsigned int)(unsigned char)buf2[i];
    }
    std::cout << '\n';
  }

  std::string s(buf2, sizeof(buf2) - outbytes);
  std::cout << "Output       : \"" << s << "\"\n";

  char const* charset;
  bool is_utf8 = g_get_charset(&charset);
  std::cout << "g_get_charset() = " << charset << std::endl;

  if (!is_utf8)
  {
    std::cout << "charset is not \"UTF-8\", fix that.\n";
    exit(1);
  }

  Glib::ustring str(s);
  std::cout << "strlen = " << str.length() << '\n';

  std::cout << "Glib::ustring: \"" << str << "\"\n";

  std::cout << "Printable characters:\n";
  Glib::ustring output;
  for (Glib::ustring::iterator iter = str.begin(); iter != str.end(); ++iter)
  {
    Glib::ustring::iterator::value_type c = *iter;
    if (g_unichar_isprint(c))
    {
      output += c;
      std::cout << c << '\n';
    }
    else
      output += '?';
  }
  std::cout << "Printable characters: \"" << output << "\"\n";
}
