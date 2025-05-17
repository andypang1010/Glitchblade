//
//  CUJSON.h
//
//  (Fork of) Ultralightweight JSON parser in ANSI C.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
//  Copyright (c) 2009 Dave Gamble
//
//  Fork 2024 by Walker White
//  This fork is to prevent naming collisions in with later versions of this
//  package.  We should really abandon this back for the modern C++ JSON, but
//  we are not ready to do that yet. WidgetValues are tied too tightly to this.
//
#ifndef __CU_JSON_H__
#define __CU_JSON_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

#pragma mark -
#pragma mark JSON Types

// CUJSON Types: 
/** JSON False Type */
#define CUJSON_False  (1 << 0)
/** JSON True Type */
#define CUJSON_True   (1 << 1)
/** JSON Nullptr Type */
#define CUJSON_NULL   (1 << 2)
/** JSON Number (int or float) Type */
#define CUJSON_Number (1 << 3)
/** JSON String Type */
#define CUJSON_String (1 << 4)
/** JSON Array Type */
#define CUJSON_Array  (1 << 5)
/** JSON Object Type */
#define CUJSON_Object (1 << 6)
/** JSON Reference Type (for allocations) */
#define CUJSON_IsReference 256
/** JSON String constant Type (for allocations) */
#define CUJSON_StringIsConst 512

#pragma mark -
#pragma mark JSON Structs

/**
 * Instances are a parsed JSON tree 
 *
 * The struct is coded as an arbitrary branching tree.  Each node is part of a doubly
 * linked sibling list.  Child pointers point to the first element of such a list.
 *
 * As a struct, this uses a C-style object-oriented interface.  While it is possible
 * to access the struct directly, most access is done via the function interface
 */
typedef struct CUJSON {
    /** The next entry in a sibling list */
    struct CUJSON *next;
    /** The previous entry in a sibling list */
    struct CUJSON *prev;
    /** The first entry in a sibling list of children */
    struct CUJSON *child;
    
    /** The item's name string, if this item part of an object. */
    char *string;

    /** The type of the item, as above. */
    int type;

    /** The item's string, if type==CUJSON_String */
    char *valuestring;
    /** The item's number, if type==CUJSON_Number */
    int valueint;
    /** The item's number, if type==CUJSON_Number */
    double valuedouble;

} CUJSON;

/**
 * Redefined malloc and free for custom memory management
 */
typedef struct CUJSON_Hooks {
	/** 
	 * Returns an allocated block of the given size
	 *
	 * @param sz  The size to allocate
	 *
	 * @return an allocated block of the given size
	 */
	void *(*malloc_fn)(size_t sz);
	
	/**
	 * Frees a previously allocated pointer
	 *
	 * @param ptr the pointer to free
	 */
	void (*free_fn)(void *ptr);
} CUJSON_Hooks;

/**
 * Globally redefine malloc, realloc and free for the CUJSON parser
 * 
 * Once this function is called, all future JSON nodes will be allocated
 * using the new functions.
 *
 * @param hooks	Custom allocation functions
 */
extern void CUJSON_InitHooks(CUJSON_Hooks* hooks);

#pragma mark -
#pragma mark JSON Tree Allocation

/**
 * Returns a newly allocated CUJSON tree for the given JSON string
 *
 * This function returns the root node of a CUJSON tree.  The tree can be traversed 
 * directly or by one of the access functions.
 *
 * This method allocates memory, and it is the responsibility of the caller to free
 * this memory when it is no longer needed.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on the root of the CUJSON tree.
 *
 * @param value The JSON string
 *
 * @return a newly allocate CUJSON tree for the given JSON string
 */
extern CUJSON *CUJSON_Parse(const char *value);

/**
 * Returns a newly allocated CUJSON tree for the given JSON string
 *
 * This function returns the root node of a CUJSON tree.  The tree can be traversed 
 * directly or by one of the access functions.  
 * 
 * This parse function allows you to require (and check) that the JSON is null terminated, 
 * and to retrieve the pointer to the final byte parsed. If you supply a ptr in 
 * return_parse_end and parsing fails, then return_parse_end will contain a pointer to 
 * the error. If not, then {@link CUJSON_GetErrorPtr()} does the job. 
 *
 * This method allocates memory, and it is the responsibility of the caller to free
 * this memory when it is no longer needed.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on the root of the CUJSON tree.
 *
 * @param value 	The JSON string
 * @param return_parse_end 			A pointer to store any parsing errors
 * @param require_null_terminated 	Whether to require that the JSON is null terminated
 *
 * @return a newly allocate CUJSON tree for the given JSON string
 */
