#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>

#include "libwf.h"
#include "ghttp.h"

enum{
	OFF_GDB,
	GDB_test,
	GDB_char,
	GDB_ipc,
	GDB_slist,
	GDB_httpget
};

int gdb_ctrl = OFF_GDB;

static unsigned char globel_buf[4096] = {0};

void slist_test()
{
struct test_t
{
        int a;
        struct slist_node slist;
};
	struct slist_head test_list, new_list;
	struct test_t *p = NULL, *pos = NULL, *n = NULL;
	int i=0;
		
	INIT_SLIST_HEAD(&test_list); 
	INIT_SLIST_HEAD(&new_list);
	p = slist_del_get_head_entry(&test_list, struct test_t, slist);
	if(p){
		printf("[%p] \n", p);
		fflush(stdout);
		free(p);
	}
	for(i=0; i<10; i++)
	{
		p = (struct test_t *)malloc(sizeof(struct test_t));
		if( p == NULL)
		{
			--i;
			continue;
		}
		p->a = i;
		slist_add(&test_list, &p->slist);
	}
	p = slist_del_get_head_entry(&test_list, struct test_t, slist);
	if(p)
		free(p);
	p = slist_del_get_head_entry(&test_list, struct test_t, slist);
	if(p)
		free(p);

	for(i=10; i<15; i++)
	{
		p = (struct test_t *)malloc(sizeof(struct test_t));
		if( p == NULL)
		{
			--i;
			continue;
		}
		p->a = i;
		slist_add(&new_list, &p->slist);
	}

	slist_splice_init(&new_list, &test_list);

	slist_for_each_entry(pos, &test_list, slist)
		printf("%d[%p]  ", pos->a, pos);
	printf("\n");

	slist_for_each_entry(pos, &new_list, slist)
		printf("%d ", pos->a);
	printf("\n");

	slist_while_get_head_entry(pos, &test_list, slist)
	{
		printf("[%p]  ", pos);
		fflush(stdout);
		free(pos);
	}
	printf("\n");
	//slist_while_get_head_entry(pos, &test_list, slist)
	//	free(pos);
	slist_while_get_head_entry(pos, &new_list, slist)
		free(pos);
}

void ipc_test()
{
	int ret;
	
	ret = ipc_client_call("/home/wolf_ipc", 0, 0);
	WF_PVAR_INT(ret);
}
void char_test(int argc, char **argv)
{
#define SWITCH_NUM  3
	int i=1;
#if SWITCH_NUM == 0
	printf("%s \n", time2str_pformat(time(NULL), NULL, "<now: %Y/%M/%D  ## %h:%m:%s >", 64));
	printf("%s \n", timenow2str(NULL));
#elif SWITCH_NUM == 1
	char buf[1024]="edqrae[g-reahre�ٴ����346222223 45\n";
	int len = strlen(buf);

	printf("len: %d    asc: %d  \n", len, str_asc_num(buf, len));
#elif SWITCH_NUM == 2
	char out[2048]={'\0'};
	char *buf = argv[++i], *sub = argv[++i], *rep = argv[++i];
	int num;

	num = str_replace(buf, sub, rep, out);
	printf("in: %s \n", buf);
	printf("out: %s \n", out);
	printf("num: %d \n", num);
#elif SWITCH_NUM == 3
	char src[128]="http://www.baidu.com/index.php?ch=en&var=abc#frag2";
	char src2[128]="how are you ? OK!";
	char dest[128]={'\0'};
	urlencode((unsigned char *)src, (unsigned char *)dest);
	printf("%%3A%%2F\n[%d] %s \n[%d] %s \n", strlen(src), src, strlen(dest), dest);
	urlencode((unsigned char *)src2, (unsigned char *)dest);
	printf("%%3A%%2F\n[%d] %s \n[%d] %s \n", strlen(src2), src2, strlen(dest), dest);
#endif
}
void test()
{
	#define TOTAL	10
	int a[12] = {56, 84, 5, 854, 24, 0, 5, 45, 0, 48, 486, 42};
	int i, j, k;
	printf("--------------- test ------------\n");

	for(i=0; i<TOTAL; i++)
		printf("%d\t", i);
	printf("\n");
	for(i=0; i<TOTAL; i++)
		printf("%d\t", a[i]);
	printf("\n");
	
	bubble_sort_int(a, 0, 9);

	for(i=0; i<TOTAL; i++)
		printf("%d\t", a[i]);
	printf("\n");
}

