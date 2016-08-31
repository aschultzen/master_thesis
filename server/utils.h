/**
 * @file utils.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing function prototypes and includes for utils.c
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "list.h"
#include "config.h"

/** @brief Terminates program and prints the line
 *		   number where die was called from.
 *
 *  @param line_number Line number where die() was written
 *  @param format String with error description.
 *	@return Void
 */
void die (int line_number, const char * format, ...);

/** @brief Extracts IP address from file descriptor
 *
 *	@param session_fd file descriptor for the session
 *	@param ip Buffer to store the IP address as string.
 */
void get_ip_str(int session_fd, char *ip);

/** @brief Extracts IP address from sockaddr struct
 *
 *	Used by get_ip_str() to extract IP address from
 *	sockaddr struct.
 *
 *	@param session_fd file descriptor for the session
 *	@param ip Buffer to store the IP address as string.
 *	@return Void
 */
void extract_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);

/** @brief Print function with time-stamp
 *
 *	Print function like printf() but with time-stamp
 *  in square bracket appended before the String.
 * 	Example: [04/13/16 - 08:50:41] Waiting for connections..
 *
 *	@param format String to print
 *	@return Void
 */
void t_print(const char* format, ...);

/** @brief Loads config from file using config_map_entry struct
 *
 *	Uses the config_map_entry struct to find the correct entry
 * 	in the config file, cast it to correct type and fill the
 * 	respective memory area (pointer).
 *
 *	@param cme Pointer to the config_map_entry struct
 *	@param path Path to config file
 *	@param entries Entries in the config file
 *	@return 1 if success, 0 if fail.
 */
int load_config(struct config_map_entry *cme, char *path, int entries);

/** @brief Calculates the checksum of a given string of NMEA data.
 *
 *  Used to check the integrity of NMEA data from the
 *	GPS receiver before potential analysis.
 *
 *	@param nmea String containing NMEA data to check
 *	@return 1 if success, 0 if fail.
 */
int calculate_nmea_checksum(char *s);

/** @brief Extracts words from a String
 *
 *	Used to extract a substring from a string by using a
 *  delimiter. The from and to parameters defines which
 *	occurrence of the delimiter in the parent string to
 *	use as start and end for the substring.
 *
 *	@param start The delimiter number to start from
 *	@param end The delimiter number to stop
 *	@param delimiter Symbol/character to use as delimit
 *	@param buffer Buffer to store the word(s)
 *	@param buffsize Size of buffer
 *	@param string Pointer to parent string
 *	@param str_len Length of parent string
 *	@return 1 if success, 0 if no string within the delimits was found.
 */
int substring_extractor(int start, int end, char delimiter, char *buffer, int buffsize, char *string, int str_len);

/** @brief Counts bytes from start to first occurence of null character
 *
 *	@param buffer Buffer to search through
 *	@param buf_len Length of the buffer in bytes
 */
int str_len_u(char *buffer, int buf_len);

/** @brief Calls a script using shell_invoke to get mjd(now).
 *
 *	@param buffer Buffer to store response
 */
int get_today_mjd(char *buffer);

/** @brief Run a script or a program through the shell
 *
 *	@param path Path to program
 *	@param output Buffer to store response
 */
int shell_invoke(char *path, char *output);
#endif /* !UTILS_H */