#include "pugilua_lib.h"

#include <pugixml.hpp>
#include <LuaBridge.h>
#include <RefCountedPtr.h>
#include <string>
#include <iostream>
#include <sstream>

namespace pugi {
	namespace lua {

		static char const* version = "0.1.4";

		class options {
		public:
			options& with(int o) { options_|=o; return *this; }
			options& without(int o) { options_&=~o; return *this; }
			int get() const { return options_; }
			options(int o):options_(o){}
		private:
			int options_;
		};


		class lxpath_node;
		class lxpath_node_set;
		class lxml_node;

		////////////////////
		static int encoding_auto = (int)pugi::encoding_auto;
		static int encoding_utf8 = (int)pugi::encoding_utf8;	
		static int encoding_utf16_le = (int)pugi::encoding_utf16_le;
		static int encoding_utf16_be = (int)pugi::encoding_utf16_be;
		static int encoding_utf16 = (int)pugi::encoding_utf16;	
		static int encoding_utf32_le = (int)pugi::encoding_utf32_le;
		static int encoding_utf32_be = (int)pugi::encoding_utf32_be;
		static int encoding_utf32 = (int)pugi::encoding_utf32;	
		static int encoding_wchar = (int)pugi::encoding_wchar;	
		static int encoding_latin1 = (int)pugi::encoding_latin1;

		////////////////////
		static int status_ok = (int)pugi::status_ok;
		static int status_file_not_found = (int)pugi::status_file_not_found;
		static int status_io_error = (int)pugi::status_io_error;
		static int status_out_of_memory = (int)pugi::status_out_of_memory;
		static int status_internal_error = (int)pugi::status_internal_error;
		static int status_unrecognized_tag = (int)pugi::status_unrecognized_tag;
		static int status_bad_pi = (int)pugi::status_bad_pi;
		static int status_bad_comment = (int)pugi::status_bad_comment;
		static int status_bad_cdata = (int)pugi::status_bad_cdata;
		static int status_bad_doctype = (int)pugi::status_bad_doctype;
		static int status_bad_pcdata = (int)pugi::status_bad_pcdata;
		static int status_bad_start_element = (int)pugi::status_bad_start_element;
		static int status_bad_attribute = (int)pugi::status_bad_attribute;
		static int status_bad_end_element = (int)pugi::status_bad_end_element;
		static int status_end_element_mismatch  = (int)pugi::status_end_element_mismatch ;

		////////////////////
		static int xpath_type_none = (int)pugi::xpath_type_none;
		static int xpath_type_node_set = (int)pugi::xpath_type_node_set;
		static int xpath_type_number = (int)pugi::xpath_type_number;
		static int xpath_type_string = (int)pugi::xpath_type_string;
		static int xpath_type_boolean = (int)pugi::xpath_type_boolean;

		////////////////////
		static int format_indent = (int)pugi::format_indent;
		static int format_write_bom = (int)pugi::format_write_bom;	
		static int format_raw = (int)pugi::format_raw;
		static int format_no_declaration = (int)pugi::format_no_declaration;
		static int format_no_escapes = (int)pugi::format_no_escapes;	
		static int format_save_file_text = (int)pugi::format_save_file_text;
		static int format_default = (int)pugi::format_default;

		////////////////////
		static int parse_cdata = (int)pugi::parse_cdata;
		static int parse_comments = (int)pugi::parse_comments;
		static int parse_declaration = (int)pugi::parse_declaration;
		static int parse_default = (int)pugi::parse_default;
		static int parse_doctype = (int)pugi::parse_doctype;
		static int parse_eol = (int)pugi::parse_eol;
		static int parse_escapes = (int)pugi::parse_escapes;
		static int parse_full = (int)pugi::parse_full;
		static int parse_minimal = (int)pugi::parse_minimal;
		static int parse_pi = (int)pugi::parse_pi;
		static int parse_ws_pcdata = (int)pugi::parse_ws_pcdata;
		static int parse_ws_pcdata_single = (int)pugi::parse_ws_pcdata_single;
		static int parse_wconv_attribute = (int)pugi::parse_wconv_attribute;
		static int parse_wnorm_attribute = (int)pugi::parse_wnorm_attribute;

		////////////////////
		static int node_null = (int)pugi::node_null;		
		static int node_document = (int)pugi::node_document;	
		static int node_element = (int)pugi::node_element;	
		static int node_pcdata = (int)pugi::node_pcdata;	
		static int node_cdata = (int)pugi::node_cdata;		
		static int node_comment = (int)pugi::node_comment;	
		static int node_pi = (int)pugi::node_pi;		
		static int node_declaration = (int)pugi::node_declaration;
		static int node_doctype = (int)pugi::node_doctype;	