int ghttp_get_file(char *path, char *url)
{
	ghttp_request *request = NULL;
	FILE * pFile=NULL;
	char *buf=NULL;
	int ret = 0;

	ghttp_status req_status;
	ghttp_proc req_proc;
	int bytes_read=0,recvbytes=0;
	int status_code=0;
	char *redirect = NULL;
	char *tmp_pchar = NULL;

	request = ghttp_request_new();
	if( ghttp_set_uri(request, url) < 0 ){
			printf("invalid url: %s \n", url);
       		ret = -1;
			goto END;
	}
	
	if(!path)
		path = ghttp_get_resource_name(request);
	if(!path)
		path = "httpget.html";
	
	pFile = fopen ( path , "wb" );
	if(pFile == NULL){
		printf("error: %s [%s]\n", wf_std_error(NULL), path);
		ret = -2;
		goto END;
	}
	printf("host: %s \n", ghttp_get_host(request));
	if( ghttp_set_type(request, ghttp_type_get) < 0 ){
    		ret = -3;
		goto END;
	}
	if (ghttp_set_sync(request, ghttp_async) < 0){
		ret = -3;
		goto END;
	}
	if( ghttp_prepare(request) < 0 ){
		ret = -3;
		goto END;
	}

	do
	{
		req_status = ghttp_process(request);
		if( req_status == ghttp_error ){
			printf("%s \n", ghttp_get_error(request));
			ret = -4;
			goto END;
		}
		if (req_status != ghttp_error ) 
		{
			if( req_status == ghttp_done )
			{
				status_code = ghttp_status_code(request);
				if(status_code != 200){
					fclose(pFile);
					pFile = NULL;
					break;
				}
			}

			req_proc = ghttp_get_proc(request);
			if( req_proc == ghttp_proc_response || req_proc == ghttp_proc_done )
			{
				if( !tmp_pchar )
				{
					default_cursor();
					tmp_pchar = (char *)ghttp_get_header(request, "Content-Length");
					printf("Content-Length: %s \n", tmp_pchar ? tmp_pchar : "null");
					tmp_pchar = (char *)ghttp_get_header(request, "Transfer-Encoding");
					printf("Transfer-Encoding: %s \n", tmp_pchar ? tmp_pchar : "null");
					tmp_pchar = (char *)ghttp_get_header(request, "Content-Encoding");
					printf("Content-Encoding: %s \n", tmp_pchar ? tmp_pchar : "null");
					tmp_pchar = (char *)1;
					
					hide_cursor();
					printf("recvbytes: ");
					setBlueWhite();
					save_cursor();
				}
			
				ghttp_flush_response_buffer(request);
				if(ghttp_get_body_len(request) > 0)
				{
					buf = ghttp_get_body(request);
					bytes_read = ghttp_get_body_len(request);
					recvbytes += bytes_read;
					if(buf)
						fwrite(buf,bytes_read,1,pFile);
				}

				recover_cursor();
				printf("%d", recvbytes);
				fflush(stdout);
			}
		}
	}while (req_status == ghttp_not_done);
	default_cursor();
	show_cursor();

	ret = status_code;
	switch(status_code)
	{
	case 200:
	default:
		break;
	case 302:
		buf = (char *)ghttp_get_header(request, "Location");
		if(buf){
			redirect = (char *)malloc(strlen(buf)+1);
			if(redirect == NULL){
				ret = -1;
				goto END;
			}
			strcpy(redirect, buf);
		}
		break;
	}
	
END:
	ghttp_clean(request);
	ghttp_request_destroy(request);
    	if(pFile)
		fclose(pFile);
	if(redirect){
		printf("redirect: %s \n", redirect);
		ret = ghttp_get_file(path, redirect);
		free(redirect);
	}
	
	return ret;
}

