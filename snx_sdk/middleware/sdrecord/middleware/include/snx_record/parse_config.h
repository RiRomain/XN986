/*
 * parse_config.h
 *
 *  Created on: 2013-8-19
 *      Author: wanglei
 */

#ifndef __PARSE_CONFIG_H_
#define __PARSE_CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "xmllib.h"
void write_config(char *filename,xml_doc config, int record_id, char *key, char *value);
void read_config(char *filename,xml_doc config, int record_id, char *key,char *value);
#endif