		////////////////////
		class lxml_attribute {
		public:
			lxml_attribute(pugi::xml_attribute const& a):att(a) {}
			lxml_attribute() {}

		public:
			bool valid() const { return (bool)att; }
			bool empty() const { return att.empty(); }
			std::string name() const { return att.name(); }
			std::string value() const { return att.value(); }
			double number() const { return att.as_double(); }
			bool as_bool() const { return att.as_bool(); }

			bool set_name(char const* n) { return att.set_name(n); }
			bool set_value(char const* v) { return att.set_value(v); }

			bool same_as(lxml_attribute const& other) const { return other.att==att; }
			
			RefCountedPtr<lxml_attribute> next_attribute() const {
				return RefCountedPtr<lxml_attribute>(new lxml_attribute(att.next_attribute()));
			}

			RefCountedPtr<lxml_attribute> previous_attribute() const {
				return RefCountedPtr<lxml_attribute>(new lxml_attribute(att.previous_attribute()));
			}

			size_t hash_value() const {
				return att.hash_value();
			}

		public: // non-lua interface
			pugi::xml_attribute const& get() const {
				return att;
			}

		private:
			pugi::xml_attribute att;
		};

		///////////////////////
		class lxml_parse_result {
		public:
			lxml_parse_result(pugi::xml_parse_result const& r);
			lxml_parse_result();

		public:
			std::string description() const;

			bool valid() const;

			int status() const {
				return res.status;
			}

			int encoding() const {
				return res.encoding;
			}

			ptrdiff_t offset() const {
				return res.offset;
			}

		private:
			pugi::xml_parse_result res;
		};


		///////////////
		class lxml_text {
		public:
			lxml_text(pugi::xml_text const& t):text(t) {}
			lxml_text() {}
		
		public:
			bool valid() const {
				return (bool)text;
			}

			bool empty() const {
				return text.empty();
			}

			bool set(char const* str) {
				return text.set(str);
			}

			std::string string() const {
				return text.get();
			}

			double number() const {
				return text.as_double();
			}

			bool as_bool() const {
				return text.as_bool();
			}

			bool same_as(lxml_text const& other) const {
				return other.text==text;
			}

			RefCountedPtr<lxml_node> data() const;

		private:
			pugi::xml_text text;
		};

		///////////////
		class lxml_node {
		public:
			lxml_node(pugi::xml_node const& n);
			lxml_node();

		public:
			bool valid() const;

			std::string name() const;
			std::string value() const;

			bool empty() const;

			int type() const;

			RefCountedPtr<lxml_attribute> first_attribute() const;
			RefCountedPtr<lxml_attribute> last_attribute() const;

			RefCountedPtr<lxml_node> first_child() const;
			RefCountedPtr<lxml_node> last_child() const;

			RefCountedPtr<lxml_node> parent() const;
			RefCountedPtr<lxml_node> root() const;

			RefCountedPtr<lxml_node> child(char const* name) const;
			RefCountedPtr<lxml_attribute> attribute(char const* name) const;

			RefCountedPtr<lxml_node> next() const;
			RefCountedPtr<lxml_node> previous() const;

			RefCountedPtr<lxml_node> next_sibling(char const* name) const;
			RefCountedPtr<lxml_node> previous_sibling(char const* name) const;

			std::string child_value(char const* name) const;

			bool set_name(char const* rhs);
			bool set_value(char const* rhs);

			RefCountedPtr<lxml_attribute> append_attribute(const char* name);
			RefCountedPtr<lxml_attribute> prepend_attribute(const char* name);
			RefCountedPtr<lxml_attribute> insert_attribute_after(const char* name, RefCountedPtr<lxml_attribute> attr);
			RefCountedPtr<lxml_attribute> insert_attribute_before(const char* name, RefCountedPtr<lxml_attribute> attr);

			RefCountedPtr<lxml_attribute> append_attribute_copy(RefCountedPtr<lxml_attribute> proto);
			RefCountedPtr<lxml_attribute> prepend_attribute_copy(RefCountedPtr<lxml_attribute> proto);
			RefCountedPtr<lxml_attribute> insert_attribute_copy_after(RefCountedPtr<lxml_attribute> proto, RefCountedPtr<lxml_attribute> attr);
			RefCountedPtr<lxml_attribute> insert_attribute_copy_before(RefCountedPtr<lxml_attribute> proto, RefCountedPtr<lxml_attribute> attr);

			RefCountedPtr<lxml_node> append(int type);
			RefCountedPtr<lxml_node> prepend(int type);
			RefCountedPtr<lxml_node> insert_after(int type, RefCountedPtr<lxml_node> _node);
			RefCountedPtr<lxml_node> insert_before(int type, RefCountedPtr<lxml_node> _node);

