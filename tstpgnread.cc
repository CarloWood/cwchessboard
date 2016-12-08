// This program tests what is the best way to read a file
// with as goal to get to the end fast (maximum disk speed).
// That means that we want to read an integer number of
// blocks (likely 4096 bytes) at a time.

#include "sys.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <gio/gio.h>
#include "debug.h"

char const* filename="/opt/verylarge/chessgames/ftp.cis.uab.edu/crafty/enormous.pgn";
char const* warmupfile="/home/carlo/chess/fics2008.pgn";

//#define REFERENCE_IMPLEMENTATION
//#define BUFFERSIZE_IMPLEMENTATION
//#define FSTREAM_READ_IMPLEMENTATION
//#define FSTREAM_GETLINE_IMPLEMENTATION
//#define GFILE_IMPLEMENTATION
//#define GFILE_ASYNC_IMPLEMENTATION
#define CWCHESS_PGN_IMPLEMENTATION

#ifdef GFILE_ASYNC_IMPLEMENTATION
#include <gio/gio.h>
#include "PgnDatabase.h"		// Needed for cwchess::pgn::DatabaseSeekable::S_buffer_size.
#endif
#ifdef CWCHESS_PGN_IMPLEMENTATION
#include <giomm/init.h>
#include "PgnDatabase.h"
#endif
#if defined(GFILE_ASYNC_IMPLEMENTATION) || defined(CWCHESS_PGN_IMPLEMENTATION)
std::ostream* global_os;
#endif

struct timeval before, after;

inline void start_timer(void)
{
  gettimeofday(&before, NULL);
}

uint64_t stop_timer(void)
{
  gettimeofday(&after, NULL);
  timersub(&after, &before, &after);
  uint64_t t = after.tv_sec;
  t *= 1000000;
  t += after.tv_usec;
  return t;
}

char buf[4096 * 50];

#ifdef REFERENCE_IMPLEMENTATION
void benchmark_reference(std::ostream& os, char const* filename)
{
  int fd = open(filename, O_RDONLY);  
  size_t len = 0;
  start_timer();
  while(len < 945950820)
  {
    int s = read(fd, buf, sizeof(buf));
    if (s == -1)
    {
      perror("read");
      exit(0);
    }
    else if (s == 0)
      break;
    len += s;
  }
  uint64_t microseconds = stop_timer();
  os << "Reference implementation (buffersize " << sizeof(buf) << "): " << microseconds << " microseconds." << std::endl;
  close(fd);
}
#endif

#ifdef BUFFERSIZE_IMPLEMENTATION
void benchmark_buffersize(std::ostream& os, char const* filename)
{
  size_t const bufsize = 4096;
  int fd = open(filename, O_RDONLY);  
  size_t len = 0;
  start_timer();
  while(len < 945950820)
  {
    int s = read(fd, buf, bufsize);
    if (s == -1)
    {
      perror("read");
      exit(0);
    }
    else if (s == 0)
      break;
    len += s;
  }
  uint64_t microseconds = stop_timer();
  os << "read(2) (buffersize " << bufsize << "): " << microseconds << " microseconds." << std::endl;
  close(fd);
}
#endif

#ifdef FSTREAM_READ_IMPLEMENTATION
void benchmark_fstream_read(std::ostream& os, char const* filename)
{
  std::ifstream inputfile;
  inputfile.open(filename);
  start_timer();
  size_t len = 0;
  while(inputfile)
  {
    inputfile.read(buf, 4096);
    len += inputfile.gcount();
  }
  uint64_t microseconds = stop_timer();
  os << "std::ifstream::read (buffersize 4096): " << microseconds << " microseconds. Size read: " << len << std::endl;
  inputfile.close();
}
#endif

