#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#endif
#include <lll_lib.h>
#include <generated.h>
#include <stdio.h>
#include <stdlib.h>



#ifdef _MSC_VER
#define MAX_PATH_LENGTH MAX_PATH
#define EXPAND_FILE_PATH(s,d) GetFullPathNameA((s),MAX_PATH,(d),NULL)
#define GET_EXECUATBLE_FILE_PATH(bf,l,o) \
	do{ \
		(o)=GetModuleFileNameA(NULL,(bf),(l)); \
	} while (0)
#else
#define MAX_PATH_LENGTH PATH_MAX
#define EXPAND_FILE_PATH(s,d) realpath((s),(d))
#define GET_EXECUATBLE_FILE_PATH(bf,l,o) \
	do{ \
		ssize_t __o=readlink("/proc/self/exe",(bf),(l)); \
		if (__o!=-1){ \
			(o)=__o; \
			*((bf)+(o))=0; \
		} \
		else{ \
			(o)=0; \
			*(bf)=0; \
		} \
	} while (0)
#endif



#define PRINT_STATIC_STR(s) fwrite((void*)(s),sizeof(char),sizeof((s))/sizeof(char)-1,stdout)
#define _STR(x) #x
#define STR(x) _STR(x)
#define ASSEMBLY_STACK_SIZE 16384
#define COMPILER_STACK_SIZE 65536
#define VM_STACK_SIZE 65536
#define FLAG_EXPAND_PATH 1
#define FLAG_GENERATE_ASSEMBLY 2
#define FLAG_GENERATE_COMPILED_OBJECT 4
#define FLAG_HELP 8
#define FLAG_NO_LOGO 16
#define FLAG_NO_RUN 32
#define FLAG_PRINT_ASSEMBLY 64
#define FLAG_PRINT_OBJECT 128
#define FLAG_VERBOSE 256
#define _FLAG_ASSEMBLY_GENERATED 512
#define OPTIMIZE_LEVEL_NO_OPTIMIZE 0
#define OPTIMIZE_LEVEL_REMOVE_PADDING 1
#define OPTIMIZE_LEVEL_STRIP_DEBUG_DATA 2
#define OPTIMIZE_LEVEL_STRIP_GLOBAL_OPTIMIZE 3



uint8_t a_st[ASSEMBLY_STACK_SIZE];
uint8_t c_st[COMPILER_STACK_SIZE];
uint8_t vm_st[VM_STACK_SIZE];
uint8_t ol;
uint16_t fl;
char* i_fp;
uint32_t i_fpl;
char** fp;
uint32_t fpl;
lll_internal_function_table_t i_ft;



uint8_t load_file(const char* f_nm,lll_assembly_data_t* a_dt,lll_compilation_data_t* c_dt,FILE** f,lll_input_data_stream_t* is,char* f_fp);



uint8_t cmp_str(const char* a,const char* b){
	while (1){
		if (*a!=*b){
			return 0;
		}
		if (!(*a)){
			return 1;
		}
		a++;
		b++;
	}
}



void print_str(const char* s){
	while (*s){
		putchar(*s);
		s++;
	}
}



void print_int(int64_t v){
	if (!v){
		putchar('0');
		return;
	}
	if (v<0){
		v=-v;
		putchar('-');
	}
	char bf[20];
	uint8_t i=0;
	while (v){
		bf[i]=v%10;
		v/=10;
		i++;
	};
	while (i){
		i--;
		putchar(bf[i]+48);
	}
}