int test_httpget(int argc, char **argv)
{
	int i=1;
	char *path=NULL, *url = NULL;
	int ret=0;
	printf("----------- test httpget ----------\n");

	while(argv[++i])
	{
		if( strcmp(argv[i], "-O") == 0 && argv[++i])
			path = argv[i];
		else{
			url = argv[i];
			break;
		}
	}
#if 1
	if(gdb_ctrl == GDB_httpget)
		ret = ghttp_download_file("httpget_gdb.html", "http://www.baidu.com");
	else
		ret = ghttp_download_file(path, url);
#else
	if(gdb_ctrl == GDB_httpget)
		ret = ghttp_get_file("httpget_gdb.html", "http://www.baidu.com");
	else
		ret = ghttp_get_file(path, url);
#endif
	if(ret < 0)
		printf("failed get [%d] \n", ret);
	else if(ret == 0 || ret == 200)
		printf("get done [%d] \n", ret);
	else
		printf("code: %d \n", ret);

	return ret;
}

int json_test(int argc, char **argv)
{
	cJSON *data;
	int fmt = 0;
	printf("----------- test json ----------\n");

	if(argc < 5){
		printf("test json in-file out-file fmt-code \n");
		return -1;
	}

	fmt = atoi(argv[4]);
	if(fmt < 0)
		fmt = 1;

	data = json_load_file(argv[2]);
	if(!data){
		printf("load %s failed \n", argv[2]);
		return -1;
	}
	printf("load %s OK \n", argv[2]);
	if( json_dump_file(data, argv[3], fmt) < 0 ){
		printf("dump %s failed \n", argv[3]);
		return -1;
	}
	printf("dump %s OK \n", argv[3]);

	free(data);
	return 0;
}