			RefCountedPtr<lxml_node> append_child(const char* name);
			RefCountedPtr<lxml_node> prepend_child(const char* name);
			RefCountedPtr<lxml_node> insert_child_after(const char* name, RefCountedPtr<lxml_node> _node);
			RefCountedPtr<lxml_node> insert_child_before(const char* name, RefCountedPtr<lxml_node> _node);

			RefCountedPtr<lxml_node> append_copy(RefCountedPtr<lxml_node> proto);
			RefCountedPtr<lxml_node> prepend_copy(RefCountedPtr<lxml_node> proto);
			RefCountedPtr<lxml_node> insert_copy_after(RefCountedPtr<lxml_node> proto, RefCountedPtr<lxml_node> _node);
			RefCountedPtr<lxml_node> insert_copy_before(RefCountedPtr<lxml_node> proto, RefCountedPtr<lxml_node> _node);

			bool remove_attribute(RefCountedPtr<lxml_attribute> a);
			bool remove_attribute_by_name(const char* name);

			bool remove_child(RefCountedPtr<lxml_node> n);
			bool remove_child_by_name(const char* name);

			RefCountedPtr<lxml_node> find_child_by_name_and_attribute(const char* name, const char* attr_name, const char* attr_value) const;
			RefCountedPtr<lxml_node> find_child_by_attribute(const char* attr_name, const char* attr_value) const;

			std::string path() const;

			RefCountedPtr<lxml_node> first_element_by_path(const char* path) const;

			RefCountedPtr<lxpath_node> select_single_node(const char* query) const;
				
			RefCountedPtr<lxpath_node_set> select_nodes(char const* query) const;

			std::string string() const;

			std::string as_string_with_options(char const* indent, int flags, int encoding, int depth) const;

			RefCountedPtr<lxml_text> text() const;

			bool same_as(lxml_node const& other) const;

			ptrdiff_t offset_debug() const;

			size_t hash_value() const;

			//todo: text(), xml_tree_walker somehow

		public: // non-lua interface
			pugi::xml_node const& get() const;

		private:
			pugi::xml_node node;
		};

		///////////////////
		class lxml_document {
		public:
			RefCountedPtr<lxml_node> root() const;

			bool valid() const;

			void reset();
			void reset_with(lxml_document const* other);

			RefCountedPtr<lxml_parse_result> load_file(char const* path);
			RefCountedPtr<lxml_parse_result> load_file_with_options(char const* path,int options, int encoding);
			RefCountedPtr<lxml_parse_result> load(char const* contents);
			RefCountedPtr<lxml_parse_result> load_with_options(char const* contents,int options);
			bool save_file(char const* path) const;
			bool save_file_with_options(char const* path,char const* indent,int flags,int encoding) const;

		private:
			pugi::xml_document doc;
		};

		/////////////////
		class lxpath_node {
		public:
			lxpath_node(pugi::xpath_node const& n);
			lxpath_node();

			bool valid() const;

			RefCountedPtr<lxml_node> node() const;
			RefCountedPtr<lxml_attribute> attribute() const;
			void from_node(RefCountedPtr<lxml_node> other);
			void from_attribute(RefCountedPtr<lxml_attribute>,RefCountedPtr<lxml_node> other);
			RefCountedPtr<lxml_node> parent() const;
			bool same_as(RefCountedPtr<lxpath_node> other) const;

		private:
			pugi::xpath_node _node;
		};

		/////////////////////
		class lxpath_node_set {
		public:
			lxpath_node_set(pugi::xpath_node_set const& s);
			lxpath_node_set();
		public:

			int type() const;

			size_t size() const;

			// todo: think if 1..size is a better convention for lua
			RefCountedPtr<lxpath_node> get(size_t i);

			void sort(bool reverse);

			bool empty() const;

			RefCountedPtr<lxpath_node> first() const;

		public:
			// enums
			static int type_unsorted () { return pugi::xpath_node_set::type_unsorted; }
			static int type_sorted () { return pugi::xpath_node_set::type_sorted; }
			static int type_sorted_reverse () { return pugi::xpath_node_set::type_sorted_reverse; }
		private:
			pugi::xpath_node_set node_set;
		};

	}
}

namespace pugi {
	namespace lua {

		///////////////////////
		lxml_parse_result::lxml_parse_result(pugi::xml_parse_result const& r):res(r) { }
		lxml_parse_result::lxml_parse_result() { }

		std::string lxml_parse_result::description() const {
			return res.description();
		}

		bool lxml_parse_result::valid() const {
			return (bool)res;
		}


		///////////////
		lxml_node::lxml_node(pugi::xml_node const& n):node(n){}
		lxml_node::lxml_node() { }

		pugi::xml_node const& lxml_node::get() const {
			return node;
		}