extern CUJSON *CUJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

/**
 * Returns a newly allocated string representing a JSON tree
 *
 * This function returns a text representation of the CUJSON tree for transfer/storage.  
 * The string is formatted using a traditional pretty-print strategy.
 *
 * This method allocates memory, and it is the responsibility of the caller to free
 * this memory when it is no longer needed.  To delete the memory, simply call the
 * standard C function free.
 *
 * @param item The CUJSON tree
 *
 * @return a newly allocated string representing a JSON tree
 */
extern char  *CUJSON_Print(const CUJSON *item);

/**
 * Returns a newly allocated string representing a JSON tree
 *
 * This function returns a text representation of the CUJSON tree for transfer/storage.  
 * The string is unformatted, putting the data into as concise a format as possible.
 *
 * This method allocates memory, and it is the responsibility of the caller to free
 * this memory when it is no longer needed.  To delete the memory, simply call the
 * standard C function free.
 *
 * @param item The CUJSON tree
 *
 * @return a newly allocated string representing a JSON tree
 */
extern char  *CUJSON_PrintUnformatted(const CUJSON *item);

/**
 * Returns a newly allocated string representing a JSON tree
 *
 * This function returns a text representation of the CUJSON tree for transfer/storage.  
 * The text is generated using a  buffered strategy. The value prebuffer is a guess at 
 * the final size. Guessing well reduces reallocation
 *
 * The end result may or may not be formatted.  If it is formatted, the data is presented
 * using a traditional pretty-print strategy.
 *
 * This method allocates memory, and it is the responsibility of the caller to free
 * this memory when it is no longer needed.  To delete the memory, simply call the
 * standard C function free.
 *
 * @param item 		The CUJSON tree
 * @param prebuffer	A guess at the final size of the JSON string
 * @param fmt		Boolean indicating whether to format the string
 *
 * @return a newly allocated string representing a JSON tree
 */
extern char *CUJSON_PrintBuffered(const CUJSON *item, int prebuffer, int fmt);

/**
 * Deletes a CUJSON node and all subentities. 
 *
 * This function does not just delete children.  It also deletes any siblings
 * to the right of this node.
 *
 * @param c  The CUJSON node to delete 
 */
extern void   CUJSON_Delete(CUJSON *c);

#pragma mark -
#pragma mark JSON Node Allocation

/**
 * Returns a newly allocated CUJSON node of type NULL.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @return a newly allocated CUJSON node of type NULL.
 */
extern CUJSON *CUJSON_CreateNull(void);

/**
 * Returns a newly allocated CUJSON node of type True.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @return a newly allocated CUJSON node of type True.
 */
extern CUJSON *CUJSON_CreateTrue(void);

/**
 * Returns a newly allocated CUJSON node of type False.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @return a newly allocated CUJSON node of type False.
 */
extern CUJSON *CUJSON_CreateFalse(void);

/**
 * Returns a newly allocated CUJSON node of type True or False.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @param b	The boolean value to create
 *
 * @return a newly allocated CUJSON node of type True or False.
 */
extern CUJSON *CUJSON_CreateBool(int b);

/**
 * Returns a newly allocated CUJSON node of type number.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @param num	The number value to create
 *
 * @return a newly allocated CUJSON node of type number.
 */
extern CUJSON *CUJSON_CreateNumber(double num);

/**
 * Returns a newly allocated CUJSON node of type string.
 *
 * The source string is copied and can be safely deleted or modified.
 * 
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @param num	The string value to create
 *
 * @return a newly allocated CUJSON node of type string.
 */
extern CUJSON *CUJSON_CreateString(const char *string);

/**
 * Returns a newly allocated CUJSON node of type Array.
 *
 * The array is initially empty.  Arrays are represented as the children of the
 * returned node.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @return a newly allocated CUJSON node of type Array.
 */
extern CUJSON *CUJSON_CreateArray(void);

/**
 * Returns a newly allocated CUJSON node of type Object.
 *
 * The object is initially empty. Objects are represented as the children of the
 * returned node.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @return a newly allocated CUJSON node of type Object.
 */