char *time_period_2_str(struct wf_time_period *time)
{
	static char time_period_str[32] = {'\0'};
	char buf[20] = {'\0'};
	int i=0, m=0;
	for(i=0; i<7; i++){
		if(i == 0 && time->week_flags & 0x01){
			buf[m] = 7 + '0';
			++m;
			buf[m] = ' ';
			++m;
		}
		else if(time->week_flags & (1 << i)){
			buf[m] = i + '0';
			++m;
			buf[m] = ' ';
			++m;
		}
	}
	buf[m] = '\0';
	
	sprintf(time_period_str, "[%s  %d:%d -- %d:%d]", buf, 
		time->start_hour, time->start_min, time->end_hour, time->end_min);
	return &time_period_str[0];
}
void time_set(unsigned char week_flags, int start_hour, int start_min, int end_hour, int end_min, 
	struct wf_time_period *time)
{
	time->week_flags = week_flags;
	time->start_hour = start_hour;
	time->start_min = start_min;
	time->end_hour = end_hour;
	time->end_min = end_min;
}
int time_cmp(int hour, int min, int start_hour, int start_min, int end_hour, int end_min)
{
	struct tm src;
	struct wf_time_period dst;

	memset(&src, 0, sizeof(src));
	memset(&dst, 0, sizeof(dst));
	src.tm_hour = hour;
	src.tm_min = min;
	time_set(0x7F, start_hour, start_min, end_hour, end_min, &dst);
	printf("cmp %d:%d   %s \n", hour, min, time_period_2_str(&dst));

	return wf_time_period_cmp(&src, &dst);
}
int time_overlap(struct wf_time_period *time_new, struct wf_time_period *time_old)
{
	printf("overlap %s  ", time_period_2_str(time_new));
	printf("%s\n", time_period_2_str(time_old));
	return wf_time_period_is_overlap(time_new, time_old);
}
int time_test(int argc, char **argv)
{
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
	printf("%d  %d:%d \n", now->tm_wday, now->tm_hour, now->tm_min);

	printf("cmp result: %d \n", time_cmp(23, 54, 23, 55, 0, 5));
	printf("cmp result: %d \n", time_cmp(23, 55, 23, 55, 0, 5));
	printf("cmp result: %d \n", time_cmp(23, 56, 23, 55, 0, 5));
	printf("cmp result: %d \n", time_cmp(0, 4, 23, 55, 0, 5));
	printf("cmp result: %d \n", time_cmp(0, 5, 23, 55, 0, 5));
	printf("cmp result: %d \n", time_cmp(0, 6, 23, 55, 0, 5));

	printf("cmp result: %d \n", time_cmp(23, 59, 23, 55, 0, 0));
	printf("cmp result: %d \n", time_cmp(0, 0, 23, 55, 0, 0));
	printf("cmp result: %d \n", time_cmp(0, 1, 23, 55, 0, 0));

	printf("cmp result: %d \n", time_cmp(3, 39, 3, 40, 3, 50));
	printf("cmp result: %d \n", time_cmp(3, 40, 3, 40, 3, 50));
	printf("cmp result: %d \n", time_cmp(3, 41, 3, 40, 3, 50));
	printf("cmp result: %d \n", time_cmp(3, 49, 3, 40, 3, 50));
	printf("cmp result: %d \n", time_cmp(3, 50, 3, 40, 3, 50));
	printf("cmp result: %d \n\n", time_cmp(3, 51, 3, 40, 3, 50));

	struct wf_time_period time_new, time_old;

	time_set(0x7F, 10, 5, 17, 35, &time_new);
	time_set(0x7F, 11, 0, 18, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x7F, 10, 5, 12, 35, &time_new);
	time_set(0x7F, 14, 0, 18, 0, &time_old);
	printf("overlap result: %d [0]\n", time_overlap(&time_new, &time_old));
	time_set(0x04, 10, 5, 16, 35, &time_new);
	time_set(0x20, 14, 0, 18, 0, &time_old);
	printf("overlap result: %d [0]\n", time_overlap(&time_new, &time_old));

	time_set(0x51, 20, 0, 18, 0, &time_new);
	time_set(0x61, 10, 0, 21, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x51, 20, 0, 18, 0, &time_old);
	time_set(0x61, 10, 0, 21, 0, &time_new);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x51, 20, 0, 18, 0, &time_new);
	time_set(0x51, 10, 0, 21, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x51, 20, 0, 18, 0, &time_new);
	time_set(0x51, 10, 0, 15, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));

	time_set(0x51, 17, 0, 14, 0, &time_new);
	time_set(0x61, 19, 0, 15, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x51, 17, 0, 12, 0, &time_new);
	time_set(0x61, 14, 0, 20, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));

	time_set(0x2A, 20, 0, 18, 0, &time_new);
	time_set(0x54, 10, 0, 21, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x2A, 20, 0, 18, 0, &time_new);
	time_set(0x54, 19, 0, 23, 0, &time_old);
	printf("overlap result: %d [0]\n", time_overlap(&time_new, &time_old));
	time_set(0x2A, 20, 0, 18, 0, &time_old);
	time_set(0x54, 10, 0, 21, 0, &time_new);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x2A, 20, 0, 18, 0, &time_new);
	time_set(0x2A, 10, 0, 21, 0, &time_old);
	printf("overlap result: %d [1]\n", time_overlap(&time_new, &time_old));
	time_set(0x2A, 20, 0, 18, 0, &time_new);
	time_set(0x2A, 10, 0, 15, 0, &time_old);
	printf("overlap result: %d [0]\n", time_overlap(&time_new, &time_old));

	time_set(0x2A, 17, 0, 14, 0, &time_new);
	time_set(0x54, 19, 0, 15, 0, &time_old);
	printf("overlap result: %d [0]\n", time_overlap(&time_new, &time_old));
	time_set(0x2A, 17, 0, 12, 0, &time_new);
	time_set(0x54, 14, 0, 20, 0, &time_old);
	printf("overlap result: %d [0]\n", time_overlap(&time_new, &time_old));

	return 0;
}

int main(int argc, char **argv)
{
	int ret=0;

	switch(gdb_ctrl)
	{
	case GDB_httpget:
		ret = test_httpget(argc, argv);
		return ret;
		break;
	case GDB_slist:
		slist_test();
		return ret;
		break;
	case OFF_GDB:
	default:
		break;
	}

	if(argc >= 2)
	{
		if( strcmp(argv[1], "char") == 0 )
			char_test(argc, argv);
		else if( strcmp(argv[1], "ipc") == 0 )
			ipc_test();
		else if( strcmp(argv[1], "slist") == 0 )
			slist_test();
		else if( strcmp(argv[1], "httpget") == 0 )
			ret = test_httpget(argc, argv);
		else if( strcmp(argv[1], "json") == 0 )
			ret = json_test(argc, argv);
		else if( strcmp(argv[1], "time") == 0 )
			ret = time_test(argc, argv);
		else
			test();
	}
	else{
		test();
	}

	return ret;
}



