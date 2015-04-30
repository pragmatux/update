/*
	Example program for manipulating update sources using the C-language
	bindings of the Pragmatux update manager.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "update.h"

const char *usage = "\
update-status list\n\
update-status enable SOURCE\n\
update-status disable SOURCE\n";

int list()
{
	struct update_source_list *sources;
	struct update_source *source;
	int number = 1;

	update_source_list_available (&sources);

	for (source = update_source_list_next(sources, 0);
	     source != 0;
	     source = update_source_list_next(sources, source))
	{
		printf("%i: %s (%s)\n",
		       number++,
		       update_source_description(source),
		       update_source_is_enabled(source) ? "enabled" : "disabled");
	}

	update_source_list_free(sources);

	return EXIT_SUCCESS;
}

int enable(const char *choice)
{
	struct update_source_list *sources;
	struct update_source *source;
	int number = 1;
	int target = atoi(choice);
	int status = EXIT_FAILURE;

	update_source_list_available (&sources);

	for (source = update_source_list_next(sources, 0);
	     source != 0;
	     source = update_source_list_next(sources, source))
	{
		if (number++ == target) {
			status = update_source_enable(source);
			break;
		}
	}

	update_source_list_free(sources);

	return status;
}

int disable(const char *choice)
{
	struct update_source_list *sources;
	struct update_source *source;
	int number = 1;
	int target = atoi(choice);
	int status = EXIT_FAILURE;

	update_source_list_available (&sources);

	for (source = update_source_list_next(sources, 0);
	     source != 0;
	     source = update_source_list_next(sources, source))
	{
		if (number++ == target) {
			update_source_disable(source);
			status = EXIT_SUCCESS;
			break;
		}
	}

	update_source_list_free(sources);

	return status;
}

int main(int argc, char* argv[])
{

	if (argc < 2) {
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "list") == 0) {
		return list();
	} else if ((argc == 3) && (strcmp(argv[1], "enable") == 0)) {
		return enable(argv[2]);
	} else if ((argc == 3) && (strcmp(argv[1], "disable") == 0)) {
		return disable(argv[2]);
	} else {
		fprintf(stderr, usage);
		return EXIT_FAILURE;
	}
}
