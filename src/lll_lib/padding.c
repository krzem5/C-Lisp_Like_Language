#include <lll/common.h>
#include <lll/core.h>
#include <lll/types.h>



lll_object_offset_t _remove_padding_internal(lll_object_t* o,lll_compilation_data_t* c_dt,lll_object_offset_t* rm){
	lll_object_offset_t eoff=0;
	while (o->t==LLL_OBJECT_TYPE_NOP){
		eoff++;
		o++;
		(*rm)++;
	}
	*(o-*rm)=*o;
	switch (o->t){
		case LLL_OBJECT_TYPE_UNKNOWN:
		case LLL_OBJECT_TYPE_CHAR:
		case LLL_OBJECT_TYPE_STRING:
		case LLL_OBJECT_TYPE_IDENTIFIER:
		case LLL_OBJECT_TYPE_INT:
		case LLL_OBJECT_TYPE_FLOAT:
			return eoff+1;
		case LLL_OBJECT_TYPE_FUNC:
			{
				(*(c_dt->ft.dt+o->dt.fn.id))->off-=*rm;
				lll_object_offset_t off=1;
				lll_arg_count_t l=o->dt.fn.ac;
				while (l){
					l--;
					off+=_remove_padding_internal(o+off,c_dt,rm);
				}
				return off+eoff;
			}
		case LLL_OBJECT_TYPE_INTERNAL_FUNC:
			{
				lll_object_offset_t off=1;
				lll_arg_count_t l=o->dt.fn.ac;
				while (l){
					l--;
					off+=_remove_padding_internal(o+off,c_dt,rm);
				}
				return off+eoff;
			}
		case LLL_OBJECT_TYPE_OPERATION_LIST:
			{
				lll_object_offset_t off=1;
				lll_statement_count_t l=o->dt.sc;
				while (l){
					l--;
					off+=_remove_padding_internal(o+off,c_dt,rm);
				}
				return off+eoff;
			}
		case LLL_OBJECT_TYPE_DEBUG_DATA:
			return _remove_padding_internal(o+1,c_dt,rm)+eoff;
	}
	lll_object_offset_t off=1;
	lll_arg_count_t l=o->dt.ac;
	while (l){
		l--;
		off+=_remove_padding_internal(o+off,c_dt,rm);
	}
	return off+eoff;
}



__LLL_FUNC void lll_remove_object_padding(lll_compilation_data_t* c_dt,lll_object_t* o){
	lll_object_offset_t rm=0;
	_remove_padding_internal(o,c_dt,&rm);
}