		bool lxml_node::valid() const {
			return (bool)node;
		}

		RefCountedPtr<lxml_node> lxml_node::child(char const* name) const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.child(name)));
		}

		std::string lxml_node::name() const {
			return node.name();
		}

		std::string lxml_node::value() const {
			return node.value();
		}

		RefCountedPtr<lxpath_node_set> lxml_node::select_nodes(char const* query) const {
			try {
				return RefCountedPtr<lxpath_node_set>(new lxpath_node_set(node.select_nodes(query)));
			} catch (pugi::xpath_exception const& e) {
				std::cerr<<"Error: "<<e.what()<<std::endl;
				return RefCountedPtr<lxpath_node_set>(new lxpath_node_set());
			}
		}

		bool lxml_node::empty() const {
			return node.empty();
		}

		int lxml_node::type() const {
			return node.type();
		}

		RefCountedPtr<lxml_attribute> lxml_node::first_attribute() const {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.first_attribute()));
		}

		RefCountedPtr<lxml_attribute> lxml_node::last_attribute() const {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.last_attribute()));
		}

		RefCountedPtr<lxml_node> lxml_node::first_child() const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.first_child()));
		}

		RefCountedPtr<lxml_node> lxml_node::last_child() const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.last_child()));
		}

		RefCountedPtr<lxml_node> lxml_node::parent() const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.parent()));
		}

		RefCountedPtr<lxml_node> lxml_node::root() const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.root()));
		}

		RefCountedPtr<lxml_attribute> lxml_node::attribute(char const* name) const {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.attribute(name)));
		}

		RefCountedPtr<lxml_node> lxml_node::next() const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.next_sibling()));
		}

		RefCountedPtr<lxml_node> lxml_node::previous() const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.previous_sibling()));
		}

		RefCountedPtr<lxml_node> lxml_node::next_sibling(char const* name) const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.next_sibling(name)));
		}

		RefCountedPtr<lxml_node> lxml_node::previous_sibling(char const* name) const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.previous_sibling(name)));
		}

		std::string lxml_node::child_value(char const* name) const {
			return node.child_value(name);
		}

		bool lxml_node::set_name(char const* rhs) {
			return node.set_name(rhs);
		}

		bool lxml_node::set_value(char const* rhs) {
			return node.set_value(rhs);
		}

		RefCountedPtr<lxml_attribute> lxml_node::append_attribute(const char* name) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.append_attribute(name)));
		}

		RefCountedPtr<lxml_attribute> lxml_node::prepend_attribute(const char* name) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.prepend_attribute(name)));
		}

		RefCountedPtr<lxml_attribute> lxml_node::insert_attribute_after(const char* name, RefCountedPtr<lxml_attribute> attr) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.insert_attribute_after(name,attr->get())));
		}

		RefCountedPtr<lxml_attribute> lxml_node::insert_attribute_before(const char* name, RefCountedPtr<lxml_attribute> attr) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.insert_attribute_before(name,attr->get())));
		}

		RefCountedPtr<lxml_attribute> lxml_node::append_attribute_copy(RefCountedPtr<lxml_attribute> proto) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.append_copy(proto->get())));
		}

		RefCountedPtr<lxml_attribute> lxml_node::prepend_attribute_copy(RefCountedPtr<lxml_attribute> proto) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.prepend_copy(proto->get())));
		}

		RefCountedPtr<lxml_attribute> lxml_node::insert_attribute_copy_after(RefCountedPtr<lxml_attribute> proto, RefCountedPtr<lxml_attribute> attr) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.insert_copy_after(proto->get(),attr->get())));
		}

		RefCountedPtr<lxml_attribute> lxml_node::insert_attribute_copy_before(RefCountedPtr<lxml_attribute> proto, RefCountedPtr<lxml_attribute> attr) {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(node.insert_copy_before(proto->get(),attr->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::append(int type) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.append_child((xml_node_type)type)));
		}

		RefCountedPtr<lxml_node> lxml_node::prepend(int type) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.prepend_child((xml_node_type)type)));
		}

		RefCountedPtr<lxml_node> lxml_node::insert_after(int type, RefCountedPtr<lxml_node> _node) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.insert_child_after((xml_node_type)type,_node->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::insert_before(int type, RefCountedPtr<lxml_node> _node) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.insert_child_before((xml_node_type)type,_node->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::append_child(const char* name) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.append_child(name)));
		}

		RefCountedPtr<lxml_node> lxml_node::prepend_child(const char* name) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.prepend_child(name)));
		}

		RefCountedPtr<lxml_node> lxml_node::insert_child_after(const char* name, RefCountedPtr<lxml_node> _node) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.insert_child_after(name,_node->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::insert_child_before(const char* name, RefCountedPtr<lxml_node> _node) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.insert_child_before(name,_node->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::append_copy(RefCountedPtr<lxml_node> proto) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.append_copy(proto->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::prepend_copy(RefCountedPtr<lxml_node> proto) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.prepend_copy(proto->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::insert_copy_after(RefCountedPtr<lxml_node> proto, RefCountedPtr<lxml_node> _node) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.insert_copy_after(proto->get(),_node->get())));
		}

		RefCountedPtr<lxml_node> lxml_node::insert_copy_before(RefCountedPtr<lxml_node> proto, RefCountedPtr<lxml_node> _node) {
			return RefCountedPtr<lxml_node>(new lxml_node(node.insert_copy_before(proto->get(),_node->get())));
		}

		bool lxml_node::remove_attribute(RefCountedPtr<lxml_attribute> a) {
			return node.remove_attribute(a->get());
		}

		bool lxml_node::remove_attribute_by_name(const char* name) {
			return node.remove_attribute(name);
		}

		bool lxml_node::remove_child(RefCountedPtr<lxml_node> n) {
			return node.remove_child(n->get());
		}

		bool lxml_node::remove_child_by_name(const char* name) {
			return node.remove_child(name);
		}

		RefCountedPtr<lxml_node> lxml_node::find_child_by_name_and_attribute(const char* name, const char* attr_name, const char* attr_value) const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.find_child_by_attribute(name,attr_name,attr_value)));
		}

		RefCountedPtr<lxml_node> lxml_node::find_child_by_attribute(const char* attr_name, const char* attr_value) const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.find_child_by_attribute(attr_name,attr_name,attr_value)));
		}

		std::string lxml_node::path() const {
			return node.path();
		}

		RefCountedPtr<lxml_node> lxml_node::first_element_by_path(const char* path) const {
			return RefCountedPtr<lxml_node>(new lxml_node(node.first_element_by_path(path)));
		}

		RefCountedPtr<lxpath_node> lxml_node::select_single_node(char const* query) const {
			try {
				return RefCountedPtr<lxpath_node>(new lxpath_node(node.select_single_node(query)));
			} catch (pugi::xpath_exception const& e) {
				std::cerr<<"Error: "<<e.what()<<std::endl;
				return RefCountedPtr<lxpath_node>(new lxpath_node());
			}
		}

		std::string lxml_node::string() const {
			std::stringstream ss;
			node.print(ss);
			return ss.str();
		}

		std::string lxml_node::as_string_with_options(char const* indent, int flags, int encoding, int depth) const {
			std::stringstream ss;
			node.print(ss,indent,(unsigned int)flags,(pugi::xml_encoding)encoding,(unsigned int)depth);
			return ss.str();
		}

		RefCountedPtr<lxml_text> lxml_node::text() const {
			return RefCountedPtr<lxml_text>(new lxml_text(node.text()));
		}

		bool lxml_node::same_as(lxml_node const& other) const {
			return other.node==node;
		}

		size_t lxml_node::hash_value() const {
			return node.hash_value();
		}

		ptrdiff_t lxml_node::offset_debug() const {
			return node.offset_debug();
		}

		///////////////////
		RefCountedPtr<lxml_node> lxml_text::data() const {
			return RefCountedPtr<lxml_node>(new lxml_node(text.data()));
		}

		///////////////////
		RefCountedPtr<lxml_parse_result> lxml_document::load_file(char const* path) {
			return RefCountedPtr<lxml_parse_result>(new lxml_parse_result(doc.load_file(path)));
		}

		RefCountedPtr<lxml_node> lxml_document::root() const {
			return RefCountedPtr<lxml_node>(new lxml_node(pugi::xml_node(doc)));
		}

		bool lxml_document::valid() const {
			return (bool)doc;
		}

		void lxml_document::reset() {
			doc.reset();
		}

		void lxml_document::reset_with(lxml_document const* other) {
			doc.reset(other->doc);
		}

		RefCountedPtr<lxml_parse_result> lxml_document::load(char const* contents) {
			return RefCountedPtr<lxml_parse_result>(new lxml_parse_result(doc.load(contents)));
		}

		RefCountedPtr<lxml_parse_result> lxml_document::load_with_options(char const* contents,int options)
		{
			return RefCountedPtr<lxml_parse_result>(new lxml_parse_result(doc.load(contents,(unsigned int)options)));
		}

		RefCountedPtr<lxml_parse_result> lxml_document::load_file_with_options(char const* path,int options, int encoding)
		{
			return RefCountedPtr<lxml_parse_result>(new lxml_parse_result(doc.load_file(path,(unsigned int)options,(pugi::xml_encoding)encoding)));
		}

		bool lxml_document::save_file_with_options(char const* path,char const* indent,int flags,int encoding) const
		{
			return doc.save_file(path,indent,(unsigned int)flags,(pugi::xml_encoding)encoding);
		}

		bool lxml_document::save_file(char const* path) const {
			return doc.save_file(path);
		}
		
		//////////////////////
		lxpath_node::lxpath_node(pugi::xpath_node const& n):_node(n){}
		lxpath_node::lxpath_node() { }

		bool lxpath_node::valid() const {
			return (bool)_node;
		}

		RefCountedPtr<lxml_node> lxpath_node::node() const {
			return RefCountedPtr<lxml_node>(new lxml_node(_node.node()));
		}

		RefCountedPtr<lxml_attribute> lxpath_node::attribute() const {
			return RefCountedPtr<lxml_attribute>(new lxml_attribute(_node.attribute()));
		}

		void lxpath_node::from_node(RefCountedPtr<lxml_node> other) {
			_node=other->get();
		}

		void lxpath_node::from_attribute(RefCountedPtr<lxml_attribute> a,RefCountedPtr<lxml_node> other) {
			_node=pugi::xpath_node(a->get(),other->get());
		}

		RefCountedPtr<lxml_node> lxpath_node::parent() const {
			return RefCountedPtr<lxml_node>(new lxml_node(_node.parent()));
		}

		bool lxpath_node::same_as(RefCountedPtr<lxpath_node> other) const {
			return other->_node==_node;
		}

		/////////////////////
		lxpath_node_set::lxpath_node_set(pugi::xpath_node_set const& s):node_set(s) { }
		lxpath_node_set::lxpath_node_set() { }

		int lxpath_node_set::type() const {
			return node_set.type();
		}

		size_t lxpath_node_set::size() const {
			return node_set.size();
		}

		RefCountedPtr<lxpath_node> lxpath_node_set::get(size_t i) {
			return RefCountedPtr<lxpath_node>(new lxpath_node(node_set[i]));
		}

		void lxpath_node_set::sort(bool reverse) {
			node_set.sort(reverse);
		}

		bool lxpath_node_set::empty() const {
			return node_set.empty();
		}

		RefCountedPtr<lxpath_node> lxpath_node_set::first() const {
			return RefCountedPtr<lxpath_node>(new lxpath_node(node_set.first()));
		}
	}
}

