// $Id: jsoon.h 12079 2011-04-11 05:05:55Z cedric.shih $
/*
 * Copyright (c) 2007-2008 Mantaray Technology, Incorporated.
 * Rm. A407, No.18, Si Yuan Street, Taipei, 100, Taiwan.
 * Phone: +886-2-23681570. Fax: +886-2-23682417.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted without specific written permission
 * from above copyright holder.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY MANTARAY TECHNOLOGY INCORPORATED
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * MANTARAY TECHNOLOGY INCORPORATED BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 */

#ifndef JSOON_H_
#define JSOON_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @file

 A helper library for json-c.

 */

#include <sys/types.h>
#include <errno.h>
#ifdef WIN32
#define EOVERFLOW 75
#endif

#include <json/json.h>

/**
 * @brief Get version of jsoon.
 *
 * @return a non-null version string
 */
const char *jsoon_get_version(void);

/**
 * @brief Error numbers of jsoon.
 */
enum jsoon_errno {
	JSOON_EOK = 0, /**< Success. */
	JSOON_EINVAL = EINVAL, /**< Invalid argument(s). */
	JSOON_ENOMEM = ENOMEM, /**< Out of memory. */
	JSOON_ENOATTR = ENOENT, /**< No such attribute. */
	JSOON_EEXIST = EEXIST, /**< Attribute already exists. */
	JSOON_EOVERFLOW = EOVERFLOW, /**< Buffer overflows. */
	JSOON_ERANGE = ERANGE, /**< Array index out of bound. */
	JSOON_EUNKNOWN = 0x8000, /**< Unexpected error. */
	JSOON_ETYPE, /**< Type mismatch. */
	JSOON_ENOTOBJ, /**< Not an object type json object. */
	JSOON_ENOTARR, /**< Not an array type json object. */
};

/**
 * @brief Translate error number to localized string representation.
 *
 * @param[in] err - error number to translate
 * @return localized string of given error number
 */
const char *jsoon_strerror(enum jsoon_errno err);

/**
 * @brief Create a json object.
 *
 * @param obj - address to store reference of created json object
 * @return 0 on success; JSOON_EINVAL if obj argument is NULL; JSOON_ENOMEM on
 insufficient memory
 */
inline int jsoon_new(struct json_object **obj);

/**
 * @brief Release resources on a json object (and its children).
 *
 * @param obj - json object to put (NULL safe)
 */
inline void jsoon_free(struct json_object *obj);

/**
 * @brief Get object attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param obj - address to store reference of object attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if
 json argument is not object type; JSOON_ENOATTR if attribute doesn't exist;
 JSOON_ETYPE if specified attribute is not object type; JSOON_EUNKNOWN on
 unexpected error
 * @see jsoon_find_obj()
 */
inline int jsoon_get_obj(const struct json_object *json, const char *name,
		struct json_object **obj);

/**
 * @brief Get array attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param array - address to store reference of array attribute
 * @param len - address to store the size of array
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ
 if json argument is not array type; JSOON_ENOATTR if attribute doesn't exist;
 JSOON_ETYPE if specified attribute is not object type; JSOON_EUNKNOWN on
 unexpected error
 * @see jsoon_find_array()
 */
inline int jsoon_get_array(const struct json_object *json, const char *name,
		struct json_object **array, unsigned int *len);

/**
 * @brief Get integer attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param value - address to store the value of integer attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ
 if json argument is not object type; JSOON_ENOATTR if attribute doesn't exist;
 JSOON_ETYPE if specified attribute is not integer type; JSOON_EUNKNOWN on
 unexpected error
 * @see jsoon_find_int()
 */
inline int jsoon_get_int(const struct json_object *json,
		const char *name, int *value);

int jsoon_get_ranged_int(const struct json_object *obj, const char *name,
		int min, int max, int *out);

/**
 * @brief Get double attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param value - address to store the value of double attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ
 if json argument is not object type; JSOON_ENOATTR if attribute doesn't exist;
 JSOON_ETYPE if specified attribute is not double type; JSOON_EUNKNOWN on
 unexpected error
 * @see jsoon_find_double()
 */
inline int jsoon_get_double(const struct json_object *json,
		const char *name, double *value);

int jsoon_get_ranged_double(const struct json_object *json,
		const char *name, double min, double max, double *value);

