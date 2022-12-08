// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "libaegisub/fs.h"
#include "libaegisub/lua/ffi.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <filesystem>

using namespace agi::fs;
using namespace agi::lua;

namespace agi {
AGI_DEFINE_TYPE_NAME(DirectoryIterator);
}

namespace {
template<typename Func>
auto wrap(char **err, Func f) -> decltype(f()) {
	try {
		return f();
	}
	catch (std::exception const& e) {
		*err = strdup(e.what());
	}
	catch (agi::Exception const& e) {
		*err = strndup(e.GetMessage());
	}
	catch (...) {
		*err = strdup("Unknown error");
	}
	return 0;
}

template<typename Ret>
bool setter(const char *path, char **err, Ret (*f)(std::filesystem::path const&)) {
	return wrap(err, [=]{
		f(path);
		return true;
	});
}

bool lfs_chdir(const char *dir, char **err) {
	return setter(dir, err, &std::filesystem::current_path);
}

char *currentdir(char **err) {
	return wrap(err, []{
		return strndup(std::filesystem::current_path().string());
	});
}

bool mkdir(const char *dir, char **err) {
    return setter(dir, err, &std::filesystem::create_directories);
}

bool lfs_rmdir(const char *dir, char **err) {
	return setter(dir, err, &std::filesystem::remove);
}

bool touch(const char *path, char **err) {
	return setter(path, err, &Touch);
}

char *dir_next(DirectoryIterator &it, char **err) {
	if (it == end(it)) return nullptr;
	return wrap(err, [&]{
		auto str = strndup(*it);
		++it;
		return str;
	});
}

void dir_close(DirectoryIterator &it) {
	it = DirectoryIterator();
}

void dir_free(DirectoryIterator *it) {
	delete it;
}

DirectoryIterator *dir_new(const char *path, char **err) {
	return wrap(err, [=]{
		return new DirectoryIterator(path, "");
	});
}

const char *get_mode(const char *path, char **err) {
	return wrap(err, [=]() -> const char * {
		switch (std::filesystem::status(path).type()) {
            case std::filesystem::file_type::not_found: return nullptr;         break;
            case std::filesystem::file_type::regular:   return "file";          break;
            case std::filesystem::file_type::directory: return "directory";     break;
            case std::filesystem::file_type::symlink:   return "link";          break;
            case std::filesystem::file_type::block:     return "block device";  break;
            case std::filesystem::file_type::character: return "char device";   break;
            case std::filesystem::file_type::fifo:      return "fifo";          break;
            case std::filesystem::file_type::socket:    return "socket";        break;
            //case std::filesystem::file_type::reparse_file:   return "reparse point"; break;
			default:                  return "other";         break;
		}
	});
}

time_t get_mtime(const char *path, char **err) {
	return wrap(err, [=] { return ModifiedTime(path); });
}

uintmax_t get_size(const char *path, char **err) {
	return wrap(err, [=] { return Size(path); });
}
}

extern "C" int luaopen_lfs_impl(lua_State *L) {
	agi::lua::register_lib_table(L, {"DirectoryIterator"},
		"chdir", lfs_chdir,
		"currentdir", currentdir,
		"mkdir", mkdir,
		"rmdir", lfs_rmdir,
		"touch", touch,
		"get_mtime", get_mtime,
		"get_mode", get_mode,
		"get_size", get_size,
		"dir_new", dir_new,
		"dir_free", dir_free,
		"dir_next", dir_next,
		"dir_close", dir_close);
	return 1;
}