void register_pugilua (lua_State* L) {
	using namespace pugi::lua;
	luabridge::getGlobalNamespace(L)
		.beginNamespace("pugi")

		.addVariable("version",&version,false)

		.beginClass<options>("options")
		.addConstructor<void (*)(int)>()
		.addProperty("int",&options::get)
		.addFunction("with",&options::with)
		.addFunction("without",&options::without)
		.endClass()

		.addVariable("encoding_auto",&encoding_auto,false)
		.addVariable("encoding_utf8",&encoding_utf8,false)	
		.addVariable("encoding_utf16_le",&encoding_utf16_le,false)
		.addVariable("encoding_utf16_be",&encoding_utf16_be,false)
		.addVariable("encoding_utf16",&encoding_utf16,false)	
		.addVariable("encoding_utf32_le",&encoding_utf32_le,false)
		.addVariable("encoding_utf32_be",&encoding_utf32_be,false)
		.addVariable("encoding_utf32",&encoding_utf32,false)	
		.addVariable("encoding_wchar",&encoding_wchar,false)	
		.addVariable("encoding_latin1",&encoding_latin1,false)

		.addVariable("xpath_type_none",&xpath_type_none,false)
		.addVariable("xpath_type_node_set",&xpath_type_node_set,false)
		.addVariable("xpath_type_number",&xpath_type_number,false)
		.addVariable("xpath_type_string",&xpath_type_string,false)

		.addVariable("format_indent",&format_indent,false)
		.addVariable("format_write_bom",&format_write_bom,false)	
		.addVariable("format_raw",&format_raw,false)
		.addVariable("format_no_declaration",&format_no_declaration,false)
		.addVariable("format_no_escapes",&format_no_escapes,false)	
		.addVariable("format_save_file_text",&format_save_file_text,false)
		.addVariable("format_default",&format_default,false)

		.addVariable("status_ok",&status_ok,false)
		.addVariable("status_file_not_found",&status_file_not_found,false)
		.addVariable("status_io_error",&status_io_error,false)
		.addVariable("status_out_of_memory",&status_out_of_memory,false)
		.addVariable("status_internal_error",&status_internal_error,false)
		.addVariable("status_unrecognized_tag",&status_unrecognized_tag,false)
		.addVariable("status_bad_pi",&status_bad_pi,false)
		.addVariable("status_bad_comment",&status_bad_comment,false)
		.addVariable("status_bad_cdata",&status_bad_cdata,false)
		.addVariable("status_bad_doctype",&status_bad_doctype,false)
		.addVariable("status_bad_pcdata",&status_bad_pcdata,false)
		.addVariable("status_bad_start_element",&status_bad_start_element,false)
		.addVariable("status_bad_attribute",&status_bad_attribute,false)
		.addVariable("status_bad_end_element",&status_bad_end_element,false)
		.addVariable("status_end_element_mismatch ",&status_end_element_mismatch ,false)

		.addVariable("parse_cdata",&parse_cdata,false)
		.addVariable("parse_comments",&parse_comments,false)
		.addVariable("parse_declaration",&parse_declaration,false)
		.addVariable("parse_default",&parse_default,false)
		.addVariable("parse_doctype",&parse_doctype,false)
		.addVariable("parse_eol",&parse_eol,false)
		.addVariable("parse_escapes",&parse_escapes,false)
		.addVariable("parse_full",&parse_full,false)
		.addVariable("parse_minimal",&parse_minimal,false)
		.addVariable("parse_pi",&parse_pi,false)
		.addVariable("parse_ws_pcdata",&parse_ws_pcdata,false)
		.addVariable("parse_ws_pcdata_single",&parse_ws_pcdata_single,false)
		.addVariable("parse_wconv_attribute",&parse_wconv_attribute,false)
		.addVariable("parse_wnorm_attribute",&parse_wnorm_attribute,false)


		.addVariable("node_null",&node_null,false)		
		.addVariable("node_document",&node_document,false)	
		.addVariable("node_element",&node_element,false)	
		.addVariable("node_pcdata",&node_pcdata,false)	
		.addVariable("node_cdata",&node_cdata,false)		
		.addVariable("node_comment",&node_comment,false)	
		.addVariable("node_pi",&node_pi,false)		
		.addVariable("node_declaration",&node_declaration,false)
		.addVariable("node_doctype",&node_doctype,false)	

		.beginClass<lxml_attribute>("xml_attribute")
		.addConstructor<void (*)()>()
		.addProperty("valid",&lxml_attribute::valid)
		.addProperty("empty",&lxml_attribute::empty)
		.addProperty("name",&lxml_attribute::name)
		.addProperty("value",&lxml_attribute::value)
		.addProperty("number",&lxml_attribute::number)
		.addProperty("bool",&lxml_attribute::as_bool)
		.addProperty("hash_value",&lxml_attribute::hash_value)
		.addFunction("set_name",&lxml_attribute::set_name)
		.addFunction("set_value",&lxml_attribute::set_value)
		.addFunction("next_attribute",&lxml_attribute::next_attribute)
		.addFunction("previous_attribute",&lxml_attribute::previous_attribute)
		.addFunction("same_as",&lxml_attribute::same_as)
		.endClass()

		.beginClass<lxml_parse_result>("xml_parse_result")
		.addConstructor<void (*)()>()
		.addProperty("description",&lxml_parse_result::description)
		.addProperty("valid",&lxml_parse_result::valid)
		.addProperty("status",&lxml_parse_result::status)
		.addProperty("encoding",&lxml_parse_result::encoding)
		.addProperty("offset",&lxml_parse_result::offset)
		.endClass()

		.beginClass<lxml_text>("xml_text")
		.addConstructor<void (*)()>()
		.addProperty("valid",&lxml_text::valid)
		.addProperty("string",&lxml_text::string)
		.addProperty("number",&lxml_text::number)
		.addProperty("bool",&lxml_text::as_bool)
		.addProperty("empty",&lxml_text::empty)
		.addFunction("set",&lxml_text::set)
		.addFunction("same_as",&lxml_text::same_as)
		.addFunction("data",&lxml_text::data)
		.endClass()

		.beginClass<lxml_node>("xml_node")
		.addConstructor<void (*)()>()
		.addProperty("valid",&lxml_node::valid)
		.addProperty("name",&lxml_node::name)
		.addProperty("value",&lxml_node::value)
		.addProperty("type",&lxml_node::type)
		.addProperty("path",&lxml_node::path)
		.addProperty("string",&lxml_node::string)
		.addProperty("hash_value",&lxml_node::hash_value)
		.addProperty("offset_debug",&lxml_node::offset_debug)
		.addFunction("child",&lxml_node::child)
		.addFunction("first_attribute",&lxml_node::first_attribute)
		.addFunction("last_attribute",&lxml_node::last_attribute)
		.addFunction("first_child",&lxml_node::first_child)
		.addFunction("last_child",&lxml_node::last_child)
		.addFunction("parent",&lxml_node::parent)
		.addFunction("root",&lxml_node::root)
		.addFunction("attribute",&lxml_node::attribute)
		.addFunction("next",&lxml_node::next)
		.addFunction("previous",&lxml_node::previous)
		.addFunction("next_sibling",&lxml_node::next_sibling)
		.addFunction("previous_sibling",&lxml_node::previous_sibling)
		.addFunction("child_value",&lxml_node::child_value)
		.addFunction("set_name",&lxml_node::set_name)
		.addFunction("set_value",&lxml_node::set_value)
		.addFunction("append_attribute",&lxml_node::append_attribute)
		.addFunction("prepend_attribute",&lxml_node::prepend_attribute)
		.addFunction("insert_attribute_after",&lxml_node::insert_attribute_after)
		.addFunction("insert_attribute_before",&lxml_node::insert_attribute_before)
		.addFunction("append_attribute_copy",&lxml_node::append_attribute_copy)
		.addFunction("prepend_attribute_copy",&lxml_node::prepend_attribute_copy)
		.addFunction("insert_attribute_copy_after",&lxml_node::insert_attribute_copy_after)
		.addFunction("insert_attribute_copy_before",&lxml_node::insert_attribute_copy_before)
		.addFunction("append",&lxml_node::append)
		.addFunction("prepend",&lxml_node::prepend)
		.addFunction("insert_after",&lxml_node::insert_after)
		.addFunction("insert_before",&lxml_node::insert_before)
		.addFunction("append_child",&lxml_node::append_child)
		.addFunction("prepend_child",&lxml_node::prepend_child)
		.addFunction("insert_child_after",&lxml_node::insert_child_after)
		.addFunction("insert_child_before",&lxml_node::insert_child_before)
		.addFunction("append_copy",&lxml_node::append_copy)
		.addFunction("prepend_copy",&lxml_node::prepend_copy)
		.addFunction("insert_copy_after",&lxml_node::insert_copy_after)
		.addFunction("insert_copy_before",&lxml_node::insert_copy_before)
		.addFunction("remove_attribute",&lxml_node::remove_attribute)
		.addFunction("remove_attribute_by_name",&lxml_node::remove_attribute_by_name)
		.addFunction("remove_child",&lxml_node::remove_child)
		.addFunction("remove_child_by_name",&lxml_node::remove_child_by_name)
		.addFunction("find_child_by_name_and_attribute",&lxml_node::find_child_by_name_and_attribute)
		.addFunction("find_child_by_attribute",&lxml_node::find_child_by_attribute)
		.addFunction("first_element_by_path",&lxml_node::first_element_by_path)
		.addFunction("select_single_node",&lxml_node::select_single_node)
		.addFunction("select_nodes",&lxml_node::select_nodes)
		.addFunction("text",&lxml_node::text)
		.addFunction("same_as",&lxml_node::same_as)
		.addFunction("as_string_with_options",&lxml_node::as_string_with_options)
		.endClass()

		.beginClass<lxml_document>("xml_document")
		.addConstructor<void (*)()>()
		.addProperty("valid",&lxml_document::valid)
		.addFunction("root",&lxml_document::root)
		.addFunction("reset",&lxml_document::reset)
		.addFunction("reset_with",&lxml_document::reset_with)
		.addFunction("load_file",&lxml_document::load_file)
		.addFunction("load_file_with_options",&lxml_document::load_file_with_options)
		.addFunction("load",&lxml_document::load)
		.addFunction("load_with_options",&lxml_document::load_with_options)
		.addFunction("save_file",&lxml_document::save_file)
		.addFunction("save_file_with_options",&lxml_document::save_file_with_options)
		.endClass()

		.beginClass<lxpath_node>("xpath_node")
		.addConstructor<void (*)()>()
		.addProperty("valid",&lxpath_node::valid)
		.addFunction("node",&lxpath_node::node)
		.addFunction("attribute",&lxpath_node::attribute)
		.addFunction("from_node",&lxpath_node::from_node)
		.addFunction("from_attribute",&lxpath_node::from_attribute)
		.addFunction("parent",&lxpath_node::parent)
		.addFunction("same_as",&lxpath_node::same_as)
		.endClass()

		.beginClass<lxpath_node_set>("xpath_node_set")
		.addConstructor<void (*)()>()
		.addProperty("type",&lxpath_node_set::type)
		.addProperty("size",&lxpath_node_set::size)
		.addProperty("empty",&lxpath_node_set::empty)
		.addStaticProperty("type_unsorted",&lxpath_node_set::type_unsorted)
		.addStaticProperty("type_sorted",&lxpath_node_set::type_sorted)
		.addStaticProperty("type_sorted_reverse",&lxpath_node_set::type_sorted_reverse)
		.addFunction("get",&lxpath_node_set::get)
		.addFunction("sort",&lxpath_node_set::sort)
		.addFunction("first",&lxpath_node_set::first)
		.endClass()

		.endNamespace()
		;
}