/**
 * @brief Get boolean attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param value - address to store the value of boolean attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ
 if json argument is not object type; JSOON_ENOATTR if attribute doesn't exist;
 JSOON_ETYPE if specified attribute is not boolean type; JSOON_EUNKNOWN on
 unexpected error
 * @see jsoon_find_bool()
 */
inline int jsoon_get_bool(const struct json_object *json, const char *name,
		int *value);

/**
 * @brief Get string attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param string - address to store the pointer of string attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ
 if json argument is not object type; JSOON_ENOATTR if attribute doesn't exist;
 JSOON_ETYPE if specified attribute is not string type; JSOON_EUNKNOWN on
 unexpected error
 * @see jsoon_find_str()
 */
inline int jsoon_get_str(const struct json_object *json,
		const char *name, const char **string);

/**
 * @brief Get and copy string attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param buffer - buffer for copying the value of string attribute
 * @param size - size of buffer
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ
 if json argument is not object type; JSOON_ENOATTR if attribute doesn't exist;
 JSOON_ETYPE if specified attribute is not string type; JSOON_EUNKNOWN on
 unexpected error; JSOON_EOVERFLOW if buffer size is not enough for the string
 value
 * @see jsoon_find_strcpy()
 */
int jsoon_get_strcpy(const struct json_object *json,
		const char *name, char *buffer, size_t size);

/**
 * @brief Find object attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param obj - address to store reference of object attribute
 * @return 0 on success or attribute not found with value of obj argument set
 to NULL; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if json argument
 is not object type; JSOON_ETYPE if specified attribute is not object type;
 JSOON_EUNKNOWN on unexpected error
 * @see jsoon_get_obj()
 */
inline int jsoon_find_obj(const struct json_object *json, const char *name,
		struct json_object **obj);

/**
 * @brief Find array attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param array - address to store reference of array attribute
 * @param len - address to store the size of array
 * @return 0 on success or attribute not found with value of array argument
 set to NULL; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if json
 argument is not array type; JSOON_ETYPE if specified attribute is not object type; JSOON_EUNKNOWN on unexpected error
 * @see jsoon_get_array()
 */
inline int jsoon_find_array(const struct json_object *json, const char *name,
		struct json_object **array, int *maxlen);

/**
 * @brief Find integer attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param preset - preset value if attribute not found
 * @param value - address to store the value of integer attribute
 * @return 0 on success or attribute not found with value of value argument
 set to preset argument; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if
 json argument is not object type; JSOON_ETYPE if specified attribute is not
 integer type; JSOON_EUNKNOWN on unexpected error
 * @see jsoon_get_int()
 */
inline int jsoon_find_int(const struct json_object *json,
		const char *name, int preset, int *value);

int jsoon_find_ranged_int(const struct json_object *json,
		const char *name, int preset, int min, int max, int *value);

/**
 * @brief Find double attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param preset - preset value if attribute not found
 * @param value - address to store the value of double attribute
 * @return 0 on success or attribute not found with value of value argument set
 to preset argument; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if json
 argument is not object type; JSOON_ETYPE if specified attribute is not double
 type; JSOON_EUNKNOWN on unexpected error
 * @see jsoon_get_double()
 */
inline int jsoon_find_double(const struct json_object *json,
		const char *name, double preset, double *value);

int jsoon_find_ranged_double(const struct json_object *json,
		const char *name, double preset, double min, double max,
		double *value);

/**
 * @brief Find boolean attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param preset - preset value if attribute not found
 * @param value - address to store the value of boolean attribute
 * @return 0 on success or attribute not found with value of value argument set
 to preset argument; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if json
 argument is not object type; JSOON_ETYPE if specified attribute is not boolean
 type; JSOON_EUNKNOWN on unexpected error
 * @see jsoon_get_bool()
 */
inline int jsoon_find_bool(const struct json_object *json, const char *name,
		int preset, int *value);

/**
 * @brief Find string attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param preset - preset value if attribute not found
 * @param string - address to store the value of string attribute
 * @return 0 on success or attribute not found with value of string argument
 set to preset argument; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if
 json argument is not object type; JSOON_ETYPE if specified attribute is not
 string type; JSOON_EUNKNOWN on unexpected error
 * @see jsoon_get_str()
 */