#ifdef FSTREAM_GETLINE_IMPLEMENTATION
void benchmark_fstream_getline(std::ostream& os, char const* filename)
{
  std::ifstream inputfile;
  inputfile.open(filename);
  start_timer();
  size_t len = 0;
  std::string line;
  while(getline(inputfile, line))
  {
    len += line.length() + 1;
  }
  uint64_t microseconds = stop_timer();
  os << "std::getline(std::ifstream, ...): " << microseconds << " microseconds. Size read: " << len << std::endl;
  inputfile.close();
}
#endif

#ifdef GFILE_IMPLEMENTATION
void benchmark_gfile(std::ostream& os, char const* filename)
{
  g_type_init();
  GFile* file = g_file_new_for_path(filename);
  GCancellable* cancellable = g_cancellable_new();
  GError* error = NULL;
  GFileInputStream* stream = g_file_read(file, cancellable, &error);
  if (stream == NULL)
  {
    std::cerr << "g_file_read failed: " << error->message << std::endl;
    exit(1);
  }
  start_timer();
  size_t len = 0;
  gssize res;
  do
  {
    res = g_input_stream_read(G_INPUT_STREAM(stream), buf, 4096, cancellable, &error);
    if (res == -1)
    {
      std::cerr << "g_input_stream_read: " << error->message << std::endl;
      exit(1);
    }
    len += res;
  }
  while(res > 0);
  uint64_t microseconds = stop_timer();
  os << "g_input_stream_read (buffersize 4096): " << microseconds << " microseconds. Size read: " << len << std::endl;
  g_object_unref(stream);
  g_object_unref(cancellable);
  g_object_unref(file);
}
#endif

#ifdef GFILE_ASYNC_IMPLEMENTATION
GMainLoop* gmain_loop;
GCancellable* cancellable;
void async_ready_callback(GObject* source_object, GAsyncResult* async_res, gpointer user_data);
void async_ready_callback2(GObject* source_object, GAsyncResult* async_res, gpointer user_data);

void benchmark_gfile_async(std::ostream& os, char const* filename)
{
  g_type_init();
  global_os = &os;
  GFile* file = g_file_new_for_path(filename);
  cancellable = g_cancellable_new();
  g_file_read_async(file, G_PRIORITY_DEFAULT, cancellable, async_ready_callback, file);
  gmain_loop = g_main_loop_new(NULL, false);
  g_main_loop_run(gmain_loop);
  g_object_unref(cancellable);
  g_object_unref(file);
}
#endif

#ifdef CWCHESS_PGN_IMPLEMENTATION
Glib::RefPtr<Glib::MainLoop> main_loop;
Glib::RefPtr<cwchess::pgn::Database> pgn_data_base;
void open_finished(size_t len);

void benchmark_cwchess_pgn(std::ostream& os, char const* filename)
{
  using namespace cwchess;
  Gio::init();
  global_os = &os;
  pgn_data_base = pgn::DatabaseSeekable::open(filename, sigc::ptr_fun(&open_finished));
  main_loop = Glib::MainLoop::create(false);
  start_timer();
  main_loop->run();
}
#endif

void clear_disk_cache(void)
{
  // Free pagecache.
  system("sudo sh -c \"sync; echo 1 > /proc/sys/vm/drop_caches\"");
  // Warm up (read libraries etc).
  start_timer();
  uint64_t microseconds = stop_timer();
  std::ofstream dump("/dev/null");
  //std::ostream& dump(std::cout);
#ifdef REFERENCE_IMPLEMENTATION
  benchmark_reference(dump, warmupfile);
#endif
#ifdef BUFFERSIZE_IMPLEMENTATION
  benchmark_buffersize(dump, warmupfile);
#endif
#ifdef FSTREAM_READ_IMPLEMENTATION
  benchmark_fstream_read(dump, warmupfile);
#endif
#ifdef FSTREAM_GETLINE_IMPLEMENTATION
  benchmark_fstream_getline(dump, warmupfile);
#endif
#ifdef GFILE_IMPLEMENTATION
  benchmark_gfile(dump, warmupfile);
#endif
#ifdef GFILE_ASYNC_IMPLEMENTATION
  benchmark_gfile_async(dump, warmupfile);
#endif
#ifdef CWCHESS_PGN_IMPLEMENTATION
  benchmark_cwchess_pgn(dump, warmupfile);
#endif
  dump.close();
  // Sleep to let other running application catch up too.
  sleep(1);
}