extern CUJSON *CUJSON_CreateObject(void);

/**
 * Sets both attributes (int and double) of object when assigning an int
 * 
 * @param object	The CUJSON node
 * @param val		The value to assign
 */
#define CUJSON_SetIntValue(object,val) ((object) ? (object)->valueint = (object)->valuedouble = (val) : (val))

/**
 * Sets both attributes (int and double) of object when assigning a double
 * 
 * @param object	The CUJSON node
 * @param val		The value to assign
 */
#define CUJSON_SetNumberValue(object,val) ((object) ? (object)->valueint = (object)->valuedouble = (val) : (val))

#pragma mark -
#pragma mark JSON Array/Object Allocation

/**
 * Returns a newly allocated CUJSON node of type Array, containing ints.
 *
 * Arrays are represented as the children of the returned node.
 *
 * The array has size count. The values are all initialized to the contents of numbers.
 * The source array is copied and can be safely deleted.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @param numbers	The source array to encode
 * @param count		The size of the source array
 *
 * @return a newly allocated CUJSON node of type Array.
 */
extern CUJSON *CUJSON_CreateIntArray(const int *numbers, int count);

/**
 * Returns a newly allocated CUJSON node of type Array, containing floats.
 *
 * Arrays are represented as the children of the returned node.
 *
 * The array has size count. The values are all initialized to the contents of numbers.
 * The source array is copied and can be safely deleted.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @param numbers	The source array to encode
 * @param count		The size of the source array
 *
 * @return a newly allocated CUJSON node of type Array.
 */
extern CUJSON *CUJSON_CreateFloatArray(const float *numbers, int count);

/**
 * Returns a newly allocated CUJSON node of type Array, containing doubles.
 *
 * Arrays are represented as the children of the returned node.
 *
 * The array has size count. The values are all initialized to the contents of numbers.
 * The source array is copied and can be safely deleted.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @param numbers	The source array to encode
 * @param count		The size of the source array
 *
 * @return a newly allocated CUJSON node of type Array.
 */
extern CUJSON *CUJSON_CreateDoubleArray(const double *numbers, int count);

/**
 * Returns a newly allocated CUJSON node of type Array, containing strings.
 *
 * Arrays are represented as the children of the returned node.
 *
 * The array has size count. The values are all initialized to the contents of strings.
 * The source array is copied and can be safely deleted.
 *
 * This method allocates memory and it is the responsibility of the caller to free this
 * memory when it is no longer used.  To delete the memory, call the function 
 * {@link CUJSON_Delete} on this node.
 *
 * @param strings	The source array to encode
 * @param count		The size of the source array
 *
 * @return a newly allocated CUJSON node of type Array.
 */
extern CUJSON *CUJSON_CreateStringArray(const char **strings, int count);

/**
 * Appends an item to the specified array.
 *
 * Arrays are represented as the children of the provided node.  This function appends
 * item to the end of that sibling list.
 *
 * @param array	The CUJSON node for the array
 * @param item	The item to append
 */
extern void CUJSON_AddItemToArray(CUJSON *array, CUJSON *item);

/**
 * Appends an item to the specified object.
 *
 * Objects are represented as the children of the provided node.  This function appends
 * item to the end of that sibling list, using the object key.
 *
 * @param object	The CUJSON node for the object
 * @param string 	The item key (e.g. field name)
 * @param item		The item to append
 */
extern void	CUJSON_AddItemToObject(CUJSON *object, const char *string, CUJSON *item);

/**
 * Appends an item to the specified object.
 *
 * Objects are represented as the children of the provided node.  This function appends
 * item to the end of that sibling list, using the object key.
 *
 * You should only use this function when the  string is definitely const (i.e. a literal, 
 * or as good as), and will definitely survive the CUJSON object.  This cuts down on
 * allocation overhead.
 *
 * @param object	The CUJSON node for the object
 * @param string 	The item key (e.g. field name)
 * @param item		The item to append
 */
extern void	CUJSON_AddItemToObjectCS(CUJSON *object, const char *string, CUJSON *item);	

/**
 * Appends a reference to the specified array.
 *
 * Arrays are represented as the children of the provided node.  This function appends
 * item to the end of that sibling list.
 *
 * You should use this when you want to add an existing CUJSON to a new CUJSON, but don't 
 * want to corrupt your item (i.e. array should not take ownership of this node). 
 *
 * @param array	The CUJSON node for the array
 * @param item	The item to append
 */