inline int jsoon_find_str(const struct json_object *json,
		const char *name, const char *preset, const char **string);

/**
 * @brief Find string attribute from a json object.
 *
 * @param json - json object to get attribute
 * @param name - attribute name
 * @param preset - preset value if attribute not found
 * @param buffer - buffer for copying the value of string attribute
 * @param size - size of buffer
 * @return 0 on success or attribute not found with preset argument copied to
 specified buffer; JSOON_EINVAL if any argument is NULL; JSOON_ENOTOBJ if json
 argument is not object type; JSOON_ETYPE if specified attribute is not string
 type; JSOON_EUNKNOWN on unexpected error; JSOON_EOVERFLOW if buffer size is not
 enough for the string or preset value
 * @see jsoon_get_strcpy()
 */
int jsoon_find_strcpy(const struct json_object *json, const char *name,
		const char *preset, char *buffer, size_t size);

/**
 * @brief Add object attribute to a json object.
 *
 * @param json - json object to add attribute
 * @param name - attribute name
 * @param obj - address to store the reference of the new object attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOMEM on
 insufficient memory; JSOON_ENOTOBJ if json argument is not object type;
 JSOON_EEXIST if attribute already exists; JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_add_obj(struct json_object *json,
		const char *name, struct json_object **obj);

/**
 * @brief Add array attribute to a json object.
 *
 * @param json - json object to add attribute
 * @param name - attribute name
 * @param array - address to store the reference of the new array attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTOBJ if json argument is not object type;
 JSOON_EEXIST if attribute already exists; JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_add_array(struct json_object *json,
		const char *name, struct json_object **array);

/**
 * @brief Add integer attribute to a json object.
 *
 * @param json - json object to add attribute
 * @param name - attribute name
 * @param value - value of integer attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTOBJ if json argument is not object type;
 JSOON_EEXIST if attribute already exists; JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_add_int(struct json_object *json,
		const char *name, int value);

/**
 * @brief Add double attribute to a json object.
 *
 * @param json - json object to add attribute
 * @param name - attribute name
 * @param value - value of double attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTOBJ if json argument is not object type;
 JSOON_EEXIST if attribute already exists; JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_add_double(struct json_object *json,
		const char *name, double value);

/**
 * @brief Add boolean attribute to a json object.
 *
 * @param json - json object to add attribute
 * @param name - attribute name
 * @param value - value of boolean attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTOBJ if json argument is not object type;
 JSOON_EEXIST if attribute already exists; JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_add_bool(struct json_object *json, const char *name,
		int bool);

/**
 * @brief Add string attribute to a json object.
 *
 * @param json - json object to add attribute
 * @param name - attribute name
 * @param value - value of string attribute
 * @return 0 on success; JSOON_EINVAL if any argument is NULL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTOBJ if json argument is not object type;
 JSOON_EEXIST if attribute already exists; JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_add_str(struct json_object *json, const char *name,
		const char *value);

/**
 * @brief Create a json array object.
 *
 * @param obj - address to store reference of created json object
 * @return 0 on success; JSOON_EINVAL if obj argument is NULL; JSOON_ENOMEM on
 insufficient memory
 */
inline int jsoon_array_new(struct json_object **array);

/**
 * @brief Add object element into a json array object.
 *
 * @param json - json array object to add element
 * @param obj - address to store the reference of created object element
 * @return 0 on success; JSOON_EINVAL if any argument is NULLL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTARR if json argument is not array type;
 JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_array_add_obj(struct json_object *json,
		struct json_object **obj);

/**
 * @brief Add array element into a json array object.
 *
 * @param json - json array object to add element
 * @param array - address to store the reference of created array element
 * @return 0 on success; JSOON_EINVAL if any argument is NULLL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTARR if json argument is not array type;
 JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_array_add_array(struct json_object *json,
		struct json_object **array);

/**
 * @brief Add string element into a json array object.
 *
 * @param json - json array object to add element
 * @param string - value of string element
 * @return 0 on success; JSOON_EINVAL if any argument is NULLL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTARR if json argument is not array type;
 JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_array_add_str(struct json_object *json,
		const char *string);

/**
 * @brief Add integer element into a json array object.
 *
 * @param json - json array object to add element
 * @param value - value of integer element
 * @return 0 on success; JSOON_EINVAL if any argument is NULLL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTARR if json argument is not array type;
 JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_array_add_int(struct json_object *json,
		int value);

/**
 * @brief Add double element into a json array object.
 *
 * @param json - json array object to add element
 * @param value - value of double element
 * @return 0 on success; JSOON_EINVAL if any argument is NULLL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTARR if json argument is not array type;
 JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_array_add_double(struct json_object *json,
		double value);

/**
 * @brief Add boolean element into a json array object.
 *
 * @param json - json array object to add element
 * @param value - value of boolean element
 * @return 0 on success; JSOON_EINVAL if any argument is NULLL; JSOON_ENOMEM
 on insufficient memory; JSOON_ENOTARR if json argument is not array type;
 JSOON_EUNKNOWN on unexpected error
 */
