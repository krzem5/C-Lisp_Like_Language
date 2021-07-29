#include <lll_lib.h>
#include <_lll_lib_internal.h>
#include <stdint.h>
#include <stdlib.h>



#define WRITE_FIELD(f,os) LLL_WRITE_TO_OUTPUT_DATA_STREAM((os),(uint8_t*)(&(f)),sizeof((f)))
#define WRITE_SIGNED_INTEGER(os,n) _write_integer((os),((n)<0?((~(n))<<1)|1:(n)<<1))



void _write_integer(lll_output_data_stream_t* os,uint64_t v){
	while (v>0x7f){
		LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,(uint8_t)((v&0x7f)|0x80));
		v>>=7;
	}
	LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,(uint8_t)v);
}



lll_object_offset_t _write_object(lll_output_data_stream_t* os,const lll_object_t* o){
	lll_object_offset_t eoff=0;
	while (o->t==LLL_OBJECT_TYPE_NOP){
		eoff++;
		o++;
	}
	WRITE_FIELD(o->t,os);
	switch (o->t){
		case LLL_OBJECT_TYPE_UNKNOWN:
			return eoff+1;
		case LLL_OBJECT_TYPE_CHAR:
			WRITE_FIELD(o->dt.c,os);
			return eoff+1;
		case LLL_OBJECT_TYPE_INT:
			WRITE_SIGNED_INTEGER(os,o->dt.i);
			return eoff+1;
		case LLL_OBJECT_TYPE_FLOAT:
			WRITE_FIELD(o->dt.f,os);
			return eoff+1;
		case LLL_OBJECT_TYPE_STRING:
			_write_integer(os,o->dt.s);
			return eoff+1;
		case LLL_OBJECT_TYPE_IDENTIFIER:
			_write_integer(os,o->dt.id);
			return eoff+1;
		case LLL_OBJECT_TYPE_FUNC:
			{
				lll_object_offset_t off=1;
				_write_integer(os,o->dt.fn.id);
				lll_arg_count_t l=o->dt.fn.ac;
				LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,l);
				while (l){
					l--;
					off+=_write_object(os,o+off);
				}
				return off+eoff;
			}
		case LLL_OBJECT_TYPE_IMPORT:
			_write_integer(os,o->dt.ii);
			return eoff+1;
		case LLL_OBJECT_TYPE_OPERATION_LIST:
			{
				lll_object_offset_t off=1;
				lll_statement_count_t l=o->dt.sc;
				_write_integer(os,l);
				while (l){
					l--;
					off+=_write_object(os,o+off);
				}
				return off+eoff;
			}
		case LLL_OBJECT_TYPE_DEBUG_DATA:
			_write_integer(os,o->dt.dbg.fpi);
			_write_integer(os,o->dt.dbg.ln);
			_write_integer(os,o->dt.dbg.cn);
			_write_integer(os,o->dt.dbg.ln_off);
			return eoff+_write_object(os,o+1)+1;
	}
	lll_object_offset_t off=1;
	lll_arg_count_t l=o->dt.ac;
	LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,l);
	while (l){
		l--;
		off+=_write_object(os,o+off);
	}
	return off+eoff;
}