lll_return_t load_import(const lll_string_t* nm,lll_compilation_data_t* o,lll_error_t* e){
	char bf[MAX_PATH_LENGTH];
	for (lll_string_length_t m=0;m<nm->l;m++){
		bf[m]=nm->v[m];
	}
	bf[nm->l]=0;
	FILE* f=NULL;
	lll_assembly_data_t a_dt={0};
	lll_input_data_stream_t is;
	char f_fp[MAX_PATH_LENGTH];
	uint8_t st[COMPILER_STACK_SIZE];
	lll_set_assembly_data_stack(&a_dt,a_st,ASSEMBLY_STACK_SIZE);
	lll_set_compilation_data_stack(o,st,COMPILER_STACK_SIZE);
	if (!load_file(bf,&a_dt,o,&f,&is,f_fp)){
		if (f){
			fclose(f);
		}
		lll_free_compilation_data(o);
		e->t=LLL_ERROR_UNKNOWN;
		return LLL_RETURN_ERROR;
	}
	if (fl&_FLAG_ASSEMBLY_GENERATED){
		lll_free_assembly_function_table(&(a_dt.ft));
		lll_free_string_table(&(a_dt.st));
		PRINT_STATIC_STR("Importing Assembly into Compiled Objects is Not Allowed\n");
		e->t=LLL_ERROR_UNKNOWN;
		return LLL_RETURN_ERROR;
	}
	if (f){
		fclose(f);
	}
	return LLL_RETURN_NO_ERROR;
}



uint8_t load_file(const char* f_nm,lll_assembly_data_t* a_dt,lll_compilation_data_t* c_dt,FILE** f,lll_input_data_stream_t* is,char* f_fp){
	char bf[MAX_PATH_LENGTH];
	uint32_t j=0;
	for (uint32_t i=0;i<i_fpl;i++){
		if (!(*(i_fp+i))){
			uint32_t k=0;
			while (*(f_nm+k)){
				*(bf+j)=*(f_nm+k);
				j++;
				k++;
			}
			*(bf+j)='.';
			*(bf+j+1)='l';
			*(bf+j+2)='l';
			*(bf+j+3)='l';
			*(bf+j+4)='c';
			*(bf+j+5)=0;
			if (fl&FLAG_VERBOSE){
				PRINT_STATIC_STR("Trying to Open File '");
				print_str(bf);
				PRINT_STATIC_STR("'...\n");
			}
			FILE* nf=fopen(bf,"rb");// lgtm [cpp/path-injection]
			if (nf){
				if (!(fl&FLAG_EXPAND_PATH)||!EXPAND_FILE_PATH(bf,f_fp)){
					*(f_fp+j)=0;
					while (j){
						j--;
						*(f_fp+j)=*(bf+j);
					}
				}
				if (fl&FLAG_VERBOSE){
					PRINT_STATIC_STR("Found File '");
					print_str(f_fp);
					PRINT_STATIC_STR("'\n");
				}
				*f=nf;
				lll_stream_create_input_from_file(nf,is);
				lll_error_t e;
				if (!lll_load_compiled_object(is,c_dt,&e)){
					lll_free_identifier_table(&(c_dt->idt));
					lll_free_export_table(&(c_dt->et));
					lll_free_function_table(&(c_dt->ft));
					lll_free_string_table(&(c_dt->st));
					if (e.t==LLL_ERROR_INVALID_FILE_FORMAT){
						PRINT_STATIC_STR("File '");
						print_str(f_fp);
						PRINT_STATIC_STR("'is not a Compiled Object.\n");
					}
					else{
						lll_print_error(is,&e);
					}
					return 0;
				}
				if (fl&FLAG_PRINT_OBJECT){
					lll_output_data_stream_t os;
					lll_stream_create_output_from_file(stdout,&os);
					lll_print_object(c_dt,c_dt->h,&os);
					putchar('\n');
				}
				if (fl&FLAG_VERBOSE){
					PRINT_STATIC_STR("File Successfully Read.\n");
				}
				return 1;
			}
			*(bf+j)=0;
			if (fl&FLAG_VERBOSE){
				PRINT_STATIC_STR("Trying to Open File '");
				print_str(bf);
				PRINT_STATIC_STR("'...\n");
			}
			nf=fopen(bf,"rb");// lgtm [cpp/path-injection]
			if (nf){
				if (!(fl&FLAG_EXPAND_PATH)||!EXPAND_FILE_PATH(bf,f_fp)){
					*(f_fp+j)=0;
					while (j){
						j--;
						*(f_fp+j)=*(bf+j);
					}
				}
				if (fl&FLAG_VERBOSE){
					PRINT_STATIC_STR("Found File '");
					print_str(f_fp);
					PRINT_STATIC_STR("'\n");
				}
				*f=nf;
				lll_stream_create_input_from_file(nf,is);
				if (fl&FLAG_VERBOSE){
					PRINT_STATIC_STR("Trying to Load File as Assembly...\n");
				}
				lll_error_t e;
				if (!lll_load_assembly(is,a_dt,&e)){
					lll_free_assembly_function_table(&(a_dt->ft));
					lll_free_string_table(&(a_dt->st));
					if (e.t==LLL_ERROR_INVALID_FILE_FORMAT){
						if (fl&FLAG_VERBOSE){
							PRINT_STATIC_STR("File is not an Assembly. Falling Back to Compiled Object...\n");
						}
						lll_stream_create_input_from_file(nf,is);
						if (!lll_load_compiled_object(is,c_dt,&e)){
							lll_free_identifier_table(&(c_dt->idt));
							lll_free_export_table(&(c_dt->et));
							lll_free_function_table(&(c_dt->ft));
							lll_free_string_table(&(c_dt->st));
							if (e.t==LLL_ERROR_INVALID_FILE_FORMAT){
								if (fl&FLAG_VERBOSE){
									PRINT_STATIC_STR("File is not a Compiled Object. Falling Back to Standard Compilation...\n");
								}
								lll_stream_create_input_from_file(nf,is);
								lll_init_compilation_data(f_fp,is,c_dt);
								if (!lll_parse_all_objects(c_dt,&i_ft,load_import,&e)){
									if (e.t!=LLL_ERROR_UNKNOWN){
										lll_print_error(is,&e);
									}
									return 0;
								}
								if (fl&FLAG_PRINT_OBJECT){
									lll_output_data_stream_t os;
									lll_stream_create_output_from_file(stdout,&os);
									lll_print_object(c_dt,c_dt->h,&os);
									putchar('\n');
								}
								if (fl&FLAG_VERBOSE){
									PRINT_STATIC_STR("File Successfully Read.\n");
								}
							}
							else{
								lll_print_error(is,&e);
								return 0;
							}
						}
						else{
							if (fl&FLAG_PRINT_OBJECT){
								lll_output_data_stream_t os;
								lll_stream_create_output_from_file(stdout,&os);
								lll_print_object(c_dt,c_dt->h,&os);
								putchar('\n');
							}
							if (fl&FLAG_VERBOSE){
								PRINT_STATIC_STR("File Successfully Read.\n");
							}
						}
					}
					else{
						lll_print_error(is,&e);
						return 0;
					}
				}
				else{
					fl|=_FLAG_ASSEMBLY_GENERATED;
					if (fl&FLAG_VERBOSE){
						PRINT_STATIC_STR("File Successfully Read.\n");
					}
				}
				return 1;
			}
			j=0;
			continue;
		}
		*(bf+j)=*(i_fp+i);
		j++;
	}
	PRINT_STATIC_STR("Unable to Find File '");
	print_str(f_nm);
	PRINT_STATIC_STR("'\n");
	return 0;
}



