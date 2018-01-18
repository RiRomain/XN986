#ifndef __XMLLIB_H_2012_07_01__
#define __XMLLIB_H_2012_07_01__

#if __cplusplus
extern "C" {
#endif

typedef struct _xmllib_xml_doc_ *xml_doc;
typedef struct _xmllib_xml_tag_ *xml_tag;

xml_doc xml_load( const char *file );
xml_doc xml_load_string( const char *buf, unsigned int length );

// 加载XML扩展节点,扩展节点指节点属性中有src="path"的节点,可以通过这个方法把src指定的内容添加到当前节点中
int xml_load_extend( xml_doc doc, const char *doc_file, const char *ext_node_path );
// 保存XML扩展节点
int xml_save_extend( xml_doc doc, const char *doc_file, const char *ext_node_path );

int xml_save( xml_doc doc, const char *file );
int xml_save_string( xml_doc doc, char *buf, unsigned int length );

void xml_destroy( xml_doc doc );
const char *xml_read( xml_doc doc, const char *xml_path );
int xml_read_int( xml_doc doc, const char *xml_path );
double xml_read_double( xml_doc doc, const char *xml_path );
int xml_write_format( xml_doc doc, const char *xml_path, const char *format, ... );
int xml_remove( xml_doc doc, const char *xml_path );

xml_tag xml_get_tag( xml_doc doc, const char *xml_path );
xml_tag xml_get_next_tag( xml_tag tag );
xml_tag xml_get_prev_tag( xml_tag tag );
xml_tag xml_get_first_child_tag( xml_tag tag );

xml_tag xml_tag_get_tag( xml_tag tag, const char *xml_path );

const char *xml_tag_read( xml_tag tag, const char *xml_path );
int xml_tag_read_int( xml_tag tag, const char *xml_path );
float xml_tag_read_float( xml_tag tag, const char *xml_path );
int xml_tag_write_format( xml_tag tag, const char *xml_path, const char *format, ... );
int xml_tag_remove( xml_tag tag, const char *xml_path );
xml_tag xml_tag_add( xml_tag root, const char *new_tag );
static inline xml_tag xml_add( xml_doc doc, const char *xml_path, const char *new_tag ) {
	xml_tag tag = xml_get_tag( doc, xml_path );
	if ( tag == 0 ) {
		return 0;
	}
	return xml_tag_add( tag, new_tag );
}

#if __cplusplus
}
#endif

#endif // __XMLLIB_H_2012_07_01__