int main()
{
  if (!Glib::thread_supported())
      Glib::thread_init();
  Debug(debug::init());

#ifdef REFERENCE_IMPLEMENTATION
  clear_disk_cache();
  benchmark_reference(std::cout, filename);
#endif

#ifdef BUFFERSIZE_IMPLEMENTATION
  clear_disk_cache();
  benchmark_buffersize(std::cout, filename);
#endif

#ifdef FSTREAM_READ_IMPLEMENTATION
  clear_disk_cache();
  benchmark_fstream_read(std::cout, filename);
#endif

#ifdef FSTREAM_GETLINE_IMPLEMENTATION
  clear_disk_cache();
  benchmark_fstream_getline(std::cout, filename);
#endif

#ifdef GFILE_IMPLEMENTATION
  clear_disk_cache();
  benchmark_gfile(std::cout, filename);
#endif

#ifdef GFILE_ASYNC_IMPLEMENTATION
  clear_disk_cache();
  benchmark_gfile_async(std::cout, filename);
#endif

#ifdef CWCHESS_PGN_IMPLEMENTATION
  clear_disk_cache();
  benchmark_cwchess_pgn(std::cout, filename);
#endif
}

#ifdef GFILE_ASYNC_IMPLEMENTATION
size_t len = 0;

void async_ready_callback(GObject* source_object, GAsyncResult* async_res, gpointer user_data)
{
  GFile* file = G_FILE(user_data);
  GError* error = NULL;
  GFileInputStream* stream = g_file_read_finish(file, async_res, &error);
  if (stream == NULL)
  {
    std::cerr << "g_file_read_finish failed: " << error->message << std::endl;
    exit(1);
  }
  start_timer();
  len = 0;
  g_input_stream_read_async(G_INPUT_STREAM(stream), buf, cwchess::pgn::DatabaseSeekable::S_buffer_size,
      G_PRIORITY_DEFAULT, cancellable, async_ready_callback2, stream);
}

void async_ready_callback2(GObject* source_object, GAsyncResult* async_res, gpointer user_data)
{
  GInputStream* stream = G_INPUT_STREAM(user_data);
  GError* error = NULL;
  gssize res = g_input_stream_read_finish(stream, async_res, &error);
  if (res == -1)
  {
    std::cerr << "g_input_stream_read_finish: " << error->message << std::endl;
    exit(1);
  }
  len += res;
  if (res > 0)
    g_input_stream_read_async(G_INPUT_STREAM(stream), buf, cwchess::pgn::DatabaseSeekable::S_buffer_size,
        G_PRIORITY_DEFAULT, cancellable, async_ready_callback2, stream);
  else
  {
    uint64_t microseconds = stop_timer();
    *global_os << "g_input_stream_read_async (buffersize " << cwchess::pgn::DatabaseSeekable::S_buffer_size << "): " <<
        microseconds << " microseconds. Size read: " << len << std::endl;
    g_object_unref(stream);
    g_main_loop_quit(gmain_loop);
  }
}
#endif

#ifdef CWCHESS_PGN_IMPLEMENTATION
void open_finished(size_t len)
{
  uint64_t microseconds = stop_timer();
  *global_os << "cwchess::pgn::DatabaseSeekable (buffersize " << cwchess::pgn::DatabaseSeekable::S_buffer_size << "): " <<
      microseconds << " microseconds. Size read: " << len << "; number of lines: " << pgn_data_base->number_of_lines() << "; number of characters: " <<
      pgn_data_base->number_of_characters() << std::endl;
  main_loop->quit();
}
#endif