extern void CUJSON_AddItemReferenceToArray(CUJSON *array, CUJSON *item);

/**
 * Appends a reference to the specified array.
 *
 * Objects are represented as the children of the provided node.  This function appends
 * item to the end of that sibling list, using the object key.
 *
 * You should use this when you want to add an existing CUJSON to a new CUJSON, but don't 
 * want to corrupt your item (i.e. array should not take ownership of this node). 
 *
 * @param object	The CUJSON node for the object
 * @param string 	The item key (e.g. field name)
 * @param item		The item to append
 */
extern void	CUJSON_AddItemReferenceToObject(CUJSON *object, const char *string, CUJSON *item);

/**
 * Returns an item removed from the specified array.
 *
 * Arrays are represented as the children of the provided node.  This function removes 
 * the item from the giving position in place.
 *
 * @param array	The CUJSON node for the array
 * @param which	The position to remove
 *
 * @return an item removed from the specified array.
 */
extern CUJSON *CUJSON_DetachItemFromArray(CUJSON *array, int which);

/**
 * Removes an item from the specified array.
 *
 * Arrays are represented as the children of the provided node.  This function removes 
 * the item from the giving position in place.
 *
 * @param array	The CUJSON node for the array
 * @param which	The position to remove
 */
extern void   CUJSON_DeleteItemFromArray(CUJSON *array, int which);

/**
 * Returns an item removed from the specified object.
 *
 * Objects are represented as the children of the provided node.  This function removes 
 * the item with the given key/name.
 *
 * @param object	The CUJSON node for the object
 * @param string	The key identifying the object
 *
 * @return an item removed from the specified object.
 */
extern CUJSON *CUJSON_DetachItemFromObject(CUJSON *object, const char *string);

/**
 * Removes an item from the specified object.
 *
 * Objects are represented as the children of the provided node.  This function removes 
 * the item with the given key/name.
 *
 * @param object	The CUJSON node for the object
 * @param string	The key identifying the object
 */
extern void   CUJSON_DeleteItemFromObject(CUJSON *object, const char *string);

/**
 * Inserts an item to the specified array.
 *
 * Arrays are represented as the children of the provided node.  This function inserts
 * item in to the appropriate position and shifts all other elements to the right.
 *
 * @param array		The CUJSON node for the array
 * @param which		The position to insert at
 * @param newitem	The item to insert
 */
extern void CUJSON_InsertItemInArray(CUJSON *array, int which, CUJSON *newitem);

/**
 * Replaces an item in the specified array.
 *
 * Arrays are represented as the children of the provided node.  This function replaces
 * item in to the appropriate position.
 *
 * @param array		The CUJSON node for the array
 * @param which		The position to replace at
 * @param newitem	The item to replace with
 */
extern void CUJSON_ReplaceItemInArray(CUJSON *array, int which, CUJSON *newitem);

/**
 * Replaces an item in the specified object.
 *
 * Objects are represented as the children of the provided node.  This function replaces
 * item with the given key/name.
 *
 * @param object	The CUJSON node for the object
 * @param string	The key to replace at
 * @param newitem	The item to replace with
 */
extern void CUJSON_ReplaceItemInObject(CUJSON *object,const char *string,CUJSON *newitem);

/**
 * Creates a NULL CUJSON node and adds it to the given object node
 *
 * Ownership of the new node is passed to object.  It will be deleted when the object
 * is itself deleted.
 *
 * @param object	The parent object to add to
 * @param name		The key for the new node
 */
#define CUJSON_AddNullToObject(object,name) CUJSON_AddItemToObject(object, name, CUJSON_CreateNull())

/**
 * Creates a True CUJSON node and adds it to the given object node
 *
 * Ownership of the new node is passed to object.  It will be deleted when the object
 * is itself deleted.
 *
 * @param object	The parent object to add to
 * @param name		The key for the new node
 */
#define CUJSON_AddTrueToObject(object,name) CUJSON_AddItemToObject(object, name, CUJSON_CreateTrue())

/**
 * Creates a False CUJSON node and adds it to the given object node
 *
 * Ownership of the new node is passed to object.  It will be deleted when the object
 * is itself deleted.
 *
 * @param object	The parent object to add to
 * @param name		The key for the new node
 */
