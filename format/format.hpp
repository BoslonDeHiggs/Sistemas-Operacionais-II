#pragma once

#include <string>
#include <time.h>
#include <cstdint>
#include <bits/stdc++.h>

// ANSI color escape codes
#define RESET           "\033[0m"
#define WRITEOVER       "\033[F\033[K"  /*Write over line*/
#define SAVE            "\033[s"        /*Save cursor*/
#define RESTORE         "\033[u"        /*Restore cursor*/

#define BLACK           "\033[30m"      /* Black */
#define RED             "\033[31m"      /* Red */
#define GREEN           "\033[32m"      /* Green */
#define YELLOW          "\033[33m"      /* Yellow */
#define BLUE            "\033[34m"      /* Blue */
#define MAGENTA         "\033[35m"      /* Magenta */
#define CYAN            "\033[36m"      /* Cyan */
#define WHITE           "\033[37m"      /* White */

#define BRIGHT_BLACK    "\033[90m"      /* Bright Black */
#define BRIGHT_RED      "\033[91m"      /* Bright Red */
#define BRIGHT_GREEN    "\033[92m"      /* Bright Green */
#define BRIGHT_YELLOW   "\033[93m"      /* Bright Yellow */
#define BRIGHT_BLUE     "\033[94m"      /* Bright Blue */
#define BRIGHT_MAGENTA  "\033[95m"      /* Bright Magenta */
#define BRIGHT_CYAN     "\033[96m"      /* Bright Cyan */
#define BRIGHT_WHITE    "\033[97m"      /* Bright White */

using namespace std;

string format_time(time_t time);

void print_rcv_msg(time_t time, string name, string msg);

void print_send_msg(time_t time, string name, string code, string msg);

void print_client_msg(string msg);

void print_server_ntf(time_t time, string msg, string name);

void print_server_follow_ntf(time_t time, string msg, string name_1, string name_2);

void print_error_msg(string msg);

void save_cursor();

void restore_cursor();