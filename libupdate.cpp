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
} // end anonymous namespace

struct update_list
: public vector<unique_ptr<record>>
{
	vector<unique_ptr<iterator>> infos;
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