#define CUJSON_AddFalseToObject(object,name) CUJSON_AddItemToObject(object, name, CUJSON_CreateFalse())

/**
 * Creates a boolean CUJSON node and adds it to the given object node
 *
 * Ownership of the new node is passed to object.  It will be deleted when the object
 * is itself deleted.
 *
 * @param object	The parent object to add to
 * @param name		The key for the new node
 * @param b			The boolean value
 */
#define CUJSON_AddBoolToObject(object,name,b) CUJSON_AddItemToObject(object, name, CUJSON_CreateBool(b))

/**
 * Creates a number CUJSON node and adds it to the given object node
 *
 * Ownership of the new node is passed to object.  It will be deleted when the object
 * is itself deleted.
 *
 * @param object	The parent object to add to
 * @param name		The key for the new node
 * @param n			The number value
 */
#define CUJSON_AddNumberToObject(object,name,n) CUJSON_AddItemToObject(object, name, CUJSON_CreateNumber(n))

/**
 * Creates a string CUJSON node and adds it to the given object node
 *
 * The source string is copied and can be safely deleted or modified.
 *
 * Ownership of the new node is passed to object.  It will be deleted when the object
 * is itself deleted.
 *
 * @param object	The parent object to add to
 * @param name		The key for the new node
 * @param s			The string value
 */
#define CUJSON_AddStringToObject(object,name,s) CUJSON_AddItemToObject(object, name, CUJSON_CreateString(s))

#pragma mark -
#pragma mark JSON Accessors

/**
 * Returns the number of items in an array (or object). 
 *
 * Arrays are represented as the children of the provided node. 
 *
 * @param array	The array node to query
 *
 * @return the number of items in an array (or object). 
 */
extern int	  CUJSON_GetArraySize(const CUJSON *array);

/**
 * Returns the item from the array at the given position.
 *
 * Arrays are represented as the children of the provided node. If there is no child
 * at the given position, this function returns NULL.
 *
 * @param array	The array node to query
 * @param item	The item position
 *
 * @return the item from the array at the given position.
 */
extern CUJSON *CUJSON_GetArrayItem(const CUJSON *array, int item);

/**
 * Returns the item from the object with the given key.
 *
 * Objects are represented as the children of the provided node. If there is no child
 * with the given key, this function returns NULL.  Key comparison is case insensitive.
 *
 * @param object	The object node to query
 * @param string	The item name/key
 *
 * @return the item from the object with the given key.
 */
extern CUJSON *CUJSON_GetObjectItem(const CUJSON *object, const char *string);

/**
 * Returns true if the object has an item with the given key.
 *
 * Objects are represented as the children of the provided node. Key comparison is case
 * insensitive.
 *
 * @param object	The object node to query
 * @param string	The item name/key
 *
 * @return true if the object has an item with the given key.
 */
extern int CUJSON_HasObjectItem(const CUJSON *object, const char *string);

/** Macro for iterating over an array */
#define CUJSON_ArrayForEach(pos, head) for(pos = (head)->child; pos != NULL; pos = pos->next)

extern int CUJSON_strcasecmp(const char *s1, const char *s2);
    
#pragma mark -
#pragma mark JSON Misc

/**
 * Returns a pointer to the parse error in a failed parse.
 *
 * You will probably need to look a few chars back to make sense of it. It is defined
 * when CUJSON_Parse() returns 0. It is NULL when CUJSON_Parse() succeeds. 
 */
extern const char *CUJSON_GetErrorPtr(void);

/**
 * Returns a duplicate a CUJSON item 
 *
 * This function will create a new, identical CUJSON item to the one you pass, in new 
 * memory that will need to be released. With recurse != 0, it will duplicate any children 
 * connected to the item.
 *
 * The item->next and ->prev pointers are always zero in the duplicate.
 *
 * @param item		The item to duplicate
 * @param recurse	Whether to duplicate the children as well
 *
 * @return a duplicate a CUJSON item 
 */
extern CUJSON *CUJSON_Duplicate(const CUJSON *item, int recurse);

/**
 * Minifies a JSON string in place
 *
 * This function strips and formatting or spacing to make the JSON as small as possible.
 * This method does not allocate any new memory, since it modifies the string in place.
 *
 * @param json 	The string to minify
 */
extern void CUJSON_Minify(char *json);

#ifdef __cplusplus
}
#endif

#endif /** __CU_JSON_H__ */