inline int jsoon_array_add_bool(struct json_object *json,
		int value);

/**
 * @brief Get object element from a json array object.
 *
 * @param json - json array object to get element
 * @param idx - array index of element
 * @param obj - address to store the reference of object element
 * @return 0 on success; JSOON_EINVAL if any pointer argument is NULL;
 JSOON_NOTARR if json argument is not array type; JSOON_ERANGE on array index
 out of bound; JSOON_ETYPE if element is not object type
 */
inline int jsoon_array_get_obj(struct json_object *json, unsigned int idx,
		struct json_object **obj);

/**
 * @brief Get array element from a json array object.
 *
 * @param json - json array object to get element
 * @param idx - array index of element
 * @param array - address to store the reference of array element
 * @return 0 on success; JSOON_EINVAL if any pointer argument is NULL;
 JSOON_NOTARR if json argument is not array type; JSOON_ERANGE on array index
 out of bound; JSOON_ETYPE if element is not array type
 */
inline int jsoon_array_get_array(struct json_object *json, unsigned int idx,
		struct json_object **array);

/**
 * @brief Get string element from a json array object.
 *
 * @param json - json array object to get element
 * @param idx - array index of element
 * @param value - address to store the pointer to string value
 * @return 0 on success; JSOON_EINVAL if any pointer argument is NULL;
 JSOON_NOTARR if json argument is not array type; JSOON_ERANGE on array index
 out of bound; JSOON_ETYPE if element is not string type
 */
inline int jsoon_array_get_str(struct json_object *json, unsigned int idx,
		const char **value);

/**
 * @brief Get integer element from a json array object.
 *
 * @param json - json array object to get element
 * @param idx - array index of element
 * @param value - address to store the integer value
 * @return 0 on success; JSOON_EINVAL if any pointer argument is NULL;
 JSOON_NOTARR if json argument is not array type; JSOON_ERANGE on array index
 out of bound; JSOON_ETYPE if element is not integer type
 */
inline int jsoon_array_get_int(struct json_object *json, unsigned int idx,
		int *value);

/**
 * @brief Get double element from a json array object.
 *
 * @param json - json array object to get element
 * @param idx - array index of element
 * @param value - address to store the double value
 * @return 0 on success; JSOON_EINVAL if any pointer argument is NULL;
 JSOON_NOTARR if json argument is not array type; JSOON_ERANGE on array index
 out of bound; JSOON_ETYPE if element is not double type
 */
inline int jsoon_array_get_double(struct json_object *json, unsigned int idx,
		double *value);

/**
 * @brief Get boolean element from a json array object.
 *
 * @param json - json array object to get element
 * @param idx - array index of element
 * @param value - address to store the boolean value
 * @return 0 on success; JSOON_EINVAL if any pointer argument is NULL;
 JSOON_NOTARR if json argument is not array type; JSOON_ERANGE on array index
 out of bound; JSOON_ETYPE if element is not boolean type
 */
inline int jsoon_array_get_bool(struct json_object *json, unsigned int idx,
		int *value);

/**
 * @brief Marshal json object into string representation.
 *
 * @param json - json object to marshal
 * @return string representation of json object
 */
inline const char *jsoon_to_str(const struct json_object *json);

/**
 * @brief Translate json type to string representation.
 *
 * @param type - json type
 * @return string representation of json type
 */
const char *jsoon_type2str(enum json_type type);

#ifdef __cplusplus
}
#endif

#endif /* JSOON_H_ */
