/*
	C-language bindings for the Pragmatux update manager
*/

#include <pstreams/pstream.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <utility>
#include <limits>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include "update.h"

using redi::ipstream;
using namespace std;

namespace {
struct record
{
	string package;
	string installed_version;
	string available_version;
};

const string available_dir ("/etc/apt/sources.available.d/");
const string sourceslist_dir ("/etc/apt/sources.list.d/");

} // end anonymous namespace

struct update_list
: public vector<unique_ptr<record>>
{
	vector<unique_ptr<iterator>> infos;
};

struct update_source_entry
{
	update_source_entry (const string &s) : name (s) {}
	string name;
};

struct update_source_list
{
	typedef vector<unique_ptr<update_source_entry>> entry_list;

	entry_list entries;
	vector<unique_ptr<entry_list::iterator>> iterators;
		/* Keep a list of iterators so we can delete them. */
};

struct update_source 
: public update_source_list::entry_list::iterator
{
	template<typename T>
	update_source(T &&t) 
	: update_source_list::entry_list::iterator (t) {}
};

struct update_info
: public update_list::iterator
{
	template<typename T>
	update_info(T &&t) : update_list::iterator (t) {}
};

int
update_check (struct update_list **list)
{
	ipstream in("update check");
	unique_ptr<update_list> updates (new update_list ());

	while (in) {
		string field;
		unique_ptr<record> rec (new record);

		in >> field;
		if (in.eof ()) break;
		if (field != "Package:") return update_error;
		in >> rec->package;

		in >> field;
		if (field != "Installed-Version:") return update_error;
		in >> rec->installed_version;

		in >> field;
		if (field != "Available-Version:") return update_error;
		in >> rec->available_version;

		in.ignore (numeric_limits<streamsize>::max (), '\n');

		updates->push_back (move (rec));
	}

	in.close ();
	int status = in.rdbuf ()->status ();
	status = WEXITSTATUS(status);

	if (status == update_okay)
		*list = updates.release ();

	return status;
}

void
update_list_free(struct update_list *list)
{
	if (list) delete list;
}

int
update_list_size(struct update_list *list)
{
	return list->size ();
}

struct update_info *
update_list_next (struct update_list *list, struct update_info *cursor)
{
	struct update_info *next = 0;

	if (list->empty ()) {
		next = 0;
	}
	else if (!cursor) {
		next = new update_info (list->begin ());
		list->infos.push_back (unique_ptr<update_info> (next));
	}
	else {
		next = cursor;
		++(*next);
		if (*next == list->end ()) next = 0;
	}

	return next;
}

const char *
update_info_package (const struct update_info *info)
{
	return (**info)->package.c_str ();
}

const char *
update_info_version_available (const struct update_info *info)
{
	return (**info)->available_version.c_str ();
}

const char *
update_info_version_installed (const struct update_info *info)
{
	return (**info)->installed_version.c_str ();
}

int update_update_all ()
{
	ipstream cmd ("update update-all");
	while (cmd.ignore ());
	cmd.close ();
	int status = cmd.rdbuf ()->status ();
	return WEXITSTATUS(status);
}

int
update_source_list_available (update_source_list **pplist)
{
	unique_ptr<update_source_list> list (new update_source_list());

	/* Walk directory, create an update_source for each. */

	DIR *dir = opendir (available_dir.c_str ());
	while (dir != 0 && true) {
		struct dirent *e = readdir (dir);
		if (e) {
			if (e->d_name[0] == '.')
				continue; /* Skip hidden, ., and .. */

			list->entries.push_back (
				unique_ptr<update_source_entry> (
					new update_source_entry (e->d_name)));
		} else {
			break;
		}
	}

	closedir (dir);
	int size = list->entries.size ();
	*pplist = list.release ();
	return size;
}

void
update_source_list_free (update_source_list *plist)
{
	if (plist)
		delete plist;
}

struct update_source *
update_source_list_next (update_source_list *list, update_source *cursor)
{
	if (list->entries.size () == 0) {
		cursor = 0;
	}
	else if (cursor == 0) {
		cursor = new update_source (list->entries.begin ());
		list->iterators.push_back (unique_ptr<update_source> (cursor));
	}
	else {
		++(*cursor);
		if (*cursor == list->entries.end ())
			cursor = 0;
	}

	return cursor;
}

const char *
update_source_description (const update_source *source)
{
	/* Look for the first line of the form "# Description: This is a
	 * description" in the relevant sources.list.*/

	if (source == 0)
		return 0;

	const update_source_entry &entry = ***source;
	ifstream file (available_dir + entry.name);
	static string result;

	while (file) {
		string line;
		getline (file, line);
		if (line.length () == 0)
			continue;
		if (line[0] != '#')
			continue;

		size_t c = line.find_first_not_of ("# \t");
		if (c == string::npos)
			continue;

		string label ("Description: ");
		if (line.find (label) != c)
			continue;

		c = line.find_first_not_of (" \t", c + label.length ());
		result = line.substr (c);
		break;
	}

	return result.c_str ();
}

int
update_source_is_enabled (const update_source *source)
{
	/* A source is enabled if a symlink to it exists in
	   /etc/apt/sources.list.d. */

	if (source == 0)
		return 0;

	const update_source_entry &entry = ***source;

	struct stat link_stat;
	string link_path = sourceslist_dir + entry.name;
	int status = stat (link_path.c_str (), &link_stat);
	if (status != 0)
		return 0;

	struct stat source_stat;
	string source_path = available_dir + entry.name;
	status = stat (source_path.c_str (), &source_stat);
	if (status != 0)
		return 0;

	return link_stat.st_dev == source_stat.st_dev &&
	       link_stat.st_ino == source_stat.st_ino;
}

int
update_source_enable (const update_source *source)
{
	/* Put a symlink to the source in /etc/apt/sources.list.d. */

	if (source == 0)
		return -1;

	const update_source_entry &entry = ***source;
	const string contents = "../sources.available.d/" + entry.name;
	const string link_path = sourceslist_dir + entry.name;

	return symlink (contents.c_str (), link_path.c_str ());
}

void
update_source_disable (const update_source *source)
{
	/* Remove the symlink in /etc/apt/sources.list.d. */

	if (update_source_is_enabled (source)) {
		const string link_path = sourceslist_dir + (**source)->name;
		unlink (link_path.c_str ());
	}
}
