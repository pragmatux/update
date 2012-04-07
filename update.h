/*
	C-language bindings for the Pragmatux update manager
*/

#ifndef UPDATE_H_INCLUDED
#define UPDATE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	update_okay = 0,
	update_error = 1,
	update_unreachable = 64,
	update_invalid = 65
} update_result;

struct update_list;
struct update_info;

/*
	Return a list describing the available updates.  This list must
	eventually be freed with update_list_free().  Return a value from
	update_result.
*/
int
update_check (struct update_list **);

/*
	Free an update list.
*/
void
update_list_free (struct update_list *);

/*
	Return the next update info following the cursor in a update list.  If
	the cursor is null, return the first update in the list.  A null return
	indicates the end of the list.
*/
struct update_info *
update_list_next (struct update_list *, struct update_info *cursor);

/*
	Return the name of the package available for update.
*/
const char *
update_info_package (const struct update_info *);

/*
	Return the version of the package to be installed by the available
	update.
*/
const char *
update_info_version_available (const struct update_info *);

/*
	Return the version of the package that is currently installed.
*/
const char *
update_info_version_installed (const struct update_info *);

/*
	Install all updates discovered by the last check.  Return a value from
	update_result.
*/
int
update_update_all ();

#ifdef __cplusplus
}
#endif

#endif /* UPDATE_H_INCLUDED */
