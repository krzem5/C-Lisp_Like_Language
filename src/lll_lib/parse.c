#include <lll_lib.h>
#include <_lll_lib_internal.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>



lll_object_offset_t _patch_module(lll_object_t* mo,const import_module_data_t* im_dt){
	lll_object_offset_t eoff=0;
	lll_object_t* o=im_dt->d+(mo-im_dt->s);
	while (mo->t==LLL_OBJECT_TYPE_NOP){
		eoff++;
		o->t=LLL_OBJECT_TYPE_NOP;
		o++;
		mo++;
	}
	*o=*mo;
	switch (o->t){
		case LLL_OBJECT_TYPE_UNKNOWN:
		case LLL_OBJECT_TYPE_CHAR:
		case LLL_OBJECT_TYPE_INT:
		case LLL_OBJECT_TYPE_FLOAT:
			return eoff+1;
		case LLL_OBJECT_TYPE_STRING:
			o->dt.s=*(im_dt->sm+o->dt.s);
			return eoff+1;
		case LLL_OBJECT_TYPE_IDENTIFIER:
			for (lll_export_table_length_t i=0;i<im_dt->eiml;i++){
				if ((im_dt->eim+i)->a==o->dt.id){
					o->dt.id=(im_dt->eim+i)->b;
					return eoff+1;
				}
			}
			o->dt.id=LLL_IDENTIFIER_ADD_INDEX(o->dt.id,im_dt->off[LLL_IDENTIFIER_GET_ARRAY_ID(o->dt.id)]);
			return eoff+1;
		case LLL_OBJECT_TYPE_FUNC:
			o->dt.fn.id+=im_dt->f_off;
		case LLL_OBJECT_TYPE_INTERNAL_FUNC:
			{
				lll_object_offset_t off=1;
				lll_arg_count_t l=o->dt.fn.ac;
				while (l){
					l--;
					off+=_patch_module(mo+off,im_dt);
				}
				return off+eoff;
			}
		case LLL_OBJECT_TYPE_OPERATION_LIST:
			{
				lll_object_offset_t off=1;
				lll_statement_count_t l=o->dt.sc;
				while (l){
					l--;
					off+=_patch_module(mo+off,im_dt);
				}
				return off+eoff;
			}
		case LLL_OBJECT_TYPE_DEBUG_DATA:
			o->dt.dbg.fpi=*(im_dt->sm+o->dt.dbg.fpi);
			return eoff+_patch_module(mo+1,im_dt)+1;
	}
	lll_object_offset_t off=1;
	lll_arg_count_t l=o->dt.ac;
	while (l){
		l--;
		off+=_patch_module(mo+off,im_dt);
	}
	return off+eoff;
}



uint8_t _read_single_char(lll_input_data_stream_t* is,lll_file_offset_t st,lll_error_t* e,lll_char_t* o){
	int c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
	if (c==LLL_END_OF_DATA){
		e->t=LLL_ERROR_UNMATCHED_QUOTES;
		e->dt.r.off=st;
		e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
		return 1;
	}
	if (c=='\r'||c=='\n'){
		e->t=LLL_ERROR_UNMATCHED_QUOTES;
		e->dt.r.off=st;
		e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
		return 1;
	}
	if (c=='\\'){
		c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
		if (c=='\r'||c=='\n'){
			e->t=LLL_ERROR_UNMATCHED_QUOTES;
			e->dt.r.off=st;
			e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
			return 1;
		}
		if (c==LLL_END_OF_DATA){
			e->t=LLL_ERROR_UNTERMINATED_ESCAPE_SEQUENCE;
			e->dt.r.off=st;
			e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
			return 1;
		}
		if (c=='\''||c=='"'||c=='\\'){
			*o=(lll_char_t)c;
			return 0;
		}
		else if (c=='x'){
			c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			if (c==LLL_END_OF_DATA){
				e->t=LLL_ERROR_UNMATCHED_QUOTES;
				e->dt.r.off=st;
				e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
				return 1;
			}
			if (c>96){
				c-=32;
			}
			if (c<48||(c>57&&c<65)||c>70){
				if (c=='\r'||c=='\n'){
					e->t=LLL_ERROR_UNMATCHED_QUOTES;
					e->dt.r.off=st;
					e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
					return 1;
				}
				e->t=LLL_ERROR_UNKNOWN_HEXADECIMAL_CHARCTER;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				return 1;
			}
			uint8_t v=(c>64?c-55:c-48)<<4;
			c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			if (c==LLL_END_OF_DATA){
				e->t=LLL_ERROR_UNMATCHED_QUOTES;
				e->dt.r.off=st;
				e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
				return 1;
			}
			if (c>96){
				c-=32;
			}
			if (c<48||(c>57&&c<65)||c>70){
				if (c=='\r'||c=='\n'){
					e->t=LLL_ERROR_UNMATCHED_QUOTES;
					e->dt.r.off=st;
					e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st-1;
					return 1;
				}
				e->t=LLL_ERROR_UNKNOWN_HEXADECIMAL_CHARCTER;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				return 1;
			}
			c=(c>64?c-55:c-48)|v;
		}
		else if (c=='t'){
			c='\t';
		}
		else if (c=='n'){
			c='\n';
		}
		else if (c=='v'){
			c='\v';
		}
		else if (c=='f'){
			c='\f';
		}
		else if (c=='r'){
			c='\r';
		}
		else{
			e->t=LLL_ERROR_UNKNOWN_ESCAPE_CHARACTER;
			e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
			e->dt.r.sz=1;
			return 1;
		}
	}
	*o=(lll_char_t)c;
	return 0;
}



