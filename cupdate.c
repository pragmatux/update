/*
	Example program using the C-language bindings of the Pragmatux update
	manager.
*/

#include <stdio.h>
#include <string.h>
#include <update.h>

void
print_status (int status)
{
	char *msg = 0;

	switch (status) {
	case 0:
		/* pass */
		break;

	case -1:
		msg = /* Error: */ "usage: cupdate check\n"
		            "              cupdate update-all";
		break;

	case update_unreachable:
		msg = "The repository is unreachable over the network.";
		break;

	case update_invalid:
		msg = "The repository was reachable, but is invalid.";
		break;

	case update_error:
	default:
		msg = "unspecified";
		break;
	}

	if (msg)
		fprintf (stderr, "Error: %s\n", msg);
}

void
print_update_list (struct update_list *list)
{
	struct update_info *info = 0;
	while ((info = update_list_next (list, info)))
	{
		printf ("Package %s is upgradable "
			"from version %s to version %s\n",
			update_info_package (info),
			update_info_version_installed (info),
			update_info_version_available (info));
	}
}

int main(int argc, char* argv[])
{
	int status = -1;
	struct update_list *list;

	if (argc == 1) {
		/* pass */
	}
	else if (strcmp (argv[1], "check") == 0) {
		status = update_check (&list);
		if (status == update_okay) {
			print_update_list (list);
			update_list_free (list);
		}
	}
	else if (strcmp (argv[1], "update-all") == 0) {
		status = update_update_all ();
	}

	print_status (status);
	return status;
}