__LLL_IMPORT_EXPORT void lll_write_assembly(lll_output_data_stream_t* os,const lll_assembly_data_t* a_dt){
	uint32_t n=ASSEMBLY_FILE_MAGIC_NUMBER;
	LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,(uint8_t*)(&n),sizeof(uint32_t));
	lll_version_t v=LLL_VERSION;
	LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,(uint8_t*)(&v),sizeof(lll_version_t));
	_write_integer(os,a_dt->tm);
	_write_integer(os,a_dt->ic);
	_write_integer(os,a_dt->vc);
	_write_integer(os,a_dt->ft.l);
	for (lll_function_index_t i=0;i<a_dt->ft.l;i++){
		_write_integer(os,*(a_dt->ft.dt+i));
	}
	_write_integer(os,a_dt->st.l);
	for (lll_string_index_t i=0;i<a_dt->st.l;i++){
		const lll_string_t* s=*(a_dt->st.dt+i);
		_write_integer(os,s->l);
		LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,(uint8_t*)s->v,s->l*sizeof(lll_char_t));
	}
	const lll_assembly_instruction_t* ai=a_dt->h;
	for (lll_instruction_index_t i=0;i<a_dt->ic;i++){
		LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,(uint8_t)ai->t);
		switch (LLL_ASSEMBLY_INSTRUCTION_GET_TYPE(ai)){
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_INT:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_INT:
				WRITE_SIGNED_INTEGER(os,ai->dt.i);
				break;
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_FLOAT:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_FLOAT:
				LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,(uint8_t*)(&(ai->dt.f)),sizeof(lll_float_t));
				break;
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_CHAR:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_PRINT_CHAR:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_CHAR:
				LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,(uint8_t)ai->dt.c);
				break;
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_PUSH_II:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JMP:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JB:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JBE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JA:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JAE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JNE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JZ:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_JNZ:
				if (LLL_ASSEMBLY_INSTRUCTION_IS_RELATIVE(ai)){
					WRITE_SIGNED_INTEGER(os,ai->dt.rj);
				}
				else{
					_write_integer(os,ai->dt.rj);
				}
				break;
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_LOAD:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_POP:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_MINUS_ONE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_ZERO:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_ONE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_TWO:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_THREE:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_STORE_FOUR:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_INC:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_DEC:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_PRINT_VAR:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_VAR:
				_write_integer(os,ai->dt.v);
				break;
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_LOADS:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_PRINT_STR:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_RET_STR:
				_write_integer(os,ai->dt.s);
				break;
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_NOT:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_ADD:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_SUB:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_MULT:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_DIV:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_FDIV:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_MOD:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_AND:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_OR:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_XOR:
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_INV:
				if (LLL_ASSEMBLY_INSTRUCTION_IS_INPLACE(ai)){
					_write_integer(os,ai->dt.v);
				}
				break;
			case LLL_ASSEMBLY_INSTRUCTION_TYPE_CALL:
				LLL_WRITE_CHAR_TO_OUTPUT_DATA_STREAM(os,(uint8_t)ai->dt.ac);
				break;
		}
		ai++;
	}
}



__LLL_IMPORT_EXPORT void lll_write_object(lll_output_data_stream_t* os,const lll_object_t* o){
	_write_object(os,o);
}



__LLL_IMPORT_EXPORT void lll_write_compiled_object(lll_output_data_stream_t* os,const lll_compilation_data_t* c_dt){
	uint32_t n=COMPLIED_OBJECT_FILE_MAGIC_NUMBER;
	LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,(uint8_t*)(&n),sizeof(uint32_t));
	lll_version_t v=LLL_VERSION;
	LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,(uint8_t*)(&v),sizeof(lll_version_t));
	_write_integer(os,c_dt->tm);
	for (uint8_t i=0;i<LLL_MAX_SHORT_IDENTIFIER_LENGTH;i++){
		const lll_identifier_list_t* l=c_dt->i_dt.s+i;
		_write_integer(os,l->l);
		for (lll_identifier_list_length_t j=0;j<l->l;j++){
			_write_integer(os,(l->dt+j)->sc);
			_write_integer(os,(l->dt+j)->i);
		}
	}
	_write_integer(os,c_dt->i_dt.ill);
	for (lll_identifier_list_length_t i=0;i<c_dt->i_dt.ill;i++){
		_write_integer(os,(c_dt->i_dt.il+i)->sc);
		_write_integer(os,(c_dt->i_dt.il+i)->i);
	}
	_write_integer(os,c_dt->im.l);
	for (lll_import_index_t i=0;i<c_dt->im.l;i++){
		_write_integer(os,*(c_dt->im.dt+i));
	}
	_write_integer(os,c_dt->f_dt.l);
	for (lll_function_index_t i=0;i<c_dt->f_dt.l;i++){
		const lll_function_t* k=*(c_dt->f_dt.dt+i);
		_write_integer(os,k->off);
		_write_integer(os,k->al);
		for (lll_arg_count_t i=0;i<k->al;i++){
			_write_integer(os,k->a[i]);
		}
	}
	_write_integer(os,c_dt->st.l);
	for (lll_string_index_t i=0;i<c_dt->st.l;i++){
		const lll_string_t* s=*(c_dt->st.dt+i);
		_write_integer(os,s->l);
		LLL_WRITE_TO_OUTPUT_DATA_STREAM(os,(uint8_t*)s->v,s->l*sizeof(lll_char_t));
	}
	_write_integer(os,c_dt->_n_sc_id);
	_write_object(os,c_dt->h);
}