uint8_t _read_object_internal(lll_compilation_data_t* c_dt,int c,const extra_compilation_data_t* e_c_dt,lll_error_t* e){
	uint8_t fl=e_c_dt->fl;
	const scope_data_t* l_sc=&(e_c_dt->sc);
	scope_data_t n_l_sc={
		NULL
	};
	const scope_data_t* b_l_sc=NULL;
	lll_input_data_stream_t* is=c_dt->is;
	lll_file_offset_t st_off=0;
	int ec=-1;
	lll_object_t* o=NULL;
	lll_arg_count_t ac=0;
	lll_statement_count_t sc=0;
	while (c!=LLL_END_OF_DATA){
		if ((c>8&&c<14)||c==' '){
			while ((c>8&&c<14)||c==' '){
				c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			}
		}
		else if (c==';'){
			while (c!='\n'&&c!='\r'&&c!=LLL_END_OF_DATA){
				c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			}
		}
		else if (c==')'){
			if (n_l_sc.m){
				free(n_l_sc.m);
			}
			if (!o||ec!=')'){
				e->t=LLL_ERROR_UNMATCHED_CLOSE_PARENTHESES;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				return LLL_RETURN_ERROR;
			}
			if (o->t==LLL_OBJECT_TYPE_UNKNOWN){
				c_dt->_s.off+=sizeof(lll_object_t);
				o->t=LLL_OBJECT_TYPE_INT;
				o->dt.i=0;
			}
			else if (o->t==LLL_OBJECT_TYPE_ASSIGN){
				if (ac<2){
					o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
					o->dt.sc=o->dt.ac;
				}
				else{
					lll_object_t* a=o+1;
					while (a->t==LLL_OBJECT_TYPE_DEBUG_DATA){
						a++;
					}
					if (a->t!=LLL_OBJECT_TYPE_IDENTIFIER){
						o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
						o->dt.sc=o->dt.ac;
					}
					else{
						o->dt.ac=ac;
					}
				}
			}
			else if (o->t==LLL_OBJECT_TYPE_FUNC){
				if (!ac){
					o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
					o->dt.sc=0;
				}
				else{
					lll_object_offset_t off=1;
					lll_arg_count_t i=0;
					for (;i<ac;i++){
						while ((o+off)->t==LLL_OBJECT_TYPE_NOP||(o+off)->t==LLL_OBJECT_TYPE_DEBUG_DATA){
							(o+off)->t=LLL_OBJECT_TYPE_NOP;
							off++;
						}
						if ((o+off)->t!=LLL_OBJECT_TYPE_IDENTIFIER){
							break;
						}
						off++;
					}
					o->dt.fn.ac=ac-i;
					o->dt.fn.id=c_dt->ft.l;
					c_dt->ft.l++;
					void* tmp=realloc(c_dt->ft.dt,c_dt->ft.l*sizeof(lll_function_t*));
					if (!tmp){
						ASSERT(!"Unable to Reallocate Function Array");
					}
					c_dt->ft.dt=tmp;
					lll_function_t* f=malloc(sizeof(lll_function_t)+i*sizeof(lll_identifier_index_t));
					f->off=(lll_object_offset_t)(o-c_dt->h);
					f->al=i;
					off=1;
					lll_object_offset_t eoff=1;
					for (lll_arg_count_t j=0;j<i;j++){
						while ((o+off)->t==LLL_OBJECT_TYPE_NOP){
							off++;
						}
						ASSERT((o+off)->t==LLL_OBJECT_TYPE_IDENTIFIER);
						f->a[j]=(o+off)->dt.id;
						off++;
						eoff=off;
					}
					for (off=1;off<eoff;off++){
						(o+off)->t=LLL_OBJECT_TYPE_NOP;
					}
					*(c_dt->ft.dt+c_dt->ft.l-1)=f;
				}
			}
			else if (o->t==LLL_OBJECT_TYPE_INTERNAL_FUNC){
				if (!ac){
					o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
					o->dt.sc=0;
				}
				else{
					lll_object_offset_t off=1;
					while ((o+off)->t==LLL_OBJECT_TYPE_NOP||(o+off)->t==LLL_OBJECT_TYPE_DEBUG_DATA){
						(o+off)->t=LLL_OBJECT_TYPE_NOP;
						off++;
					}
					if ((o+off)->t!=LLL_OBJECT_TYPE_STRING){
						o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
						o->dt.sc=ac;
					}
					else{
						(o+off)->t=LLL_OBJECT_TYPE_NOP;
						lll_string_t* s=*(c_dt->st.dt+(o+off)->dt.s);
						if (s->l>255){
							e->t=LLL_ERROR_INTERNAL_FUNCTION_NAME_TOO_LONG;
							e->dt.r.off=st_off;
							e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st_off;
							return LLL_RETURN_ERROR;
						}
						o->dt.fn.ac=ac-1;
						o->dt.fn.id=lll_lookup_internal_function(e_c_dt->i_ft,(char*)s->v);
						if (o->dt.fn.id==LLL_MAX_FUNCTION_INDEX){
							e->t=LLL_ERROR_UNKNOWN_INTERNAL_FUNCTION;
							uint8_t i=0;
							while (i<s->l){
								e->dt.str[i]=s->v[i];
								i++;
							}
							e->dt.str[i]=0;
							return LLL_RETURN_ERROR;
						}
					}
				}
			}
			else if (fl&(EXTRA_COMPILATION_DATA_IMPORT|EXTRA_COMPILATION_DATA_EXPORT)){
				o->dt.sc=sc;
			}
			else{
				o->dt.ac=ac;
			}
			return LLL_RETURN_NO_ERROR;
		}
		else if (c=='}'){
			if (n_l_sc.m){
				free(n_l_sc.m);
			}
			if (!o||ec!='}'){
				e->t=LLL_ERROR_UNMATCHED_CURLY_CLOSE_BRACKETS;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				return LLL_RETURN_ERROR;
			}
			o->dt.sc=sc;
			return LLL_RETURN_NO_ERROR;
		}
		else if (c=='('||c=='{'){
			if (!o){
				if (!lll_insert_debug_object(c_dt,is,e)){
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				st_off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				o=(lll_object_t*)(c_dt->_s.ptr+c_dt->_s.off);
				if (c=='{'){
					c_dt->_s.off+=sizeof(lll_object_t);
					if (c_dt->_s.off>=c_dt->_s.sz){
						e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
						e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
						e->dt.r.sz=1;
						if (n_l_sc.m){
							free(n_l_sc.m);
						}
						return LLL_RETURN_ERROR;
					}
					o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
					ec='}';
				}
				else{
					o->t=LLL_OBJECT_TYPE_UNKNOWN;
					ec=')';
				}
			}
			else{
				if (o->t==LLL_OBJECT_TYPE_IF&&(ac&1)){
					if (n_l_sc.m){
						free(n_l_sc.m);
						l_sc=b_l_sc;
					}
					n_l_sc.l_sc=c_dt->_n_sc_id;
					n_l_sc.ml=(n_l_sc.l_sc+65)>>6;
					n_l_sc.m=malloc(n_l_sc.ml*sizeof(uint64_t));
					n_l_sc.m[n_l_sc.ml-1]=0;
					for (scope_data_mask_length_t i=0;i<l_sc->ml;i++){
						*(n_l_sc.m+i)=*(l_sc->m+i);
					}
					n_l_sc.m[n_l_sc.ml-1]|=1ull<<(n_l_sc.l_sc&63);
					c_dt->_n_sc_id++;
					b_l_sc=l_sc;
					l_sc=&n_l_sc;
				}
				lll_file_offset_t arg_s=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				if (o->t==LLL_OBJECT_TYPE_UNKNOWN){
					e->t=LLL_ERROR_NO_SYMBOL;
					e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
					e->dt.r.sz=1;
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				extra_compilation_data_t n_e_c_dt={
					fl&EXTRA_COMPILATION_DATA_INSIDE_FUNCTION,
					*l_sc,
					e_c_dt->i_ft,
					e_c_dt->il
				};
				if (!_read_object_internal(c_dt,c,&n_e_c_dt,e)){
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				if (o->t==LLL_OBJECT_TYPE_OPERATION_LIST){
					if (sc==UINT16_MAX){
						e->t=LLL_ERROR_TOO_MANY_STATEMENTS;
						e->dt.r.off=arg_s;
						e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-arg_s-1;
						if (n_l_sc.m){
							free(n_l_sc.m);
						}
						return LLL_RETURN_ERROR;
					}
					sc++;
				}
				else{
					if (ac==UINT8_MAX){
						e->t=LLL_ERROR_TOO_MANY_ARGUMENTS;
						e->dt.r.off=arg_s;
						e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-arg_s-1;
						if (n_l_sc.m){
							free(n_l_sc.m);
						}
						return LLL_RETURN_ERROR;
					}
					ac++;
				}
			}
			c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
		}
		else if (o&&o->t==LLL_OBJECT_TYPE_UNKNOWN){
			char* str=(char*)(c_dt->_s.ptr+c_dt->_s.off);
			uint16_t sz=0;
_read_symbol:
			if (sz==UINT16_MAX){
				e->t=LLL_ERROR_SYMBOL_TOO_LONG;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-sz-3;
				e->dt.r.sz=sz;
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			*(str+sz)=c;
			sz++;
			if (sz+c_dt->_s.off>=c_dt->_s.sz){
				e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			if (c==LLL_END_OF_DATA){
				break;
			}
			if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
				goto _read_symbol;
			}
			if (sz==1){
				if (*str=='!'){
					o->t=LLL_OBJECT_TYPE_NOT;
				}
				else if (*str=='='){
					o->t=LLL_OBJECT_TYPE_ASSIGN;
				}
				else if (*str=='?'){
					o->t=LLL_OBJECT_TYPE_IF;
				}
				else if (*str=='+'){
					o->t=LLL_OBJECT_TYPE_ADD;
				}
				else if (*str=='-'){
					o->t=LLL_OBJECT_TYPE_SUB;
				}
				else if (*str=='*'){
					o->t=LLL_OBJECT_TYPE_MULT;
				}
				else if (*str=='/'){
					o->t=LLL_OBJECT_TYPE_DIV;
				}
				else if (*str=='%'){
					o->t=LLL_OBJECT_TYPE_MOD;
				}
				else if (*str=='&'){
					o->t=LLL_OBJECT_TYPE_BIT_AND;
				}
				else if (*str=='|'){
					o->t=LLL_OBJECT_TYPE_BIT_OR;
				}
				else if (*str=='^'){
					o->t=LLL_OBJECT_TYPE_BIT_XOR;
				}
				else if (*str=='~'){
					o->t=LLL_OBJECT_TYPE_BIT_NOT;
				}
				else if (*str=='<'){
					o->t=LLL_OBJECT_TYPE_LESS;
				}
				else if (*str=='>'){
					o->t=LLL_OBJECT_TYPE_MORE;
				}
				else if (*str=='@'){
					o->t=LLL_OBJECT_TYPE_EXIT;
				}
				else{
					goto _unknown_symbol;
				}
			}
			else if (sz==2){
				if (*str==':'&&*(str+1)=='>'){
					o->t=LLL_OBJECT_TYPE_PRINT;
				}
				else if (*str=='<'&&*(str+1)==':'){
					o->t=LLL_OBJECT_TYPE_INPUT;
				}
				else if (*str=='&'&&*(str+1)=='&'){
					o->t=LLL_OBJECT_TYPE_AND;
				}
				else if (*str=='|'&&*(str+1)=='|'){
					o->t=LLL_OBJECT_TYPE_OR;
				}
				else if (*str=='<'&&*(str+1)=='-'){
					o->t=LLL_OBJECT_TYPE_CALL;
				}
				else if (*str=='-'&&*(str+1)=='>'){
					o->t=LLL_OBJECT_TYPE_FOR;
				}
				else if (*str=='>'&&*(str+1)=='-'){
					o->t=LLL_OBJECT_TYPE_WHILE;
				}
				else if (*str=='>'&&*(str+1)=='<'){
					o->t=LLL_OBJECT_TYPE_LOOP;
				}
				else if (*str=='/'&&*(str+1)=='/'){
					o->t=LLL_OBJECT_TYPE_FLOOR_DIV;
				}
				else if (*str=='<'&&*(str+1)=='='){
					o->t=LLL_OBJECT_TYPE_LESS_EQUAL;
				}
				else if (*str=='='&&*(str+1)=='='){
					o->t=LLL_OBJECT_TYPE_EQUAL;
				}
				else if (*str=='!'&&*(str+1)=='='){
					o->t=LLL_OBJECT_TYPE_NOT_EQUAL;
				}
				else if (*str=='>'&&*(str+1)=='='){
					o->t=LLL_OBJECT_TYPE_MORE_EQUAL;
				}
				else if (*str=='@'&&*(str+1)=='@'){
					o->t=(fl&EXTRA_COMPILATION_DATA_INSIDE_FUNCTION?LLL_OBJECT_TYPE_RETURN:LLL_OBJECT_TYPE_EXIT);
				}
				else if (*str=='-'&&*(str+1)=='-'){
					o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
					sc=0;
					fl|=EXTRA_COMPILATION_DATA_IMPORT;
				}
				else if (*str=='#'&&*(str+1)=='#'){
					o->t=LLL_OBJECT_TYPE_OPERATION_LIST;
					sc=0;
					fl|=EXTRA_COMPILATION_DATA_EXPORT;
				}
				else{
					goto _unknown_symbol;
				}
			}
			else if (sz==3){
				if (*str==','&&*(str+1)==','&&*(str+2)==','){
					o->t=LLL_OBJECT_TYPE_FUNC;
					fl|=EXTRA_COMPILATION_DATA_INSIDE_FUNCTION;
				}
				else if (*str=='.'&&*(str+1)=='.'&&*(str+2)=='.'){
					o->t=LLL_OBJECT_TYPE_INTERNAL_FUNC;
				}
				else{
					goto _unknown_symbol;
				}
			}
			else{
_unknown_symbol:
				e->t=LLL_ERROR_UNKNOWN_SYMBOL;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-sz-1;
				e->dt.r.sz=sz;
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			c_dt->_s.off+=sizeof(lll_object_t);
			if (c_dt->_s.off>=c_dt->_s.sz){
				e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			if (o->t==LLL_OBJECT_TYPE_IF||o->t==LLL_OBJECT_TYPE_FOR||o->t==LLL_OBJECT_TYPE_FUNC){
				n_l_sc.l_sc=c_dt->_n_sc_id;
				n_l_sc.ml=(n_l_sc.l_sc+65)>>6;
				n_l_sc.m=malloc(n_l_sc.ml*sizeof(uint64_t));
				n_l_sc.m[n_l_sc.ml-1]=0;
				for (scope_data_mask_length_t i=0;i<l_sc->ml;i++){
					*(n_l_sc.m+i)=*(l_sc->m+i);
				}
				n_l_sc.m[n_l_sc.ml-1]|=1ull<<(n_l_sc.l_sc&63);
				c_dt->_n_sc_id++;
				b_l_sc=l_sc;
				l_sc=&n_l_sc;
			}
		}
		else if (c=='|'){
			c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			if (c!='#'){
				e->t=LLL_ERROR_UNEXPECTED_CHARACTER;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-3;
				e->dt.r.sz=1;
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			int lc=c;
			do{
				lc=c;
				c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
				if (c==LLL_END_OF_DATA){
					e->t=LLL_ERROR_UNMATCHED_OPEN_QUOTE;
					e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-3;
					e->dt.r.sz=1;
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
			} while (c!='|'||lc!='#');
			c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
		}
		else{
			if (!lll_insert_debug_object(c_dt,is,e)){
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			lll_file_offset_t arg_s=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
			lll_object_t* arg=(lll_object_t*)(c_dt->_s.ptr+c_dt->_s.off);
			c_dt->_s.off+=sizeof(lll_object_t);
			if (c_dt->_s.off>=c_dt->_s.sz){
				e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			if (c=='\''){
				arg->t=LLL_OBJECT_TYPE_CHAR;
				if (_read_single_char(is,arg_s,e,&(arg->dt.c))){
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				if (arg->dt.c=='\''){
					e->t=LLL_ERROR_EMPTY_CHAR_STRING;
					e->dt.r.off=arg_s;
					e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-arg_s-1;
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
				if (c!='\''){
					e->t=LLL_ERROR_UNTERMINATED_CHAR_STRING;
					e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
					e->dt.r.sz=1;
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			}
			else if (c=='"'){
				arg->t=LLL_OBJECT_TYPE_STRING;
				lll_string_length_t ln=0;
				lll_char_t* s=(lll_char_t*)(c_dt->_s.ptr+c_dt->_s.off);
				while (1){
					if (_read_single_char(is,arg_s,e,s+ln)){
						if (n_l_sc.m){
							free(n_l_sc.m);
						}
						return LLL_RETURN_ERROR;
					}
					if (*(s+ln)=='"'){
						break;
					}
					ln++;
				}
				arg->dt.s=lll_create_string(&(c_dt->st),s,ln);
				c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
			}
			else if ((c>47&&c<58)||c=='-'){
				int8_t m=1;
				if (c=='-'){
					m=-1;
					c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
					if (c==LLL_END_OF_DATA){
						break;
					}
				}
				lll_integer_t v=0;
				if (c=='0'){
					c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
					if (c==LLL_END_OF_DATA){
						break;
					}
					if (c=='x'||c=='X'){
						c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
						if (c==LLL_END_OF_DATA){
							break;
						}
_hexadecimal:
						if (c>96){
							c-=32;
						}
						if (c<48||(c>57&&c<65)||c>70){
							e->t=LLL_ERROR_UNKNOWN_HEXADECIMAL_CHARCTER;
							e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
							e->dt.r.sz=1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						v=(v<<4)+(c>64?c-55:c-48);
						c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
						if (c==LLL_END_OF_DATA){
							break;
						}
						if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
							goto _hexadecimal;
						}
					}
					else if (c=='o'||c=='O'){
						c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
						if (c==LLL_END_OF_DATA){
							break;
						}
_octal:
						if (c<48||c>55){
							e->t=LLL_ERROR_UNKNOWN_OCTAL_CHARCTER;
							e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
							e->dt.r.sz=1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						v=(v<<3)+(c-48);
						c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
						if (c==LLL_END_OF_DATA){
							break;
						}
						if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
							goto _octal;
						}
					}
					else if (c=='b'||c=='B'){
						c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
						if (c==LLL_END_OF_DATA){
							break;
						}
_binary:
						if (c!=48&&c!=49){
							e->t=LLL_ERROR_UNKNOWN_BINARY_CHARCTER;
							e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
							e->dt.r.sz=1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						v=(v<<1)+(c-48);
						c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
						if (c==LLL_END_OF_DATA){
							break;
						}
						if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
							goto _binary;
						}
					}
					else if (c=='.'||c=='e'||c=='E'){
						goto _parse_float;
					}
					else if (c>47&&c<58){
						goto _decimal;
					}
					if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
						e->t=LLL_ERROR_UNKNOWN_DECIMAL_CHARCTER;
						e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
						e->dt.r.sz=1;
						if (n_l_sc.m){
							free(n_l_sc.m);
						}
						return LLL_RETURN_ERROR;
					}
				}
				else{
_decimal:
					v=v*10+(c-48);
					c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
					if (c==LLL_END_OF_DATA){
						break;
					}
					if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
						if (c=='.'||c=='e'||c=='E'){
							goto _parse_float;
						}
						if (c<48||c>57){
							e->t=LLL_ERROR_UNKNOWN_DECIMAL_CHARCTER;
							e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
							e->dt.r.sz=1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						goto _decimal;
					}
				}
				arg->t=LLL_OBJECT_TYPE_INT;
				arg->dt.i=v*m;
				goto _skip_float_parse;
_parse_float:;
				int16_t ex=0;
				if (c=='.'){
_float:
					c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
					if (c==LLL_END_OF_DATA){
						break;
					}
					if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
						if (c=='e'||c=='E'){
							goto _parse_float_exponent;
						}
						if (c<48||c>57){
							e->t=LLL_ERROR_UNKNOWN_DECIMAL_CHARCTER;
							e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
							e->dt.r.sz=1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						ex--;
						v=v*10+(c-48);
						goto _float;
					}
				}
				else{
_parse_float_exponent:;
					int8_t em=1;
					int16_t ev=0;
					c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
					if (c==LLL_END_OF_DATA){
						break;
					}
					if (c=='-'){
						em=-1;
					}
					else if (c=='+');
					else{
						goto _add_exponent_char;
					}
_float_exponent:
					c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
					if (c==LLL_END_OF_DATA){
						break;
					}
_add_exponent_char:
					if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
						if (c<48||c>57){
							e->t=LLL_ERROR_UNKNOWN_DECIMAL_CHARCTER;
							e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
							e->dt.r.sz=1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						ev=ev*10+(c-48);
						goto _float_exponent;
					}
					ex+=ev*em;
				}
				arg->t=LLL_OBJECT_TYPE_FLOAT;
				arg->dt.f=((lll_float_t)v)*pow(5,ex)*pow(2,ex)*m;
_skip_float_parse:;
			}
			else if ((c>64&&c<91)||c=='_'||(c>96&&c<123)){
				lll_char_t* str=(lll_char_t*)(c_dt->_s.ptr+c_dt->_s.off);
				lll_string_length_t sz=0;
				lll_string_checksum_t cs=0;
_read_identifier:
				*(str+sz)=c;
				cs^=(lll_string_checksum_t)c;
				sz++;
				if (sz+c_dt->_s.off>=c_dt->_s.sz){
					e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
					e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
					e->dt.r.sz=1;
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				c=LLL_READ_FROM_INPUT_DATA_STREAM(is);
				if (c==LLL_END_OF_DATA){
					break;
				}
				if ((c>47&&c<58)||(c>64&&c<91)||c=='_'||(c>96&&c<123)){
					goto _read_identifier;
				}
				if (c<9||(c>13&&c!=' '&&c!=';'&&c!=')'&&c!='('&&c!='}'&&c!='{')){
					e->t=LLL_ERROR_UNKNOWN_IDENTIFIER_CHARACTER;
					e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
					e->dt.r.sz=1;
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				if ((sz==3&&*str=='n'&&*(str+1)=='i'&&*(str+2)=='l')||(sz==5&&*str=='f'&&*(str+1)=='a'&&*(str+2)=='l'&&*(str+3)=='s'&&*(str+4)=='e')){
					arg->t=LLL_OBJECT_TYPE_INT;
					arg->dt.i=0;
				}
				else if (sz==4&&*str=='t'&&*(str+1)=='r'&&*(str+2)=='u'&&*(str+3)=='e'){
					arg->t=LLL_OBJECT_TYPE_INT;
					arg->dt.i=1;
				}
				else{
					arg->t=LLL_OBJECT_TYPE_IDENTIFIER;
					if (sz<=LLL_MAX_SHORT_IDENTIFIER_LENGTH){
						lll_identifier_list_t* k=c_dt->idt.s+sz-1;
						lll_scope_t mx_sc=LLL_MAX_SCOPE;
						lll_identifier_index_t mx_i=0;
						for (lll_identifier_list_length_t i=0;i<k->l;i++){
							lll_identifier_t* si=k->dt+i;
							lll_string_t* si_s=*(c_dt->st.dt+si->i);
							if (si_s->c!=cs){
								continue;
							}
							for (lll_string_length_t j=0;j<sz;j++){
								if (*(str+j)!=si_s->v[j]){
									goto _next_short_identifier;
								}
							}
							if (si->sc==l_sc->l_sc){
								arg->dt.id=LLL_CREATE_IDENTIFIER(i,sz-1);
								goto _identifier_found;
							}
							else if ((l_sc->m[si->sc>>6]&(1ull<<(si->sc&0x3f)))&&(mx_sc==LLL_MAX_SCOPE||si->sc>mx_sc)){
								mx_sc=si->sc;
								mx_i=LLL_CREATE_IDENTIFIER(i,sz-1);
							}
_next_short_identifier:;
						}
						if (mx_sc!=LLL_MAX_SCOPE){
							arg->dt.id=mx_i;
							goto _identifier_found;
						}
						if (o->t!=LLL_OBJECT_TYPE_ASSIGN&&o->t!=LLL_OBJECT_TYPE_FUNC){
							e->t=LLL_ERROR_UNKNOWN_IDENTIFIER;
							e->dt.r.off=arg_s;
							e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-arg_s-1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						k->l++;
						k->dt=realloc(k->dt,k->l*sizeof(lll_identifier_t));
						(k->dt+k->l-1)->sc=l_sc->l_sc;
						(k->dt+k->l-1)->i=lll_create_string(&(c_dt->st),str,sz);
						arg->dt.id=LLL_CREATE_IDENTIFIER(k->l-1,sz-1);
					}
					else{
						lll_scope_t mx_sc=LLL_MAX_SCOPE;
						lll_identifier_index_t mx_i=0;
						for (lll_identifier_list_length_t i=0;i<c_dt->idt.ill;i++){
							lll_identifier_t* k=c_dt->idt.il+i;
							lll_string_t* s=*(c_dt->st.dt+k->i);
							if (s->c!=cs||s->l!=sz){
								continue;
							}
							for (lll_string_length_t j=0;j<sz;j++){
								if (*(str+j)!=s->v[j]){
									goto _next_long_identifier;
								}
							}
							if (k->sc==l_sc->l_sc){
								arg->dt.id=LLL_CREATE_IDENTIFIER(i,LLL_MAX_SHORT_IDENTIFIER_LENGTH);
								goto _identifier_found;
							}
							else if ((l_sc->m[k->sc>>6]&(1ull<<(k->sc&0x3f)))&&(mx_sc==LLL_MAX_SCOPE||k->sc>mx_sc)){
								mx_sc=k->sc;
								mx_i=LLL_CREATE_IDENTIFIER(i,LLL_MAX_SHORT_IDENTIFIER_LENGTH);
							}
_next_long_identifier:;
						}
						if (mx_sc!=LLL_MAX_SCOPE){
							arg->dt.id=mx_i;
							goto _identifier_found;
						}
						if (o->t!=LLL_OBJECT_TYPE_ASSIGN&&o->t!=LLL_OBJECT_TYPE_FUNC){
							e->t=LLL_ERROR_UNKNOWN_IDENTIFIER;
							e->dt.r.off=arg_s;
							e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-arg_s-1;
							if (n_l_sc.m){
								free(n_l_sc.m);
							}
							return LLL_RETURN_ERROR;
						}
						c_dt->idt.ill++;
						c_dt->idt.il=realloc(c_dt->idt.il,c_dt->idt.ill*sizeof(lll_identifier_t));
						(c_dt->idt.il+c_dt->idt.ill-1)->sc=l_sc->l_sc;
						(c_dt->idt.il+c_dt->idt.ill-1)->i=lll_create_string(&(c_dt->st),str,sz);
						arg->dt.id=LLL_CREATE_IDENTIFIER(c_dt->idt.ill-1,LLL_MAX_SHORT_IDENTIFIER_LENGTH);
					}
_identifier_found:;
				}
			}
			else{
				e->t=LLL_ERROR_UNEXPECTED_CHARACTER;
				e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
				e->dt.r.sz=1;
				if (n_l_sc.m){
					free(n_l_sc.m);
				}
				return LLL_RETURN_ERROR;
			}
			if (!o){
				return LLL_RETURN_NO_ERROR;
			}
			if (o->t==LLL_OBJECT_TYPE_OPERATION_LIST){
				if (sc==UINT16_MAX){
					e->t=LLL_ERROR_TOO_MANY_STATEMENTS;
					e->dt.r.off=arg_s;
					e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-arg_s-1;
					return LLL_RETURN_ERROR;
				}
				sc++;
				if ((fl&EXTRA_COMPILATION_DATA_IMPORT)&&arg->t==LLL_OBJECT_TYPE_STRING){
					lll_compilation_data_t im={0};
					if (e_c_dt->il(*(c_dt->st.dt+arg->dt.s),&im,e)==LLL_RETURN_ERROR){
						return LLL_RETURN_ERROR;
					}
					c_dt->_s.off-=sizeof(lll_object_t)*2;
					arg->t=LLL_OBJECT_TYPE_NOP;
					while (arg->t==LLL_OBJECT_TYPE_NOP||arg->t==LLL_OBJECT_TYPE_DEBUG_DATA){
						arg->t=LLL_OBJECT_TYPE_NOP;
						arg--;
					}
					arg++;
					import_module_data_t im_dt={
						.sm=malloc(im.st.l*sizeof(lll_string_index_t)),
						.f_off=c_dt->ft.l,
						.s=im.h,
						.d=arg,
						.eiml=im.et.l
					};
					for (lll_string_index_t i=0;i<im.st.l;i++){
						lll_string_t* s=*(im.st.dt+i);
						for (lll_string_index_t j=0;j<c_dt->st.l;j++){
							lll_string_t* d=*(c_dt->st.dt+j);
							if (s->c!=d->c||s->l!=d->l){
								continue;
							}
							for (lll_string_length_t k=0;k<s->l;k++){
								if (s->v[k]!=d->v[k]){
									goto _check_next_string;
								}
							}
							*(im_dt.sm+i)=j;
							goto _merge_next_string;
_check_next_string:;
						}
						*(im_dt.sm+i)=c_dt->st.l;
						c_dt->st.l++;
						c_dt->st.dt=realloc(c_dt->st.dt,c_dt->st.l*sizeof(lll_string_t*));
						lll_string_t* n=malloc(sizeof(lll_string_t)+(s->l+1)*sizeof(lll_char_t));
						n->l=s->l;
						n->c=s->c;
						for (lll_string_length_t j=0;j<s->l;j++){
							n->v[j]=s->v[j];
						}
						n->v[n->l]=0;
						*(c_dt->st.dt+c_dt->st.l-1)=n;
_merge_next_string:;
					}
					lll_identifier_list_length_t s_si[LLL_MAX_SHORT_IDENTIFIER_LENGTH];
					for (uint8_t i=0;i<LLL_MAX_SHORT_IDENTIFIER_LENGTH;i++){
						lll_identifier_list_t* il=c_dt->idt.s+i;
						lll_identifier_list_t mil=im.idt.s[i];
						im_dt.off[i]=il->l;
						if (mil.l){
							s_si[i]=il->l;
							il->l+=mil.l;
							void* tmp=realloc(il->dt,il->l*sizeof(lll_identifier_t));
							if (!tmp){
								ASSERT(!"Unable to Reallocate Fixed-Length Identifier Array",e,LLL_RETURN_ERROR);
							}
							il->dt=tmp;
							for (lll_identifier_list_length_t j=0;j<mil.l;j++){
								(il->dt+s_si[i]+j)->sc=(mil.dt+j)->sc+c_dt->_n_sc_id;
								(il->dt+s_si[i]+j)->i=*(im_dt.sm+(mil.dt+j)->i);
							}
						}
					}
					im_dt.off[LLL_MAX_SHORT_IDENTIFIER_LENGTH]=c_dt->idt.ill;
					lll_identifier_list_length_t si=c_dt->idt.ill;
					if (im.idt.ill){
						c_dt->idt.ill+=im.idt.ill;
						void* tmp=realloc(c_dt->idt.il,c_dt->idt.ill*sizeof(lll_identifier_t));
						if (!tmp){
							ASSERT(!"Unable to Reallocate Variable-Length Identifier Array",e,LLL_RETURN_ERROR);
						}
						c_dt->idt.il=tmp;
						for (lll_identifier_list_length_t j=0;j<im.idt.ill;j++){
							(c_dt->idt.il+si+j)->sc=(im.idt.il+j)->sc+c_dt->_n_sc_id;
							(c_dt->idt.il+si+j)->i=*(im_dt.sm+(im.idt.il+j)->i);
						}
					}
					im_dt.eim=malloc(im.et.l*sizeof(identifier_pair_t));
					for (lll_export_table_length_t i=0;i<im.et.l;i++){
						lll_identifier_index_t eii=*(im.et.dt+i);
						(im_dt.eim+i)->a=eii;
						lll_scope_t mx_sc=LLL_MAX_SCOPE;
						lll_identifier_index_t mx_i=0;
						lll_identifier_t* ei;
						uint8_t j=LLL_IDENTIFIER_GET_ARRAY_ID(eii);
						if (j==LLL_MAX_SHORT_IDENTIFIER_LENGTH){
							ei=c_dt->idt.il+si+LLL_IDENTIFIER_GET_ARRAY_INDEX(eii);
							for (lll_identifier_list_length_t k=0;k<c_dt->idt.ill;k++){
								lll_identifier_t* n=c_dt->idt.il+k;
								if (n->i!=ei->i){
									continue;
								}
								if (n->sc==l_sc->l_sc){
									ei->sc=LLL_MAX_SCOPE;
									(im_dt.eim+i)->b=LLL_CREATE_IDENTIFIER(k,LLL_MAX_SHORT_IDENTIFIER_LENGTH);
									goto _export_identifier_found;
								}
								if ((l_sc->m[n->sc>>6]&(1ull<<(n->sc&0x3f)))&&(mx_sc==LLL_MAX_SCOPE||n->sc>mx_sc)){
									mx_sc=n->sc;
									mx_i=LLL_CREATE_IDENTIFIER(k,LLL_MAX_SHORT_IDENTIFIER_LENGTH);
								}
							}
						}
						else{
							ei=c_dt->idt.s[j].dt+s_si[j]+LLL_IDENTIFIER_GET_ARRAY_INDEX(eii);
							lll_identifier_list_t* il=c_dt->idt.s+j;
							for (lll_identifier_list_length_t k=0;k<il->l;k++){
								lll_identifier_t* n=il->dt+k;
								if (n->i!=ei->i){
									continue;
								}
								if (n->sc==l_sc->l_sc){
									ei->sc=LLL_MAX_SCOPE;
									(im_dt.eim+i)->b=LLL_CREATE_IDENTIFIER(k,j);
									goto _export_identifier_found;
								}
								if ((l_sc->m[n->sc>>6]&(1ull<<(n->sc&0x3f)))&&(mx_sc==LLL_MAX_SCOPE||n->sc>mx_sc)){
									mx_sc=n->sc;
									mx_i=LLL_CREATE_IDENTIFIER(k,j);
								}
							}
						}
						if (mx_sc!=LLL_MAX_SCOPE){
							ei->sc=LLL_MAX_SCOPE;
							(im_dt.eim+i)->b=mx_i;
						}
						else{
							ei->sc=l_sc->l_sc;
							(im_dt.eim+i)->b=eii;
						}
_export_identifier_found:;
					}
					lll_function_index_t j=c_dt->ft.l;
					c_dt->ft.l+=im.ft.l;
					c_dt->ft.dt=realloc(c_dt->ft.dt,c_dt->ft.l*sizeof(lll_function_t*));
					for (lll_function_index_t i=0;i<im.ft.l;i++){
						lll_function_t* f=*(im.ft.dt+i);
						lll_function_t* nf=malloc(sizeof(lll_function_t)+f->al*sizeof(lll_identifier_index_t));
						nf->off=(lll_object_offset_t)(f->off+(arg-c_dt->h));
						nf->al=f->al;
						for (lll_arg_count_t k=0;k<f->al;k++){
							nf->a[k]=f->a[k];
						}
						*(c_dt->ft.dt+i+j)=nf;
					}
					c_dt->_n_sc_id+=im._n_sc_id;
					lll_object_offset_t im_sz=_patch_module(im.h,&im_dt);
					free(im_dt.sm);
					free(im_dt.eim);
					lll_free_compilation_data(&im);
					c_dt->_s.off+=im_sz*sizeof(lll_object_t);
					if (c_dt->_s.off>=c_dt->_s.sz){
						e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
						e->dt.r.off=0;
						e->dt.r.sz=1;
						return LLL_RETURN_ERROR;
					}
				}
				else if ((fl&EXTRA_COMPILATION_DATA_EXPORT)&&arg->t==LLL_OBJECT_TYPE_IDENTIFIER){
					sc--;
					for (lll_export_table_length_t j=0;j<c_dt->et.l;j++){
						if (*(c_dt->et.dt+j)==arg->dt.id){
							goto _skip_export;
						}
					}
					c_dt->et.l++;
					c_dt->et.dt=realloc(c_dt->et.dt,c_dt->et.l*sizeof(lll_identifier_index_t));
					*(c_dt->et.dt+c_dt->et.l-1)=arg->dt.id;
_skip_export:;
					arg->t=LLL_OBJECT_TYPE_NOP;
					while (arg->t==LLL_OBJECT_TYPE_NOP||arg->t==LLL_OBJECT_TYPE_DEBUG_DATA){
						arg->t=LLL_OBJECT_TYPE_NOP;
						arg--;
					}
				}
			}
			else{
				if (o->t==LLL_OBJECT_TYPE_IF&&(ac&1)){
					if (n_l_sc.m){
						free(n_l_sc.m);
						l_sc=b_l_sc;
					}
					n_l_sc.l_sc=c_dt->_n_sc_id;
					n_l_sc.ml=(n_l_sc.l_sc+65)>>6;
					n_l_sc.m=malloc(n_l_sc.ml*sizeof(uint64_t));
					n_l_sc.m[n_l_sc.ml-1]=0;
					for (scope_data_mask_length_t i=0;i<l_sc->ml;i++){
						*(n_l_sc.m+i)=*(l_sc->m+i);
					}
					n_l_sc.m[n_l_sc.ml-1]|=1ull<<(n_l_sc.l_sc&63);
					c_dt->_n_sc_id++;
					b_l_sc=l_sc;
					l_sc=&n_l_sc;
				}
				if (ac==UINT8_MAX){
					e->t=LLL_ERROR_TOO_MANY_ARGUMENTS;
					e->dt.r.off=arg_s;
					e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-arg_s-1;
					if (n_l_sc.m){
						free(n_l_sc.m);
					}
					return LLL_RETURN_ERROR;
				}
				ac++;
			}
		}
	}
	if (!o){
		o=(lll_object_t*)(c_dt->_s.ptr+c_dt->_s.off);
		c_dt->_s.off+=sizeof(lll_object_t);
		if (c_dt->_s.off>=c_dt->_s.sz){
			e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
			e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-1;
			e->dt.r.sz=1;
			if (n_l_sc.m){
				free(n_l_sc.m);
			}
			return LLL_RETURN_ERROR;
		}
		o->t=LLL_OBJECT_TYPE_INT;
		o->dt.i=0;
		return LLL_RETURN_NO_ERROR;
	}
	e->t=LLL_ERROR_UNMATCHED_OPEN_PARENTHESES;
	e->dt.r.off=st_off;
	e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(is)-st_off-1;
	if (n_l_sc.m){
		free(n_l_sc.m);
	}
	return LLL_RETURN_ERROR;
}



__LLL_IMPORT_EXPORT void lll_init_compilation_data(const char* fp,lll_input_data_stream_t* is,lll_compilation_data_t* o){
	o->is=is;
	o->tm=(lll_time_t)time(NULL);
	o->h=NULL;
	for (uint8_t i=0;i<LLL_MAX_SHORT_IDENTIFIER_LENGTH;i++){
		o->idt.s[i].dt=NULL;
		o->idt.s[i].l=0;
	}
	o->idt.il=NULL;
	o->idt.ill=0;
	o->et.dt=NULL;
	o->et.l=0;
	o->ft.dt=NULL;
	o->ft.l=0;
	o->st.dt=NULL;
	o->st.l=0;
	o->_n_sc_id=1;
	lll_char_t bf[512];
	lll_string_length_t i=0;
	while (*(fp+i)){
		bf[i]=*(fp+i);
		i++;
	}
	bf[i]=0;
	IGNORE(lll_create_string(&(o->st),bf,i));
}



__LLL_IMPORT_EXPORT __LLL_RETURN lll_parse_object(lll_compilation_data_t* c_dt,lll_internal_function_table_t* i_ft,lll_import_loader_t il,lll_error_t* e,lll_object_t** o){
	if (!c_dt->_s.ptr){
		e->t=LLL_ERROR_NO_STACK;
		return LLL_RETURN_ERROR;
	}
	lll_object_t* a=(lll_object_t*)(c_dt->_s.ptr+c_dt->_s.off);
	int c=LLL_READ_FROM_INPUT_DATA_STREAM(c_dt->is);
	if (c==LLL_END_OF_DATA){
		c_dt->_s.off+=sizeof(lll_object_t);
		a->t=LLL_OBJECT_TYPE_INT;
		a->dt.i=0;
		return LLL_RETURN_NO_ERROR;
	}
	extra_compilation_data_t e_c_dt={
		0,
		{
			malloc(sizeof(uint64_t)),
			0,
			1
		},
		i_ft,
		il
	};
	e_c_dt.sc.m[0]=1;
	if (!_read_object_internal(c_dt,c,&e_c_dt,e)){
		free(e_c_dt.sc.m);
		return LLL_RETURN_ERROR;
	}
	free(e_c_dt.sc.m);
	*o=a;
	return LLL_RETURN_NO_ERROR;
}



__LLL_IMPORT_EXPORT __LLL_RETURN lll_parse_all_objects(lll_compilation_data_t* c_dt,lll_internal_function_table_t* i_ft,lll_import_loader_t il,lll_error_t* e){
	if (!c_dt->_s.ptr){
		e->t=LLL_ERROR_NO_STACK;
		return LLL_RETURN_ERROR;
	}
	c_dt->h=(lll_object_t*)(c_dt->_s.ptr+c_dt->_s.off);
	c_dt->_s.off+=sizeof(lll_object_t);
	if (c_dt->_s.off>=c_dt->_s.sz){
		e->t=LLL_ERROR_INTERNAL_STACK_OVERFLOW;
		e->dt.r.off=LLL_GET_INPUT_DATA_STREAM_OFFSET(c_dt->is)-1;
		e->dt.r.sz=1;
		return LLL_RETURN_ERROR;
	}
	c_dt->h->t=LLL_OBJECT_TYPE_OPERATION_LIST;
	extra_compilation_data_t e_c_dt={
		0,
		{
			malloc(sizeof(uint64_t)),
			0,
			1
		},
		i_ft,
		il
	};
	e_c_dt.sc.m[0]=1;
	lll_statement_count_t sc=0;
	while (1){
		int c=LLL_READ_FROM_INPUT_DATA_STREAM(c_dt->is);
		if (c==LLL_END_OF_DATA){
			c_dt->h->dt.sc=sc;
			free(e_c_dt.sc.m);
			return LLL_RETURN_NO_ERROR;
		}
		lll_file_offset_t off=LLL_GET_INPUT_DATA_STREAM_OFFSET(c_dt->is);
		if (!_read_object_internal(c_dt,c,&e_c_dt,e)){
			free(e_c_dt.sc.m);
			return LLL_RETURN_ERROR;
		}
		if (sc==UINT16_MAX){
			e->t=LLL_ERROR_TOO_MANY_STATEMENTS;
			e->dt.r.off=off-1;
			e->dt.r.sz=LLL_GET_INPUT_DATA_STREAM_OFFSET(c_dt->is)-off-1;
			free(e_c_dt.sc.m);
			return LLL_RETURN_ERROR;
		}
		sc++;
	}
}