uint8_t write_assembly(char* o_fp,const lll_assembly_data_t* a_dt){
	uint16_t i=0;
	while (*(o_fp+i)){
		i++;
	}
	*(o_fp+i)='.';
	*(o_fp+i+1)='l';
	*(o_fp+i+2)='l';
	*(o_fp+i+3)='l';
	*(o_fp+i+4)='a';
	*(o_fp+i+5)=0;
	if (fl&FLAG_VERBOSE){
		PRINT_STATIC_STR("Writing Assembly to File '");
		print_str(o_fp);
		PRINT_STATIC_STR("'...\n");
	}
	FILE* f=fopen(o_fp,"wb");// lgtm [cpp/path-injection]
	if (!f){
		PRINT_STATIC_STR("Unable to Open Output File '");
		print_str(o_fp);
		PRINT_STATIC_STR("'\n");
		return 0;
	}
	lll_output_data_stream_t os;
	lll_stream_create_output_from_file(f,&os);
	lll_write_assembly(&os,a_dt);
	if (fl&FLAG_VERBOSE){
		PRINT_STATIC_STR("File Written Successfully.\n");
	}
	fclose(f);
	return 1;
}



uint8_t write_compiled(char* o_fp,const lll_compilation_data_t* c_dt){
	uint16_t i=0;
	while (*(o_fp+i)){
		i++;
	}
	*(o_fp+i)='.';
	*(o_fp+i+1)='l';
	*(o_fp+i+2)='l';
	*(o_fp+i+3)='l';
	*(o_fp+i+4)='c';
	*(o_fp+i+5)=0;
	if (fl&FLAG_VERBOSE){
		PRINT_STATIC_STR("Writing Compiled Object to File '");
		print_str(o_fp);
		PRINT_STATIC_STR("'...\n");
	}
	FILE* f=fopen(o_fp,"wb");// lgtm [cpp/path-injection]
	if (!f){
		PRINT_STATIC_STR("Unable to Open Output File '");
		print_str(o_fp);
		PRINT_STATIC_STR("'\n");
		return 0;
	}
	lll_output_data_stream_t os;
	lll_stream_create_output_from_file(f,&os);
	lll_write_compiled_object(&os,c_dt);
	if (fl&FLAG_VERBOSE){
		PRINT_STATIC_STR("File Written Successfully.\n");
	}
	fclose(f);
	return 1;
}



