// SPDX-FileCopyrightText: 2025 Geoffrey D. Bennett <g@b4.vu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <glib.h>

#include "cli.h"

// Forward declaration - defined in alsa.c
struct alsa_elem *get_elem_by_name(GPtrArray *elems, const char *name);
void alsa_init(void);
void alsa_set_elem_value(struct alsa_elem *elem, long value);
long alsa_get_elem_value(struct alsa_elem *elem);

// External from alsa.c
extern GArray *alsa_cards;

// Minimal structures to avoid GTK dependency
struct alsa_elem {
  struct alsa_card *card;
  int numid;
  char *name;
  int type;
  int count;
  int index;
  int min_val;
  int max_val;
  int is_simulated;
  long value;
  long *values;
};

struct alsa_card {
  int num;
  char *device;
  snd_ctl_t *handle;
  GPtrArray *elems;
};

// Print CLI usage/help
void cli_print_help(void) {
  printf("Usage: alsa-scarlett-gui --cli <command> [options]\n");
  printf("\n");
  printf("Commands:\n");
  printf("  set <control> <value>     Set mixer control to value (in dB or raw units)\n");
  printf("  get <control>             Get current value of mixer control\n");
  printf("  list                      List all available mixer controls\n");
  printf("  help                      Show this help message\n");
  printf("\n");
  printf("Examples:\n");
  printf("  alsa-scarlett-gui --cli set \"Mix A 1 Input 1 Playback Volume\" -10\n");
  printf("  alsa-scarlett-gui --cli get \"Mix A 1 Input 1 Playback Volume\"\n");
  printf("  alsa-scarlett-gui --cli list\n");
}

// Print available mixer controls
void cli_list_controls(void) {
  if (!alsa_cards || alsa_cards->len == 0) {
    fprintf(stderr, "No Scarlett devices found\n");
    return;
  }

  struct alsa_card *card = &g_array_index(alsa_cards, struct alsa_card, 0);
  if (!card || !card->elems) {
    fprintf(stderr, "No controls available\n");
    return;
  }

  printf("Available mixer controls:\n\n");

  for (int i = 0; i < card->elems->len; i++) {
    struct alsa_elem *elem = g_ptr_array_index(card->elems, i);

    if (!elem || !elem->name)
      continue;

    // Filter to show mainly mixer gains
    if (!strstr(elem->name, "Playback Volume") &&
        !strstr(elem->name, "Capture Volume"))
      continue;

    if (elem->type == SND_CTL_ELEM_TYPE_INTEGER) {
      long value = alsa_get_elem_value(elem);
      printf("  %s\n", elem->name);
      printf("    Current: %ld (range: %d to %d)\n", 
             value, elem->min_val, elem->max_val);
      printf("\n");
    }
  }
}

// Find and set a control value
static int cli_set_control(const char *name, const char *value_str) {
  if (!alsa_cards || alsa_cards->len == 0) {
    fprintf(stderr, "Error: No Scarlett devices found\n");
    return 1;
  }

  struct alsa_card *card = &g_array_index(alsa_cards, struct alsa_card, 0);
  if (!card || !card->elems) {
    fprintf(stderr, "Error: No controls available\n");
    return 1;
  }

  // Parse the value
  char *endptr;
  long value = strtol(value_str, &endptr, 10);
  if (*endptr != '\0') {
    fprintf(stderr, "Error: Invalid value '%s'. Must be an integer.\n", value_str);
    return 1;
  }

  // Find the control
  struct alsa_elem *elem = get_elem_by_name(card->elems, name);
  if (!elem) {
    fprintf(stderr, "Error: Control '%s' not found\n", name);
    return 1;
  }

  // Validate range
  if (value < elem->min_val || value > elem->max_val) {
    fprintf(stderr, 
            "Error: Value %ld out of range [%d, %d] for '%s'\n",
            value, elem->min_val, elem->max_val, name);
    return 1;
  }

  // Set the value
  alsa_set_elem_value(elem, value);
  printf("Set '%s' to %ld\n", name, value);
  return 0;
}

// Find and get a control value
static int cli_get_control(const char *name) {
  if (!alsa_cards || alsa_cards->len == 0) {
    fprintf(stderr, "Error: No Scarlett devices found\n");
    return 1;
  }

  struct alsa_card *card = &g_array_index(alsa_cards, struct alsa_card, 0);
  if (!card || !card->elems) {
    fprintf(stderr, "Error: No controls available\n");
    return 1;
  }

  // Find the control
  struct alsa_elem *elem = get_elem_by_name(card->elems, name);
  if (!elem) {
    fprintf(stderr, "Error: Control '%s' not found\n", name);
    return 1;
  }

  // Get and print the value
  long value = alsa_get_elem_value(elem);
  printf("%ld\n", value);
  return 0;
}

// CLI mode entry point
int cli_run(int argc, char **argv) {
  // Initialize ALSA without GTK
  alsa_init();

  // Give ALSA a moment to enumerate cards
  sleep(1);

  if (argc < 2) {
    cli_print_help();
    return 1;
  }

  const char *command = argv[1];

  if (strcmp(command, "help") == 0) {
    cli_print_help();
    return 0;
  }

  if (strcmp(command, "list") == 0) {
    cli_list_controls();
    return 0;
  }

  if (strcmp(command, "set") == 0) {
    if (argc < 4) {
      fprintf(stderr, "Error: 'set' requires control name and value\n");
      fprintf(stderr, "Usage: alsa-scarlett-gui --cli set <control> <value>\n");
      return 1;
    }
    return cli_set_control(argv[2], argv[3]);
  }

  if (strcmp(command, "get") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Error: 'get' requires control name\n");
      fprintf(stderr, "Usage: alsa-scarlett-gui --cli get <control>\n");
      return 1;
    }
    return cli_get_control(argv[2]);
  }

  fprintf(stderr, "Error: Unknown command '%s'\n", command);
  cli_print_help();
  return 1;
}
