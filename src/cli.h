// SPDX-FileCopyrightText: 2025 Geoffrey D. Bennett <g@b4.vu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// CLI mode entry point
// Returns 0 on success, 1 on failure
int cli_run(int argc, char **argv);

// Print CLI usage/help
void cli_print_help(void);

// Print available mixer controls
void cli_list_controls(void);