int main(int argc,const char** argv){
	int32_t ec=1;
	ol=OPTIMIZE_LEVEL_STRIP_DEBUG_DATA;
	fl=0;
	i_fp=malloc(sizeof(char));
	if (!i_fp){
		PRINT_STATIC_STR("Unable to Allocate Space for Include File Path Array\n");
		return 1;
	}
	*i_fp=0;
	i_fpl=1;
	char e_fp[MAX_PATH_LENGTH];
	uint32_t e_fp_l;
	GET_EXECUATBLE_FILE_PATH(e_fp,MAX_PATH_LENGTH,e_fp_l);
	while (e_fp[e_fp_l]!='/'&&e_fp[e_fp_l]!='\\'){
		if (!e_fp_l){
			goto _skip_std_lib_path;
		}
		e_fp_l--;
	}
	e_fp[e_fp_l+1]='l';
	e_fp[e_fp_l+2]='i';
	e_fp[e_fp_l+3]='b';
	e_fp[e_fp_l+4]='/';
	e_fp[e_fp_l+5]=0;
	i_fpl+=e_fp_l+6;
	i_fp=realloc(i_fp,i_fpl*sizeof(char));
	for (uint32_t i=0;i<e_fp_l+6;i++){
		*(i_fp+i+1)=e_fp[i];
	}
_skip_std_lib_path:
	fp=NULL;
	fpl=0;
	lll_create_internal_function_table(&i_ft);
	lll_register_standard_internal_functions(&i_ft);
	const char* o_fp=NULL;
	FILE* f=NULL;
	lll_assembly_data_t a_dt={0};
	lll_compilation_data_t c_dt={0};
	uint32_t im_fpl=UINT32_MAX;
	for (int i=1;i<argc;i++){
		const char* e=argv[i];
		if ((*e=='-'&&*(e+1)=='a'&&*(e+2)==0)||cmp_str(e,"--generate-assembly")){
			fl|=FLAG_GENERATE_ASSEMBLY;
		}
		else if ((*e=='-'&&*(e+1)=='c'&&*(e+2)==0)||cmp_str(e,"--generate-compiled-object")){
			fl|=FLAG_GENERATE_COMPILED_OBJECT;
		}
		else if ((*e=='-'&&*(e+1)=='e'&&*(e+2)==0)||cmp_str(e,"--expand-file-paths")){
			fl|=FLAG_EXPAND_PATH;
		}
		else if ((*e=='-'&&*(e+1)=='f'&&*(e+2)==0)||cmp_str(e,"--file")){
			i++;
			if (i==argc){
				break;
			}
			goto _read_file_argument;
		}
		else if ((*e=='-'&&*(e+1)=='h'&&*(e+2)==0)||cmp_str(e,"--help")){
			fl|=FLAG_HELP;
		}
		else if ((*e=='-'&&*(e+1)=='I'&&*(e+2)==0)||cmp_str(e,"--include")){
			i++;
			if (i==argc){
				break;
			}
			e=argv[i];
			uint32_t sz=0;
			while (*(e+sz)){
				sz++;
			}
			if (sz){
				uint32_t j=i_fpl;
				i_fpl+=sz+(*(e+sz-1)!='\\'&&*(e+sz-1)!='/'?2:1);
				void* tmp=realloc(i_fp,i_fpl*sizeof(char));
				if (!tmp){
					PRINT_STATIC_STR("Unable to Allocate Space for Include File Path Array\n");
					goto _error;
				}
				i_fp=tmp;
				for (uint32_t k=0;k<sz;k++){
					*(i_fp+j)=*(e+k);
					j++;
				}
				if (*(e+sz-1)!='\\'&&*(e+sz-1)!='/'){
					*(i_fp+j)='/';
					j++;
				}
				*(i_fp+j)=0;
			}
		}
		else if ((*e=='-'&&*(e+1)=='L'&&*(e+2)==0)||cmp_str(e,"--no-logo")){
			fl|=FLAG_NO_LOGO;
		}
		else if ((*e=='-'&&*(e+1)=='o'&&*(e+2)==0)||cmp_str(e,"--output")){
			if (o_fp){
				PRINT_STATIC_STR("Multplie Output Files Supplied\n");
				goto _help;
			}
			i++;
			if (i==argc){
				break;
			}
			o_fp=argv[i];
		}
		else if (*e=='-'&&*(e+1)=='O'&&*(e+3)==0){
			if (*(e+2)=='0'){
				ol=OPTIMIZE_LEVEL_NO_OPTIMIZE;
			}
			else if (*(e+2)=='1'){
				ol=OPTIMIZE_LEVEL_REMOVE_PADDING;
			}
			else if (*(e+2)=='2'){
				ol=OPTIMIZE_LEVEL_STRIP_DEBUG_DATA;
			}
			else if (*(e+2)=='3'){
				ol=OPTIMIZE_LEVEL_STRIP_GLOBAL_OPTIMIZE;
			}
			else{
				goto _unkown_switch;
			}
		}
		else if ((*e=='-'&&*(e+1)=='P'&&*(e+2)==0)||cmp_str(e,"--print-assembly")){
			fl|=FLAG_PRINT_ASSEMBLY;
		}
		else if ((*e=='-'&&*(e+1)=='p'&&*(e+2)==0)||cmp_str(e,"--print-objects")){
			fl|=FLAG_PRINT_OBJECT;
		}
		else if ((*e=='-'&&*(e+1)=='R'&&*(e+2)==0)||cmp_str(e,"--no-run")){
			fl|=FLAG_NO_RUN;
		}
		else if ((*e=='-'&&*(e+1)=='v'&&*(e+2)==0)||cmp_str(e,"--verbose")){
			fl|=FLAG_VERBOSE;
		}
		else if (*e=='-'){
_unkown_switch:
			PRINT_STATIC_STR("Unknown Switch '");
			print_str((char*)e);
			PRINT_STATIC_STR("'\n");
			goto _help;
		}
		else{
_read_file_argument:
			fpl++;
			void* tmp=realloc(fp,fpl*sizeof(char*));
			if (!tmp){
				PRINT_STATIC_STR("Unable to Allocate Space for File Path Array\n");
				goto _error;
			}
			fp=tmp;
			*(fp+fpl-1)=(char*)e;
		}
	}
	im_fpl=fpl;
	if (!(fl&FLAG_NO_LOGO)){
		PRINT_STATIC_STR("lll v"STR(LLL_VERSION_MAJOR)"."STR(LLL_VERSION_MINOR)"."STR(LLL_VERSION_PATCH)" ("LLL_VERSION_BUILD_DATE", "LLL_VERSION_BUILD_TIME")\n");
	}
	if (fl&FLAG_VERBOSE){
		PRINT_STATIC_STR("Configuration:\n  Optimization Level:\n");
		if (ol==OPTIMIZE_LEVEL_NO_OPTIMIZE){
			PRINT_STATIC_STR("    No Optimization\n");
		}
		if (ol>=OPTIMIZE_LEVEL_REMOVE_PADDING){
			PRINT_STATIC_STR("    Padding Reduction\n");
		}
		if (ol>=OPTIMIZE_LEVEL_STRIP_DEBUG_DATA){
			PRINT_STATIC_STR("    Debug Data Stripping\n");
		}
		if (ol>=OPTIMIZE_LEVEL_STRIP_GLOBAL_OPTIMIZE){
			PRINT_STATIC_STR("    Global Optimization\n");
		}
		if (fl&FLAG_GENERATE_ASSEMBLY){
			PRINT_STATIC_STR("  Assembly Generation Mode\n");
		}
		if (fl&FLAG_GENERATE_COMPILED_OBJECT){
			PRINT_STATIC_STR("  Compiled Object Generation Mode\n");
		}
		if (fl&FLAG_EXPAND_PATH){
			PRINT_STATIC_STR("  Path Expand Mode\n");
		}
		if (fl&FLAG_HELP){
			PRINT_STATIC_STR("  Help Print Mode\n");
		}
		if (!(fl&FLAG_NO_LOGO)){
			PRINT_STATIC_STR("  Compiler Logo Mode\n");
		}
		if (fl&FLAG_PRINT_ASSEMBLY){
			PRINT_STATIC_STR("  Assembly Print Mode\n");
		}
		if (fl&FLAG_PRINT_OBJECT){
			PRINT_STATIC_STR("  Object Print Mode\n");
		}
		if (!(fl&FLAG_NO_RUN)){
			PRINT_STATIC_STR("  Program Run Mode\n");
		}
		if (fl&FLAG_VERBOSE){
			PRINT_STATIC_STR("  Verbose Mode\n");
		}
		PRINT_STATIC_STR("Include Path: \n  - '");
		for (uint32_t i=0;i<i_fpl;i++){
			if (*(i_fp+i)){
				putchar(*(i_fp+i));
			}
			else{
				if (i+1<i_fpl){
					PRINT_STATIC_STR("'\n  - '");
				}
				else{
					PRINT_STATIC_STR("'\n");
				}
			}
		}
	}
	if (fl&FLAG_HELP){
		goto _help;
	}
	if (!fpl){
		PRINT_STATIC_STR("No Input Files Supplied\n");
		goto _help;
	}
	for (uint32_t j=0;j<fpl;j++){
		lll_set_assembly_data_stack(&a_dt,a_st,ASSEMBLY_STACK_SIZE);
		lll_set_compilation_data_stack(&c_dt,c_st,COMPILER_STACK_SIZE);
		char f_fp[MAX_PATH_LENGTH];
		lll_input_data_stream_t is;
		fl&=~_FLAG_ASSEMBLY_GENERATED;
		if (!load_file(*(fp+j),&a_dt,&c_dt,&f,&is,f_fp)){
			goto _error;
		}
		if (!(fl&_FLAG_ASSEMBLY_GENERATED)){
			if (ol>=OPTIMIZE_LEVEL_STRIP_GLOBAL_OPTIMIZE){
				if (fl&FLAG_VERBOSE){
					PRINT_STATIC_STR("Performing Global Optimization...\n");
				}
				lll_optimize_object(&c_dt,c_dt.h);
			}
			if (ol>=OPTIMIZE_LEVEL_STRIP_DEBUG_DATA){
				if (fl&FLAG_VERBOSE){
					PRINT_STATIC_STR("Removing Debug Data...\n");
				}
				lll_remove_object_debug_data(c_dt.h);
			}
			if (ol>=OPTIMIZE_LEVEL_REMOVE_PADDING){
				if (fl&FLAG_VERBOSE){
					PRINT_STATIC_STR("Removing Object Padding...\n");
				}
				lll_remove_object_padding(&c_dt,c_dt.h);
			}
			if (fl&FLAG_PRINT_OBJECT){
				lll_output_data_stream_t os;
				lll_stream_create_output_from_file(stdout,&os);
				lll_print_object(&c_dt,c_dt.h,&os);
				putchar('\n');
			}
			if (fl&FLAG_VERBOSE){
				PRINT_STATIC_STR("Optimizing Object Metadata...\n");
			}
			lll_optimize_metadata(&c_dt);
		}
		if (!(fl&_FLAG_ASSEMBLY_GENERATED)&&((fl&(FLAG_GENERATE_ASSEMBLY|FLAG_PRINT_ASSEMBLY))||!(fl&FLAG_NO_RUN))){
			if (fl&FLAG_VERBOSE){
				PRINT_STATIC_STR("Generating Assembly...\n");
			}
			lll_error_t e;
			if (!lll_generate_assembly(&c_dt,&a_dt,&e)){
				lll_print_error(&is,&e);
				goto _error;
			}
		}
		if (fl&FLAG_PRINT_ASSEMBLY){
			lll_output_data_stream_t os;
			lll_stream_create_output_from_file(stdout,&os);
			lll_print_assembly(&a_dt,&os);
			putchar('\n');
		}
		if (fl&(FLAG_GENERATE_ASSEMBLY|FLAG_GENERATE_COMPILED_OBJECT)){
			char bf[MAX_PATH_LENGTH];
			uint16_t i=0;
			if (!o_fp){
				while (*(f_fp+i)){
					*(bf+i)=*(f_fp+i);
					i++;
				}
				i--;
			}
			else{
				if (fpl==1){
					while (*(o_fp+i)){
						bf[i]=*(o_fp+i);
						i++;
					}
					if (fl&FLAG_GENERATE_ASSEMBLY){
						bf[i]=0;
						if (!write_assembly(bf,&a_dt)){// lgtm [cpp/path-injection]
							goto _error;
						}
					}
					if (fl&FLAG_GENERATE_COMPILED_OBJECT){
						bf[i]=0;
						if (!write_compiled(bf,&c_dt)){// lgtm [cpp/path-injection]
							goto _error;
						}
					}
					goto _skip_write;
				}
				else{
					while (*(o_fp+i)&&*(o_fp+i)!='.'){
						bf[i]=*(o_fp+i);
						i++;
					}
					i--;
					while (*(o_fp+i)!='\\'&&*(o_fp+i)!='/'){
						if (i==0){
							i--;
							break;
						}
						i--;
					}
					i++;
					uint32_t j=0;
					uint32_t k=0;
					while (*(f_fp+j)&&*(f_fp+j)!='.'){
						if (*(f_fp+j)=='\\'||*(f_fp+j)=='/'){
							k=j+1;
						}
						j++;
					}
					while (k<j){
						bf[i]=*(f_fp+k);
						i++;
						k++;
					}
					bf[i]='.';
				}
			}
			i++;
			if (fl&FLAG_GENERATE_ASSEMBLY){
				bf[i]=0;
				if (!write_assembly(bf,&a_dt)){// lgtm [cpp/path-injection]
					goto _error;
				}
			}
			if (fl&FLAG_GENERATE_COMPILED_OBJECT){
				bf[i]=0;
				if (!write_compiled(bf,&c_dt)){// lgtm [cpp/path-injection]
					goto _error;
				}
			}
_skip_write:;
		}
		if (!(fl&FLAG_NO_RUN)){
			lll_input_data_stream_t ris;
			lll_output_data_stream_t ros;
			lll_stream_create_input_from_file(stdin,&ris);
			lll_stream_create_output_from_file(stdout,&ros);
			lll_stack_data_t st;
			lll_setup_stack(&st,vm_st,VM_STACK_SIZE);
			lll_error_t e={
				LLL_ERROR_UNKNOWN
			};
			lll_return_code_t r=lll_execute_assembly(&a_dt,&st,&i_ft,&ris,&ros,&e);
			if (e.t!=LLL_ERROR_UNKNOWN){
				lll_print_error(NULL,&e);
				goto _error;
			}
			if (r){
				ec=r;
				goto _error;
			}
		}
		fclose(f);
		f=NULL;
		lll_free_assembly_data(&a_dt);
		lll_free_compilation_data(&c_dt);
	}
	while (im_fpl<fpl){
		free(*(fp+im_fpl));
		im_fpl++;
	}
	free(i_fp);
	if (fp){
		free(fp);
	}
	lll_free_internal_function_table(&i_ft);
	return 0;
_help:
	PRINT_STATIC_STR(HELP_TEXT);
_error:
	while (im_fpl<fpl){
		free(*(fp+im_fpl));
		im_fpl++;
	}
	free(i_fp);
	if (fp){
		free(fp);
	}
	lll_free_assembly_data(&a_dt);
	lll_free_compilation_data(&c_dt);
	if (f){
		fclose(f);
	}
	lll_free_internal_function_table(&i_ft);
	return ec;
}
