// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file text_file_writer.cpp
/// @brief Write plain text files line by line
/// @ingroup utility
///

#include "text_file_writer.h"

#include "options.h"

#include <libaegisub/io.h>
#include <libaegisub/fs.h>
#include <libaegisub/charset_conv.h>

#include <boost/algorithm/string/case_conv.hpp>

TextFileWriter::TextFileWriter(std::filesystem::path const& filename, std::string encoding)
: file(new agi::io::Save(filename, true))
{
	if (encoding.empty())
		encoding = OPT_GET("App/Save Charset")->GetString();
	if (encoding != "utf-8" && encoding != "UTF-8") {
		conv = std::make_unique<agi::charset::IconvWrapper>("utf-8", encoding.c_str(), true);
		newline = conv->Convert(newline);
	}

	try {
		// Write the BOM
		WriteLineToFile("\xEF\xBB\xBF", false);
	}
	catch (agi::charset::ConversionFailure&) {
		// If the BOM could not be converted to the target encoding it isn't needed
	}
}

TextFileWriter::~TextFileWriter() {
	try {
		file->Close();
	}
	catch (agi::fs::FileSystemError const&e) {
#if wxCHECK_VERSION (3, 1, 0)
		wxString m = wxString::FromUTF8(e.GetMessage());
#else
		wxString m = wxString::FromUTF8(e.GetMessage().c_str(), e.GetMessage().size());
#endif
		if (!m.empty())
			wxMessageBox(m, "Exception in agi::io::Save", wxOK | wxCENTRE | wxICON_ERROR);
		else
			wxMessageBox(e.GetMessage(), "Exception in agi::io::Save", wxOK | wxCENTRE | wxICON_ERROR);
	}
}

void TextFileWriter::WriteLineToFile(std::string const& line, bool addLineBreak) {
	if (conv) {
		auto converted = conv->Convert(line);
		file->Get().write(converted.data(), converted.size());
	}
	else
		file->Get().write(line.data(), line.size());

	if (addLineBreak)
		file->Get().write(newline.data(), newline.size());
}
