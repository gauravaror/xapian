/** @file xapian-inspect.cc
 * @brief Inspect the contents of a brass table for development or debugging.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <cstdio> // For sprintf().

#include "brass_cursor.h"
#include "brass_defs.h"
#include "brass_table.h"
#include "brass_version.h"
#include "stringutils.h"
#include "utils.h"

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "xapian-inspect"
#define PROG_DESC "Inspect the contents of a brass table for development or debugging"

#define OPT_HELP 1
#define OPT_VERSION 2

static bool keys = true, tags = true;

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] TABLE\n\n"
"Options:\n"
"  -r, --root=ROOT  specify the root block id (default: 0)\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit" << endl;
}

static void
display_nicely(const string & data) {
    string::const_iterator i;
    for (i = data.begin(); i != data.end(); ++i) {
	unsigned char ch = *i;
	if (ch < 32 || ch >= 127) {
	    switch (ch) {
		case '\n': cout << "\\n"; break;
		case '\r': cout << "\\r"; break;
		case '\t': cout << "\\t"; break;
		default: {
		    char buf[20];
		    sprintf(buf, "\\x%02x", (int)ch);
		    cout << buf;
		}
	    }
	} else if (ch == '\\') {
	    cout << "\\\\";
	} else {
	    cout << ch;
	}
    }
}

static void
show_help()
{
    cout << "Commands:\n"
	    "next   : Next entry (alias 'n' or '')\n"
	    "prev   : Previous entry (alias 'p')\n"
	    "goto K : Goto entry with key K (alias 'g')\n"
	    "until K: Display entries until key K (alias 'u')\n"
	    "open T : Open table T instead (alias 'o') - e.g. open postlist\n"
	    "keys   : Toggle showing keys (default: true) (alias 'k')\n"
	    "tags   : Toggle showing tags (default: true) (alias 't')\n"
	    "root N : Reopen with root block N (alias 'r')\n"
	    "help   : Show this (alias 'h' or '?')\n"
	    "quit   : Quit this utility (alias 'q')" << endl;
}

static void
show_entry(BrassCursor & cursor)
{
    if (cursor.after_end()) {
	cout << "After end" << endl;
	return;
    }
    if (keys) {
	cout << "Key: ";
	display_nicely(cursor.current_key);
	cout << endl;
    }
    if (tags) {
	cout << "Tag: ";
	cursor.read_tag();
	display_nicely(cursor.current_tag);
	cout << endl;
    }
}

static void
do_until(BrassCursor & cursor, const string & target)
{
    if (cursor.after_end()) {
	cout << "At end already." << endl;
	return;
    }

    if (!target.empty()) {
	int cmp = target.compare(cursor.current_key);
	if (cmp <= 0) {
	    if (cmp)
		cout << "Already after specified key." << endl;
	    else
		cout << "Already at specified key." << endl;
	    return;
	}
    }

    while (cursor.next()) {
	int cmp = 1;
	if (!target.empty()) {
	    cmp = target.compare(cursor.current_key);
	    if (cmp < 0) {
		cout << "No exact match, stopping at entry before." << endl;
		cursor.prev();
		return;
	    }
	}
	show_entry(cursor);
	if (cmp == 0) {
	    return;
	}
    }

    cout << "Reached end." << endl;
}

int
main(int argc, char **argv)
{
    const struct option long_opts[] = {
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{"root",	required_argument, 0, 'r'},
	{NULL,		0, 0, 0}
    };

    bool root_specified = false;
    brass_block_t root_block = 0;
    int c;
    while ((c = gnu_getopt_long(argc, argv, "r:", long_opts, 0)) != -1) {
        switch (c) {
	    case OPT_HELP:
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    case 'r': {
		char * end;
		root_specified = true;
		root_block = strtoul(optarg, &end, 0);
		if (*end == '\0')
		    break;
		// FALL THRU
	    }
            default:
		show_usage();
		exit(1);
        }
    }

    if (argc - optind != 1) {
	show_usage();
	exit(1);
    }

    // Path to the table to inspect.
    string table_name(argv[optind]);
    bool arg_is_directory = dir_exists(table_name);
    if (endswith(table_name, "."BRASS_TABLE_EXTENSION)) {
	size_t cut = table_name.size() - CONST_STRLEN(BRASS_TABLE_EXTENSION);
	table_name.resize(cut);
    } else if (!endswith(table_name, '.'))
	table_name += '.';
    if (arg_is_directory && !file_exists(table_name + BRASS_TABLE_EXTENSION)) {
	cerr << argv[0] << ": You need to specify a single Btree table, not a database directory." << endl;
	exit(1);
    }

    size_t p = table_name.find_last_of('/');
#if defined __WIN32__ || defined __EMX__
    if (p == string::npos) p = 0;
    p = table_name.find_last_of('\\', p);
#endif
    if (p == string::npos) p = 0; else ++p;

    string db_dir(table_name, 0, p);

    BrassVersion version_file;
    version_file.open_most_recent(db_dir);
    unsigned block_size = version_file.get_block_size();

    if (!root_specified) {
	string leaf(table_name, db_dir.size());
	if (startswith(leaf, "postlist")) {
	    root_block = version_file.get_root_block(Brass::POSTLIST);
	} else if (startswith(leaf, "termlist")) {
	    root_block = version_file.get_root_block(Brass::TERMLIST);
	} else if (startswith(leaf, "record")) {
	    root_block = version_file.get_root_block(Brass::RECORD);
	} else if (startswith(leaf, "position")) {
	    root_block = version_file.get_root_block(Brass::POSITION);
	} else if (startswith(leaf, "spelling")) {
	    root_block = version_file.get_root_block(Brass::SPELLING);
	} else if (startswith(leaf, "synonym")) {
	    root_block = version_file.get_root_block(Brass::SYNONYM);
	} else {
	    cerr << argv[0] << ": Root block not specified, and table name "
		"unrecognised.\n"
		"Use --root BLOCK_NUMBER to specify it." << endl;
	    exit(1);
	}
    }

    show_help();
    cout << endl;

    if (!root_specified)
	cout << "Using root block " << root_block << '\n' << endl;

open_different_table:
    try {
	BrassTable table("", table_name, true);
	table.open(block_size, root_block);
	if (table.empty()) {
	    cout << "No entries!" << endl;
	}

	BrassCursor cursor(table);
	cursor.find_entry_le(string());
	cursor.next();

	while (!cin.eof()) {
	    show_entry(cursor);
	    cout << "\n";
wait_for_input:
	    cout << "? " << flush;

	    string input;
	    getline(cin, input);
	    if (cin.eof()) break;

	    if (endswith(input, '\r'))
		input.resize(input.size() - 1);

	    if (input.empty() || input == "n" || input == "next") {
		if (cursor.after_end() || !cursor.next()) {
		    cout << "At end already." << endl;
		    goto wait_for_input;
		}
		continue;
	    } else if (input == "p" || input == "prev") {
		// If the cursor has fallen off the end, point it back at
		// the last entry.
		if (cursor.after_end())
		    cursor.find_entry_le(cursor.current_key);
		if (!cursor.prev()) {
		    cout << "At start already." << endl;
		    goto wait_for_input;
		}
		continue;
	    } else if (startswith(input, "u ")) {
		do_until(cursor, input.substr(2));
		goto wait_for_input;
	    } else if (startswith(input, "until ")) {
		do_until(cursor, input.substr(6));
		goto wait_for_input;
	    } else if (input == "u" || input == "until") {
		do_until(cursor, string());
		goto wait_for_input;
	    } else if (startswith(input, "g ")) {
		if (!cursor.find_entry_le(input.substr(2))) {
		    cout << "No exact match, going to entry before." << endl;
		}
		continue;
	    } else if (startswith(input, "goto ")) {
		if (!cursor.find_entry_le(input.substr(5))) {
		    cout << "No exact match, going to entry before." << endl;
		}
		continue;
	    } else if (startswith(input, "o ")) {
		size_t slash = table_name.find_last_of('/');
		if (slash == string::npos)
		    table_name.resize(0);
		else
		    table_name.resize(slash + 1);
		table_name += input.substr(2);
		if (endswith(table_name, ".DB"))
		    table_name.resize(table_name.size() - 2);
		else if (!endswith(table_name, '.'))
		    table_name += '.';
		goto open_different_table;
	    } else if (startswith(input, "open ")) {
		size_t slash = table_name.find_last_of('/');
		if (slash == string::npos)
		    table_name.resize(0);
		else
		    table_name.resize(slash + 1);
		table_name += input.substr(5);
		if (endswith(table_name, ".DB"))
		    table_name.resize(table_name.size() - 2);
		else if (!endswith(table_name, '.'))
		    table_name += '.';
		goto open_different_table;
	    } else if (input == "t" || input == "tags") {
		tags = !tags;
		cout << "Showing tags: " << boolalpha << tags << endl;
	    } else if (input == "k" || input == "keys") {
		keys = !keys;
		cout << "Showing keys: " << boolalpha << keys << endl;
	    } else if (startswith(input, "r ")) {
		char * end;
		root_block = strtoul(input.c_str() + 2, &end, 0);
		if (*end == '\0')
		    goto open_different_table;
		cout << "Couldn't parse root block number" << endl;
		goto wait_for_input;
	    } else if (startswith(input, "root ")) {
		char * end;
		root_block = strtoul(input.c_str() + 5, &end, 0);
		if (*end == '\0')
		    goto open_different_table;
		cout << "Couldn't parse root block number" << endl;
		goto wait_for_input;
	    } else if (input == "q" || input == "quit") {
		break;
	    } else if (input == "h" || input == "help" || input == "?") {
		show_help();
		goto wait_for_input;
	    } else {
		cout << "Unknown command." << endl;
		goto wait_for_input;
	    }
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    }
}
